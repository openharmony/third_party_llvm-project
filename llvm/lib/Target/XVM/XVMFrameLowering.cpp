//===-- XVMFrameLowering.cpp - XVM Frame Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the XVM implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVM_def.h"
#include "XVM.h"
#include "XVMFrameLowering.h"
#include "XVMInstrInfo.h"
#include "XVMSubtarget.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

bool XVMFrameLowering::hasFP(const MachineFunction &MF) const { return false; }

bool XVMFrameLowering::needsSPForLocalFrame(
    const MachineFunction &MF) const {
  auto &MFI = MF.getFrameInfo();
  return MFI.getStackSize() || MFI.adjustsStack() || hasFP(MF);
}

bool XVMFrameLowering::needsSP(const MachineFunction &MF) const {
  return needsSPForLocalFrame(MF);
}

unsigned XVMFrameLowering::getSPReg(const MachineFunction &MF) {
  return XVM::SP;
}

unsigned XVMFrameLowering::getOpcSubRef(const MachineFunction &MF) {
  return XVM::SubRef_ri;
}

unsigned XVMFrameLowering::getOpcAddRef(const MachineFunction &MF) {
  return XVM::AddRef_ri;
}

unsigned
XVMFrameLowering::getOpcGlobSet(const MachineFunction &MF) {
  return XVM::G_GLOBAL_VALUE;
}

void XVMFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const
{
  auto &MFI = MF.getFrameInfo();
  if (!needsSP(MF)) {
    return;
  }
  uint64_t StackSize = MFI.getStackSize();
  auto &ST = MF.getSubtarget<XVMSubtarget>();
  const auto *TII = ST.getInstrInfo();
  auto &MRI = MF.getRegInfo();
  auto InsertPt = MBB.begin();
  DebugLoc DL;

  unsigned SPReg = getSPReg(MF);

  if (StackSize > 0) {
    /* prepare stack space for the function to use */
    BuildMI(MBB, InsertPt, DL, TII->get(XVM::SubRef_ri), SPReg).addReg(SPReg).addImm(StackSize);
  }
}

void XVMFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const
{
  uint64_t StackSize = MF.getFrameInfo().getStackSize();
  if (!needsSP(MF)) {
    return;
  }
  auto &ST = MF.getSubtarget<XVMSubtarget>();
  const auto *TII = ST.getInstrInfo();
  auto &MRI = MF.getRegInfo();
  auto InsertPt = MBB.getFirstTerminator();
  DebugLoc DL;

  if (InsertPt != MBB.end())
    DL = InsertPt->getDebugLoc();
  unsigned SPReg = getSPReg(MF);

  if (StackSize > 0) {
    BuildMI(MBB, InsertPt, DL, TII->get(XVM::AddRef_ri), SPReg).addReg(SPReg).addImm(StackSize);
  }
}

void XVMFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const
{
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
}

#endif
