//===--- XVM.h - Declare XVM target feature support -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares XVM TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_XVM_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_XVM_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY XVMTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

public:
  XVMTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    RegParmMax = 5;
    resetDataLayout("e-m:e-p:64:64-i64:64-n64-S128");
    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 64;
    TLSSupported = false;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  const char *getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  bool isValidGCCRegisterName(StringRef Name) const override { return true; }
  ArrayRef<const char *> getGCCRegNames() const override { return None; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return None;
  }

  bool allowDebugInfoForExternalRef() const override { return true; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_OpenCLKernel:
      return CCCR_OK;
    }
  }

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    StringRef CPUName(Name);
    return isValidCPUName(CPUName);
  }
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_XVM_H
