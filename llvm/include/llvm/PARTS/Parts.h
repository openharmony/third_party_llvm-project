//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTS_H
#define LLVM_PARTS_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"

namespace llvm {
namespace PARTS {

enum PartsBeCfiType {
    PartsBeCfiNone,
    PartsBeCfiFull,
    PartsBeCfiNgFull
};

enum PartsDpiType {
    PartsDpiNone,
    PartsDpiFull,
    PartsDpiFullNoType
};

bool useBeCfi();
bool useFeCfi();
bool useDataPointerConfig();
bool useDataPointerProtection();
bool useDataFieldTag();
bool useDataFieldProtection();
PartsBeCfiType getBeCfiType();
uint16_t getTypeIdFor(const Type *T);
}
}
#endif