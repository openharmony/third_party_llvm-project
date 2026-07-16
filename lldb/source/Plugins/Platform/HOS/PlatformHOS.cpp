#ifdef OHOS_LLVM
//===-- PlatformHOS.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Core/PluginManager.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/UriParser.h"
#include "llvm/Config/config.h"

#include "Plugins/Platform/OHOS/HdcClient.h"
#include "PlatformHOS.h"
#include "PlatformHOSRemoteGDBServer.h"
#include "lldb/Target/Target.h"

#include <array>
#include <optional>

#if defined(__OHOS__)
#include <sys/mman.h>
#else
#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define MAP_PRIVATE 2
#define MAP_ANON 0x20
#endif

#define MAP_ANON_MIPS 0x800
#define MAP_JIT 0x1000

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::platform_hos;
using namespace lldb_private::platform_ohos;
using namespace std::chrono;

static uint32_t g_initialize_count = 0;
static const unsigned int g_hos_default_cache_size = 2048;
static constexpr uint32_t INVALID_SDK_VERSION = 0xFFFFFFFF;

LLDB_PLUGIN_DEFINE(PlatformHOS)

using PrefixMap = std::pair<llvm::StringRef, llvm::StringRef>;

static constexpr std::array<PrefixMap, 7> kPathPrefixMap{{
    {"/data", "/data/ohos_data"},
    {"/vendor/aosp/system/lib64/libqdMetaData.system.so",
     "/system/system_ext/lib64/libqdMetaData.system.so"},
    {"/vendor/aosp/system/lib64/libgralloc.system.qti.so",
     "/system/system_ext/lib64/libgralloc.system.qti.so"},
    {"/vendor/aosp/vendor/lib64", "/vendor/lib64"},
    {"/vendor/aosp/system/lib64", "/system/lib64"},
    {"/vendor/lib64", "/system/ohos/vendor/lib64"},
    {"/system", "/system/ohos/system"},
}};

void PlatformHOS::Initialize() {
  PlatformLinux::Initialize();

  if (g_initialize_count++ == 0) {
    PluginManager::RegisterPlugin(
        PlatformHOS::GetPluginNameStatic(false),
        PlatformHOS::GetPluginDescriptionStatic(false),
        PlatformHOS::CreateInstance);
  }
}

void PlatformHOS::Terminate() {
  if (g_initialize_count > 0) {
    if (--g_initialize_count == 0) {
      PluginManager::UnregisterPlugin(PlatformHOS::CreateInstance);
    }
  }

  PlatformLinux::Terminate();
}

PlatformSP PlatformHOS::CreateInstance(bool force, const ArchSpec *arch) {
  Log *log = GetLog(LLDBLog::Platform);

  bool create = force;
  if (!create && arch && arch->IsValid()) {
    const llvm::Triple &triple = arch->GetTriple();
    switch (triple.getVendor()) {
    case llvm::Triple::PC:
      create = true;
      break;
    default:
      create = triple.isOpenHOS();
      break;
    }
  }

  if (create) {
    LLDB_LOGF(log, "PlatformHOS::%s() creating remote-hos platform",
              __FUNCTION__);
    return PlatformSP(new PlatformHOS(false));
  }

  LLDB_LOGF(log, "PlatformHOS::%s() aborting creation of remote-hos platform",
            __FUNCTION__);
  return PlatformSP();
}

PlatformHOS::PlatformHOS(bool is_host)
    : PlatformLinux(is_host), m_sdk_version(0) {
  m_connect_addr = "localhost";
}

PlatformHOS::~PlatformHOS() = default;

llvm::StringRef PlatformHOS::GetPluginNameStatic(bool is_host) {
  return is_host ? Platform::GetHostPlatformName() : "remote-hos";
}

const char *PlatformHOS::GetPluginDescriptionStatic(bool is_host) {
  return is_host ? "Local HarmonyOS user platform plug-in."
                 : "Remote HarmonyOS user platform plug-in.";
}

llvm::StringRef PlatformHOS::GetPluginName() {
  if (GetContainer())
    return "remote-hos-inner";

  return GetPluginNameStatic(IsHost());
}

