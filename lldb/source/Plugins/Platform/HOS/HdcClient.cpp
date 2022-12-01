//===-- HdcClient.cpp -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "HdcClient.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileUtilities.h"

#include "lldb/Host/ConnectionFileDescriptor.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/PosixApi.h"
#include "lldb/Utility/DataBuffer.h"
#include "lldb/Utility/DataBufferHeap.h"
#include "lldb/Utility/DataEncoder.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/Utility/Timeout.h"

#include <limits.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

// On Windows, transitive dependencies pull in <Windows.h>, which defines a
// macro that clashes with a method name.
#ifdef SendMessage
#undef SendMessage
#endif

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::platform_hos;
using namespace std::chrono;

namespace {

const seconds kReadTimeout(20);
const char *kOKAY = "OKAY";
const char *kFAIL = "FAIL";
const char *kDATA = "DATA";
const char *kDONE = "DONE";

const char *kSEND = "SEND";
const char *kRECV = "RECV";
const char *kSTAT = "STAT";

const size_t kSyncPacketLen = 8;
// Maximum size of a filesync DATA packet.
const size_t kMaxPushData = 2 * 1024;
// Default mode for pushed files.
const uint32_t kDefaultMode = 0100770; // S_IFREG | S_IRWXU | S_IRWXG

const char *kSocketNamespaceAbstract = "localabstract";
const char *kSocketNamespaceFileSystem = "localfilesystem";

Status ReadAllBytes(Connection &conn, void *buffer, size_t size) {

  Status error;
  ConnectionStatus status;
  char *read_buffer = static_cast<char *>(buffer);

  auto now = steady_clock::now();
  const auto deadline = now + kReadTimeout;
  size_t total_read_bytes = 0;
  while (total_read_bytes < size && now < deadline) {
    auto read_bytes =
        conn.Read(read_buffer + total_read_bytes, size - total_read_bytes,
                  duration_cast<microseconds>(deadline - now), status, &error);
    if (error.Fail())
      return error;
    total_read_bytes += read_bytes;
    if (status != eConnectionStatusSuccess)
      break;
    now = steady_clock::now();
  }
  if (total_read_bytes < size)
    error = Status(
        "Unable to read requested number of bytes. Connection status: %d.",
        status);
  return error;
}

} // namespace

Status HdcClient::CreateByDeviceID(const std::string &device_id,
                                   HdcClient &hdc) {
  DeviceIDList connect_devices;
  auto error = hdc.GetDevices(connect_devices);
  if (error.Fail())
    return error;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s device_id(%s)", __FILE__,
            __LINE__, __FUNCTION__, device_id.c_str());
  std::string android_serial;
  if (!device_id.empty())
    android_serial = device_id;
  else if (const char *env_serial = std::getenv("ANDROID_SERIAL"))
    android_serial = env_serial;

  if (android_serial.empty()) {
    if (connect_devices.size() != 1)
      return Status("Expected a single connected device, got instead %zu - try "
                    "setting 'ANDROID_SERIAL'",
                    connect_devices.size());
    hdc.SetDeviceID(connect_devices.front());
  } else {
    auto find_it = std::find(connect_devices.begin(), connect_devices.end(),
                             android_serial);
    if (find_it == connect_devices.end())
      return Status("Device \"%s\" not found", android_serial.c_str());

    hdc.SetDeviceID(*find_it);
  }
  return error;
}

HdcClient::HdcClient() {}

HdcClient::HdcClient(const std::string &device_id) : m_device_id(device_id) {}

HdcClient::~HdcClient() {}

void HdcClient::SetDeviceID(const std::string &device_id) {
  m_device_id = device_id;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s m_device_id(%s)", __FILE__,
            __LINE__, __FUNCTION__, m_device_id.c_str());
}

const std::string &HdcClient::GetDeviceID() const { return m_device_id; }

