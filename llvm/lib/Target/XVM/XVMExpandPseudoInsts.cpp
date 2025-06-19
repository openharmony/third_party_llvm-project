//===-- XVMExpandPseudoInsts.cpp - XVM Pseudo Instruction Expander ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------------------===//
//
// This file contains the XVM implementation of replacing certain Pseudoinstructions.
//
//===----------------------------------------------------------------------------------===//

#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVMInstrInfo.h"
#include "XVMTargetMachine.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define XVM_EXPAND_PSEUDO_NAME "XVM pseudo instruction expansion pass"
#define MO_FIRST  0
#define MO_SECOND 1
#define MO_THIRD  2

namespace {
class XVMExpandPseudo : public MachineFunctionPass {
public:
  static char ID;
  XVMExpandPseudo() : MachineFunctionPass(ID) {
    initializeXVMExpandPseudoPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return XVM_EXPAND_PSEUDO_NAME; }
private:
  bool expandMBB(MachineBasicBlock &MBB);
  bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                  MachineBasicBlock::iterator &NextMBB);
  bool expandSelectCC(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                          MachineBasicBlock::iterator &NextMBBI);
};
  char XVMExpandPseudo::ID = 0;
}

bool XVMExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;
  for (auto &MBB : MF)
    Modified |= expandMBB(MBB);
  return Modified;
}

bool XVMExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin();
  int InstNumber = std::distance(MBB.begin(), MBB.end());
  for (int i = 0; i < InstNumber; i++) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }
  return Modified;
}

bool XVMExpandPseudo::expandMI(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MBBI,
                               MachineBasicBlock::iterator &NextMBBI) {
  // XVMInstrInfo::getInstSizeInBytes expects that the total size of the
  // expanded instructions for each pseudo is correct in the Size field of the
  // tablegen definition for the pseudo.
  switch (MBBI->getOpcode()) {
  default:
    return false;
  case XVM::PseudoSelectCC_ri:
  case XVM::PseudoSelectCC_rr:
    return expandSelectCC(MBB, MBBI, NextMBBI);
  }
}

static unsigned getBranchOpcodeFromSelectCC(MachineInstr &MI) {
  assert((MI.getOpcode() == XVM::PseudoSelectCC_ri ||
          MI.getOpcode() == XVM::PseudoSelectCC_rr) &&
          "The instruction should be a pseudo select cc!");
  bool IsRROp = MI.getOpcode() == XVM::PseudoSelectCC_rr;
  unsigned Cond = MI.getOperand(3).getImm();
  unsigned NewCond;
  switch (Cond) {
#define SET_NEWCC(X, Y) \
  case ISD::X: \
    NewCond = IsRROp ? XVM::Y##_rr : XVM::Y##_ri; \
    break
  SET_NEWCC(SETGT, BSGT);
  SET_NEWCC(SETUGT, BUGT);
  SET_NEWCC(SETGE, BSGE);
  SET_NEWCC(SETUGE, BUGE);
  SET_NEWCC(SETEQ, BUEQ);
  SET_NEWCC(SETNE, BSNEQ);
  SET_NEWCC(SETLT, BSLT);
  SET_NEWCC(SETULT, BULT);
  SET_NEWCC(SETLE, BSLE);
  SET_NEWCC(SETULE, BULE);
  default:
      report_fatal_error("unimplemented select CondCode " + Twine(Cond));
  }
  return NewCond;
}

bool XVMExpandPseudo::expandSelectCC(MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator MBBI,
                                     MachineBasicBlock::iterator &NextMBBI) {
  MachineInstr &MI = *MBBI;
  const TargetInstrInfo *TII = MBB.getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB.getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();

  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  MachineFunction::iterator I = ++MBB.getIterator();

  MachineBasicBlock *TMBB = MF->CreateMachineBasicBlock();
  MachineBasicBlock *FMBB = MF->CreateMachineBasicBlock();
  MachineBasicBlock *SuccMBB = MF->CreateMachineBasicBlock();

  MF->insert(I, FMBB);
  MF->insert(I, TMBB);
  MF->insert(I, SuccMBB);

  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  SuccMBB->splice(SuccMBB->begin(), &MBB, NextMBBI, MBB.end());
  SuccMBB->transferSuccessorsAndUpdatePHIs(&MBB);

  //Construct the conditional branch in MBB
  unsigned NewCC = getBranchOpcodeFromSelectCC(MI);

  BuildMI(MBB, MBBI, DL, TII->get(NewCC)).addMBB(TMBB)
                                         .add(MI.getOperand(MO_SECOND))
                                         .add(MI.getOperand(MO_THIRD));
  BuildMI(MBB, MBBI, DL, TII->get(XVM::BR)).addMBB(FMBB);
  MBB.addSuccessor(TMBB);
  MBB.addSuccessor(FMBB);

  //Next, add the move instruction in FMBB and TMBB
  MachineOperand Tval = MI.getOperand(4);
  MachineOperand Fval = MI.getOperand(5);

  assert (Tval.isReg() && Fval.isReg() &&  "value should be reg!");

  MachineBasicBlock::iterator FMBBInsertPos = FMBB->begin();
  MachineBasicBlock::iterator TMBBInsertPos = TMBB->begin();

  Register FReg = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
  Register TReg = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);

  BuildMI(*FMBB, FMBBInsertPos, FMBB->findDebugLoc(FMBBInsertPos),
          TII->get(XVM::MOV_rr), FReg).add(Fval);
  BuildMI(*FMBB, FMBBInsertPos,
          FMBB->findDebugLoc(FMBBInsertPos),
          TII->get(XVM::BR)).addMBB(SuccMBB);
  FMBB->addSuccessor(SuccMBB);

  BuildMI(*TMBB, TMBBInsertPos, TMBB->findDebugLoc(TMBBInsertPos),
          TII->get(XVM::MOV_rr), TReg).add(Tval);
  BuildMI(*TMBB, TMBBInsertPos,
          TMBB->findDebugLoc(TMBBInsertPos),
          TII->get(XVM::BR)).addMBB(SuccMBB);
  TMBB->addSuccessor(SuccMBB);

  //Last, we add the PHI instruction in SuccMBB
  MachineBasicBlock::iterator PHIInsertPos = SuccMBB->begin();
  BuildMI(*SuccMBB, PHIInsertPos, SuccMBB->findDebugLoc(PHIInsertPos),
          TII->get(XVM::PHI), MI.getOperand(0).getReg()).addReg(FReg)
                                                        .addMBB(FMBB)
                                                        .addReg(TReg)
                                                        .addMBB(TMBB);
  MBBI->eraseFromParent();
  return true;
}

INITIALIZE_PASS(XVMExpandPseudo, "xvm-expand-pseudo",
                XVM_EXPAND_PSEUDO_NAME, false, false)
namespace llvm {
  FunctionPass *createXVMExpandPseudoPass() { return new XVMExpandPseudo(); }
}

#endif
