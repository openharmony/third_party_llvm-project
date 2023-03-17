//===-- PlatformOHOS.h ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_PlatformOHOS_h_
#define liblldb_PlatformOHOS_h_

#include <memory>
#include <string>

#include "Plugins/Platform/Linux/PlatformLinux.h"

#include "HdcClient.h"

namespace lldb_private {
namespace platform_ohos {

class PlatformOHOS : public platform_linux::PlatformLinux {
public:
  PlatformOHOS(bool is_host);

  ~PlatformOHOS() override;

  static void Initialize();

  static void Terminate();

  // lldb_private::PluginInterface functions
  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static llvm::StringRef GetPluginNameStatic(bool is_host);

  static const char *GetPluginDescriptionStatic(bool is_host);

  llvm::StringRef GetPluginName() override;

  // lldb_private::Platform functions
  Status ConnectRemote(Args &args) override;

  Status GetFile(const FileSpec &source, const FileSpec &destination) override;

  Status PutFile(const FileSpec &source, const FileSpec &destination,
                 uint32_t uid = UINT32_MAX, uint32_t gid = UINT32_MAX) override;

  uint32_t GetSdkVersion();

  bool GetRemoteOSVersion() override;

  Status DisconnectRemote() override;

  uint32_t GetDefaultMemoryCacheLineSize() override;

  ConstString GetMmapSymbolName(const ArchSpec &arch) override;

protected:
  const char *GetCacheHostname() override;

  Status DownloadModuleSlice(const FileSpec &src_file_spec,
                             const uint64_t src_offset, const uint64_t src_size,
                             const FileSpec &dst_file_spec) override;

  llvm::StringRef
  GetLibdlFunctionDeclarations(lldb_private::Process *process) override;

private:
  std::string m_device_id;
  uint32_t m_sdk_version;

  PlatformOHOS(const PlatformOHOS &other) = delete;
  PlatformOHOS& operator=(const PlatformOHOS &other) = delete;
};

} // namespace platform_ohos
} // namespace lldb_private

#endif // liblldb_PlatformOHOS_h_
