//===-- PlatformOHOS.cpp ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Core/Module.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Core/Section.h"
#include "lldb/Core/ValueObject.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Scalar.h"
#include "lldb/Utility/UriParser.h"
#include "llvm/Config/config.h"

#include "HdcClient.h"
#include "PlatformOHOS.h"
#include "PlatformOHOSRemoteGDBServer.h"
#include "lldb/Target/Target.h"

#if defined(__OHOS__)
#include <sys/mman.h>
#else 
// Define these constants from OHOS mman.h for use when targeting remote OHOS
// systems even when host has different values.
#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define MAP_PRIVATE 2
#define MAP_ANON 0x20
#endif

#define MAP_ANON_MIPS 0x800
// MAP_JIT allows to allocate anonymous memory with executable permissions.
#define MAP_JIT 0x1000

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::platform_ohos;
using namespace std::chrono;

static uint32_t g_initialize_count = 0;
static const unsigned int g_ohos_default_cache_size =
    2048; // Fits inside 4k adb packet.
static constexpr uint32_t INVALID_SDK_VERSION = 0xFFFFFFFF;

LLDB_PLUGIN_DEFINE(PlatformOHOS)

void PlatformOHOS::Initialize() {
  PlatformLinux::Initialize();

  if (g_initialize_count++ == 0) {
#if defined(__OHOS__)
    PlatformSP default_platform_sp(new PlatformOHOS(true));
    default_platform_sp->SetSystemArchitecture(HostInfo::GetArchitecture());
    Platform::SetHostPlatform(default_platform_sp);
#endif
    PluginManager::RegisterPlugin(
        PlatformOHOS::GetPluginNameStatic(false),
        PlatformOHOS::GetPluginDescriptionStatic(false),
        PlatformOHOS::CreateInstance);
  }
}

void PlatformOHOS::Terminate() {
  if (g_initialize_count > 0) {
    if (--g_initialize_count == 0) {
      PluginManager::UnregisterPlugin(PlatformOHOS::CreateInstance);
    }
  }

  PlatformLinux::Terminate();
}

PlatformSP PlatformOHOS::CreateInstance(bool force, const ArchSpec *arch) {
  Log *log = GetLog(LLDBLog::Platform);
  if (log) {
    const char *arch_name;
    if (arch && arch->GetArchitectureName())
      arch_name = arch->GetArchitectureName();
    else
      arch_name = "<null>";

    const char *triple_cstr =
        arch ? arch->GetTriple().getTriple().c_str() : "<null>";

    LLDB_LOGF(log, "PlatformOHOS::%s(force=%s, arch={%s,%s})", __FUNCTION__,
              force ? "true" : "false", arch_name, triple_cstr);
  }

  bool create = force;
  if (!create && arch && arch->IsValid()) {
    const llvm::Triple &triple = arch->GetTriple();
    switch (triple.getVendor()) {
    case llvm::Triple::PC:
      create = true;
      break;
    default:
      break;
    }

    if (create) {
      switch (triple.getEnvironment()) {
      case llvm::Triple::OpenHOS:
        break;
      default:
        create = false;
        break;
      }
    }
  }

  if (create) {
    LLDB_LOGF(log, "PlatformOHOS::%s() creating remote-ohos platform",
              __FUNCTION__);
    
    return PlatformSP(new PlatformOHOS(false));
  }

  LLDB_LOGF(log,
      "PlatformOHOS::%s() aborting creation of remote-ohos platform",
      __FUNCTION__);

  return PlatformSP();
}

PlatformOHOS::PlatformOHOS(bool is_host)
    : PlatformLinux(is_host), m_sdk_version(0) {}

PlatformOHOS::~PlatformOHOS() {}

llvm::StringRef PlatformOHOS::GetPluginNameStatic(bool is_host) {
  return is_host ? Platform::GetHostPlatformName() : "remote-ohos";
}

const char *PlatformOHOS::GetPluginDescriptionStatic(bool is_host) {
  return is_host ? "Local Open HarmonyOS user platform plug-in."
                 : "Remote Open HarmonyOS user platform plug-in.";
}

llvm::StringRef PlatformOHOS::GetPluginName() {
  return GetPluginNameStatic(IsHost());
}

HdcClient PlatformOHOS::CreateHdcClient() {
  return HdcClient(m_connect_addr, m_device_id);
}

Status PlatformOHOS::ConnectRemote(Args &args) {
  m_device_id.clear();
  m_connect_addr = "localhost";

  if (IsHost()) {
    return Status("can't connect to the host platform '%s', always connected",
                  GetPluginName().str().c_str());
  }

  if (!m_remote_platform_sp)
    m_remote_platform_sp = PlatformSP(new PlatformOHOSRemoteGDBServer());

  const char *url = args.GetArgumentAtIndex(0);
  if (!url)
    return Status("URL is null.");
  llvm::Optional<URI> uri = URI::Parse(url);
  if (!uri)
    return Status("Invalid URL: %s", url);

  Log *log = GetLog(LLDBLog::Platform);
  if (PlatformOHOSRemoteGDBServer::IsHostnameDeviceID(
          uri->hostname)) { // accepts no (empty) hostname too
    m_device_id = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as device id: \"{0}\"", m_device_id);
  } else {
    m_connect_addr = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as remote HDC server address: \"{0}\"",
             m_connect_addr);
  }

  auto error = PlatformLinux::ConnectRemote(args);
  if (error.Success()) {
    HdcClient hdc(m_connect_addr);
    error = HdcClient::CreateByDeviceID(m_device_id, hdc);
    if (error.Fail())
      return error;

    m_device_id = hdc.GetDeviceID();
  }
  return error;
}

