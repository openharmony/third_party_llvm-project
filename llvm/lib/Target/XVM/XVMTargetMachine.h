//===-- XVMTargetMachine.h - Define TargetMachine for XVM --- C++ ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the XVM specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_XVMTARGETMACHINE_H
#define LLVM_LIB_TARGET_XVM_XVMTARGETMACHINE_H

#include "XVMSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class XVMTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  XVMSubtarget Subtarget;

public:
  XVMTargetMachine(const Target &Target, const Triple &TTriple,
    StringRef Core, StringRef FString, const TargetOptions &Options,
    Optional<Reloc::Model> RelocMdl, Optional<CodeModel::Model> CodeMdl,
    CodeGenOpt::Level OptLvl, bool JustInTime);

  const XVMSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const XVMSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
}

#endif
