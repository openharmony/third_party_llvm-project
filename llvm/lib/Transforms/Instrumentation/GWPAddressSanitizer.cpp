//===- GWPAddressSanitizer.cpp - detector of uninitialized reads -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file is a part of GWPAddressSanitizer, an address basic correctness
/// checker based on tagged addressing.
//===----------------------------------------------------------------------===//

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/GWPAddressSanitizer.h"
#include "llvm/Transforms/Utils/EscapeEnumerator.h"

using namespace llvm;

#define DEBUG_TYPE "gwp_asan"

/// An instrumentation pass implementing detection of addressability bugs
/// using tagged pointers.
class GWPAddressSanitizer {
public:
  bool sanitizeFunction(Function &F);

  void initializeCallbacks(Module &M);

private:
  FunctionCallee GwpasanSetForceSampleFunc;
  FunctionCallee GwpasanSetUnforceSampleFunc;
};

PreservedAnalyses GWPAddressSanitizerPass::run(Function &F,
                                               FunctionAnalysisManager &FAM) {
  GWPAddressSanitizer GWPASan;
  if (GWPASan.sanitizeFunction(F))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

void GWPAddressSanitizer::initializeCallbacks(Module &M) {
  IRBuilder<> IRB(M.getContext());

  AttributeList Attr;
  Attr = Attr.addFnAttribute(M.getContext(), Attribute::NoUnwind);

  GwpasanSetForceSampleFunc = M.getOrInsertFunction(
      "gwp_asan_force_sample_by_funcattr", Attr, IRB.getVoidTy());
  GwpasanSetUnforceSampleFunc =  M.getOrInsertFunction(
      "gwp_asan_unforce_sample_by_funcattr", Attr, IRB.getVoidTy());
}

PreservedAnalyses ModuleGWPAddressSanitizerPass::run(Module &M,
                                                     ModuleAnalysisManager
                                                     &MAM) {
  return PreservedAnalyses::none();
}

bool GWPAddressSanitizer::sanitizeFunction(Function &F) {
  if (!F.hasFnAttribute(Attribute::GWPSanitizeSpecific))
    return false;

  // Naked functions can not have prologue/epilogue generated, so
  // don't instrument them at all.
  if (F.hasFnAttribute(Attribute::Naked))
    return false;

  initializeCallbacks(*F.getParent());
  LLVM_DEBUG(dbgs() << "Function: " << F.getName() << "\n");

  Instruction *InsertPt = &*F.getEntryBlock().begin();
  IRBuilder<> EntryIRB(InsertPt);
  EntryIRB.CreateCall(GwpasanSetForceSampleFunc);
  EscapeEnumerator EE(F, "", true);
  while (IRBuilder<> *AtExit = EE.Next()) {
    InstrumentationIRBuilder::ensureDebugInfo(*AtExit, F);
    AtExit->CreateCall(GwpasanSetUnforceSampleFunc, {});
  }
  return true;
}
