//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64EARLYPARTSCPI_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64EARLYPARTSCPI_H

#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Module.h"

using namespace llvm;

#define DEBUG_TYPE "AArch64EarlyPartsCpiPass"

class AArch64EarlyPartsCpiPass : public MachineFunctionPass {
public:
    static char ID;
    AArch64EarlyPartsCpiPass() : MachineFunctionPass(ID) {
        initializeAArch64EarlyPartsCpiPassPass(*PassRegistry::getPassRegistry());
    }
    StringRef getPassName() const override { return DEBUG_TYPE; }
    virtual bool doInitialization(Module &M) override;
    bool runOnMachineFunction(MachineFunction &) override;
private:
    const AArch64Subtarget *STI = nullptr;
    const AArch64InstrInfo *TII = nullptr;
    inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
    inline bool getAllAutCalls(MachineInstr *PhiMi, MachineRegisterInfo *MRI, SmallVector<MachineInstr *> &AutCallVec) const;
    inline void insertAuthenticateInstr(MachineRegisterInfo *MRI, MachineInstr *AutCall, MachineInstr *PhiMi);
    inline void findIndirectCallMachineInstr(MachineFunction &MF, MachineBasicBlock &MBB,
        MachineInstr *MIptr, SmallVector<MachineInstr*> &IndCallVec);
    void triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB);
    inline bool isIndirectCall(const MachineInstr &MI) const;
    inline bool isPartsAUTCALLIntrinsic(unsigned Opcode) const;
    inline const MCInstrDesc &getIndirectCallAuth(MachineInstr *MI_indcall);
    inline void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, MachineInstr &MI);
    inline void insertCOPYInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, MachineInstr &MI);
    inline bool handlePhi(MachineFunction &MF, MachineInstr *MIptr, unsigned AutCall);
    inline bool isIndirectAutCall(const MachineInstr &MI) const;
    inline void addPhiForModifier(MachineInstr *Indirect, Register *ModReg);
};

#endif