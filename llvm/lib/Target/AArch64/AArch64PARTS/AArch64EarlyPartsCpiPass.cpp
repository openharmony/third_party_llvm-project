//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AArch64RegisterInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "AArch64EarlyPartsCpiPass.h"

STATISTIC(StatAutcall, DEBUG_TYPE ": inserted authenticate and branch instructions");

using namespace llvm;
using namespace llvm::PARTS;

INITIALIZE_PASS(AArch64EarlyPartsCpiPass, "aarch64-early-parts-cpi-pass",
                "AArch64 Early Parts Cpi", false, false)

FunctionPass *llvm::createAArch64EarlyPartsCpiPass() {
    return new AArch64EarlyPartsCpiPass();
}

char AArch64EarlyPartsCpiPass::ID = 0;
bool AArch64EarlyPartsCpiPass::doInitialization(Module &M) {
    return true;
}

bool AArch64EarlyPartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
    bool found = false;
    STI = &MF.getSubtarget<AArch64Subtarget>();
    TII = STI->getInstrInfo();

    for (auto &MBB : MF) {
        for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi) {
            found= handleInstruction(MF, MBB, MIi) || found;
        }
    }
    return found;
}

/**
* @param MF
* @param MBB
* @param MIi
* @return return true when changing something, otherwise false
**/
inline bool AArch64EarlyPartsCpiPass::handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi) {
    const auto MIOpcode = MIi->getOpcode();
    if (!isPartsAUTCALLIntrinsic(MIOpcode)) {
        return false;
    }
    auto MIptr = &*MIi;
    SmallVector<MachineInstr*> IndCallVec;
    findIndirectCallMachineInstr(MF, MBB, MIptr, IndCallVec);
    if (IndCallVec.empty()) {
        outs() << "MI_indcall is NULL!!!\n";
        triggerCompilationErrorOrphanAUTCALL(MBB);
    }
    auto &MI = *MIi--;
    for (auto &MI_indcall : IndCallVec) {
        // processed items will not be processed again.
        if (isIndirectAutCall(*MI_indcall)) {
            continue;
        }
        MachineRegisterInfo *MRI = &MF.getRegInfo();
        MachineInstr *PhiMi = MRI->getVRegDef(MI_indcall->getOperand(0).getReg());
        if (PhiMi->isPHI()) {
            SmallVector<MachineInstr*> AutCallVec;
            // When some branches of the phi instruction are optimized to 
            // constant function addresses, no signature is generated, 
            // resulting in the inability to generate blraa instructions
            // after the phi instruction.
            if (!getAllAutCalls(PhiMi, MRI, AutCallVec)) {
                for (auto &AutCall : AutCallVec) {
                    // Remove variables that are not optimized to 
                    // constant function addresses, 
                    // verify and remove signatures with autia.
                    insertAuthenticateInstr(MRI, AutCall, PhiMi);
                    ++StatAutcall;
                }
                continue;
            }
        }
        replaceBranchByAuthenticatedBranch(MBB, MI_indcall, MI);
    }
    // insert a copy instruction, same dest register as MI, keep subsequent instructions do not need to modified
    insertCOPYInstr(MBB, &MI, MI);
    MI.removeFromParent();
    ++StatAutcall;
    return true;
}

inline bool AArch64EarlyPartsCpiPass::getAllAutCalls(
    MachineInstr *PhiMi, MachineRegisterInfo *MRI, SmallVector<MachineInstr *> &AutCallVec) const {
    bool ret = true;
    for (unsigned i = 1, e = PhiMi->getNumOperands(); i < e; i += 2) {
        MachineOperand &MO = PhiMi->getOperand(i);
        MachineInstr *CopyMi = MRI->getVRegDef(MO.getReg());
        MachineInstr *AutCall = CopyMi->isCopy() ? MRI->getVRegDef(CopyMi->getOperand(1).getReg()) : CopyMi;
        if (!isPartsAUTCALLIntrinsic(AutCall->getOpcode())) {
            ret = false;
            continue;
        }

        AutCallVec.push_back(AutCall);
    }
    return ret;
}