Status HdcClient::Connect() {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s new ConnectionFileDescriptor",
            __FILE__, __LINE__, __FUNCTION__);
  Status error;
  m_conn.reset(new ConnectionFileDescriptor);
  std::string port = "5037";

  const char *env_port = std::getenv("HDC_SERVER_PORT");
  if ((env_port != NULL) && (atoi(env_port) > 0)) {
    port = env_port;
    LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s  env_port(%s) port(%s)",
              __FILE__, __LINE__, __FUNCTION__, env_port, port.c_str());
  }

  std::string uri = "connect://127.0.0.1:" + port;
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s  uri(%s)", __FILE__, __LINE__,
            __FUNCTION__, uri.c_str());
  m_conn->Connect(uri.c_str(), &error);

  return error;
}

Status HdcClient::GetDevices(DeviceIDList &device_list) {
  device_list.clear();
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s call", __FILE__, __LINE__,
            __FUNCTION__);
  auto error = SendMessage("host:devices");
  if (error.Fail())
    return error;

  error = ReadResponseStatus();
  if (error.Fail())
    return error;

  std::vector<char> in_buffer;
  error = ReadMessage(in_buffer);

  llvm::StringRef response(&in_buffer[0], in_buffer.size());
  llvm::SmallVector<llvm::StringRef, 4> devices;
  response.split(devices, "\n", -1, false);

  for (const auto &device : devices)
    device_list.push_back(device.split('\t').first.str());

  // Force disconnect since hdc closes connection after host:devices response
  // is sent.
  m_conn.reset();
  return error;
}

Status HdcClient::SetPortForwarding(const uint16_t local_port,
                                    const uint16_t remote_port) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s local_port(%d) remote_port(%d)",
            __FILE__, __LINE__, __FUNCTION__, local_port, remote_port);
  char message[48];
  snprintf(message, sizeof(message), "forward:tcp:%d;tcp:%d", local_port,
           remote_port);

  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s message(%s)", __FILE__, __LINE__,
            __FUNCTION__, message);
  const auto error = SendDeviceMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus();
}

Status
HdcClient::SetPortForwarding(const uint16_t local_port,
                             llvm::StringRef remote_socket_name,
                             const UnixSocketNamespace socket_namespace) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s local_port(%d)", __FILE__,
            __LINE__, __FUNCTION__, local_port);
  char message[PATH_MAX];
  const char *sock_namespace_str =
      (socket_namespace == UnixSocketNamespaceAbstract)
          ? kSocketNamespaceAbstract
          : kSocketNamespaceFileSystem;
  snprintf(message, sizeof(message), "forward:tcp:%d;%s:%s", local_port,
           sock_namespace_str, remote_socket_name.str().c_str());

  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s message(%s)", __FILE__, __LINE__,
            __FUNCTION__, message);
  const auto error = SendDeviceMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus();
}

Status HdcClient::DeletePortForwarding(const uint16_t local_port) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s local_port(%d)", __FILE__,
            __LINE__, __FUNCTION__, local_port);
  char message[32];
  snprintf(message, sizeof(message), "killforward:tcp:%d", local_port);

  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s message(%s)", __FILE__, __LINE__,
            __FUNCTION__, message);
  const auto error = SendDeviceMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus();
}

Status HdcClient::SendMessage(const std::string &packet, const bool reconnect) {
  Status error;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s packet(%s) reconnect(%d)",
            __FILE__, __LINE__, __FUNCTION__, packet.c_str(), reconnect);
  if (!m_conn || reconnect) {
    error = Connect();
    if (error.Fail())
      return error;
  }

  char length_buffer[5];
  snprintf(length_buffer, sizeof(length_buffer), "%04x",
           static_cast<int>(packet.size()));

  ConnectionStatus status;

  m_conn->Write(length_buffer, 4, status, &error);
  if (error.Fail())
    return error;

  m_conn->Write(packet.c_str(), packet.size(), status, &error);
  return error;
}

