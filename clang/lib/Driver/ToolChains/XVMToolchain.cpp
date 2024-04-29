//===--- XVMToolchain.h - XVMToolchain ToolChain Implementations --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "XVMToolchain.h"
#include "CommonArgs.h"
#include "clang/Config/config.h"
#include "clang/Driver/Distro.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/SanitizerArgs.h"
#include "llvm/Option/ArgList.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <system_error>

#include "clang/Driver/Compilation.h"
#include "clang/Driver/InputInfo.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

// using tools::addPathIfExists;

XVMToolchain::XVMToolchain(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  getProgramPaths().push_back(D.Dir);
  getFilePaths().push_back(computeSysRoot() + "/lib");
}

Tool *XVMToolchain::buildLinker() const {
  return new tools::XVM::Linker(*this);
}

void XVMToolchain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                               ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc))
    return;

  if (!DriverArgs.hasArg(options::OPT_nobuiltininc)) {
    SmallString<128> Dir(getDriver().ResourceDir);
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }

  if (!DriverArgs.hasArg(options::OPT_nostdlibinc)) {
    SmallString<128> Dir(computeSysRoot());
    llvm::sys::path::append(Dir, "include");
    addSystemInclude(DriverArgs, CC1Args, Dir.str());
  }
}

std::string XVMToolchain::computeSysRoot() const {
  if (!getDriver().SysRoot.empty())
    return getDriver().SysRoot;
 
  SmallString<128> SysRootDir;
  llvm::sys::path::append(SysRootDir, getDriver().Dir, "..",
                          getDriver().getTargetTriple());

  if (!llvm::sys::fs::exists(SysRootDir))
    return std::string();

  return std::string(SysRootDir.str());
}

std::string XVM::Linker::getLinkerPath(const ArgList &Args) const {
  const ToolChain &ToolChain = getToolChain();
  const Driver &D = ToolChain.getDriver();
  StringRef UseLinker;
  std::string ArgAsString;
  std::string xvm_ld_executable;

  // Try to use the one passed in
  if (const Arg* A = Args.getLastArg(options::OPT_fuse_ld_EQ)) {
    UseLinker = A->getValue();
    ArgAsString = A->getAsString(Args);
    if (llvm::sys::path::is_absolute(UseLinker)) {
      if (llvm::sys::fs::can_execute(UseLinker))
        return std::string(UseLinker);
    }
    // Try one more time for the passed in
    if (!UseLinker.empty()) {
      const char * ByteArray = UseLinker.data();
      if (*ByteArray == '/') {
        xvm_ld_executable = D.Dir + std::string(UseLinker);
      } else {
        xvm_ld_executable = D.Dir + "/" + std::string(UseLinker);
      }
      if (llvm::sys::path::is_absolute(xvm_ld_executable)) {
        if (llvm::sys::fs::can_execute(xvm_ld_executable))
          return xvm_ld_executable;
      }
    }
  }
  // Try the pre-set one
  xvm_ld_executable = D.Dir + LdName;
  if (llvm::sys::path::is_absolute(xvm_ld_executable) &&
      llvm::sys::fs::can_execute(xvm_ld_executable))
    return xvm_ld_executable;
  // Return the default one
  return ToolChain.GetProgramPath(ToolChain.getDefaultLinker());
}

void XVM::Linker::ConstructJobForLTO(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const ToolChain &ToolChain = getToolChain();
  const Driver &D = ToolChain.getDriver();
  ArgStringList CmdArgs;
  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));
  Args.AddAllArgs(CmdArgs, options::OPT_L);

  std::string LinkerExec = getLinkerPath(Args);
  CmdArgs.push_back("-m");
  CmdArgs.push_back(Args.MakeArgString(ToolChain.getTripleString().data()));
  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  for (auto Input : Inputs) {
    if (Input.isFilename()) {
      CmdArgs.push_back(Input.getFilename());
    } else {
      if (std::strcmp(Input.getInputArg().getValue(0), "--lto-emit-asm") == 0) {
        CmdArgs.push_back("--lto-emit-asm");
      }
    }
  }
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(LinkerExec),
      CmdArgs, Inputs, Output));
}

void XVM::Linker::ConstructJobForNonLTO(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  const ToolChain &ToolChain = getToolChain();
  const Driver &D = ToolChain.getDriver();
  ArgStringList CmdArgs;
  if (!D.SysRoot.empty())
    CmdArgs.push_back(Args.MakeArgString("--sysroot=" + D.SysRoot));
  Args.AddAllArgs(CmdArgs, options::OPT_L);

  std::string TmpLinkOutput;
  std::string Linker = D.Dir + LlvmLinkName;
  for (auto Input : Inputs) {
    if (Input.isFilename()) {
      CmdArgs.push_back(Input.getFilename());
      TmpLinkOutput = Input.getFilename();
    }
  }
  TmpLinkOutput += ".ir";
  CmdArgs.push_back("-o");
  CmdArgs.push_back(Args.MakeArgString(TmpLinkOutput.data()));
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(Linker),
      CmdArgs, Inputs, Output));

  // construct llc cmd arguments
  CmdArgs.clear();
  CmdArgs.push_back("-march");
  CmdArgs.push_back(Args.MakeArgString(ToolChain.getTripleString().data()));
  CmdArgs.push_back(Args.MakeArgString(TmpLinkOutput.data()));
  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());
  std::string Llc_executable = D.Dir + LlcName;
  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(Llc_executable),
      CmdArgs, Inputs, Output));
}

void XVM::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs,
                                 const ArgList &Args,
                                 const char *LinkingOutput) const {
  if (Args.hasArg(options::OPT_flto) ||
      Args.hasArg(options::OPT_flto_EQ) ||
      Args.hasArg(options::OPT_flto_EQ_auto) ||
      Args.hasArg(options::OPT_flto_EQ_jobserver)) {
    ConstructJobForLTO(C, JA, Output, Inputs, Args, LinkingOutput);
  } else {
    ConstructJobForNonLTO(C, JA, Output, Inputs, Args, LinkingOutput);
  }
}
