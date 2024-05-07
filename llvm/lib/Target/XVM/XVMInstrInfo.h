//===-- XVMInstrInfo.h - XVM Instruction Information ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the XVM implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_XVMINSTRINFO_H
#define LLVM_LIB_TARGET_XVM_XVMINSTRINFO_H

#include "XVMRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "XVMGenInstrInfo.inc"

namespace llvm {

enum CondCode {
  COND_EQ,
  COND_NE,
  COND_GE,
  COND_UGE,
  COND_LE,
  COND_ULE,
  COND_GT,
  COND_UGT,
  COND_LT,
  COND_ULT,
  COND_INVALID
};

class XVMInstrInfo : public XVMGenInstrInfo {
  const XVMRegisterInfo RI;

public:
  XVMInstrInfo();

  const XVMRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;
  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;
  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;
  
  bool isCondBranch(const MachineInstr *MI) const;
  bool isCondBranchProcessed(const MachineInstr *MI) const;
  bool isUnCondBranch(const MachineInstr *MI) const;
  void negateCondBranch(MachineInstr *MI) const;

private:
  void expandMEMCPY(MachineBasicBlock::iterator) const;
  void expandMEMMOVE(MachineBasicBlock::iterator) const;
  void expandMEMSET(MachineBasicBlock::iterator) const;
};
}

#endif