Status HdcClient::SendDeviceMessage(const std::string &packet) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s packet(%s) ", __FILE__, __LINE__,
            __FUNCTION__, packet.c_str());

  std::ostringstream msg;
  msg << "host-serial:" << m_device_id << ":" << packet;
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s msg(%s) ", __FILE__, __LINE__,
            __FUNCTION__, msg.str().c_str());

  return SendMessage(msg.str());
}

Status HdcClient::ReadMessage(std::vector<char> &message) {
  message.clear();
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s call ", __FILE__, __LINE__,
            __FUNCTION__);
  char buffer[5];
  buffer[4] = 0;

  auto error = ReadAllBytes(buffer, 4);
  if (error.Fail())
    return error;

  unsigned int packet_len = 0;
  sscanf(buffer, "%x", &packet_len);

  message.resize(packet_len, 0);
  error = ReadAllBytes(&message[0], packet_len);
  if (error.Fail())
    message.clear();

  return error;
}

Status HdcClient::ReadMessageStream(std::vector<char> &message,
                                    milliseconds timeout) {
  auto start = steady_clock::now();
  message.clear();
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s call ", __FILE__, __LINE__,
            __FUNCTION__);

  Status error;
  lldb::ConnectionStatus status = lldb::eConnectionStatusSuccess;
  char buffer[1024];
  while (error.Success() && status == lldb::eConnectionStatusSuccess) {
    auto end = steady_clock::now();
    auto elapsed = end - start;
    if (elapsed >= timeout)
      return Status("Timed out");

    size_t n = m_conn->Read(buffer, sizeof(buffer),
                            duration_cast<microseconds>(timeout - elapsed),
                            status, &error);
    if (n > 0)
      message.insert(message.end(), &buffer[0], &buffer[n]);
  }
  return error;
}

Status HdcClient::ReadResponseStatus() {
  char response_id[5];
  static const size_t packet_len = 4;
  response_id[packet_len] = 0;

  auto error = ReadAllBytes(response_id, packet_len);
  if (error.Fail())
    return error;

  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s response_id(%s) ", __FILE__,
            __LINE__, __FUNCTION__, response_id);

  if (strncmp(response_id, kOKAY, packet_len) != 0)
    return GetResponseError(response_id);

  return error;
}

Status HdcClient::GetResponseError(const char *response_id) {
  if (strcmp(response_id, kFAIL) != 0)
    return Status("Got unexpected response id from hdc: \"%s\"", response_id);
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s response_id(%s) ", __FILE__,
            __LINE__, __FUNCTION__, response_id);

  std::vector<char> error_message;
  auto error = ReadMessage(error_message);
  if (error.Success())
    error.SetErrorString(
        std::string(&error_message[0], error_message.size()).c_str());

  return error;
}

Status HdcClient::SwitchDeviceTransport() {
  std::ostringstream msg;
  msg << "host:transport:" << m_device_id;

  auto error = SendMessage(msg.str());
  if (error.Fail())
    return error;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s m_device_id(%s) msg(%s)",
            __FILE__, __LINE__, __FUNCTION__, m_device_id.c_str(),
            msg.str().c_str());

  return ReadResponseStatus();
}

Status HdcClient::StartSync() {
  auto error = SwitchDeviceTransport();
  if (error.Fail())
    return Status("Failed to switch to device transport: %s",
                  error.AsCString());

  error = Sync();
  if (error.Fail())
    return Status("Sync failed: %s", error.AsCString());

  return error;
}

Status HdcClient::Sync() {
  auto error = SendMessage("sync:", false);
  if (error.Fail())
    return error;

  return ReadResponseStatus();
}

Status HdcClient::ReadAllBytes(void *buffer, size_t size) {
  return ::ReadAllBytes(*m_conn, buffer, size);
}

