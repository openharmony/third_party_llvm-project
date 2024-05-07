//===--- XVMToolchain.h - XVMToolchain ToolChain Implementations --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_XVMLTOLINUX_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_XVMLTOLINUX_H

#include "Gnu.h"
#include "clang/Driver/ToolChain.h"

namespace clang {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY XVMToolchain : public Generic_ELF {
public:
  XVMToolchain(const Driver &D, const llvm::Triple &Triple,
        const llvm::opt::ArgList &Args);

  void
  AddClangSystemIncludeArgs(const llvm::opt::ArgList &DriverArgs,
                            llvm::opt::ArgStringList &CC1Args) const override;
  std::string computeSysRoot() const override;

protected:
  Tool *buildLinker() const override;
};

} // end namespace toolchains

namespace tools {

namespace XVM {

class LLVM_LIBRARY_VISIBILITY Linker : public Tool {
public:
  Linker(const ToolChain &TC) : Tool("XVM::Linker", "ld", TC) {}
  bool hasIntegratedCPP() const override { return false; }
  bool isLinkJob() const override { return true; }
  void ConstructJob(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const override;
private:
  std::string getLinkerPath(const llvm::opt::ArgList &Args) const;
  void ConstructJobForLTO(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const;
  void ConstructJobForNonLTO(Compilation &C, const JobAction &JA,
                    const InputInfo &Output, const InputInfoList &Inputs,
                    const llvm::opt::ArgList &TCArgs,
                    const char *LinkingOutput) const;
  // The tool name used for lto linking
  const char * LdName = "/ld.lld";
  // The tool name used for linking *.ll/*.bc files;
  const char * LlvmLinkName = "/llvm-link";
  // The tool name used for the final xvm llvm-backend
  const char * LlcName = "/llc";
};

} // end namespace XVM

} // end namespace tools

} // end namespace driver
} // end namespace clang


#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_XVMLTOLINUX_H
