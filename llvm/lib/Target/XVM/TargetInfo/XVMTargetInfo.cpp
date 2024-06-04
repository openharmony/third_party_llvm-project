//===-- XVMTargetInfo.cpp - XVM Target Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "TargetInfo/XVMTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheXVMTarget() {
  static Target TheXVMTarget;
  return TheXVMTarget;
}

Target &llvm::getTheXVMleTarget() {
  static Target TheXVMleTarget;
  return TheXVMleTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetInfo() {
  RegisterTarget<Triple::xvm> X(
      getTheXVMleTarget(), "xvm", "XVM (little endian)", "XVM");
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetInfoCalledInDylib() {
  LLVMInitializeXVMTargetInfo();
}

#else

#include "../XVMDylibHandler.h"
llvm::sys::SmartMutex<true>  mutexLoadDylib;
std::map<std::string, void *> mapTargetNameHandler;

LLVM_INIT_XVM_COMP(TargetInfo)

#endif