Status HdcClient::internalShell(const char *command, milliseconds timeout,
                                std::vector<char> &output_buf) {
  output_buf.clear();

  auto error = SwitchDeviceTransport();
  if (error.Fail())
    return Status("Failed to switch to device transport: %s",
                  error.AsCString());

  StreamString hdc_command;
  hdc_command.Printf("shell:%s", command);
  error = SendMessage(hdc_command.GetString().str(), false);
  if (error.Fail())
    return error;

  error = ReadResponseStatus();
  if (error.Fail())
    return error;

  error = ReadMessageStream(output_buf, timeout);
  if (error.Fail())
    return error;

  // HDC doesn't propagate return code of shell execution - if
  // output starts with /system/bin/sh: most likely command failed.
  static const char *kShellPrefix = "/system/bin/sh:";
  if (output_buf.size() > strlen(kShellPrefix)) {
    if (!memcmp(&output_buf[0], kShellPrefix, strlen(kShellPrefix)))
      return Status("Shell command %s failed: %s", command,
                    std::string(output_buf.begin(), output_buf.end()).c_str());
  }

  return Status();
}

Status HdcClient::Shell(const char *command, milliseconds timeout,
                        std::string *output) {
  std::vector<char> output_buffer;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s command(%s)", __FILE__, __LINE__,
            __FUNCTION__, command);

  auto error = internalShell(command, timeout, output_buffer);
  if (error.Fail())
    return error;

  if (output)
    output->assign(output_buffer.begin(), output_buffer.end());
  return error;
}

Status HdcClient::ShellToFile(const char *command, milliseconds timeout,
                              const FileSpec &output_file_spec) {
  std::vector<char> output_buffer;
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s command(%s)", __FILE__, __LINE__,
            __FUNCTION__, command);

  auto error = internalShell(command, timeout, output_buffer);
  if (error.Fail())
    return error;

  const auto output_filename = output_file_spec.GetPath();
  std::error_code EC;
  llvm::raw_fd_ostream dst(output_filename, EC, llvm::sys::fs::OF_None);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s output_filename(%s)", __FILE__,
            __LINE__, __FUNCTION__, output_filename.c_str());
  if (EC)
    return Status("Unable to open local file %s", output_filename.c_str());

  dst.write(&output_buffer[0], output_buffer.size());
  dst.close();
  if (dst.has_error())
    return Status("Failed to write file %s", output_filename.c_str());
  return Status();
}

std::unique_ptr<HdcClient::SyncService>
HdcClient::GetSyncService(Status &error) {
  std::unique_ptr<SyncService> sync_service;
  error = StartSync();
  if (error.Success())
    sync_service.reset(new SyncService(std::move(m_conn)));

  return sync_service;
}

Status HdcClient::SyncService::internalPullFile(const FileSpec &remote_file,
                                                const FileSpec &local_file) {
  const auto local_file_path = local_file.GetPath();
  llvm::FileRemover local_file_remover(local_file_path);

  std::error_code EC;
  llvm::raw_fd_ostream dst(local_file_path, EC, llvm::sys::fs::OF_None);
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s local_file_path(%s)", __FILE__,
            __LINE__, __FUNCTION__, local_file_path.c_str());

  if (EC)
    return Status("Unable to open local file %s", local_file_path.c_str());

  const auto remote_file_path = remote_file.GetPath(false);
  auto error = SendSyncRequest(kRECV, remote_file_path.length(),
                               remote_file_path.c_str());
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s remote_file_path(%s)", __FILE__,
            __LINE__, __FUNCTION__, remote_file_path.c_str());

  if (error.Fail())
    return error;

  std::vector<char> chunk;
  bool eof = false;
  while (!eof) {
    error = PullFileChunk(chunk, eof);
    if (error.Fail())
      return error;
    if (!eof)
      dst.write(&chunk[0], chunk.size());
  }
  dst.close();
  if (dst.has_error())
    return Status("Failed to write file %s", local_file_path.c_str());

  local_file_remover.releaseFile();
  return error;
}

