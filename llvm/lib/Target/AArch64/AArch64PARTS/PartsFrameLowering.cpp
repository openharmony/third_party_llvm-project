//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Zaheer Gauhar <zaheer.gauhar@aalto.fi>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PartsFrameLowering.h"
#include "AArch64PartsPassCommon.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/PARTS/Parts.h"
#include <sstream>

using namespace llvm;
using namespace llvm::PARTS;
#define DEBUG_TYPE "PartsOptRasPass"

static constexpr const unsigned modReg = AArch64::X16;
static constexpr const unsigned modScratchReg = AArch64::X17;

static bool doInstrument(const MachineFunction &MF) {
    static const auto becfi = PARTS::getBeCfiType();

    // Just skip if we're not even using backward-edge CFI
    if (becfi == PartsBeCfiNone) return false;

    const Function &F = MF.getFunction();
    // Ignore function with the  no-parts attribute
    if (F.hasFnAttribute("no-parts")) return false;
    // Ignore function without function-id (if we need it)
    if (becfi == PartsBeCfiFull && !F.hasFnAttribute("parts-function_id")) return false;

    // Skip if we don't spill LR
    for (const auto &Info : MF.getFrameInfo().getCalleeSavedInfo()) {
        if (Info.getReg() != AArch64::LR) {
            return true;
        }
    }
    return false;
}

static void createBeCfiModifier(const TargetInstrInfo *TII, MachineBasicBlock &MBB, MachineInstr *MIi,
    const DebugLoc &DL) {
    auto &F = MBB.getParent()->getFunction();

    assert(F.hasFnAttribute("parts-function_id") && "missing parts-function_id attribute");

    uint64_t type_id;
    std::istringstream iss(F.getFnAttribute("parts-function_id").getValueAsString().str());
    iss >> type_id;

    const auto t1 = ((type_id) % UINT16_MAX);
    const auto t2 = ((type_id << 16) % UINT16_MAX);
    const auto t3 = ((type_id << 32) % UINT16_MAX);

    if (MIi == nullptr) {
        BuildMI(&MBB, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
        BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(16);
        BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(32);
        BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(48);
    } else{
        BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
        BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(16);
        BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(32);
        BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(48);
    }
}

static MachineInstr *insertFastModifierInstructions(const TargetInstrInfo *TII, MachineBasicBlock &MBB,
    MachineInstr *MI, MCSymbol *Sym, const DebugLoc &DL) {
    LLVM_DEBUG(dbgs() << "insertFastModifierInstructions with MI begin.\n");

    auto FirstModifierMI = BuildMI(MBB, MI, DL, TII->get(AArch64::ADR), modReg)
        .addSym(Sym);
    BuildMI(MBB, MI, DL, TII->get(AArch64::ADDXri), modScratchReg)
        .addReg(AArch64::SP)
        .addImm(0)
        .addImm(0);
    BuildMI(MBB, MI, DL, TII->get(AArch64::BFMXri), modReg)
        .addUse(modReg)
        .addReg(modScratchReg)
        .addImm(32)
        .addImm(31);
    LLVM_DEBUG(dbgs() << "insertFastModifierInstructions with MI end.\n");
    return FirstModifierMI;
}

static void insertFastModifierInstructions(const TargetInstrInfo *TII, MachineBasicBlock &MBB,
    MCSymbol *Sym, const DebugLoc &DL) {
    LLVM_DEBUG(dbgs() << "insertFastModifierInstructions begin.\n");

    BuildMI(&MBB, DL, TII->get(AArch64::ADR), modReg)
        .addSym(Sym);
    BuildMI(&MBB, DL, TII->get(AArch64::ADDXri), modScratchReg)
        .addReg(AArch64::SP)
        .addImm(0)
        .addImm(0);
    BuildMI(&MBB, DL, TII->get(AArch64::BFMXri), modReg)
        .addUse(modReg)
        .addReg(modScratchReg)
        .addImm(32)
        .addImm(31);
    LLVM_DEBUG(dbgs() << "insertFastModifierInstructions end.\n");
}

#define PARTS_SYMBOL "parts_becfi"

static MCSymbol *createBeCfiPrologueSymbol(MachineBasicBlock &MBB) {
    LLVM_DEBUG(dbgs() << "createBeCfiPrologueSymbol.\n");

    auto &MF = *MBB.getParent();
    AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
    auto Sym = MF.getContext().createNamedTempSymbol(PARTS_SYMBOL);
    AFI->setPartsSym(Sym);
    return Sym;
}

static MCSymbol *getBeCfiPrologueSymbol(MachineBasicBlock &MBB) {
    auto &MF = *MBB.getParent();
    AArch64FunctionInfo *AFI = MF.getInfo<AArch64FunctionInfo>();
    return AFI->getPartsSym();
}

static void insertPrologueFastBeCfiModifier(const TargetInstrInfo *TII, MachineBasicBlock &MBB,
    MachineInstr *MIi, const DebugLoc &DL) {
    assert(PARTS::getBeCfiType() == PartsBeCfiNgFull);
    LLVM_DEBUG(dbgs() << "insertPrologueFastBeCfiModifier.\n");

    auto Sym = createBeCfiPrologueSymbol(MBB);
    auto MI = insertFastModifierInstructions(TII, MBB, MIi, Sym, DL);
    MI->setPreInstrSymbol(*MBB.getParent(), Sym);
}

static void insertEpilogueFastBeCfiModifier(const TargetInstrInfo *TII, MachineBasicBlock &MBB,
    MachineInstr *MIi, const DebugLoc &DL) {
    assert(PARTS::getBeCfiType() == PartsBeCfiNgFull);

    auto Sym = getBeCfiPrologueSymbol(MBB);
    assert(Sym && "Missing PARTS prologue instruction symbol/label\n");
    LLVM_DEBUG(dbgs() << "insertEpilogueFastBeCfiModifier.\n");
    if (MIi == nullptr) {
        insertFastModifierInstructions(TII, MBB, Sym, DL);
    } else {
        (void)insertFastModifierInstructions(TII, MBB, MIi, Sym, DL);
    }
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
    MachineBasicBlock &MBB) {
    if (!doInstrument(*MBB.getParent()))
        return;

    const auto loc = MBB.getFirstTerminator();
    auto *MI = (loc != MBB.end() ? &*loc : nullptr);
    LLVM_DEBUG(dbgs() << "instrumentEpilogue.\n");

    if (PARTS::getBeCfiType() == PartsBeCfiFull)
        createBeCfiModifier(TII, MBB, MI, DebugLoc());
    else
        insertEpilogueFastBeCfiModifier(TII, MBB, MI, DebugLoc());

    AArch64PartsPassCommon::insertPACInstr(MBB, MI, AArch64::LR, modReg, TII->get(AArch64::AUTIB));
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
    MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI, const DebugLoc &DL) {
    if (!doInstrument(*MBB.getParent()))
        return;

    LLVM_DEBUG(dbgs() << "instrumentPrologue.\n");

    if (PARTS::getBeCfiType() == PartsBeCfiFull)
        createBeCfiModifier(TII, MBB, &*MBBI, DebugLoc());
    else
        insertPrologueFastBeCfiModifier(TII, MBB, &*MBBI, DebugLoc());

    AArch64PartsPassCommon::insertPACInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::PACIB));
}