//===- AddReferenceTrackingInfo.cpp - Reference-tracking metadata ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass attaches "memtracer" metadata (two strings: name, typename)
// to store instructions, using debug info from dbg.declare when available.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/AddReferenceTrackingInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

#define DEBUG_TYPE "reference-tracking"

const char ReferenceInfoMDKind[] = "memtracer";

void llvm::setReferenceInfo(Instruction *I, const std::string &Name,
                            const std::string &TypeName) {
  if (Name.empty() && TypeName.empty())
    return;
  LLVMContext &Ctx = I->getContext();
  Metadata *Ops[] = {
      MDString::get(Ctx, Name),
      MDString::get(Ctx, TypeName),
  };
  MDNode *MD = MDTuple::get(Ctx, Ops);
  I->setMetadata(ReferenceInfoMDKind, MD);
}

bool llvm::getReferenceInfo(const StoreInst *SI, std::string &Name,
                            std::string &TypeName) {
  MDNode *MD = SI->getMetadata(ReferenceInfoMDKind);
  if (!MD || MD->getNumOperands() != 2)
    return false;
  auto *NameNode = dyn_cast<MDString>(MD->getOperand(0));
  auto *TypeNameNode = dyn_cast<MDString>(MD->getOperand(1));
  if (!NameNode || !TypeNameNode)
    return false;
  Name = NameNode->getString().str();
  TypeName = TypeNameNode->getString().str();
  return true;
}

// Build type string from \p Info
static std::string buildTypeNameStr(const ReferenceDbgInfo &Info) {
  std::string s = Info.BasicTypeName;
  if (s.empty())
    s += "void";
  for (size_t i = 0; i < Info.PtrDepth; ++i)
    s += "*";
  for (size_t i = 0; i < Info.ArrayDims; ++i)
    s += "[]";
  return s;
}

// Attach memtracer from \p Info to instruction \p I (store or call).
static void setReferenceInfoFromDbgInfo(Instruction *I,
                                        const ReferenceDbgInfo &Info) {
  setReferenceInfo(I, Info.Name, buildTypeNameStr(Info));
}

// Helper: true if \p DT is pointer type.
static bool isPointerTag(dwarf::Tag Tag) {
  return Tag == dwarf::DW_TAG_pointer_type;
}

// Helper: true if \p DT is typedef.
static bool isTypedefTag(dwarf::Tag Tag) {
  return Tag == dwarf::DW_TAG_typedef;
}

// Parse \p Ty into \p Info: strip pointer/reference/array layers (set
// PtrDepth, ArrayDims), then fill base type fields (BasicTypeTag,
// BasicTypeName, MemberElements). Single entry point for
// "DIType -> ReferenceDbgInfo".
static void fillReferenceDbgInfoFromType(DIType *Ty, ReferenceDbgInfo &Info) {
  Info.PtrDepth = 0;
  Info.ArrayDims = 0;
  Info.BasicTypeTag = dwarf::DW_TAG_null;
  Info.BasicTypeName.clear();
  Info.MemberElements = nullptr;

  DIType *BaseType = Ty;
  while (BaseType) {
    if (DIDerivedType *DT = dyn_cast<DIDerivedType>(BaseType)) {
      if (isPointerTag(static_cast<dwarf::Tag>(DT->getTag())))
        Info.PtrDepth++;
      // if typedef, choose the top-level typedef declaration as basic type name
      if (isTypedefTag(static_cast<dwarf::Tag>(DT->getTag()))) {
        if (Info.BasicTypeName.empty())
          Info.BasicTypeName = DT->getName().str();
      }
      BaseType = DT->getBaseType();
      continue;
    }
    if (DICompositeType *CT = dyn_cast<DICompositeType>(BaseType)) {
      if (CT->getTag() == dwarf::DW_TAG_array_type) {
        Info.BasicTypeTag = static_cast<dwarf::Tag>(CT->getTag());
        if (MDTuple *Elements = CT->getElements().get())
          Info.ArrayDims = Elements->getNumOperands();
        BaseType = CT->getBaseType();
        // count the pointer depth of the element type
        Info.PtrDepth = 0;
        while (BaseType) {
          if (DIDerivedType *DT = dyn_cast<DIDerivedType>(BaseType)) {
            if (isPointerTag(static_cast<dwarf::Tag>(DT->getTag())))
              Info.PtrDepth++;
            BaseType = DT->getBaseType();
            continue;
          }
          break;
        }
      }
    }
    break;
  }

  if (!BaseType)
    return;
  if (Info.BasicTypeTag == dwarf::DW_TAG_null)
    Info.BasicTypeTag = static_cast<dwarf::Tag>(BaseType->getTag());
  if (Info.BasicTypeName.empty())
    Info.BasicTypeName = BaseType->getName().str();
  if (DICompositeType *CT = dyn_cast<DICompositeType>(BaseType))
    Info.MemberElements = CT->getElements().get();
}