Status PlatformOHOS::GetFile(const FileSpec &source,
                             const FileSpec &destination) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::GetFile(source, destination);

  FileSpec source_spec(source.GetPath(false), FileSpec::Style::posix);
  if (source_spec.IsRelative())
    source_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        source_spec.GetCString(false));

  HdcClient hdc = CreateHdcClient();
  Status error = hdc.RecvFile(source_spec, destination);
  return error;
}

Status PlatformOHOS::PutFile(const FileSpec &source,
                             const FileSpec &destination, uint32_t uid,
                             uint32_t gid) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::PutFile(source, destination, uid, gid);

  FileSpec destination_spec(destination.GetPath(false), FileSpec::Style::posix);
  if (destination_spec.IsRelative())
    destination_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        destination_spec.GetCString(false));

  // TODO: Set correct uid and gid on remote file.
  HdcClient hdc = CreateHdcClient();
  Status error = hdc.SendFile(source, destination_spec);
  return error;
}

const char *PlatformOHOS::GetCacheHostname() { return m_device_id.c_str(); }

Status PlatformOHOS::DownloadModuleSlice(const FileSpec &src_file_spec,
                                            const uint64_t src_offset,
                                            const uint64_t src_size,
                                            const FileSpec &dst_file_spec) {
  if (src_offset != 0)
    return Status("Invalid offset - %" PRIu64, src_offset);

  return GetFile(src_file_spec, dst_file_spec);
}

Status PlatformOHOS::DisconnectRemote() {
  Status error = PlatformLinux::DisconnectRemote();
  if (error.Success()) {
    m_device_id.clear();
    m_sdk_version = 0;
    m_remote_platform_sp.reset();
  }
  return error;
}

uint32_t PlatformOHOS::GetDefaultMemoryCacheLineSize() {
  return g_ohos_default_cache_size;
}

uint32_t PlatformOHOS::GetSdkVersion() {
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
  if (m_sdk_version == INVALID_SDK_VERSION) {
    return 0;
  }

  return m_sdk_version;
}

bool PlatformOHOS::GetRemoteOSVersion() {
  m_os_version = llvm::VersionTuple(GetSdkVersion());
  return !m_os_version.empty();
}

llvm::StringRef
PlatformOHOS::GetLibdlFunctionDeclarations(lldb_private::Process *process) {
  SymbolContextList matching_symbols;
  std::vector<const char *> dl_open_names = { "__dl_dlopen", "dlopen" };
  const char *dl_open_name = nullptr;
  Target &target = process->GetTarget();
  for (auto name: dl_open_names) {
    target.GetImages().FindFunctionSymbols(ConstString(name),
                                           eFunctionNameTypeFull,
                                           matching_symbols);
    if (matching_symbols.GetSize()) {
       dl_open_name = name;
       break;
    }
  }
  // Older platform versions have the dl function symbols mangled
  if (dl_open_name == dl_open_names[0])
    return R"(
              extern "C" void* dlopen(const char*, int) asm("__dl_dlopen");
              extern "C" void* dlsym(void*, const char*) asm("__dl_dlsym");
              extern "C" int   dlclose(void*) asm("__dl_dlclose");
              extern "C" char* dlerror(void) asm("__dl_dlerror");
             )";

  return PlatformPOSIX::GetLibdlFunctionDeclarations(process);
}

ConstString PlatformOHOS::GetMmapSymbolName(const ArchSpec &arch) {
  return arch.GetTriple().isArch32Bit()
             ? ConstString("__lldb_mmap")
             : PlatformLinux::GetMmapSymbolName(arch);
}
//Customize mmap adaptation for OHOS platform. 
//OHOS platform allows anonymous memory allocation with exec permission 
//only when MAP_JIT parameter is specified.
MmapArgList PlatformOHOS::GetMmapArgumentList(const ArchSpec &arch,
                                               addr_t addr, addr_t length,
                                               unsigned prot, unsigned flags,
                                               addr_t fd, addr_t offset) {
  uint64_t flags_platform = 0;
  const uint64_t map_anon = arch.IsMIPS() ? MAP_ANON_MIPS : MAP_ANON;

  if (flags & eMmapFlagsPrivate)
    flags_platform |= MAP_PRIVATE;
  if (flags & eMmapFlagsAnon)
    flags_platform |= map_anon;
  if (flags & eMmapFlagsAnon && prot & PROT_EXEC)
    flags_platform |= MAP_JIT;

  MmapArgList args({addr, length, prot, flags_platform, fd, offset});
  return args;
}
