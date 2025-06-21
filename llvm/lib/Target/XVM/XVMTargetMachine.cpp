//===-- XVMTargetMachine.cpp - Define TargetMachine for XVM ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about XVM target spec.
//
//===----------------------------------------------------------------------===//

#ifdef XVM_DYLIB_MODE

#include "XVMTargetMachine.h"
#include "XVM.h"
#include "XVMTargetTransformInfo.h"
#include "MCTargetDesc/XVMMCAsmInfo.h"
#include "TargetInfo/XVMTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/SimplifyCFGOptions.h"
#include "llvm/Transforms/Utils.h"
using namespace llvm;


extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTarget() {
  // Register the target.
  RegisterTargetMachine<XVMTargetMachine> X(getTheXVMleTarget());
  RegisterTargetMachine<XVMTargetMachine> Y(getTheXVMTarget());

  // Register backend passes
  auto &PR = *PassRegistry::getPassRegistry();
  initializeXVMCFGSortPass(PR);
  initializeXVMCFGStackifyPass(PR);
  initializeXVMCFGStructurePass(PR);
  initializeXVMUpdateRefInstrForMIPass(PR);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMTargetCalledInDylib() {
  LLVMInitializeXVMTarget();
}

// DataLayout: little endian
// Stack frame alignment: 128
// integer size & alignment: 64
static std::string computeDataLayout(const Triple &TT) {
    return "e-m:e-p:64:64-i64:64-n64-S128";
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::PIC_);
}

XVMTargetMachine::XVMTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
  this->Options.EmitAddrsig = false;

  setRequiresStructuredCFG(true);
}

namespace {
// XVM Code Generator Pass Configuration Options.
class XVMPassConfig : public TargetPassConfig {
public:
  XVMPassConfig(XVMTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  XVMTargetMachine &getXVMTargetMachine() const {
    return getTM<XVMTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
  void addPreRegAlloc() override;
  bool addPreISel() override;
};
}

bool XVMPassConfig::addPreISel() {
  addPass(createFlattenCFGPass());
  addPass(createFixIrreduciblePass());
  return false;
}

TargetPassConfig *XVMTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new XVMPassConfig(*this, PM);
}

void XVMPassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
}

TargetTransformInfo
XVMTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(XVMTTIImpl(this, F));
}

// Install an instruction selector pass using
// the ISelDag to gen XVM code.
bool XVMPassConfig::addInstSelector() {
  addPass(createXVMISelDag(getXVMTargetMachine()));

  return false;
}

void XVMPassConfig::addPreRegAlloc() {
  addPass(createXVMUpdateRefInstrForMIPass());
}


void XVMPassConfig::addPreEmitPass() {
  // Sort the blocks of the CFG into topological order,
  // a prerequisite for BLOCK and LOOP markers.
  // Currently, the algorithm is from WebAssembly.
  addPass(createXVMCFGSort());
  addPass(createXVMCFGStackify());
}

#else

#include "XVMDylibHandler.h"
LLVM_INIT_XVM_COMP(Target)

#endif
