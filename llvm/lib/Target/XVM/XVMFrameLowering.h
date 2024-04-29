//===-- XVMFrameLowering.h - Define frame lowering for XVM -----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements XVM-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_XVMFRAMELOWERING_H
#define LLVM_LIB_TARGET_XVM_XVMFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class XVMSubtarget;

class XVMFrameLowering : public TargetFrameLowering {
public:
  explicit XVMFrameLowering(const XVMSubtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override {
    return MBB.erase(MI);
  }

private:
  bool needsSP(const MachineFunction &MF) const;
  bool needsSPForLocalFrame(const MachineFunction &MF) const;
  static unsigned getSPReg(const MachineFunction &MF);
  static unsigned getOpcSubRef(const MachineFunction &MF);
  static unsigned getOpcAddRef(const MachineFunction &MF);
  static unsigned getOpcGlobSet(const MachineFunction &MF);
};
}
#endif