// Return true if ReferenceDbgInfo has enough to use (name and some type info).
static bool hasUsableReferenceDbgInfo(const ReferenceDbgInfo &Info) {
  return !Info.Name.empty() && (Info.PtrDepth > 0 || Info.ArrayDims > 0 ||
                                !Info.BasicTypeName.empty());
}

// Get ReferenceDbgInfo for \p PtrOp from the PtrToDbgVar map.
// Returns true and sets \p OutInfo when \p PtrOp is in the map.
static bool getReferenceDbgInfoFromMap(
    const DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar, Value *PtrOp,
    ReferenceDbgInfo &OutInfo) {
  auto It = PtrToDbgVar.find(PtrOp);
  if (It == PtrToDbgVar.end())
    return false;
  OutInfo = It->second;
  return true;
}

// Build ReferenceDbgInfo from a GlobalVariable using its debug info.
// The result describes the type of the value stored in the global (i.e. the
// type of the value we get when we load from it).
static ReferenceDbgInfo getReferenceDbgInfoFromGlobalVar(GlobalVariable *GV) {
  ReferenceDbgInfo Info;
  if (!GV)
    return Info;
  if (MDNode *DbgMD = GV->getMetadata("dbg")) {
    if (auto *GVE = dyn_cast<DIGlobalVariableExpression>(DbgMD)) {
      if (DIGlobalVariable *GVar = GVE->getVariable()) {
        StringRef DIName = GVar->getName();
        if (DIName.empty())
          DIName = GVar->getLinkageName();
        Info.Name = DIName.str();
        if (DIType *Ty = GVar->getType())
          fillReferenceDbgInfoFromType(Ty, Info);
      }
    }
  }
  if (Info.Name.empty())
    Info.Name = GV->getName().str();
  return Info;
}

// Build ReferenceDbgInfo from a DILocalVariable (and its DIType).
static ReferenceDbgInfo
getReferenceDbgInfoFromVariable(const DILocalVariable *Var) {
  ReferenceDbgInfo Info;
  if (!Var)
    return Info;
  Info.Name = Var->getName().str();
  DIType *Ty = Var->getType();
  if (!Ty)
    return Info;
  fillReferenceDbgInfoFromType(Ty, Info);
  return Info;
}

// Build ReferenceDbgInfo from a DIType (name not set; used for member types).
static ReferenceDbgInfo getReferenceDbgInfoFromDIType(DIType *Ty) {
  ReferenceDbgInfo Info;
  if (!Ty)
    return Info;
  fillReferenceDbgInfoFromType(Ty, Info);
  return Info;
}

//===----------------------------------------------------------------------===//
// Get ReferenceDbgInfo from global ptr / get (name,typename) for store
//===----------------------------------------------------------------------=//