inline void AArch64EarlyPartsCpiPass::insertAuthenticateInstr(
    MachineRegisterInfo *MRI, MachineInstr *AutCall, MachineInstr *PhiMi) {
    unsigned NewDestReg = MRI->createVirtualRegister(&AArch64::GPR64spRegClass);
    auto BMI = BuildMI(
        *AutCall->getParent(), *AutCall, AutCall->getDebugLoc(), TII->get(AArch64::AUTIA));
    BMI.addDef(NewDestReg);
    auto &Op1 = AutCall->getOperand(1);
    BMI.addReg(Op1.getReg(), getRegState(Op1) & ~RegState::Kill);
    auto &Op2 = AutCall->getOperand(2);
    BMI.addReg(Op2.getReg(), getRegState(Op2) & ~RegState::Kill);
    // Correct the register referenced by the phi instruction
    for (unsigned i = 1; i < PhiMi->getNumOperands(); i += 2) {
        MachineOperand &MO = PhiMi->getOperand(i);
        MachineInstr *CopyMi = MRI->getVRegDef(MO.getReg());
        MachineInstr *Tmp = CopyMi->isCopy() ? MRI->getVRegDef(CopyMi->getOperand(1).getReg()) : CopyMi;
        if (MO.isReg() && AutCall->getOperand(0).getReg() == Tmp->getOperand(0).getReg()) {
            MO.setReg(NewDestReg);
        }
    }
}

inline bool AArch64EarlyPartsCpiPass::isPartsAUTCALLIntrinsic(unsigned Opcode) const {
    switch (Opcode) {
        case AArch64::PARTS_AUTCALL:
            return true;
    }
    return false;
}

inline const MCInstrDesc& AArch64EarlyPartsCpiPass::getIndirectCallAuth(MachineInstr *MI_indcall) {
    if (MI_indcall->getOpcode() == AArch64::BLR) {
        return TII->get(AArch64::BLRAA);
    }
    // This is a tail call return, and we need to use BRAA
    // (tail-call: ~optimation where a tail-call is coverted to a direct call so that
    // the tail-called function can return immediately to the current callee, without
    // going through the currently active function.)

    return TII->get(AArch64::TCRETURNriAA);
}

inline bool AArch64EarlyPartsCpiPass::handlePhi(MachineFunction &MF, MachineInstr *MIptr, unsigned AutCall) {
    MachineRegisterInfo *MRI = &MF.getRegInfo();
    MachineInstr *PhiMi = MRI->getVRegDef(MIptr->getOperand(0).getReg());
    if (!PhiMi->isPHI()) {
        return false;
    }
    for (unsigned i = 1, e = PhiMi->getNumOperands(); i < e; i += 2) {
        MachineOperand &Opnd = PhiMi->getOperand(i);
        // An incomming value of phi is the return value of autcall
        if (Opnd.getReg() == AutCall) {
            return true;
        }
        // An incomming value of phi is a copy of autcall return value
        MachineInstr *CopyMi = MRI->getVRegDef(Opnd.getReg());
        if (CopyMi->isCopy() && (CopyMi->getOperand(1).getReg() == AutCall)) {
            return true;
        }
    }
    return false;
}

inline void AArch64EarlyPartsCpiPass::findIndirectCallMachineInstr(MachineFunction &MF,
    MachineBasicBlock &MBB, MachineInstr *MIptr, SmallVector<MachineInstr*> &IndCallVec) {
    unsigned AUTCALLinstr_oper0 = MIptr->getOperand(0).getReg();
    unsigned BLRinstr_oper0 = 0;
    MachineRegisterInfo *MRI = &MF.getRegInfo();
    for (auto &MBBTmp : MF) {
        for (auto MIi = MBBTmp.instr_begin(), MIie = MBBTmp.instr_end(); MIi != MIie; ++MIi) {
            if (isIndirectCall(*MIi)) {
                BLRinstr_oper0 = MIi->getOperand(0).getReg();
                if (AUTCALLinstr_oper0 == BLRinstr_oper0) {
                    IndCallVec.push_back(&*MIi);
                    continue;
                }
                MachineInstr *CopyMi = MRI->getVRegDef(BLRinstr_oper0);
                if (CopyMi->isCopy() && (CopyMi->getOperand(1).getReg() == AUTCALLinstr_oper0)) {
                    IndCallVec.push_back(&*MIi);
                    continue;
                }
                if (handlePhi(MF, &*MIi, AUTCALLinstr_oper0)) {
                    IndCallVec.push_back(&*MIi);
                    continue;
                }
            } else if (isIndirectAutCall(*MIi)) {
                BLRinstr_oper0 = MIi->getOperand(0).getReg();
                if (AUTCALLinstr_oper0 == BLRinstr_oper0) {
                    IndCallVec.push_back(&*MIi);
                    continue;
                }
                if (handlePhi(MF, &*MIi, AUTCALLinstr_oper0)) {
                    IndCallVec.push_back(&*MIi);
                    continue;
                }
            }
        }
    }
    return;
}

