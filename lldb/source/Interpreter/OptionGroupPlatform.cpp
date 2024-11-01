//===-- OptionGroupPlatform.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Interpreter/OptionGroupPlatform.h"

#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Target/Platform.h"

using namespace lldb;
using namespace lldb_private;

PlatformSP OptionGroupPlatform::CreatePlatformWithOptions(
    CommandInterpreter &interpreter, const ArchSpec &arch, bool make_selected,
    Status &error, ArchSpec &platform_arch) const {
  PlatformList &platforms = interpreter.GetDebugger().GetPlatformList();

  PlatformSP platform_sp;
  Log *log = GetLog(LLDBLog::Commands);
  LLDB_LOGF(log, "Hsu file(%s):%d OptionGroupPlatform::%s call make_selected:%d",
            __FILE__, __LINE__, __FUNCTION__, make_selected);
  if (!m_platform_name.empty()) {
    platform_sp = platforms.Create(m_platform_name);
    if (!platform_sp) {
      error.SetErrorStringWithFormatv(
          "unable to find a plug-in for the platform named \"{0}\"",
          m_platform_name);
    }
    if (platform_sp) {
      if (GetContainer()) {
        LLDB_LOGF(log, "Platform is created inside container.");
        platform_sp->SetContainer(true);
      }
      if (platform_arch.IsValid() && !platform_sp->IsCompatibleArchitecture(
                                         arch, {}, false, &platform_arch)) {
        error.SetErrorStringWithFormatv("platform '{0}' doesn't support '{1}'",
                                        platform_sp->GetPluginName(),
                                        arch.GetTriple().getTriple());
        platform_sp.reset();
        return platform_sp;
      }
    }
  } else if (arch.IsValid()) {
    platform_sp = platforms.GetOrCreate(arch, {}, &platform_arch, error);
  }

  if (platform_sp) {
    if (make_selected)
      platforms.SetSelectedPlatform(platform_sp);
    if (!m_os_version.empty())
      platform_sp->SetOSVersion(m_os_version);

    if (m_sdk_sysroot)
      platform_sp->SetSDKRootDirectory(m_sdk_sysroot);

    if (m_sdk_build)
      platform_sp->SetSDKBuild(m_sdk_build);
  }

  return platform_sp;
}

void OptionGroupPlatform::OptionParsingStarting(
    ExecutionContext *execution_context) {
  m_platform_name.clear();
  m_sdk_sysroot.Clear();
  m_sdk_build.Clear();
  m_os_version = llvm::VersionTuple();
  m_container = false;
}

static constexpr OptionDefinition g_option_table[] = {
    {LLDB_OPT_SET_ALL, false, "platform", 'p', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypePlatform, "Specify name of the platform to "
                                       "use for this target, creating the "
                                       "platform if necessary."},
    {LLDB_OPT_SET_ALL, false, "version", 'v', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeNone,
     "Specify the initial SDK version to use prior to connecting."},
    {LLDB_OPT_SET_ALL, false, "build", 'b', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeNone,
     "Specify the initial SDK build number."},
    {LLDB_OPT_SET_ALL, false, "sysroot", 'S', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeFilename, "Specify the SDK root directory "
                                       "that contains a root of all "
                                       "remote system files."}};

llvm::ArrayRef<OptionDefinition> OptionGroupPlatform::GetDefinitions() {
  llvm::ArrayRef<OptionDefinition> result(g_option_table);
  if (m_include_platform_option)
    return result;
  return result.drop_front();
}

Status
OptionGroupPlatform::SetOptionValue(uint32_t option_idx,
                                    llvm::StringRef option_arg,
                                    ExecutionContext *execution_context) {
  Status error;
  if (!m_include_platform_option)
    ++option_idx;

  const int short_option = g_option_table[option_idx].short_option;
  switch (short_option) {
  case 'p':
    m_platform_name.assign(std::string(option_arg));
    break;

  case 'v':
    if (m_os_version.tryParse(option_arg))
      error.SetErrorStringWithFormatv("invalid version string '{0}'",
                                      option_arg);
    break;

  case 'b':
    m_sdk_build.SetString(option_arg);
    break;

  case 'S':
    m_sdk_sysroot.SetString(option_arg);
    break;

  default:
    llvm_unreachable("Unimplemented option");
  }
  return error;
}

bool OptionGroupPlatform::PlatformMatches(
    const lldb::PlatformSP &platform_sp) const {
  if (platform_sp) {
    if (!m_platform_name.empty()) {
      if (platform_sp->GetName() != m_platform_name)
        return false;
    }

    if (m_sdk_build && m_sdk_build != platform_sp->GetSDKBuild())
      return false;

    if (m_sdk_sysroot && m_sdk_sysroot != platform_sp->GetSDKRootDirectory())
      return false;

    if (!m_os_version.empty() && m_os_version != platform_sp->GetOSVersion())
      return false;

    if (m_container != platform_sp->GetContainer())
      return false;

    return true;
  }
  return false;
}
