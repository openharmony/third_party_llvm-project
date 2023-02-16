//===----------------------------------------------------------------------===//
//
// Author: Zaheer Gauhar <zaheer.gauhar@aalto.fi>
//         Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Attributes.h"

using namespace llvm;
using namespace llvm::PARTS;

#define DEBUG_TYPE "PartsPluginPass"

namespace {

class PartsPluginPass : public ModulePass {
public:
    static char ID;
    PartsPluginPass() : ModulePass(ID) {}
    bool runOnModule(Module &M) override;

private:
   /*
   Check the input Value V and recursively change inner parameters, or if V itself needs to be replaced
   in the containint function, then uses give builder to generate PACed value and returns said value.
   @param M
   @param I
   @param V
   @return
   */
   bool handleInstruction(Function &F, Instruction &I);
   CallInst *handleAutCallInstruction(Function &F, Instruction &I, Value *value);
};

}

char PartsPluginPass::ID = 0;
static RegisterPass<PartsPluginPass> X("parts-plugin", "PARTS Plugin pass");

Pass *llvm::PARTS::createPartsPluginPass() { return new PartsPluginPass(); }

bool PartsPluginPass::runOnModule(Module &M) {
    if (!PARTS::useFeCfi())
        return false;

    bool modified = false;
    for (auto &F : M) {
        for (auto &BB: F) {
            for (auto &I: BB) {
                modified = handleInstruction(F, I) || modified;
            }
        }
    }
    return modified;
}

bool PartsPluginPass::handleInstruction(Function &F, Instruction &I) {
    auto CI = dyn_cast<CallInst>(&I);
    if (!CI || CI->isInlineAsm()) {
        return false;
    }
    auto calledValue = CI->getCalledOperand();
    IntrinsicInst *II = dyn_cast<IntrinsicInst>(calledValue);
    if (II && II->getIntrinsicID() == Intrinsic::pa_autcall) {
        Value *paced = nullptr;
        paced = handleAutCallInstruction(F, I, calledValue);
        if (paced) {
            CI->setCalledOperand(paced);
        }
    }
    return false;
}

CallInst *PartsPluginPass::handleAutCallInstruction(Function &F, Instruction &I, Value *value) {
     Instruction *Insn = dyn_cast<Instruction>(value);
     auto *Call_BB = I.getParent();
     auto *Insn_BB = Insn->getParent();

     if (Call_BB == Insn_BB) {
         return nullptr;
     }

     auto calledValue = Insn->getOperand(0);
     const auto calledValueType = calledValue->getType();
     IRBuilder<> Builder(&I);
     auto autcall = Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autcall, { calledValueType});
     auto typeIdConstant = Insn->getOperand(1);
     CallInst *paced = Builder.CreateCall(autcall, { calledValue, typeIdConstant }, "");
     return paced;
 }