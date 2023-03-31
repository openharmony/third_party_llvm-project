//===-- StackProtectorRetLowering.cpp ---------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is an enhanced function of OHOS for the stack protection. It has made
// special backward cfi protection measures against ROP attacks by adding LR/FP
// checks in the function's prologue and epilogue. It can greatly reduce the
// number of ROP gadgets in the object.
// 
//===----------------------------------------------------------------------===//

/// OHOS_LOCAL begin
#include "llvm/CodeGen/StackProtectorRetLowering.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

/// Check if backward CFI protection is required.
void StackProtectorRetLowering::setupStackProtectorRet(MachineFunction &MF) const {
  if (MF.getFunction().hasFnAttribute("sspret-randomdata")) {
    for (auto &MBB : MF) {
      for (auto &T : MBB.terminators()) {
        if (instrIsRet(T.getOpcode())) {
          MF.getFrameInfo().setStackProtectorRetNeeded(true);
          return;
        }
      }
    }
  }
}

/// Set SSPRetReg as a callee saved register to push the computed stack guard value onto the stack.
void StackProtectorRetLowering::saveStackProtectorRetRegister(
    MachineFunction &MF, std::vector<CalleeSavedInfo> &CSI) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.getStackProtectorRetNeeded())
    return;

  if (!MFI.hasStackProtectorRetRegister())
    llvm_unreachable("Missing available stack-protector-ret register");

  unsigned Reg = MFI.getStackProtectorRetRegister();
  CSI.push_back(CalleeSavedInfo(Reg));
}

/// Determine an available SSPRet register and feedback the determination result.
bool StackProtectorRetLowering::determineStackProtectorRetRegister(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.getStackProtectorRetNeeded())
    return true;

  unsigned Reg = getTargetReg();
  MFI.setStackProtectorRetRegister(Reg);

  return MFI.hasStackProtectorRetRegister();
}

/// insertStackProtectorRets - insert stack-protector-ret instrumentation.
void StackProtectorRetLowering::insertStackProtectorRets(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Function &Fn = MF.getFunction();
  const Module *M = Fn.getParent();

  if (!MFI.getStackProtectorRetNeeded())
    return;

  if (!MFI.hasStackProtectorRetRegister())
    llvm_unreachable("Invalid stack-protector-ret state.");

  GlobalVariable *cookie = dyn_cast_or_null<GlobalVariable>(M->getGlobalVariable(
    Fn.getFnAttribute("sspret-randomdata").getValueAsString(), Type::getInt8PtrTy(M->getContext())));

  if (!cookie)
    llvm_unreachable("Function needs stack-protector-ret but missing cookies available.");

  unsigned Reg = MFI.getStackProtectorRetRegister();

  std::vector<MachineInstr *> returnInstrs;
  for (auto &MBB : MF) {
    if (MBB.isReturnBlock()) {
      for (auto &MI : MBB.terminators()) {
        if (instrIsRet(MI.getOpcode())) {
          returnInstrs.push_back(&MI);
          if (!MBB.isLiveIn(Reg)) {
            MBB.addLiveIn(Reg);
          }
        }
      }
    }
  }

  if (returnInstrs.empty()) {
    return;
  }

  for (auto &MI : returnInstrs) {
    insertStackProtectorRetEpilogue(MF, *MI, cookie);
  }

  insertStackProtectorRetPrologue(MF, MF.front(), cookie);

  if (!MF.front().isLiveIn(Reg)) {
    MF.front().addLiveIn(Reg);
  }
}
/// OHOS_LOCAL end
