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

using namespace llvm;
using namespace PARTS;

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