//===-- XVMTargetMachine.cpp - Define TargetMachine for XVM ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about XVM target spec.
//
//===----------------------------------------------------------------------===//

#ifdef XVM_DYLIB_MODE
// Insert the XVM backend code here
#include "llvm/Support/Compiler.h"
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTarget() {
}

#else

#include "XVMDylibHandler.h"
LLVM_INIT_XVM_COMP(Target)

#endif
