//===-- XVM.h - Top-level interface for XVM representation ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_XVM_H
#define LLVM_LIB_TARGET_XVM_XVM_H

#include "llvm/CodeGen/TargetInstrInfo.h"
#include "MCTargetDesc/XVMMCTargetDesc.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class XVMTargetMachine;

FunctionPass *createXVMISelDag(XVMTargetMachine &TM);
FunctionPass *createXVMCFGSort();
FunctionPass *createXVMCFGStackify();
FunctionPass *createXVMCFGStructure();
FunctionPass *createXVMExpandPseudoPass();
FunctionPass *createXVMUpdateRefInstrForMIPass();

void initializeXVMCFGSortPass(PassRegistry &);
void initializeXVMCFGStackifyPass(PassRegistry&);
void initializeXVMCFGStructurePass(PassRegistry&);
void initializeXVMExpandPseudoPass(PassRegistry&);
void initializeXVMUpdateRefInstrForMIPass(PassRegistry&);
} // namespace llvm

#endif
