//===-- XVMAsmPrinter.cpp - XVM LLVM assembly writer ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the XVM assembly language.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE
// Insert the XVM backend code here
#include "llvm/Support/Compiler.h"
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMAsmPrinter() {
}

#else

#include "XVMDylibHandler.h"
LLVM_INIT_XVM_COMP(AsmPrinter)

#endif
