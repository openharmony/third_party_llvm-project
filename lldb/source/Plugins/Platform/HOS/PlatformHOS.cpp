//===-- PlatformHOS.cpp -----------------------------------------*- C++ -*-===//
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
#include "lldb/Host/StringConvert.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Scalar.h"
#include "lldb/Utility/UriParser.h"
#include "llvm/Config/config.h"

#include "HdcClient.h"
#include "PlatformHOS.h"
#include "PlatformHOSRemoteGDBServer.h"
#if defined __MINGW32__
// SetEnvironmentVariableA
#include <processenv.h>
#endif

using namespace lldb;
using namespace lldb_private;
// using namespace lldb_private::platform_android;
using namespace lldb_private::platform_hos;
using namespace std::chrono;
static uint32_t g_initialize_count = 0;
static const unsigned int g_hos_default_cache_size = 2048;
LLDB_PLUGIN_DEFINE(PlatformHOS)

static void platform_setenv(const char *env, const char *val) {
#if HAVE_SETENV || _MSC_VER
  setenv(env, val, true);
#elif defined(__MINGW32__)
  SetEnvironmentVariableA(env, val);
#else
#error "setenv not found"
#endif
}

void PlatformHOS::Initialize() {
  PlatformLinux::Initialize();
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (g_initialize_count++ == 0) {
#if defined(__HOS__)
    PlatformSP default_platform_sp(new PlatformHOS(true));
    default_platform_sp->SetSystemArchitecture(HostInfo::GetArchitecture());
    Platform::SetHostPlatform(default_platform_sp);
    if (log) {
      LLDB_LOGF(log, "Hsu file(%s)%d PlatformHOS::%s new PlatformHOS(true)",
                __FILE__, __LINE__, __FUNCTION__);
    }
#endif
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
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (log) {
    const char *triple_cstr =
        arch ? arch->GetTriple().getTriple().c_str() : "<null>";

    LLDB_LOGF(log, "PlatformHOS::%s(force=%s, triple=%s)", __FUNCTION__,
              force ? "true" : "false", triple_cstr);
  }

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
    if (const char *env = std::getenv("HDC_UTID"))
      platform_setenv("ANDROID_SERIAL", env);

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
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (log) {
    LLDB_LOGF(log, "Hsu file(%s):%d PlatformHOS::%s is_host(%d)", __FILE__,
              __LINE__, __FUNCTION__, is_host);
  }
}

PlatformHOS::~PlatformHOS() {}

ConstString PlatformHOS::GetPluginNameStatic(bool is_host) {
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (log) {
    LLDB_LOGF(log, "Hsu %s:%d PlatformHOS::GetPluginNameStatic is_host(%d)",
              __FILE__, __LINE__, is_host);
  }
  if (is_host) {
    static ConstString g_host_name(Platform::GetHostPlatformName());
    if (log) {
      LLDB_LOGF(log, "Hsu %s:%d PlatformHOS::GetPluginNameStatic g_host_name",
                __FILE__, __LINE__);
    }
    return g_host_name;
  } else {
    static ConstString g_remote_name("remote-hos");
    if (log) {
      LLDB_LOGF(log, "Hsu %s:%d PlatformHOS::GetPluginNameStatic g_remote_name",
                __FILE__, __LINE__);
    }
    return g_remote_name;
  }
}

const char *PlatformHOS::GetPluginDescriptionStatic(bool is_host) {
  if (is_host)
    return "Local HarmonyOS user platform plug-in.";
  else
    return "Remote HarmonyOS user platform plug-in.";
}
Status PlatformHOS::ConnectRemote(Args &args) {
  m_device_id.clear();
  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  if (log) {
    LLDB_LOGF(log, "Hsu %s:%d PlatformHOS::ConnectRemote call", __FILE__,
              __LINE__);
  }

  if (IsHost()) {
    return Status("can't connect to the host platform '%s', always connected",
                  GetPluginName().GetCString());
  }

  if (!m_remote_platform_sp) {
    if (log) {
      LLDB_LOGF(log,
                "Hsu %s:%d PlatformHOS::ConnectRemote new "
                "PlatformHOSRemoteGDBServer()",
                __FILE__, __LINE__);
    }
    m_remote_platform_sp = PlatformSP(new PlatformHOSRemoteGDBServer());
  }

  int port;
  llvm::StringRef scheme, host, path;
  const char *url = args.GetArgumentAtIndex(0);
  if (!url)
    return Status("URL is null.");
  if (!UriParser::Parse(url, scheme, host, port, path))
    return Status("Invalid URL: %s", url);
  if (host != "localhost")
    m_device_id = static_cast<std::string>(host);

  auto error = PlatformLinux::ConnectRemote(args);
  if (error.Success()) {
    HdcClient hdc;
    if (log) {
      LLDB_LOGF(log,
                "Hsu file(%s):%d PlatformHOS::ConnectRemote m_device_id(%s)",
                __FILE__, __LINE__, m_device_id.c_str());
    }
    error = HdcClient::CreateByDeviceID(m_device_id, hdc);
    if (error.Fail())
      return error;

    m_device_id = hdc.GetDeviceID();
    if (log) {
      LLDB_LOGF(log,
                "Hsu file(%s):%d PlatformHOS::ConnectRemote m_device_id(%s)",
                __FILE__, __LINE__, m_device_id.c_str());
    }
  }
  return error;
}

Status PlatformHOS::GetFile(const FileSpec &source,
                            const FileSpec &destination) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::GetFile(source, destination);

  FileSpec source_spec(source.GetPath(false), FileSpec::Style::posix);
  if (source_spec.IsRelative())
    source_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        source_spec.GetCString(false));

  Status error;
  auto sync_service = GetSyncService(error);
  if (error.Fail())
    return error;

  uint32_t mode = 0, size = 0, mtime = 0;
  error = sync_service->Stat(source_spec, mode, size, mtime);
  if (error.Fail())
    return error;

  if (mode != 0)
    return sync_service->PullFile(source_spec, destination);

  auto source_file = source_spec.GetCString(false);

  Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
  LLDB_LOGF(log, "Got mode == 0 on '%s': try to get file via 'shell cat'",
            source_file);

  if (strchr(source_file, '\'') != nullptr)
    return Status("Doesn't support single-quotes in filenames");

  // mode == 0 can signify that adbd cannot access the file due security
  // constraints - try "cat ..." as a fallback.
  if (log) {
    LLDB_LOGF(log, "Hsu file(%s):%d PlatformHOS::%s source_file(%s)", __FILE__,
              __LINE__, __FUNCTION__, source_file);
  }
  HdcClient hdc(m_device_id);

  char cmd[PATH_MAX];
  int len = strlen(source_file);
  std::string tempFile(source_file);
  if (m_container) {
    /*
    /data       /data/ohos_data
    /vendor      /sytem/ohos/vendor
    /system     /system/ohos/system
    */
    const std::string str_data = "/data";
    const std::string str_vendor = "/vendor";
    const std::string str_system = "/system";
    const std::string str_data_append = "/data/ohos_data";
    const std::string str_vendor_append = "/vendor/ohos/vendor";
    const std::string str_system_append = "/system/ohos/system";
    if (!strncmp(source_file, str_data.c_str(), strlen(str_data.c_str()))) {
      tempFile = str_data_append + tempFile.substr(strlen(str_data.c_str()));
      snprintf(cmd, sizeof(cmd), "cat '%s'", tempFile.c_str());
      return hdc.ShellToFile(cmd, minutes(1), destination);
    }
    if (!strncmp(source_file, str_vendor.c_str(), strlen(str_vendor.c_str()))) {
      tempFile =
          str_vendor_append + tempFile.substr(strlen(str_vendor.c_str()));
      snprintf(cmd, sizeof(cmd), "cat '%s'", tempFile.c_str());
      return hdc.ShellToFile(cmd, minutes(1), destination);
    }

    if (!strncmp(source_file, str_system.c_str(), strlen(str_system.c_str()))) {
      tempFile =
          str_system_append + tempFile.substr(strlen(str_system.c_str()));
      snprintf(cmd, sizeof(cmd), "cat '%s'", tempFile.c_str());
      return hdc.ShellToFile(cmd, minutes(1), destination);
    }
    return error;
  } else {
    snprintf(cmd, sizeof(cmd), "cat '%s'", source_file);
  }

  if (log) {
    LLDB_LOGF(log, "Hsu file(%s):%d PlatformHOS::%s source_file(%s)", __FILE__,
              __LINE__, __FUNCTION__, source_file);
  }
  return hdc.ShellToFile(cmd, minutes(1), destination);
}

