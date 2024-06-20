//===-- XVMRegisterInfo.cpp - XVM Register Information ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the XVM implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVMRegisterInfo.h"
#include "XVM.h"
#include "XVMSubtarget.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "XVMGenRegisterInfo.inc"
#include <sstream>

using namespace llvm;
#define XVM_STACK_SIZE_LIMIT 1024

XVMRegisterInfo::XVMRegisterInfo()
    : XVMGenRegisterInfo(XVM::R0) {}

const MCPhysReg *
XVMRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector XVMRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  return Reserved;
}

static void WarnSize(int Offset, MachineFunction &MF, DebugLoc& DL)
{
  if (Offset <= -512) {
      const Function &F = MF.getFunction();
      std::ostringstream OSS;
      OSS << "Looks like the XVM stack limit of " << XVM_STACK_SIZE_LIMIT << " bytes is exceeded. "
          << "Please move large on stack variables into XVM maps.\n";
      std::string StrCopy = OSS.str();
      DiagnosticInfoUnsupported DiagStackSize(F, StrCopy, DL);
      F.getContext().diagnose(DiagStackSize);
  }
}

void XVMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");
  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc DL = MI.getDebugLoc();
  if (!DL)
    /* try harder to get some debug loc */
    for (auto &I : MBB)
      if (I.getDebugLoc()) {
        DL = I.getDebugLoc();
        break;
      }

  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  Register FrameReg = getFrameRegister(MF);
  int FrameIndex = MI.getOperand(i).getIndex();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  /* Offset from frame index are offsets from FP,
   * which is pointing at current function's stack bottom (larger address)
   * Note that this stack bottom does not include saved registers in prologue
   * i.e. the bottom is above the registers
   *
   * We don't have FP, so we use SP to locate data on stack.
   * SP has been pre-determined in the prologue,
   * it is pointing at (FP - StackSize), and it is fixed during function execution
   * Therefore, we use SP+(Stacksize + FP_offset) to mimic (FP + FP_offset)
   */
  auto &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();

  if (MI.getOpcode() == XVM::MOV_rr) {
    int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);
    WarnSize(Offset, MF, DL);
    Register reg = MI.getOperand(i - 1).getReg();
    BuildMI(MBB, ++II, DL, TII.get(XVM::MOV_rr), reg).addReg(XVM::SP);
    if (StackSize + Offset) {
      BuildMI(MBB, II, DL, TII.get(XVM::AddRef_ri), reg).addReg(reg).addImm(StackSize + Offset);
    }
    MI.eraseFromParent();
    return;
  }

  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex) +
               MI.getOperand(i + 1).getImm();
  if (!isInt<32>(Offset))
    llvm_unreachable("bug in frame offset");

  WarnSize(Offset, MF, DL);

  if (MI.getOpcode() == XVM::FI_ri) {
    // Note: to be tested and modified later
    // architecture does not really support FI_ri, replace it with
    //    MOV_rr <target_reg>, frame_reg
    //    ADD_ri <target_reg>, imm
    Register reg = MI.getOperand(i - 1).getReg();
    BuildMI(MBB, ++II, DL, TII.get(XVM::MOV_rr), reg).addReg(FrameReg);
    BuildMI(MBB, II, DL, TII.get(XVM::AddRef_ri), reg).addReg(reg).addImm(StackSize + Offset);

    // Remove FI_ri instruction
    MI.eraseFromParent();
  } else {
    MI.getOperand(i).ChangeToRegister(FrameReg, false);
    MI.getOperand(i + 1).ChangeToImmediate(StackSize + Offset);
  }
}

Register XVMRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return XVM::SP;
}

const TargetRegisterClass *
XVMRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind) const {
  assert(Kind == 0 && "Only one kind of pointer on XVM");
  return &XVM::XVMGPRRegClass;
}

const uint32_t *
XVMRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID CC) const {
  return CSR_RegMask;
}
#endif
