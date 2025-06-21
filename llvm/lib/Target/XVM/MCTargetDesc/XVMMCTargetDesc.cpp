//===-- XVMMCTargetDesc.cpp - XVM Target Descriptions ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides XVM specific target descriptions.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE
#include "MCTargetDesc/XVMMCTargetDesc.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "MCTargetDesc/XVMMCAsmInfo.h"
#include "MCTargetDesc/XVMTargetStreamer.h"
#include "TargetInfo/XVMTargetInfo.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Host.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "XVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "XVMGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "XVMGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createXVMMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitXVMMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createXVMMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitXVMMCRegisterInfo(X, XVM::R11 /* RAReg doesn't exist */);
  return X;
}

static MCSubtargetInfo *createXVMMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  return createXVMMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCStreamer *createXVMMCStreamer(const Triple &T, MCContext &Ctx,
                                       std::unique_ptr<MCAsmBackend> &&MAB,
                                       std::unique_ptr<MCObjectWriter> &&OW,
                                       std::unique_ptr<MCCodeEmitter> &&Emitter,
                                       bool RelaxAll) {
  return createELFStreamer(Ctx, std::move(MAB), std::move(OW), std::move(Emitter),
                           RelaxAll);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &,
                                                 MCInstPrinter *, bool) {
  return new XVMTargetStreamer(S);
}


static MCInstPrinter *createXVMMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new XVMInstPrinter(MAI, MII, MRI);
  return nullptr;
}

namespace {

class XVMMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit XVMMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}
};

} // end anonymous namespace

static MCInstrAnalysis *createXVMInstrAnalysis(const MCInstrInfo *Info) {
  return new XVMMCInstrAnalysis(Info);
}


extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetMC() {
  for (Target *T: {&getTheXVMleTarget(), &getTheXVMTarget()}) {
    // Register the MC asm info.
    RegisterMCAsmInfo<XVMMCAsmInfo> X(*T);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createXVMMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createXVMMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T,
                                            createXVMMCSubtargetInfo);

    // Register the object streamer
    TargetRegistry::RegisterELFStreamer(*T, createXVMMCStreamer);

    // Register the MCTargetStreamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createXVMMCInstPrinter);

    // Register the MC instruction analyzer.
    TargetRegistry::RegisterMCInstrAnalysis(*T, createXVMInstrAnalysis);
  }
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetMCCalledInDylib() {
  LLVMInitializeXVMTargetMC();
}

#else

#include "../XVMDylibHandler.h"
LLVM_INIT_XVM_COMP(TargetMC)

#endif
