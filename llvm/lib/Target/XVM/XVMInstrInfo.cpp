//===-- XVMInstrInfo.cpp - XVM Instruction Information ----------*- C++ -*-===//
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

#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVM_def.h"
#include "XVMInstrInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <iterator>

#define GET_INSTRINFO_CTOR_DTOR
#include "XVMGenInstrInfo.inc"

using namespace llvm;
#define NUM_MO_BRANCH_INSTR 3
#define MO_FIRST  0
#define MO_SECOND 1
#define MO_THIRD  2
#define MO_FOURTH 3

#define NUM_OF_BRANCHES 2

static XVM::CondCode getCondFromBranchOpc(unsigned Opc) {
  switch (Opc) {
  default:
    return XVM::COND_INVALID;
  case XVM::BUEQ_rr:
  case XVM::BUEQ_ri:
  case XVM::BSEQ_ri:
    return XVM::COND_EQ;
  case XVM::BSNEQ_rr:
  case XVM::BSNEQ_ri:
    return XVM::COND_NE;
  case XVM::BSGE_rr:
  case XVM::BSGE_ri:
    return XVM::COND_GE;
  case XVM::BUGE_rr:
  case XVM::BUGE_ri:
    return XVM::COND_UGE;
  case XVM::BSLE_rr:
  case XVM::BSLE_ri:
    return XVM::COND_LE;
  case XVM::BULE_rr:
  case XVM::BULE_ri:
    return XVM::COND_ULE;
  case XVM::BSGT_rr:
  case XVM::BSGT_ri:
    return XVM::COND_GT;
  case XVM::BUGT_rr:
  case XVM::BUGT_ri:
    return XVM::COND_UGT;
  case XVM::BSLT_rr:
  case XVM::BSLT_ri:
    return XVM::COND_LT;
  case XVM::BULT_rr:
  case XVM::BULT_ri:
    return XVM::COND_ULT;
  }
}

static unsigned getBranchOpcFromCond(ArrayRef<MachineOperand> &Cond) {
  assert(Cond.size() == NUM_MO_BRANCH_INSTR && "Expected an operation and 2 operands!");
  assert(Cond[MO_FIRST].isImm() && "Expected an imm for operation!");

  switch (Cond[MO_FIRST].getImm()) {
  default:
    //Invalid operation, bail out
    return 0;
  case XVM::COND_EQ:
    return Cond[MO_THIRD].isImm() ? XVM::BSEQ_ri : XVM::BUEQ_rr;
  case XVM::COND_NE:
    return Cond[MO_THIRD].isImm() ? XVM::BSNEQ_ri : XVM::BSNEQ_rr;
  case XVM::COND_GE:
    return Cond[MO_THIRD].isImm() ? XVM::BSGE_ri : XVM::BSGE_rr;
  case XVM::COND_UGE:
    return Cond[MO_THIRD].isImm() ? XVM::BUGE_ri : XVM::BUGE_rr;
  case XVM::COND_LE:
    return Cond[MO_THIRD].isImm() ? XVM::BSLE_ri : XVM::BSLE_rr;
  case XVM::COND_ULE:
    return Cond[MO_THIRD].isImm() ? XVM::BULE_ri : XVM::BULE_rr;
  case XVM::COND_GT:
    return Cond[MO_THIRD].isImm() ? XVM::BSGT_ri : XVM::BSGT_rr;
  case XVM::COND_UGT:
    return Cond[MO_THIRD].isImm() ? XVM::BUGT_ri : XVM::BUGT_rr;
  case XVM::COND_LT:
    return Cond[MO_THIRD].isImm() ? XVM::BSLT_ri : XVM::BSLT_rr;
  case XVM::COND_ULT:
    return Cond[MO_THIRD].isImm() ? XVM::BULT_ri : XVM::BULT_rr;
  }
}

XVMInstrInfo::XVMInstrInfo()
    : XVMGenInstrInfo(XVM::ADJCALLSTACKDOWN, XVM::ADJCALLSTACKUP) {}

void XVMInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {
  if (XVM::XVMGPRRegClass.contains(DestReg, SrcReg) ||
      ((XVM::XVMRRRegClass.contains(SrcReg)) && XVM::XVMGPRRegClass.contains(DestReg)))
    BuildMI(MBB, I, DL, get(XVM::MOV_rr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  else
    llvm_unreachable("To-be-extended: reg-to-reg copy");

    return;
}

static int shiftAndGet16Bits(uint64_t num, int n) {
  return (num >> n) & 0xFFFF;
}

static inline void ReplaceImmWithMovk(MachineBasicBlock *BB,
                                      MachineBasicBlock::iterator MI,
                                      DebugLoc dl) {
    const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
    uint64_t imm = MI->getOperand(MO_THIRD).getImm();
    uint64_t MostSignificantBits = shiftAndGet16Bits(imm, MOST_SIGNIFICANT);
    uint64_t UpperSignificantBits = shiftAndGet16Bits(imm, UPPER_MIDDLE);
    uint64_t LowerSignificantBits = shiftAndGet16Bits(imm, LOWER_MIDDLE);
    uint64_t LeastSignificantBits = shiftAndGet16Bits(imm, LEAST_SIGNIFICANT);
    if (LeastSignificantBits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addReg(XVM::R2).addImm(LeastSignificantBits).addImm(MOVK_SHIFT_0);
    }
    if (LowerSignificantBits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addReg(XVM::R2).addImm(LowerSignificantBits).addImm(MOVK_SHIFT_16);
    }
    if (UpperSignificantBits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addReg(XVM::R2).addImm(UpperSignificantBits).addImm(MOVK_SHIFT_32);
    }
    if (MostSignificantBits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addReg(XVM::R2).addImm(MostSignificantBits).addImm(MOVK_SHIFT_48);
    }
}

void XVMInstrInfo::expandMEMCPY(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memcpy");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(MO_FIRST).getReg();
  Register SrcReg = MI->getOperand(MO_SECOND).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMCPY_ri) {
    ReplaceImmWithMovk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(MO_THIRD).getReg();
    BuildMI(*BB, MI, dl, get(XVM::LDD))
          .addReg(XVM::R2, RegState::Define).addReg(CopyLen).addImm(0);
  }

  BuildMI(*BB, MI, dl, get(XVM::CALL_IMM)).addImm(func_num);
  BB->erase(MI);
}

void XVMInstrInfo::expandMEMMOVE(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memmove");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(MO_FIRST).getReg();
  Register SrcReg = MI->getOperand(MO_SECOND).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMMOV_ri) {
    ReplaceImmWithMovk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(MO_THIRD).getReg();
    BuildMI(*BB, MI, dl, get(XVM::LDD))
          .addReg(XVM::R2, RegState::Define).addReg(CopyLen).addImm(0);
  }

  BuildMI(*BB, MI, dl, get(XVM::CALL_IMM)).addImm(func_num);
  BB->erase(MI);
}

void XVMInstrInfo::expandMEMSET(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memset");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(MO_FIRST).getReg();
  Register SrcReg = MI->getOperand(MO_SECOND).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMSET_ri) {
    ReplaceImmWithMovk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(MO_THIRD).getReg();
    BuildMI(*BB, MI, dl, get(XVM::LDD))
          .addReg(XVM::R2, RegState::Define).addReg(CopyLen).addImm(0);
  }

  BuildMI(*BB, MI, dl, get(XVM::CALL_IMM)).addImm(func_num);
  BB->erase(MI);
}


bool XVMInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
    default:
      return false;
    case XVM::MEMCPY_ri:
    case XVM::MEMCPY_rr:
      expandMEMCPY(MI);
      return true;
    case XVM::MEMMOV_ri:
    case XVM::MEMMOV_rr:
      expandMEMMOVE(MI);
      return true;
    case XVM::MEMSET_ri:
    case XVM::MEMSET_rr:
      expandMEMSET(MI);
      return true;
  }

  return false;
}

void XVMInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I,
                                       Register SrcReg, bool IsKill, int FI,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &XVM::XVMGPRRegClass)
    BuildMI(MBB, I, DL, get(XVM::STD))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0);
  else
    llvm_unreachable("Can't store this register to stack slot");
  return;
}

void XVMInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        Register DestReg, int FI,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RC == &XVM::XVMGPRRegClass)
    BuildMI(MBB, I, DL, get(XVM::LDD), DestReg).addFrameIndex(FI).addImm(0);
  else
    llvm_unreachable("Can't load this register from stack slot");
  return;
}

bool XVMInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  //Note: consider the case when the CFG is stackified later.
  // Maybe need to implement removeBranch and insertBranch

  bool HaveCond = false;
  for (MachineInstr &MI : MBB.terminators()) {
    switch (MI.getOpcode()) {
    default:
      // Unhandled instruction; bail out.
      return true;
    case XVM::BR      :
      if (!HaveCond)
        TBB = MI.getOperand(MO_FIRST).getMBB();
      else
        FBB = MI.getOperand(MO_FIRST).getMBB();
      break;
    case XVM::BUEQ_rr  :
    case XVM::BSNEQ_rr :
    case XVM::BSGE_rr  :
    case XVM::BUGE_rr  :
    case XVM::BSLE_rr  :
    case XVM::BULE_rr  :
    case XVM::BSGT_rr  :
    case XVM::BUGT_rr  :
    case XVM::BSLT_rr  :
    case XVM::BULT_rr  :
    case XVM::BSEQ_ri  :
    case XVM::BUEQ_ri  :
    case XVM::BSNEQ_ri :
    case XVM::BUNEQ_ri :
    case XVM::BSGE_ri  :
    case XVM::BUGE_ri  :
    case XVM::BSLE_ri  :
    case XVM::BULE_ri  :
    case XVM::BSGT_ri  :
    case XVM::BUGT_ri  :
    case XVM::BSLT_ri  :
    case XVM::BULT_ri  :
      if (HaveCond)
        return true;
      XVM::CondCode CC = getCondFromBranchOpc(MI.getOpcode());
      Cond.push_back(MachineOperand::CreateImm(CC));
      Cond.push_back(MI.getOperand(MO_SECOND));
      Cond.push_back(MI.getOperand(MO_THIRD));
      TBB = MI.getOperand(MO_FIRST).getMBB();
      HaveCond = true;
      break;
    }
  }

  return false;
}

unsigned XVMInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL,
                                    int *BytesAdded) const {
  assert(!BytesAdded && "code size not handled");

  if (Cond.empty()) {
    if (!TBB)
      return 0;

    BuildMI(&MBB, DL, get(XVM::BR)).addMBB(TBB);
    return 1;
  }

  assert(Cond.size() == NUM_MO_BRANCH_INSTR && "Expected 2 operands and an operation!");

  BuildMI(&MBB, DL, get(getBranchOpcFromCond(Cond))).addMBB(TBB)
                                                    .add(Cond[MO_SECOND])
                                                    .add(Cond[MO_THIRD]);
  if (!FBB)
    return 1;
  BuildMI(&MBB, DL, get(XVM::BR)).addMBB(FBB);
  return NUM_OF_BRANCHES;
}

unsigned XVMInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
  assert(!BytesRemoved && "code size not handled");

  MachineBasicBlock::instr_iterator I = MBB.instr_end();
  unsigned Count = 0;

  while (I != MBB.instr_begin()) {
    --I;
    if (I->isDebugInstr())
      continue;
    if (!I->isTerminator())
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.instr_end();
    ++Count;
  }

  return Count;
}

