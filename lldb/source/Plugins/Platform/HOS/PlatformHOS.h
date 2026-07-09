//===-- PlatformHOS.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_PlatformHOS_h_
#define liblldb_PlatformHOS_h_

#include <string>

#include "Plugins/Platform/Linux/PlatformLinux.h"

namespace lldb_private {
namespace platform_hos {

class PlatformHOS : public platform_linux::PlatformLinux {
public:
  PlatformHOS(bool is_host);

  ~PlatformHOS() override;

  static void Initialize();

  static void Terminate();

  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static llvm::StringRef GetPluginNameStatic(bool is_host);

  static const char *GetPluginDescriptionStatic(bool is_host);

  llvm::StringRef GetPluginName() override;

  Status ConnectRemote(Args &args) override;

  Status GetFile(const FileSpec &source, const FileSpec &destination) override;

  Status PutFile(const FileSpec &source, const FileSpec &destination,
                 uint32_t uid = UINT32_MAX, uint32_t gid = UINT32_MAX) override;

  uint32_t GetSdkVersion();

  bool GetRemoteOSVersion() override;

  Status DisconnectRemote() override;

  uint32_t GetDefaultMemoryCacheLineSize() override;

  ConstString GetMmapSymbolName(const ArchSpec &arch) override;

  MmapArgList GetMmapArgumentList(const ArchSpec &arch, lldb::addr_t addr,
                                  lldb::addr_t length, unsigned prot,
                                  unsigned flags, lldb::addr_t fd,
                                  lldb::addr_t offset) override;

protected:
  const char *GetCacheHostname() override;

  Status DownloadModuleSlice(const FileSpec &src_file_spec,
                             const uint64_t src_offset, const uint64_t src_size,
                             const FileSpec &dst_file_spec) override;

  llvm::StringRef
  GetLibdlFunctionDeclarations(lldb_private::Process *process) override;

private:
  platform_ohos::HdcClient CreateHdcClient();

  Status GetFileFromContainer(const FileSpec &source,
                              const FileSpec &destination);

  Status DoGetFile(const FileSpec &source, const FileSpec &destination);

  std::string m_connect_addr;
  std::string m_device_id;
  uint32_t m_sdk_version;

  PlatformHOS(const PlatformHOS &other) = delete;
  PlatformHOS &operator=(const PlatformHOS &other) = delete;
};

} // namespace platform_hos
} // namespace lldb_private

#endif // liblldb_PlatformHOS_h_
