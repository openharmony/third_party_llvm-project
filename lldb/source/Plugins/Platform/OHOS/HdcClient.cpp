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
#include "lldb/Utility/StreamString.h"
#include "lldb/Utility/Timeout.h"

#if defined(_WIN32)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

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
using namespace lldb_private::platform_ohos;
using namespace std::chrono;

namespace {

const seconds kReadTimeout(20);
const char *kSocketNamespaceAbstract = "localabstract";
const char *kSocketNamespaceFileSystem = "localfilesystem";

Status ReadAllBytes(Connection &conn, void *buffer, size_t size,
                    size_t *read_ptr, ConnectionStatus *status_ptr) {

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
    if (status_ptr)
      *status_ptr = status;
    if (error.Fail())
      return error;
    total_read_bytes += read_bytes;
    if (read_ptr)
      *read_ptr = total_read_bytes;
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

  std::string hdc_utid;
  if (!device_id.empty())
    hdc_utid = device_id;
  else if (const char *env_hdc_utid = std::getenv("HDC_UTID"))
    hdc_utid = env_hdc_utid;

  if (hdc_utid.empty()) {
    if (connect_devices.size() != 1)
      return Status("Expected a single connected device, got instead %zu - try "
                    "setting 'HDC_UTID'",
                    connect_devices.size());
    hdc.SetDeviceID(connect_devices.front());
  } else {
    auto find_it = std::find(connect_devices.begin(), connect_devices.end(),
                             hdc_utid);
    if (find_it == connect_devices.end())
      return Status(
          "Device \"%s\" not found, check HDC_UTID environment variable",
          hdc_utid.c_str());

    hdc.SetDeviceID(*find_it);
  }
  return error;
}

HdcClient::HdcClient() {}

HdcClient::HdcClient(const std::string &device_id) : m_device_id(device_id) {}

HdcClient::~HdcClient() {}

void HdcClient::SetDeviceID(const std::string &device_id) {
  m_device_id = device_id;
}

const std::string &HdcClient::GetDeviceID() const { return m_device_id; }

namespace {
typedef unsigned msg_len_t;
struct ChannelHandShake {
  msg_len_t size;
  char banner[12]; // must first index
  union {
    uint32_t channelId;
    char connectKey[32];
  };
} __attribute__((packed));
} // namespace

Status HdcClient::Connect() {
  Status error;
  ChannelHandShake handshake = {};
  if (m_device_id.size() > sizeof(handshake.connectKey))
    return Status("Device id is too long: %s", m_device_id.c_str());
  m_conn.reset(new ConnectionFileDescriptor);
  std::string port = "8710";
  
  const char *env_port = std::getenv("OHOS_HDC_SERVER_PORT");
  if (env_port != NULL) {
    int iEnv_port = atoi(env_port);
    if ((iEnv_port > 0) && (iEnv_port <= 65535)) {
      port = env_port;
    }
    else {
      return Status("invalid port specification: %s. $OHOS_HDC_SERVER_PORT must be a positive number in (0,65535]", env_port);
    }
  }
  
  std::string uri = "connect://localhost:" + port;
  m_conn->Connect(uri.c_str(), &error);
  ConnectionStatus status = eConnectionStatusError;
  if (error.Success()) {
    error = ReadAllBytes(&handshake, sizeof(handshake));
    if (error.Success()) {
      memset(handshake.connectKey, 0, sizeof(handshake.connectKey));
      memcpy(handshake.connectKey, m_device_id.c_str(), m_device_id.size());
      m_conn->Write(&handshake, sizeof(handshake), status, &error);
    }
  }
  return error;
}

Status HdcClient::GetDevices(DeviceIDList &device_list) {
  device_list.clear();

  auto error = SendMessage("list targets");
  if (error.Fail())
    return error;

  std::vector<char> in_buffer;
  error = ReadMessage(in_buffer);

  llvm::StringRef response(&in_buffer[0], in_buffer.size());
  llvm::SmallVector<llvm::StringRef, 4> devices;
  response.split(devices, "\n", -1, false);

  for (const auto device : devices)
    device_list.push_back(static_cast<std::string>(device.split('\t').first));

  // Force disconnect since ADB closes connection after host:devices response
  // is sent.
  m_conn.reset();
  return error;
}

Status HdcClient::SetPortForwarding(const uint16_t local_port,
                                    const uint16_t remote_port) {
  char message[48];
  snprintf(message, sizeof(message), "fport tcp:%d tcp:%d", local_port,
           remote_port);

  const auto error = SendMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus("Forwardport result:OK");
}

Status
HdcClient::SetPortForwarding(const uint16_t local_port,
                             llvm::StringRef remote_socket_name,
                             const UnixSocketNamespace socket_namespace) {
  char message[PATH_MAX];
  const char *sock_namespace_str =
      (socket_namespace == UnixSocketNamespaceAbstract)
          ? kSocketNamespaceAbstract
          : kSocketNamespaceFileSystem;
  snprintf(message, sizeof(message), "fport tcp:%d %s:%s", local_port,
           sock_namespace_str, remote_socket_name.str().c_str());

  const auto error = SendMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus("Forwardport result:OK");
}

Status HdcClient::DeletePortForwarding(std::pair<uint16_t, uint16_t> fwd) {
  char message[32];
  snprintf(message, sizeof(message), "fport rm tcp:%d tcp:%d", fwd.first,
           fwd.second);

  const auto error = SendMessage(message);
  if (error.Fail())
    return error;

  return ReadResponseStatus("Remove forward ruler success");
}

Status HdcClient::DeletePortForwarding(const uint16_t local_port,
                                       const std::string remote_socket_name,
                                       const UnixSocketNamespace socket_namespace) {
    const char *sock_namespace_str =
      (socket_namespace == UnixSocketNamespaceAbstract)
          ? kSocketNamespaceAbstract
          : kSocketNamespaceFileSystem;
    char message[PATH_MAX] = "";

    snprintf(message, sizeof(message), "fport rm tcp:%d %s:%s", local_port,
           sock_namespace_str, remote_socket_name.c_str());

    const auto error = SendMessage(message);
    if (error.Fail()){
        return error;
    }
    
    return ReadResponseStatus("Remove forward ruler success");
}

Status HdcClient::TransferFile(const char *direction, const FileSpec &src,
                               const FileSpec &dst) {
  llvm::SmallVector<char, 128> cwd;
  std::error_code ec = llvm::sys::fs::current_path(cwd);
  if (ec)
    return Status(ec);

  std::stringstream cmd;
  cmd << "file " << direction << " -m " << " -cwd ";
  cmd.write(cwd.data(), cwd.size());
  cmd << " " << src.GetPath() << " " << dst.GetPath();
  Status error = SendMessage(cmd.str());
  if (error.Fail())
    return error;

  return ReadResponseStatus("FileTransfer finish");
}

Status HdcClient::RecvFile(const FileSpec &src, const FileSpec &dst) {
  return TransferFile("recv", src, dst);
}

Status HdcClient::SendFile(const FileSpec &src, const FileSpec &dst) {
  return TransferFile("send", src, dst);
}

Status HdcClient::SendMessage(llvm::StringRef packet, const bool reconnect) {
  Status error;
  if (!m_conn || reconnect) {
    error = Connect();
    if (error.Fail())
      return error;
  }

  unsigned msg_len = packet.size() + 1;
  llvm::SmallVector<char, 128> message(msg_len + sizeof(msg_len_t), 0);
  msg_len_t len = htonl(msg_len);
  memcpy(message.data(), &len, sizeof(len));
  memcpy(message.data() + sizeof(len), packet.data(), packet.size());

  ConnectionStatus status;
  m_conn->Write(message.data(), message.size(), status, &error);
  if (error.Fail())
    return error;

  return error;
}

Status HdcClient::ReadMessage(std::vector<char> &message) {
  message.clear();

  msg_len_t packet_len;
  auto error = ReadAllBytes(&packet_len, sizeof(packet_len));
  if (error.Fail())
    return error;

  packet_len = htonl(packet_len);
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

Status HdcClient::ReadResponseStatus(const char *expected) {
  msg_len_t len;
  ConnectionStatus conn_status;
  size_t read;

  auto error = ::ReadAllBytes(*m_conn, &len, sizeof(len), &read, &conn_status);
  // Special case: we expect server to close connection
  if (expected == nullptr) {
    if (read == 0 && conn_status == eConnectionStatusEndOfFile)
      return Status();
    else if (error.Fail())
      return error;
    // Something went wrong - response is not empty
    // Read it and wrap to error object
  }

  len = htonl(len);
  llvm::SmallVector<char, 128> message(len + 1);
  error = ReadAllBytes(message.data(), len);
  if (error.Fail())
    return error;

  message[len] = 0;
  if (expected == nullptr ||
      strncmp(message.data(), expected, strlen(expected)))
    return Status("%s", message.data());

  return error;
}

Status HdcClient::ReadAllBytes(void *buffer, size_t size) {
  return ::ReadAllBytes(*m_conn, buffer, size, nullptr, nullptr);
}

Status HdcClient::Shell(const char *command, milliseconds timeout,
                        std::string *output) {
  assert(command && command[0]);
  std::string cmd = "shell ";
  cmd += command;
  Status error = SendMessage(cmd);
  if (error.Fail())
    return error;

  std::vector<char> message;
  error = ReadMessageStream(message, timeout);
  if (error.Fail())
    return error;

  (*output) = std::string(message.data(), message.size());
  return error;
}