// Get ReferenceDbgInfo for \p PtrOp when its underlying object is a global
// variable (via getReferenceDbgInfoFromGlobalVar). Returns true and sets
// \p OutInfo when found and usable.
static bool getReferenceDbgInfoFromGlobalPtr(Value *PtrOp,
                                             ReferenceDbgInfo &OutInfo) {
  Value *Underlying = getUnderlyingObject(PtrOp);
  GlobalVariable *GV = dyn_cast<GlobalVariable>(Underlying);
  if (!GV)
    return false;
  OutInfo = getReferenceDbgInfoFromGlobalVar(GV);
  return hasUsableReferenceDbgInfo(OutInfo);
}

// If the store's destination address is a global variable, fill Name and
// TypeName with the global's name and type string, and return true.
static bool getReferenceInfoFromGlobal(StoreInst &SI, std::string &Name,
                                       std::string &TypeName) {
  Value *Underlying = getUnderlyingObject(SI.getPointerOperand());
  auto *GV = dyn_cast<GlobalVariable>(Underlying);
  if (!GV)
    return false;
  ReferenceDbgInfo Info = getReferenceDbgInfoFromGlobalVar(GV);
  if (!hasUsableReferenceDbgInfo(Info))
    return false;
  Name = Info.Name;
  TypeName = buildTypeNameStr(Info);
  // Only return true when we actually have something to report.
  return !Name.empty() || !TypeName.empty();
}

//===----------------------------------------------------------------------===//
// Collect and propagate
//===----------------------------------------------------------------------===//

// Collect Value -> ReferenceDbgInfo from dbg.declare, and count store ptr,ptr.
// \return Number of store instructions that store a pointer value.
static unsigned collectPtrToDbgVarAndStorePtrCount(
    Function &F, DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar) {
  unsigned StorePtrCount = 0;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (auto *DDI = dyn_cast<DbgDeclareInst>(&I)) {
        Value *Addr = DDI->getVariableLocationOp(0);
        if (Addr && DDI->getVariable())
          PtrToDbgVar.try_emplace(
              Addr, getReferenceDbgInfoFromVariable(DDI->getVariable()));
      } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
        if (SI->getValueOperand()->getType()->isPointerTy())
          ++StorePtrCount;
      }
    }
  }
  return StorePtrCount;
}

// Apply one GEP step (array index or struct member) to BaseInfo.
// \p LastIdxOp  struct field index (ConstantInt), or null for array step only.
// \p GEPBase    base pointer of the GEP (getPointerOperand); if it is a
//               LoadInst result we use "->", else "." (A->member vs A.member).
static ReferenceDbgInfo resolveGEPStep(const ReferenceDbgInfo &BaseInfo,
                                       Value *LastIdxOp, Value *GEPBase) {
  ReferenceDbgInfo NewInfo = BaseInfo;
  if (BaseInfo.ArrayDims > 0) {
    NewInfo.ArrayDims--;
    NewInfo.Name += "[]";
    return NewInfo;
  }
  if (BaseInfo.MemberElements && LastIdxOp) {
    if (auto *CI = dyn_cast<ConstantInt>(LastIdxOp)) {
      uint64_t FieldIdx = CI->getZExtValue();
      MDTuple *Tuple = dyn_cast<MDTuple>(BaseInfo.MemberElements);
      if (Tuple && FieldIdx < Tuple->getNumOperands()) {
        if (DIDerivedType *Member =
                dyn_cast<DIDerivedType>(Tuple->getOperand(FieldIdx))) {
          DIType *MemberTy = Member->getBaseType();
          if (MemberTy) {
            NewInfo = getReferenceDbgInfoFromDIType(MemberTy);
            // Base from load => pointer value => "->";
            // else (alloca/global/GEP) => "."
            const char *Sep = (GEPBase && isa<LoadInst>(GEPBase)) ? "->" : ".";
            NewInfo.Name = BaseInfo.Name + Sep + Member->getName().str();
          }
        }
      }
    }
  }
  return NewInfo;
}