HdcClient PlatformHOS::CreateHdcClient() {
  return HdcClient(m_connect_addr, m_device_id);
}

Status PlatformHOS::ConnectRemote(Args &args) {
  m_device_id.clear();
  m_connect_addr = "localhost";

  if (IsHost()) {
    return Status::FromErrorStringWithFormat(
        "can't connect to the host platform '%s', always connected",
        GetPluginName().str().c_str());
  }

  if (!m_remote_platform_sp)
    m_remote_platform_sp = PlatformSP(new PlatformHOSRemoteGDBServer());

  const char *url = args.GetArgumentAtIndex(0);
  if (!url)
    return Status::FromErrorStringWithFormat("URL is null.");
  std::optional<URI> uri = URI::Parse(url);
  if (!uri)
    return Status::FromErrorStringWithFormat("Invalid URL: %s", url);

  Log *log = GetLog(LLDBLog::Platform);
  if (PlatformHOSRemoteGDBServer::IsHostnameDeviceID(uri->hostname)) {
    m_device_id = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as device id: \"{0}\"", m_device_id);
  } else {
    m_connect_addr = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as remote HDC server address: \"{0}\"",
             m_connect_addr);
  }

  auto error = PlatformLinux::ConnectRemote(args);
  if (error.Fail()) {
    m_remote_platform_sp.reset();
    return error;
  }

  HdcClient hdc(m_connect_addr);
  error = HdcClient::CreateByDeviceID(m_device_id, hdc);
  if (error.Fail()) {
    m_remote_platform_sp.reset();
    return error;
  }

  m_device_id = hdc.GetDeviceID();
  return error;
}

Status PlatformHOS::GetFile(const FileSpec &source,
                              const FileSpec &destination) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::GetFile(source, destination);

  FileSpec source_spec(source.GetPath(false), FileSpec::Style::posix);
  if (source_spec.IsRelative())
    source_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        source_spec.GetPath(false));

  if (GetContainer())
    return GetFileFromContainer(source_spec, destination);

  return DoGetFile(source_spec, destination);
}

Status PlatformHOS::GetFileFromContainer(const FileSpec &source,
                                        const FileSpec &destination) {
  Log *log = GetLog(LLDBLog::Platform);
  llvm::StringRef path(source.GetPath(false));

  for (const auto &map : kPathPrefixMap) {
    const llvm::StringRef prefix = map.first;
    const llvm::StringRef new_prefix = map.second;

    if (!path.starts_with(prefix))
      continue;

    FileSpec new_source(new_prefix, FileSpec::Style::posix);
    new_source.AppendPathComponent(path.substr(prefix.size()));

    LLDB_LOGF(log, "path '%s' inside container is converted to '%s'",
              source.GetPath(false).c_str(), new_source.GetPath(false).c_str());

    Status error = DoGetFile(new_source, destination);
    if (error.Fail()) {
      LLDB_LOGF(log, "failed to get file '%s': %s",
                new_source.GetPath(false).c_str(), error.AsCString());
      LLDB_LOGF(log, "try to get file '%s' as a fallback",
                source.GetPath(false).c_str());
      error = DoGetFile(source, destination);
    }
    return error;
  }

  LLDB_LOGF(log, "try to get file '%s' inside container without conversion",
            source.GetPath(false).c_str());
  return DoGetFile(source, destination);
}

Status PlatformHOS::DoGetFile(const FileSpec &source,
                              const FileSpec &destination) {
  HdcClient hdc = CreateHdcClient();
  return hdc.RecvFile(source, destination);
}

Status PlatformHOS::PutFile(const FileSpec &source, const FileSpec &destination,
                            uint32_t uid, uint32_t gid) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::PutFile(source, destination, uid, gid);

  FileSpec destination_spec(destination.GetPath(false), FileSpec::Style::posix);
  if (destination_spec.IsRelative())
    destination_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        destination_spec.GetPath(false));

  HdcClient hdc = CreateHdcClient();
  return hdc.SendFile(source, destination_spec);
}

const char *PlatformHOS::GetCacheHostname() { return m_device_id.c_str(); }

