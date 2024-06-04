//===-- XVMSubtarget.cpp - XVM Subtarget Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the XVM specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVMSubtarget.h"
#include "XVM.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Host.h"

using namespace llvm;

#define DEBUG_TYPE "xvm-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "XVMGenSubtargetInfo.inc"

void XVMSubtarget::anchor() {}

XVMSubtarget &XVMSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                            StringRef FS) {
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  return *this;
}

XVMSubtarget::XVMSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : XVMGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      FrameLowering(initializeSubtargetDependencies(CPU, FS)),
      TLInfo(TM, *this) { isCommon = true; }

#endif