// Propagate typeInfo through GEP: resolve member by index, name
// "base->member". Base info from getReferenceDbgInfoFromMap or
// getReferenceDbgInfoFromGlobalPtr.
static void propagateGEP(DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar,
                         GetElementPtrInst *GEP) {
  Value *PtrOp = GEP->getPointerOperand();
  ReferenceDbgInfo BaseInfo;
  if (!getReferenceDbgInfoFromMap(PtrToDbgVar, PtrOp, BaseInfo) &&
      !getReferenceDbgInfoFromGlobalPtr(PtrOp, BaseInfo))
    return;
  Value *LastIdxOp = GEP->getNumIndices() > 0
                         ? GEP->getOperand(GEP->getNumOperands() - 1)
                         : nullptr;
  PtrToDbgVar.try_emplace(GEP, resolveGEPStep(BaseInfo, LastIdxOp, PtrOp));
}

// Same as propagateGEP but for GEP ConstantExpr (e.g. store dest
// getelementptr inbounds (%struct.Data, ptr @global_data, i32 0, i32 1)).
static void
propagateGEPConstantExpr(DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar,
                         ConstantExpr *CE) {
  if (CE->getOpcode() != Instruction::GetElementPtr)
    return;
  Value *Base = CE->getOperand(0);
  ReferenceDbgInfo BaseInfo;
  if (!getReferenceDbgInfoFromMap(PtrToDbgVar, Base, BaseInfo) &&
      !getReferenceDbgInfoFromGlobalPtr(Base, BaseInfo))
    return;
  unsigned NumOps = CE->getNumOperands();
  Value *LastIdxOp = NumOps > 1 ? CE->getOperand(NumOps - 1) : nullptr;
  PtrToDbgVar.try_emplace(CE, resolveGEPStep(BaseInfo, LastIdxOp, Base));
}

// Propagate typeInfo through a load of pointer: result has PtrDepth - 1.
// If the load is from a value in PtrToDbgVar, propagate; otherwise if loading
// from a global variable, infer type from global's debug info and add to map.
static void propagateLoad(DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar,
                          LoadInst *LI) {
  if (!LI->getType()->isPointerTy())
    return;
  Value *PtrOp = LI->getPointerOperand();

  // If load address is from GEP constant expr, fill it first.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(PtrOp))
    if (CE->getOpcode() == Instruction::GetElementPtr)
      propagateGEPConstantExpr(PtrToDbgVar, CE);

  ReferenceDbgInfo BaseInfo;
  if (getReferenceDbgInfoFromMap(PtrToDbgVar, PtrOp, BaseInfo)) {
    if (BaseInfo.PtrDepth > 0) {
      BaseInfo.PtrDepth--;
      PtrToDbgVar.try_emplace(LI, BaseInfo);
    }
    return;
  }

  if (getReferenceDbgInfoFromGlobalPtr(PtrOp, BaseInfo))
    PtrToDbgVar.try_emplace(LI, BaseInfo);
}

// Attach memtracer to one store of pointer if we have name/type. Returns
// true if metadata was attached.
static bool
tryAttachReferenceInfo(StoreInst *SI,
                       const DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar) {
  if (!SI->getValueOperand()->getType()->isPointerTy())
    return false;

  auto It = PtrToDbgVar.find(SI->getPointerOperand());
  if (It != PtrToDbgVar.end()) {
    setReferenceInfoFromDbgInfo(SI, It->second);
    return true;
  }

  std::string GlobalName, GlobalTypeName;
  if (getReferenceInfoFromGlobal(*SI, GlobalName, GlobalTypeName)) {
    setReferenceInfo(SI, GlobalName, GlobalTypeName);
    return true;
  }

  // keep to add test for the case where this pass can not deal with
  // TODO: DELETE ME AFTER TESTING, using string directly
  setReferenceInfo(SI, "0_UNKNOWN_", "0_UNKNOWN_");
  return false;
}