Status PlatformHOS::DownloadModuleSlice(const FileSpec &src_file_spec,
                                        const uint64_t src_offset,
                                        const uint64_t src_size,
                                        const FileSpec &dst_file_spec) {
  if (src_offset != 0)
    return Status::FromErrorStringWithFormat("Invalid offset - %" PRIu64,
                                              src_offset);

  return GetFile(src_file_spec, dst_file_spec);
}

Status PlatformHOS::DisconnectRemote() {
  Status error = PlatformLinux::DisconnectRemote();
  if (error.Success()) {
    m_device_id.clear();
    m_sdk_version = 0;
    m_remote_platform_sp.reset();
  }
  return error;
}

uint32_t PlatformHOS::GetDefaultMemoryCacheLineSize() {
  return g_hos_default_cache_size;
}

uint32_t PlatformHOS::GetSdkVersion() {
  if (!IsConnected())
    return 0;

  if (m_sdk_version != 0)
    return m_sdk_version;

  std::string version_string;
  HdcClient hdc = CreateHdcClient();
  Status error =
      hdc.Shell("param get const.ohos.apiversion", seconds(5), &version_string);
  version_string = llvm::StringRef(version_string).trim().str();

  if (error.Fail() || version_string.empty()) {
    Log *log = GetLog(LLDBLog::Platform);
    LLDB_LOGF(log, "Get SDK version failed. (error: %s, output: %s)",
              error.AsCString(), version_string.c_str());
    m_sdk_version = INVALID_SDK_VERSION;
    return 0;
  }

  m_sdk_version = INVALID_SDK_VERSION;
  llvm::to_integer(version_string, m_sdk_version);
  if (m_sdk_version == INVALID_SDK_VERSION)
    return 0;

  return m_sdk_version;
}

bool PlatformHOS::GetRemoteOSVersion() {
  m_os_version = llvm::VersionTuple(GetSdkVersion());
  return !m_os_version.empty();
}

llvm::StringRef
PlatformHOS::GetLibdlFunctionDeclarations(lldb_private::Process *process) {
  SymbolContextList matching_symbols;
  std::vector<const char *> dl_open_names = {"__dl_dlopen", "dlopen"};
  const char *dl_open_name = nullptr;
  Target &target = process->GetTarget();
  for (auto name : dl_open_names) {
    target.GetImages().FindFunctionSymbols(ConstString(name),
                                           eFunctionNameTypeFull,
                                           matching_symbols);
    if (matching_symbols.GetSize()) {
      dl_open_name = name;
      break;
    }
  }
  if (dl_open_name == dl_open_names[0])
    return R"(
              extern "C" void* dlopen(const char*, int) asm("__dl_dlopen");
              extern "C" void* dlsym(void*, const char*) asm("__dl_dlsym");
              extern "C" int   dlclose(void*) asm("__dl_dlclose");
              extern "C" char* dlerror(void) asm("__dl_dlerror");
             )";

  return PlatformPOSIX::GetLibdlFunctionDeclarations(process);
}

ConstString PlatformHOS::GetMmapSymbolName(const ArchSpec &arch) {
  return arch.GetTriple().isArch32Bit() ? ConstString("__lldb_mmap")
                                      : PlatformLinux::GetMmapSymbolName(arch);
}

MmapArgList PlatformHOS::GetMmapArgumentList(const ArchSpec &arch, addr_t addr,
                                             addr_t length, unsigned prot,
                                             unsigned flags, addr_t fd,
                                             addr_t offset) {
  uint64_t flags_platform = 0;
  const uint64_t map_anon = arch.IsMIPS() ? MAP_ANON_MIPS : MAP_ANON;

  if (flags & eMmapFlagsPrivate)
    flags_platform |= MAP_PRIVATE;
  if (flags & eMmapFlagsAnon)
    flags_platform |= map_anon;
  if (flags & eMmapFlagsAnon && prot & PROT_EXEC)
    flags_platform |= MAP_JIT;

  return MmapArgList({addr, length, prot, flags_platform, fd, offset});
}
#endif /* OHOS_LLVM */
