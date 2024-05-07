//===-- XVMTargetInfo.cpp - XVM Target Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE
// Insert the XVM backend code here
#include "llvm/Support/Compiler.h"
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetInfo() {
}

#else

#include "../XVMDylibHandler.h"
llvm::sys::SmartMutex<true>  mutexLoadDylib;
std::map<std::string, void *> mapTargetNameHandler;

LLVM_INIT_XVM_COMP(TargetInfo)

#endif