Status HdcClient::SyncService::internalPushFile(const FileSpec &local_file,
                                                const FileSpec &remote_file) {
  const auto local_file_path(local_file.GetPath());
  std::ifstream src(local_file_path.c_str(), std::ios::in | std::ios::binary);
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s local_file_path(%s)", __FILE__,
            __LINE__, __FUNCTION__, local_file_path.c_str());

  if (!src.is_open())
    return Status("Unable to open local file %s", local_file_path.c_str());

  std::stringstream file_description;
  file_description << remote_file.GetPath(false).c_str() << "," << kDefaultMode;
  std::string file_description_str = file_description.str();
  auto error = SendSyncRequest(kSEND, file_description_str.length(),
                               file_description_str.c_str());
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s file_description_str(%s)",
            __FILE__, __LINE__, __FUNCTION__, file_description_str.c_str());

  if (error.Fail())
    return error;

  char chunk[kMaxPushData];
  while (!src.eof() && !src.read(chunk, kMaxPushData).bad()) {
    size_t chunk_size = src.gcount();
    error = SendSyncRequest(kDATA, chunk_size, chunk);
    if (error.Fail())
      return Status("Failed to send file chunk: %s", error.AsCString());
  }
  error = SendSyncRequest(
      kDONE,
      llvm::sys::toTimeT(
          FileSystem::Instance().GetModificationTime(local_file)),
      nullptr);
  if (error.Fail())
    return error;

  std::string response_id;
  uint32_t data_len;
  error = ReadSyncHeader(response_id, data_len);
  if (error.Fail())
    return Status("Failed to read DONE response: %s", error.AsCString());
  if (response_id == kFAIL) {
    std::string error_message(data_len, 0);
    error = ReadAllBytes(&error_message[0], data_len);
    if (error.Fail())
      return Status("Failed to read DONE error message: %s", error.AsCString());
    return Status("Failed to push file: %s", error_message.c_str());
  } else if (response_id != kOKAY)
    return Status("Got unexpected DONE response: %s", response_id.c_str());

  // If there was an error reading the source file, finish the hdc file
  // transfer first so that hdc isn't expecting any more data.
  if (src.bad())
    return Status("Failed read on %s", local_file_path.c_str());
  return error;
}

Status HdcClient::SyncService::internalStat(const FileSpec &remote_file,
                                            uint32_t &mode, uint32_t &size,
                                            uint32_t &mtime) {
  const std::string remote_file_path(remote_file.GetPath(false));
  auto error = SendSyncRequest(kSTAT, remote_file_path.length(),
                               remote_file_path.c_str());
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s remote_file_path(%s)", __FILE__,
            __LINE__, __FUNCTION__, remote_file_path.c_str());

  if (error.Fail())
    return Status("Failed to send request: %s", error.AsCString());

  static const size_t stat_len = strlen(kSTAT);
  static const size_t response_len = stat_len + (sizeof(uint32_t) * 3);

  std::vector<char> buffer(response_len);
  error = ReadAllBytes(&buffer[0], buffer.size());
  if (error.Fail())
    return Status("Failed to read response: %s", error.AsCString());

  DataExtractor extractor(&buffer[0], buffer.size(), eByteOrderLittle,
                          sizeof(void *));
  offset_t offset = 0;

  const void *command = extractor.GetData(&offset, stat_len);
  if (!command)
    return Status("Failed to get response command");
  const char *command_str = static_cast<const char *>(command);
  if (strncmp(command_str, kSTAT, stat_len))
    return Status("Got invalid stat command: %s", command_str);

  mode = extractor.GetU32(&offset);
  size = extractor.GetU32(&offset);
  mtime = extractor.GetU32(&offset);
  return Status();
}

Status HdcClient::SyncService::PullFile(const FileSpec &remote_file,
                                        const FileSpec &local_file) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s remote_file(%s) local_file(%s)",
            __FILE__, __LINE__, __FUNCTION__, remote_file.GetPath().c_str(),
            local_file.GetPath().c_str());

  return executeCommand([this, &remote_file, &local_file]() {
    return internalPullFile(remote_file, local_file);
  });
}

