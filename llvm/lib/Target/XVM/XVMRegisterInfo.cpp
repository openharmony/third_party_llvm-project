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
#define TEMPLATE_INT_32 32

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

static void WarnSize(int Offset, MachineFunction &MF, DebugLoc& DL) {
  if (Offset <= -XVM_STACK_SIZE_LIMIT) {
    const Function &F = MF.getFunction();
    std::ostringstream OSS;
    OSS << "Looks like the XVM stack limit of " << XVM_STACK_SIZE_LIMIT << " bytes is exceeded. "
        << "Please move large on stack variables into XVM maps.\n";
    std::string StrCopy = OSS.str();
    DiagnosticInfoUnsupported DiagStackSize(F, StrCopy, DL);
    F.getContext().diagnose(DiagStackSize);
  }
}

void XVMRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MBBIterator,
                                          int AdjStackP, unsigned FIOpNum,
                                          RegScavenger *RegScav) const {
  assert(AdjStackP == 0 && "Unexpected");
  unsigned counter = 0;
  MachineInstr &MInstr = *MBBIterator;
  MachineBasicBlock &MBBlock = *MInstr.getParent();
  MachineFunction &MFunction = *MBBlock.getParent();
  DebugLoc DLoc = MInstr.getDebugLoc();
  if (!DLoc)
    /* try harder to get some debug loc */
    for (auto &RetryIterator : MBBlock)
      if (RetryIterator.getDebugLoc()) {
        DLoc = RetryIterator.getDebugLoc();
        break;
      }

  while (!MInstr.getOperand(counter).isFI()) {
    ++counter;
    assert(counter < MInstr.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  Register FrameReg = getFrameRegister(MFunction);
  int FrameIndex = MInstr.getOperand(counter).getIndex();
  const TargetInstrInfo &TII = *MFunction.getSubtarget().getInstrInfo();

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
  auto &MFI = MFunction.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();

  if (MInstr.getOpcode() == XVM::MOV_rr) {
    int Offset = MFunction.getFrameInfo().getObjectOffset(FrameIndex);
    WarnSize(Offset, MFunction, DLoc);
    Register reg = MInstr.getOperand(counter - 1).getReg();
    BuildMI(MBBlock, ++MBBIterator, DLoc, TII.get(XVM::MOV_rr), reg).addReg(XVM::SP);
    if (StackSize + Offset) {
      BuildMI(MBBlock, MBBIterator, DLoc, TII.get(XVM::AddRef_ri), reg).addReg(reg).addImm(StackSize + Offset);
    }
    MInstr.eraseFromParent();
    return;
  }

  int Offset = MFunction.getFrameInfo().getObjectOffset(FrameIndex) +
               MInstr.getOperand(counter + 1).getImm();
  if (!isInt<TEMPLATE_INT_32>(Offset))
    llvm_unreachable("bug in frame offset");

  WarnSize(Offset, MFunction, DLoc);

  if (MInstr.getOpcode() == XVM::FI_ri) {
    // Note: to be tested and modified later
    // architecture does not really support FI_ri, replace it with
    //    MOV_rr <target_reg>, frame_reg
    //    ADD_ri <target_reg>, imm
    Register reg = MInstr.getOperand(counter - 1).getReg();
    BuildMI(MBBlock, ++MBBIterator, DLoc, TII.get(XVM::MOV_rr), reg).addReg(FrameReg);
    BuildMI(MBBlock, MBBIterator, DLoc, TII.get(XVM::AddRef_ri), reg).addReg(reg).addImm(StackSize + Offset);

    // Remove FI_ri instruction
    MInstr.eraseFromParent();
  } else {
    MInstr.getOperand(counter).ChangeToRegister(FrameReg, false);
    MInstr.getOperand(counter + 1).ChangeToImmediate(StackSize + Offset);
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