bool XVMInstrInfo::isCondBranchProcessed(const MachineInstr *MI) const{
  switch (MI->getOpcode()) {
  case XVM::LOOP_BUEQ_rr  :
  case XVM::LOOP_BSNEQ_rr :
  case XVM::LOOP_BSGE_rr  :
  case XVM::LOOP_BUGE_rr  :
  case XVM::LOOP_BSLE_rr  :
  case XVM::LOOP_BULE_rr  :
  case XVM::LOOP_BSGT_rr  :
  case XVM::LOOP_BUGT_rr  :
  case XVM::LOOP_BSLT_rr  :
  case XVM::LOOP_BULT_rr  :
  case XVM::LOOP_BSEQ_ri  :
  case XVM::LOOP_BUEQ_ri  :
  case XVM::LOOP_BSNEQ_ri :
  case XVM::LOOP_BUNEQ_ri :
  case XVM::LOOP_BSGE_ri  :
  case XVM::LOOP_BUGE_ri  :
  case XVM::LOOP_BSLE_ri  :
  case XVM::LOOP_BULE_ri  :
  case XVM::LOOP_BSGT_ri  :
  case XVM::LOOP_BUGT_ri  :
  case XVM::LOOP_BSLT_ri  :
  case XVM::LOOP_BULT_ri  :
    return true;
  default      :
    return false;
  }
}
bool XVMInstrInfo::isCondBranch(const MachineInstr *MI) const {
  switch (MI->getOpcode()) {
  case XVM::BUEQ_rr  :
  case XVM::BSNEQ_rr :
  case XVM::BSGE_rr  :
  case XVM::BUGE_rr  :
  case XVM::BSLE_rr  :
  case XVM::BULE_rr  :
  case XVM::BSGT_rr  :
  case XVM::BUGT_rr  :
  case XVM::BSLT_rr  :
  case XVM::BULT_rr  :
  case XVM::BSEQ_ri  :
  case XVM::BUEQ_ri  :
  case XVM::BSNEQ_ri :
  case XVM::BUNEQ_ri :
  case XVM::BSGE_ri  :
  case XVM::BUGE_ri  :
  case XVM::BSLE_ri  :
  case XVM::BULE_ri  :
  case XVM::BSGT_ri  :
  case XVM::BUGT_ri  :
  case XVM::BSLT_ri  :
  case XVM::BULT_ri  :
    return true;
  default      :
    return false;
  }
  return false;
}

bool XVMInstrInfo::isUnCondBranch(const MachineInstr *MI) const{
  return MI->getOpcode() == XVM::BR;
}

void XVMInstrInfo::negateCondBranch(MachineInstr *MI) const{
  assert(isCondBranch(MI));
  switch (MI->getOpcode()) {
    case XVM::BUEQ_rr  : MI->setDesc(get(XVM::BSNEQ_rr)); break;
    case XVM::BSNEQ_rr : MI->setDesc(get(XVM::BUEQ_rr)); break;
    case XVM::BSGE_rr  : MI->setDesc(get(XVM::BSLT_rr)); break;
    case XVM::BUGE_rr  : MI->setDesc(get(XVM::BULT_rr)); break;
    case XVM::BSLE_rr  : MI->setDesc(get(XVM::BSGT_rr)); break;
    case XVM::BULE_rr  : MI->setDesc(get(XVM::BUGT_rr)); break;
    case XVM::BSGT_rr  : MI->setDesc(get(XVM::BSLE_rr)); break;
    case XVM::BUGT_rr  : MI->setDesc(get(XVM::BULE_rr)); break;
    case XVM::BSLT_rr  : MI->setDesc(get(XVM::BSGE_rr)); break;
    case XVM::BULT_rr  : MI->setDesc(get(XVM::BUGE_rr)); break;
    case XVM::BSEQ_ri  : MI->setDesc(get(XVM::BSNEQ_ri)); break;
    case XVM::BUEQ_ri  : MI->setDesc(get(XVM::BUNEQ_ri)); break;
    case XVM::BSNEQ_ri : MI->setDesc(get(XVM::BSEQ_ri)); break;
    case XVM::BUNEQ_ri : MI->setDesc(get(XVM::BUEQ_ri)); break;
    case XVM::BSGE_ri  : MI->setDesc(get(XVM::BSLT_ri)); break;
    case XVM::BUGE_ri  : MI->setDesc(get(XVM::BULT_ri)); break;
    case XVM::BSLE_ri  : MI->setDesc(get(XVM::BSGT_ri)); break;
    case XVM::BULE_ri  : MI->setDesc(get(XVM::BUGT_ri)); break;
    case XVM::BSGT_ri  : MI->setDesc(get(XVM::BSLE_ri)); break;
    case XVM::BUGT_ri  : MI->setDesc(get(XVM::BULE_ri)); break;
    case XVM::BSLT_ri  : MI->setDesc(get(XVM::BSGE_ri)); break;
    case XVM::BULT_ri  : MI->setDesc(get(XVM::BUGE_ri)); break;
  default      :
    llvm_unreachable("Unknown Branch Opcode Existing in isCondBranch but missing in negateCondBranch");
  }
}

#endif