Status HdcClient::SyncService::PushFile(const FileSpec &local_file,
                                        const FileSpec &remote_file) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s remote_file(%s) local_file(%s)",
            __FILE__, __LINE__, __FUNCTION__, remote_file.GetPath().c_str(),
            local_file.GetPath().c_str());
  return executeCommand([this, &local_file, &remote_file]() {
    return internalPushFile(local_file, remote_file);
  });
}

Status HdcClient::SyncService::Stat(const FileSpec &remote_file, uint32_t &mode,
                                    uint32_t &size, uint32_t &mtime) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Hsu file(%s):%d HdcClient::%s remote_file(%s) mode(%d)",
            __FILE__, __LINE__, __FUNCTION__, remote_file.GetPath().c_str(),
            mode);
  return executeCommand([this, &remote_file, &mode, &size, &mtime]() {
    return internalStat(remote_file, mode, size, mtime);
  });
}

bool HdcClient::SyncService::IsConnected() const {
  return m_conn && m_conn->IsConnected();
}

HdcClient::SyncService::SyncService(std::unique_ptr<Connection> &&conn)
    : m_conn(std::move(conn)) {}

Status
HdcClient::SyncService::executeCommand(const std::function<Status()> &cmd) {
  if (!m_conn)
    return Status("SyncService is disconnected");

  const auto error = cmd();
  if (error.Fail())
    m_conn.reset();

  return error;
}

HdcClient::SyncService::~SyncService() {}

Status HdcClient::SyncService::SendSyncRequest(const char *request_id,
                                               const uint32_t data_len,
                                               const void *data) {
  const DataBufferSP data_sp(new DataBufferHeap(kSyncPacketLen, 0));
  DataEncoder encoder(data_sp->GetBytes(), data_sp->GetByteSize(),
                      eByteOrderLittle, sizeof(void *));
  auto offset = encoder.PutData(0, request_id, strlen(request_id));
  encoder.PutUnsigned(offset, 4, data_len);

  Status error;
  ConnectionStatus status;
  m_conn->Write(data_sp->GetBytes(), kSyncPacketLen, status, &error);
  if (error.Fail())
    return error;

  if (data)
    m_conn->Write(data, data_len, status, &error);
  return error;
}

Status HdcClient::SyncService::ReadSyncHeader(std::string &response_id,
                                              uint32_t &data_len) {
  char buffer[kSyncPacketLen];

  auto error = ReadAllBytes(buffer, kSyncPacketLen);
  if (error.Success()) {
    response_id.assign(&buffer[0], 4);
    DataExtractor extractor(&buffer[4], 4, eByteOrderLittle, sizeof(void *));
    offset_t offset = 0;
    data_len = extractor.GetU32(&offset);
  }

  return error;
}

Status HdcClient::SyncService::PullFileChunk(std::vector<char> &buffer,
                                             bool &eof) {
  buffer.clear();

  std::string response_id;
  uint32_t data_len;
  auto error = ReadSyncHeader(response_id, data_len);
  if (error.Fail())
    return error;

  if (response_id == kDATA) {
    buffer.resize(data_len, 0);
    error = ReadAllBytes(&buffer[0], data_len);
    if (error.Fail())
      buffer.clear();
  } else if (response_id == kDONE) {
    eof = true;
  } else if (response_id == kFAIL) {
    std::string error_message(data_len, 0);
    error = ReadAllBytes(&error_message[0], data_len);
    if (error.Fail())
      return Status("Failed to read pull error message: %s", error.AsCString());
    return Status("Failed to pull file: %s", error_message.c_str());
  } else
    return Status("Pull failed with unknown response: %s", response_id.c_str());

  return Status();
}

Status HdcClient::SyncService::ReadAllBytes(void *buffer, size_t size) {
  return ::ReadAllBytes(*m_conn, buffer, size);
}
