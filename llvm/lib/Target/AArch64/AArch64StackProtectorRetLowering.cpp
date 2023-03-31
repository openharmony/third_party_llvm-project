//===-- AArch64StackProtectorRetLowering.cpp --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a stack-protector-ret function to insert code to mitigate
// against ROP attacks.
//
//===----------------------------------------------------------------------===//

/// OHOS_LOCAL begin
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "AArch64StackProtectorRetLowering.h"
#include "AArch64TargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetOptions.h"
#include <cstdlib>

using namespace llvm;

void AArch64StackProtectorRetLowering::insertStackProtectorRetPrologue(
    MachineFunction &MF, MachineBasicBlock &MBB, GlobalVariable *cookie) const {

  MachineBasicBlock::instr_iterator MI = MBB.instr_begin();
  DebugLoc MBBDL = MBB.findDebugLoc(MI);
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  unsigned REG = MF.getFrameInfo().getStackProtectorRetRegister();

  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::ADRP), REG)
      .addGlobalAddress(cookie, 0, AArch64II::MO_PAGE);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::LDRXui), REG)
      .addReg(REG)
      .addGlobalAddress(cookie, 0, AArch64II::MO_PAGEOFF | AArch64II::MO_NC);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::EORXrr), REG)
      .addReg(REG)
      .addReg(AArch64::FP);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::EORXrr), REG)
      .addReg(REG)
      .addReg(AArch64::LR);
}

void AArch64StackProtectorRetLowering::insertStackProtectorRetEpilogue(
    MachineFunction &MF, MachineInstr &MI, GlobalVariable *cookie) const {

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc MBBDL = MI.getDebugLoc();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  unsigned REG = MF.getFrameInfo().getStackProtectorRetRegister();

  MBB.addLiveIn(AArch64::X9);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::ADRP), AArch64::X9)
      .addGlobalAddress(cookie, 0, AArch64II::MO_PAGE);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::LDRXui), AArch64::X9)
      .addReg(AArch64::X9)
      .addGlobalAddress(cookie, 0, AArch64II::MO_PAGEOFF | AArch64II::MO_NC);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::EORXrr), AArch64::X9)
      .addReg(AArch64::X9)
      .addReg(AArch64::FP);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::EORXrr), AArch64::X9)
      .addReg(AArch64::X9)
      .addReg(AArch64::LR);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::SUBSXrr), REG)
      .addReg(REG)
      .addReg(AArch64::X9);
  BuildMI(MBB, MI, MBBDL, TII->get(AArch64::SSP_RET_TRAP))
      .addReg(REG);
}

/// Check if it is a return instruction on the AArch64 architecture.
bool AArch64StackProtectorRetLowering::instrIsRet(unsigned opcode) const {
  switch (opcode) {
    case AArch64::RET_ReallyLR:
    case AArch64::RET:
      return true;
    default:
      return false;
  }
}

/// Returns a caller-saved register for the AArch64 architecture.
unsigned AArch64StackProtectorRetLowering::getTargetReg(void) const {
  return AArch64::X15;
}

/// Set SSPRetReg as a callee saved register to push the computed stack guard value onto the stack.
void AArch64StackProtectorRetLowering::saveStackProtectorRetRegister(
    MachineFunction &MF, std::vector<CalleeSavedInfo> &CSI) const {

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.getStackProtectorRetNeeded())
    return;

  if (!MFI.hasStackProtectorRetRegister())
    llvm_unreachable("Saving unset stack-protector-ret register");

  unsigned Reg = MFI.getStackProtectorRetRegister();

  CSI.insert(CSI.begin(), CalleeSavedInfo(Reg));
}
/// OHOS_LOCAL end
