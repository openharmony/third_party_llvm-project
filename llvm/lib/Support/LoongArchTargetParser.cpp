//====-- LoongArchTargetParser - Parser for LoongArch features --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise LoongArch hardware features
// such as CPU/ARCH and extension names.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/LoongArchTargetParser.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;
using namespace llvm::LoongArch;

const FeatureInfo AllFeatures[] = {
#define LOONGARCH_FEATURE(NAME, KIND) {NAME, KIND},
#include "llvm/Support/LoongArchTargetParser.def"
};

const ArchInfo AllArchs[] = {
#define LOONGARCH_ARCH(NAME, KIND, FEATURES)                                   \
  {NAME, LoongArch::ArchKind::KIND, FEATURES},
#include "llvm/Support/LoongArchTargetParser.def"
};

bool LoongArch::isValidArchName(StringRef Arch) {
  for (const auto A : AllArchs)
    if (A.Name == Arch)
      return true;
  return false;
}

bool LoongArch::getArchFeatures(StringRef Arch,
                                std::vector<StringRef> &Features) {
  for (const auto A : AllArchs) {
    if (A.Name == Arch) {
      for (const auto F : AllFeatures)
        if ((A.Features & F.Kind) == F.Kind)
          Features.push_back(F.Name);
      return true;
    }
  }
  return false;
}

bool LoongArch::isValidCPUName(StringRef Name) { return isValidArchName(Name); }

void LoongArch::fillValidCPUList(SmallVectorImpl<StringRef> &Values) {
  for (const auto A : AllArchs)
    Values.emplace_back(A.Name);
}

StringRef LoongArch::getDefaultArch(bool Is64Bit) {
  // TODO: use a real 32-bit arch name.
  return Is64Bit ? "loongarch64" : "";
}
