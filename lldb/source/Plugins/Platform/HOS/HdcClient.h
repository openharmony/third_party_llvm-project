//===-- HdcClient.h ---------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_HdcClient_h_
#define liblldb_HdcClient_h_

#include "lldb/Utility/Status.h"
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace lldb_private {

class FileSpec;

namespace platform_hos {

class HdcClient {
public:
  enum UnixSocketNamespace {
    UnixSocketNamespaceAbstract,
    UnixSocketNamespaceFileSystem,
  };

  using DeviceIDList = std::list<std::string>;

  class SyncService {
    friend class HdcClient;

  public:
    ~SyncService();

    Status PullFile(const FileSpec &remote_file, const FileSpec &local_file);

    Status PushFile(const FileSpec &local_file, const FileSpec &remote_file);

    Status Stat(const FileSpec &remote_file, uint32_t &mode, uint32_t &size,
                uint32_t &mtime);

    bool IsConnected() const;

  private:
    explicit SyncService(std::unique_ptr<Connection> &&conn);

    Status SendSyncRequest(const char *request_id, const uint32_t data_len,
                           const void *data);

    Status ReadSyncHeader(std::string &response_id, uint32_t &data_len);

    Status PullFileChunk(std::vector<char> &buffer, bool &eof);

    Status ReadAllBytes(void *buffer, size_t size);

    Status internalPullFile(const FileSpec &remote_file,
                            const FileSpec &local_file);

    Status internalPushFile(const FileSpec &local_file,
                            const FileSpec &remote_file);

    Status internalStat(const FileSpec &remote_file, uint32_t &mode,
                        uint32_t &size, uint32_t &mtime);

    Status executeCommand(const std::function<Status()> &cmd);

    std::unique_ptr<Connection> m_conn;
  };

  static Status CreateByDeviceID(const std::string &device_id, HdcClient &hdc);
  HdcClient();
  explicit HdcClient(const std::string &device_id);

  ~HdcClient();
  const std::string &GetDeviceID() const;

  Status GetDevices(DeviceIDList &device_list);

  Status SetPortForwarding(const uint16_t local_port,
                           const uint16_t remote_port);

  Status SetPortForwarding(const uint16_t local_port,
                           llvm::StringRef remote_socket_name,
                           const UnixSocketNamespace socket_namespace);

  Status DeletePortForwarding(const uint16_t local_port);

  Status Shell(const char *command, std::chrono::milliseconds timeout,
               std::string *output);

  Status ShellToFile(const char *command, std::chrono::milliseconds timeout,
                     const FileSpec &output_file_spec);

  std::unique_ptr<SyncService> GetSyncService(Status &error);

  Status SwitchDeviceTransport();

private:
  Status Connect();

  void SetDeviceID(const std::string &device_id);

  Status SendMessage(const std::string &packet, const bool reconnect = true);

  Status SendDeviceMessage(const std::string &packet);

  Status ReadMessage(std::vector<char> &message);

  Status ReadMessageStream(std::vector<char> &message,
                           std::chrono::milliseconds timeout);

  Status GetResponseError(const char *response_id);

  Status ReadResponseStatus();

  Status Sync();

  Status StartSync();

  Status internalShell(const char *command, std::chrono::milliseconds timeout,
                       std::vector<char> &output_buf);

  Status ReadAllBytes(void *buffer, size_t size);

  std::string m_device_id;
  std::unique_ptr<Connection> m_conn;
};

} // namespace platform_hos
} // namespace lldb_private

#endif // liblldb_HdcClient_h_