Status PlatformHOS::PutFile(const FileSpec &source, const FileSpec &destination,
                            uint32_t uid, uint32_t gid) {
  if (IsHost() || !m_remote_platform_sp)
    return PlatformLinux::PutFile(source, destination, uid, gid);

  FileSpec destination_spec(destination.GetPath(false), FileSpec::Style::posix);
  if (destination_spec.IsRelative())
    destination_spec = GetRemoteWorkingDirectory().CopyByAppendingPathComponent(
        destination_spec.GetCString(false));

  // TODO: Set correct uid and gid on remote file.
  Status error;
  auto sync_service = GetSyncService(error);
  if (error.Fail())
    return error;
  return sync_service->PushFile(source, destination_spec);
}

const char *PlatformHOS::GetCacheHostname() { return m_device_id.c_str(); }

Status PlatformHOS::DownloadModuleSlice(const FileSpec &src_file_spec,
                                        const uint64_t src_offset,
                                        const uint64_t src_size,
                                        const FileSpec &dst_file_spec) {
  if (src_offset != 0)
    return Status("Invalid offset - %" PRIu64, src_offset);

  return GetFile(src_file_spec, dst_file_spec);
}

Status PlatformHOS::DisconnectRemote() {
  Status error = PlatformLinux::DisconnectRemote();
  if (error.Success()) {
    m_device_id.clear();
    m_sdk_version = 0;
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
  HdcClient hdc(m_device_id);
  Status error =
      hdc.Shell("getprop ro.build.version.sdk", seconds(5), &version_string);
  version_string = llvm::StringRef(version_string).trim().str();

  if (error.Fail() || version_string.empty()) {
    Log *log = GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM);
    LLDB_LOGF(log, "Get SDK version failed. (error: %s, output: %s)",
              error.AsCString(), version_string.c_str());
    return 0;
  }

  m_sdk_version = StringConvert::ToUInt32(version_string.c_str());
  return m_sdk_version;
}

