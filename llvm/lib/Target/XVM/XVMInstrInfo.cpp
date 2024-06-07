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

static CondCode getCondFromBranchOpc(unsigned Opc) {
  switch (Opc) {
  default:
    return COND_INVALID;
  case XVM::BUEQ_rr:
  case XVM::BUEQ_ri:
  case XVM::BSEQ_ri:
    return COND_EQ;
  case XVM::BSNEQ_rr:
  case XVM::BSNEQ_ri:
    return COND_NE;
  case XVM::BSGE_rr:
  case XVM::BSGE_ri:
    return COND_GE;
  case XVM::BUGE_rr:
  case XVM::BUGE_ri:
    return COND_UGE;
  case XVM::BSLE_rr:
  case XVM::BSLE_ri:
    return COND_LE;
  case XVM::BULE_rr:
  case XVM::BULE_ri:
    return COND_ULE;
  case XVM::BSGT_rr:
  case XVM::BSGT_ri:
    return COND_GT;
  case XVM::BUGT_rr:
  case XVM::BUGT_ri:
    return COND_UGT;
  case XVM::BSLT_rr:
  case XVM::BSLT_ri:
    return COND_LT;
  case XVM::BULT_rr:
  case XVM::BULT_ri:
    return COND_ULT;
  }
}

static unsigned getBranchOpcFromCond(ArrayRef<MachineOperand> &Cond) {
  assert(Cond.size() == 3 && "Expected an operation and 2 operands!");
  assert(Cond[0].isImm() && "Expected an imm for operation!");

  switch (Cond[0].getImm()) {
  default:
    //Invalid operation, bail out
    return 0;
  case COND_EQ:
    return Cond[2].isImm() ? XVM::BSEQ_ri : XVM::BUEQ_rr;
  case COND_NE:
    return Cond[2].isImm() ? XVM::BSNEQ_ri : XVM::BSNEQ_rr;
  case COND_GE:
    return Cond[2].isImm() ? XVM::BSGE_ri : XVM::BSGE_rr;
  case COND_UGE:
    return Cond[2].isImm() ? XVM::BUGE_ri : XVM::BUGE_rr;
  case COND_LE:
    return Cond[2].isImm() ? XVM::BSLE_ri : XVM::BSLE_rr;
  case COND_ULE:
    return Cond[2].isImm() ? XVM::BULE_ri : XVM::BULE_rr;
  case COND_GT:
    return Cond[2].isImm() ? XVM::BSGT_ri : XVM::BSGT_rr;
  case COND_UGT:
    return Cond[2].isImm() ? XVM::BUGT_ri : XVM::BUGT_rr;
  case COND_LT:
    return Cond[2].isImm() ? XVM::BSLT_ri : XVM::BSLT_rr;
  case COND_ULT:
    return Cond[2].isImm() ? XVM::BULT_ri : XVM::BULT_rr;
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

static int ShiftAndGet16Bits(uint64_t num, int n) {
  return (num >> n) & 0xFFFF;
}

static inline void replace_imm_with_movk(MachineBasicBlock *BB,
				                                 MachineBasicBlock::iterator MI,
				                                 DebugLoc dl)
{
    const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
    uint64_t imm = MI->getOperand(2).getImm();
    uint64_t most_significant_bits = ShiftAndGet16Bits(imm, 48);
    uint64_t upper_significant_bits = ShiftAndGet16Bits(imm, 32);
    uint64_t lower_significant_bits = ShiftAndGet16Bits(imm, 16);
    uint64_t least_significant_bits = ShiftAndGet16Bits(imm, 0);
    if (least_significant_bits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addImm(0).addImm(least_significant_bits).addImm(0);
    }
    if (lower_significant_bits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addImm(0).addImm(lower_significant_bits).addImm(1);
    }
    if (upper_significant_bits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addImm(0).addImm(upper_significant_bits).addImm(2);
    }
    if (most_significant_bits) {
      BuildMI(*BB, MI, dl, TII.get(XVM::MOVK_ri))
        .addReg(XVM::R2, RegState::Define).addImm(0).addImm(most_significant_bits).addImm(3);
    }
}

void XVMInstrInfo::expandMEMCPY(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memcpy");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(0).getReg();
  Register SrcReg = MI->getOperand(1).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMCPY_ri) {
    replace_imm_with_movk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(2).getReg();
    BuildMI(*BB, MI, dl, get(XVM::LDD))
          .addReg(XVM::R2, RegState::Define).addReg(CopyLen).addImm(0);
  }

  BuildMI(*BB, MI, dl, get(XVM::CALL_IMM)).addImm(func_num);
  BB->erase(MI);
}

void XVMInstrInfo::expandMEMMOVE(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memmove");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(0).getReg();
  Register SrcReg = MI->getOperand(1).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMMOV_ri) {
    replace_imm_with_movk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(2).getReg();
    BuildMI(*BB, MI, dl, get(XVM::LDD))
          .addReg(XVM::R2, RegState::Define).addReg(CopyLen).addImm(0);
  }

  BuildMI(*BB, MI, dl, get(XVM::CALL_IMM)).addImm(func_num);
  BB->erase(MI);
}

void XVMInstrInfo::expandMEMSET(MachineBasicBlock::iterator MI) const {
  int func_num = GetFuncIndex("memset");
  MachineBasicBlock *BB = MI->getParent();
  Register DstReg = MI->getOperand(0).getReg();
  Register SrcReg = MI->getOperand(1).getReg();
  DebugLoc dl = MI->getDebugLoc();

  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R0, RegState::Define).addReg(DstReg).addImm(0);
  BuildMI(*BB, MI, dl, get(XVM::LDD))
        .addReg(XVM::R1, RegState::Define).addReg(SrcReg).addImm(0);
  if (MI->getOpcode() == XVM::MEMSET_ri) {
    replace_imm_with_movk(BB, MI, dl);
  } else {
    Register CopyLen = MI->getOperand(2).getReg();
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
        TBB = MI.getOperand(0).getMBB();
      else
        FBB = MI.getOperand(0).getMBB();
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
      CondCode CC = getCondFromBranchOpc(MI.getOpcode());
      Cond.push_back(MachineOperand::CreateImm(CC));
      Cond.push_back(MI.getOperand(1));
      Cond.push_back(MI.getOperand(2));
      TBB = MI.getOperand(0).getMBB();
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

  assert(Cond.size() == 3 && "Expected 2 operands and an operation!");

  BuildMI(&MBB, DL, get(getBranchOpcFromCond(Cond))).addMBB(TBB)
                                                    .add(Cond[1])
                                                    .add(Cond[2]);
  if (!FBB)
    return 1;
  BuildMI(&MBB, DL, get(XVM::BR)).addMBB(FBB);
  return 2;
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