inline bool AArch64EarlyPartsCpiPass::isIndirectCall(const MachineInstr &MI) const {
    switch(MI.getOpcode()) {
        case AArch64::BLR:  // Normal indirect call
        case AArch64::TCRETURNri: // Indirect tail call
            return true;
    }
    return false;
}

inline bool AArch64EarlyPartsCpiPass::isIndirectAutCall(const MachineInstr &MI) const {
    switch (MI.getOpcode()) {
        case AArch64::BLRAA: // Normal indirect call
        case AArch64::TCRETURNriAA: // Indirect tail call
            return true;
    }
    return false;
}

void AArch64EarlyPartsCpiPass::triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB) {
    LLVM_DEBUG(MBB.dump());
    llvm_unreachable("failed to find BLR for AUTCALL");
}

inline void AArch64EarlyPartsCpiPass::addPhiForModifier(MachineInstr *Indirect, Register *ModReg) {
    MachineRegisterInfo *MRI = &Indirect->getParent()->getParent()->getRegInfo();
    MachineInstr *PhiMi = MRI->getVRegDef(Indirect->getOperand(0).getReg());
    if (!PhiMi->isPHI()) {
        return;
    }
    *ModReg = MRI->createVirtualRegister(&AArch64::GPR64spRegClass);
    auto BMI = BuildMI(*PhiMi->getParent(), *PhiMi, PhiMi->getDebugLoc(), TII->get(AArch64::PHI));
    BMI.addDef(*ModReg);
    for (unsigned i = 1, e = PhiMi->getNumOperands(); i < e; i += 2) {
        MachineOperand &Opnd = PhiMi->getOperand(i);
        MachineInstr *CopyMi = MRI->getVRegDef(Opnd.getReg());
        MachineInstr *AutCall = CopyMi->isCopy() ? MRI->getVRegDef(CopyMi->getOperand(1).getReg()) : CopyMi;
        BMI.addReg(AutCall->getOperand(2).getReg());
        BMI.addMBB(AutCall->getParent());
    }
    return;
}

inline void AArch64EarlyPartsCpiPass::replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
    MachineInstr *MI_indcall, MachineInstr &MI) {

    if (isIndirectAutCall(*MI_indcall)) {
        return;
    }
    Register ModReg = MI.getOperand(2).getReg();

    addPhiForModifier(MI_indcall, &ModReg);
    if (MI_indcall->getOpcode() == AArch64::TCRETURNri) {
        MachineRegisterInfo *MRI = &MI_indcall->getMF()->getRegInfo();
        Register TcModReg = MRI->createVirtualRegister(&AArch64::tcGPR64RegClass);
        auto CopyMi = BuildMI(*MI_indcall->getParent(), *MI_indcall, MI_indcall->getDebugLoc(),
            TII->get(AArch64::COPY), TcModReg);
        CopyMi.addUse(ModReg);
        ModReg = TcModReg;
    }

    auto BMI = BuildMI(*MI_indcall->getParent(), *MI_indcall, MI_indcall->getDebugLoc(), getIndirectCallAuth(MI_indcall));
    BMI.addUse(MI_indcall->getOperand(0).getReg());
    if (MI_indcall->getOpcode() == AArch64::TCRETURNri) {
        BMI.add(MI_indcall->getOperand(1)); // Copy FPDiff from original tail call pseudo instruction
    }
    BMI.addUse(ModReg);
    BMI.copyImplicitOps(*MI_indcall);

    MI_indcall->removeFromParent();
}
inline void AArch64EarlyPartsCpiPass::insertCOPYInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall,
    MachineInstr &MI) {
    auto dstOperand = MI.getOperand(0);
    auto srcOperand = MI.getOperand(1);

    auto COPYMI = BuildMI(MBB, *MI_indcall, MI_indcall->getDebugLoc(), TII->get(AArch64::COPY));
    COPYMI.add(dstOperand);
    COPYMI.add(srcOperand);
}