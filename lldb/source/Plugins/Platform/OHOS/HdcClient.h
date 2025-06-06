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

namespace platform_ohos {

class HdcClient {
public:
  enum UnixSocketNamespace {
    UnixSocketNamespaceAbstract,
    UnixSocketNamespaceFileSystem,
  };

  using DeviceIDList = std::list<std::string>;

  static Status CreateByDeviceID(const std::string &device_id, HdcClient &hdc);

  explicit HdcClient(const std::string &connect_addr,
                     const std::string &device_id = "");

  HdcClient(HdcClient &&);

  ~HdcClient();

  const std::string &GetDeviceID() const;

  Status GetDevices(DeviceIDList &device_list);

  Status SetPortForwarding(const uint16_t local_port,
                           const uint16_t remote_port);

  Status SetPortForwarding(const uint16_t local_port,
                           llvm::StringRef remote_socket_name,
                           const UnixSocketNamespace socket_namespace);

  Status DeletePortForwarding(std::pair<uint16_t, uint16_t> fwd);
  
  Status DeletePortForwarding(const uint16_t local_port, const std::string remote_socket_name,
                              const UnixSocketNamespace socket_namespace);

  Status RecvFile(const FileSpec &src, const FileSpec &dst);

  Status SendFile(const FileSpec &src, const FileSpec &dst);

  Status Shell(const char *command, std::chrono::milliseconds timeout,
               std::string *output);

private:
  bool IsServerLocal();
  Status Connect();

  Status LocalTransferFile(const char *direction, const FileSpec &src,
                           const FileSpec &dst);

  Status FileCheck(int FD, size_t &file_size);
  Status PullFileChunk(std::vector<char> &buffer);

  Status FileInit(size_t file_size, uint32_t perm, uint32_t u_id, uint32_t g_id,
                  const std::string &remote_path);
  Status PushFileChunk(std::vector<char> &buffer, size_t chunk_size,
                       size_t index);

  void SetDeviceID(const std::string &device_id);

  Status SendMessage(llvm::StringRef packet, const bool reconnect = true);

  Status ReadMessage(std::vector<char> &message);

  Status SendCommandMessage(uint16_t command, llvm::ArrayRef<char> packet);

  Status ReadCommandMessagePrefix(uint16_t &command, std::vector<char> &message,
                                  size_t prefix_size);
  Status ReadCommandMessage(uint16_t &command, std::vector<char> &message);

  Status ExpectCommandMessage(uint16_t expected_command,
                              std::vector<char> &message);
  Status ExpectCommandMessagePrefix(uint16_t expected_command,
                                    std::vector<char> &message,
                                    size_t prefix_size);

  Status ReadMessageStream(std::vector<char> &message,
                           std::chrono::milliseconds timeout);

  Status ReadResponseStatus(const char *expected);

  Status ReadAllBytes(void *buffer, size_t size);

  std::string m_connect_addr;
  std::string m_device_id;
  std::unique_ptr<Connection> m_conn;
};

} // namespace platform_ohos
} // namespace lldb_private

#endif // liblldb_HdcClient_h_