// Propagate Load/GEP info and attach memtracer to store ptr,ptr. Requires
// PtrToDbgVar from collectPtrToDbgVarAndStorePtrCount. Fills
// \p StorePtrToMemtracerMD with (stored pointer value -> memtracer MDNode) for
// each pointer store after metadata is attached to the store. Returns true if
// any metadata was attached.
static bool processInstructionsForReferenceInfo(
    Function &F, DenseMap<Value *, ReferenceDbgInfo> &PtrToDbgVar,
    DenseMap<Value *, MDNode *> &StorePtrToMemtracerMD) {
  StorePtrToMemtracerMD.clear();
  bool Changed = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        propagateLoad(PtrToDbgVar, LI);
        continue;
      }
      if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
        propagateGEP(PtrToDbgVar, GEP);
        continue;
      }
      if (auto *SI = dyn_cast<StoreInst>(&I)) {
        // If store dest is GEP constant expr, fill it first.
        Value *StorePtrOp = SI->getPointerOperand();
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(StorePtrOp))
          if (CE->getOpcode() == Instruction::GetElementPtr)
            propagateGEPConstantExpr(PtrToDbgVar, CE);
        Changed |= tryAttachReferenceInfo(SI, PtrToDbgVar);
        if (SI->getValueOperand()->getType()->isPointerTy())
          if (MDNode *MD = SI->getMetadata(ReferenceInfoMDKind))
            StorePtrToMemtracerMD[SI->getValueOperand()] = MD;
      }
    }
  }
  return Changed;
}

// Return true if \p CB is a call to malloc, realloc, calloc, or C++ new.
static bool isAllocationCall(const CallBase *CB) {
  const Function *Callee = CB->getCalledFunction();
  if (!Callee || !Callee->hasName())
    return false;
  StringRef Name = Callee->getName();
  if (Name == "malloc" || Name == "realloc" || Name == "calloc")
    return true;
  if (Name.startswith("_Znwm") || Name.startswith("_Znam"))
    return true;
  return false;
}

// Third pass: for each call to malloc/realloc/new, use the call result as key
// in StorePtrToMemtracerMD to copy the same memtracer MDNode onto the call.
static bool processAllocCallsForReferenceInfo(
    Function &F, const DenseMap<Value *, MDNode *> &StorePtrToMemtracerMD) {
  bool Changed = false;
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      auto *CB = dyn_cast<CallBase>(&I);
      if (!CB || !CB->getType()->isPointerTy())
        continue;
      if (!isAllocationCall(CB))
        continue;
      auto It = StorePtrToMemtracerMD.find(CB);
      if (It == StorePtrToMemtracerMD.end() || !It->second)
        continue;
      CB->setMetadata(ReferenceInfoMDKind, It->second);
      Changed = true;
    }
  }
  return Changed;
}

// Core logic for one function: collect dbg map, then propagate and attach.
static bool runOnFunctionImpl(Function &F) {
  DenseMap<Value *, ReferenceDbgInfo> PtrToDbgVar;
  unsigned StorePtrCount = collectPtrToDbgVarAndStorePtrCount(F, PtrToDbgVar);
  if (StorePtrCount == 0)
    return false;

  DenseMap<Value *, MDNode *> StorePtrToMemtracerMD;
  bool Changed = processInstructionsForReferenceInfo(F, PtrToDbgVar,
                                                     StorePtrToMemtracerMD);
  Changed |= processAllocCallsForReferenceInfo(F, StorePtrToMemtracerMD);
  return Changed;
}

//===----------------------------------------------------------------------===//
// New pass manager
//===----------------------------------------------------------------------===//

PreservedAnalyses
AddReferenceTrackingInfoPass::run(Function &F, FunctionAnalysisManager &AM) {
  bool Changed = runOnFunctionImpl(F);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

PreservedAnalyses AddReferenceTrackingInfoPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  bool Changed = false;
  for (auto &F : M)
    Changed |= runOnFunctionImpl(F);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