Status PlatformHOS::DownloadSymbolFile(const lldb::ModuleSP &module_sp,
                                       const FileSpec &dst_file_spec) {
  // For oat file we can try to fetch additional debug info from the device
  ConstString extension = module_sp->GetFileSpec().GetFileNameExtension();
  if (extension != ".oat" && extension != ".odex")
    return Status(
        "Symbol file downloading only supported for oat and odex files");

  // If we have no information about the platform file we can't execute oatdump
  if (!module_sp->GetPlatformFileSpec())
    return Status("No platform file specified");

  // Symbolizer isn't available before SDK version 23
  if (GetSdkVersion() < 23)
    return Status("Symbol file generation only supported on SDK 23+");

  // If we already have symtab then we don't have to try and generate one
  if (module_sp->GetSectionList()->FindSectionByName(ConstString(".symtab")) !=
      nullptr)
    return Status("Symtab already available in the module");

  HdcClient hdc(m_device_id);
  std::string tmpdir;
  Status error;
  if (m_container) {
    error = hdc.Shell("mktemp --directory --tmpdir /data/ohos_data/local/tmp",
                      seconds(5), &tmpdir);
  } else {
    error = hdc.Shell("mktemp --directory --tmpdir /data/local/tmp", seconds(5),
                      &tmpdir);
  }
  if (error.Fail() || tmpdir.empty())
    return Status("Failed to generate temporary directory on the device (%s)",
                  error.AsCString());
  tmpdir = llvm::StringRef(tmpdir).trim().str();

  // Create file remover for the temporary directory created on the device
  std::unique_ptr<std::string, std::function<void(std::string *)>>
      tmpdir_remover(&tmpdir, [&hdc](std::string *s) {
        StreamString command;
        command.Printf("rm -rf %s", s->c_str());
        Status error = hdc.Shell(command.GetData(), seconds(5), nullptr);

        Log *log(GetLogIfAllCategoriesSet(LIBLLDB_LOG_PLATFORM));
        if (log && error.Fail())
          LLDB_LOGF(log, "Failed to remove temp directory: %s",
                    error.AsCString());
      });

  FileSpec symfile_platform_filespec(tmpdir);
  symfile_platform_filespec.AppendPathComponent("symbolized.oat");

  // Execute oatdump on the remote device to generate a file with symtab
  StreamString command;
  command.Printf("oatdump --symbolize=%s --output=%s",
                 module_sp->GetPlatformFileSpec().GetCString(false),
                 symfile_platform_filespec.GetCString(false));
  error = hdc.Shell(command.GetData(), minutes(1), nullptr);
  if (error.Fail())
    return Status("Oatdump failed: %s", error.AsCString());

  // Download the symbolfile from the remote device
  return GetFile(symfile_platform_filespec, dst_file_spec);
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
    target.GetImages().FindFunctionSymbols(
        ConstString(name), eFunctionNameTypeFull, matching_symbols);
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

std::unique_ptr<HdcClient::SyncService> PlatformHOS::GetSyncService(Status &error) {
  HdcClient hdc(m_device_id);
  std::unique_ptr<HdcClient::SyncService> adb_sync_svc = hdc.GetSyncService(error);
  return (error.Success()) ? std::move(adb_sync_svc) : nullptr;
}
