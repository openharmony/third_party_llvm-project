//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "llvm/PARTS/Parts.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SHA256.h"
#include <regex>
#include <map>
using namespace llvm;
using namespace PARTS;
static std::map<const Type *, uint64_t> TypeIdCache;
static cl::opt<bool> EnableFeCfi(
    "FECFI", cl::Hidden,
    cl::desc("forward-edge CFI"),
    cl::init(false));

static cl::opt<bool> EnableBeCfi(
    "FGBECFI", cl::Hidden,
    cl::desc("backward-edge CFI"),
    cl::init(false));

static cl::opt<bool> EnableDfiDpTag(
    "DFIDPTAG", cl::Hidden,
    cl::desc("Data field DFI identify by tag"),
    cl::init(false));

static cl::opt<bool> EnableDfiDppTag(
    "DFIDPPTAG", cl::Hidden,
    cl::desc("Data pointer DFI identify by tag"),
    cl::init(false));

static cl::opt<bool> EnableDfiDpp(
    "DFIDPP", cl::Hidden,
    cl::desc("Data pointer DFI identify by config"),
    cl::init(false));

bool llvm::PARTS::useFeCfi() {
    return EnableFeCfi;
}

bool llvm::PARTS::useBeCfi() {
    return EnableBeCfi;
}

bool llvm::PARTS::useDataPointerConfig() {
    return EnableDfiDpp;
}

bool llvm::PARTS::useDataPointerProtection() {
    return EnableDfiDpp || EnableDfiDppTag;
}

bool llvm::PARTS::useDataFieldTag() {
    return EnableDfiDpTag || EnableDfiDppTag;
}

bool llvm::PARTS::useDataFieldProtection() {
    return EnableDfiDpTag;
}

PartsBeCfiType PARTS::getBeCfiType() {
    if (EnableBeCfi) {
        return PartsBeCfiFull;
    }
    return PartsBeCfiNone;
}

static void buildTypeString(const Type *T, llvm::raw_string_ostream &O)
{
    if (T->isPointerTy()) {
        O << "ptr.";
        buildTypeString(T->getNonOpaquePointerElementType(), O);
    } else if (T->isStructTy()) {
        if (dyn_cast<StructType>(T)->isLiteral()) {
            return;
        }
        auto Sty = dyn_cast<StructType>(T)->getStructName();
        std::regex E("^(\\w+\\.\\w+)(\\.\\w+)?$");
        O << std::regex_replace(Sty.str(), E, "$1");
    } else if (T->isArrayTy()) {
        O << "ptr.";
        buildTypeString(T->getArrayElementType(), O);
    } else if (T->isFunctionTy()) {
        auto *FuncTy = dyn_cast<::llvm::FunctionType>(T);
        O << "f.";
        buildTypeString(FuncTy->getReturnType(), O);
        for (auto *Param = FuncTy->param_begin(); Param != FuncTy->param_end(); Param++) {
            buildTypeString(*Param, O);
        }
    } else if (T->isVectorTy()) {
        O << "vec." << dyn_cast<::llvm::VectorType>(T)->getElementCount().getKnownMinValue();
        buildTypeString(dyn_cast<::llvm::VectorType>(T)->getElementType(), O);
    } else if (T->isVoidTy()) {
        O << "v";
    } else {
        /* Make sure we've handled all cases we want to*/
        if (!(T->isIntegerTy() || T->isFloatingPointTy())) {
            outs() << "buildTypeString: assert!!\n";
        }
        T->print(O);
    }
}

static uint16_t getTypeId(const std::string &Context)
{
    uint16_t TheTypeId = 0;
    llvm::SHA256 ALG;
    auto HashCode = ALG.hash(ArrayRef<uint8_t>(reinterpret_cast<const uint8_t *>(Context.c_str()), Context.size()));
    std::copy(HashCode.begin(), HashCode.begin() + 2, reinterpret_cast<uint8_t *>(&TheTypeId));
    return TheTypeId;
}

uint16_t llvm::PARTS::getTypeIdFor(const Type *T)
{
    if (!T->isPointerTy()) {
        return 0;
    }
    auto TypeIdIter = TypeIdCache.find(T);
    if (TypeIdIter != TypeIdCache.end()) {
        return TypeIdIter->second;
    }
    std::string Buf;
    llvm::raw_string_ostream TypeIdStr(Buf);
    buildTypeString(T, TypeIdStr);
    TypeIdStr.flush();
    uint16_t TheTypeId = getTypeId(Buf);
    TypeIdCache.emplace(T, TheTypeId);
    return TheTypeId;
}