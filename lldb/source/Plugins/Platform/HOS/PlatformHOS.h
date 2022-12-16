//===-- PlatformHOS.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_PlatformHOS_h_
#define liblldb_PlatformHOS_h_

#include <memory>
#include <string>

//#include "Plugins/Platform/Android/PlatformAndroid.h"
#include "HdcClient.h"
#include "Plugins/Platform/Linux/PlatformLinux.h"

namespace lldb_private {
namespace platform_hos {

// class PlatformHOS : public platform_android::PlatformAndroid {
class PlatformHOS : public platform_linux::PlatformLinux {
public:
  //  PlatformHOS(bool is_host) : PlatformAndroid(is_host) {}
  PlatformHOS(bool is_host);
  // lldb_private::PluginInterface functions
  ~PlatformHOS() override;
  static void Initialize();

  static void Terminate();

  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static ConstString GetPluginNameStatic(bool is_host);

  static const char *GetPluginDescriptionStatic(bool is_host);

  ConstString GetPluginName() override { return GetPluginNameStatic(IsHost()); }

  uint32_t GetPluginVersion() override { return 1; }

  Status ConnectRemote(Args &args) override;

  Status GetFile(const FileSpec &source, const FileSpec &destination) override;

  Status PutFile(const FileSpec &source, const FileSpec &destination,
                 uint32_t uid = UINT32_MAX, uint32_t gid = UINT32_MAX) override;

  uint32_t GetSdkVersion();

  bool GetRemoteOSVersion() override;

  Status DisconnectRemote() override;

  uint32_t GetDefaultMemoryCacheLineSize() override;

protected:
  const char *GetCacheHostname() override;

  Status DownloadModuleSlice(const FileSpec &src_file_spec,
                             const uint64_t src_offset, const uint64_t src_size,
                             const FileSpec &dst_file_spec) override;

  Status DownloadSymbolFile(const lldb::ModuleSP &module_sp,
                            const FileSpec &dst_file_spec) override;

  llvm::StringRef
  GetLibdlFunctionDeclarations(lldb_private::Process *process) override;

private:
  std::unique_ptr<HdcClient::SyncService> GetSyncService(Status &error);

  std::string m_device_id;
  uint32_t m_sdk_version;
};

} // namespace platform_hos
} // namespace lldb_private

#endif // liblldb_PlatformHOS_h_
