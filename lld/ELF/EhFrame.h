//===- EhFrame.h ------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_EHFRAME_H
#define LLD_ELF_EHFRAME_H

// OHOS_LOCAL begin

#include "lld/Common/LLVM.h"

namespace lld::elf {

struct EhSectionPiece;

struct EhPointerEncoding {
  uint8_t encoding;
  size_t offsetInCie = size_t(-1);
};

struct EhPointerEncodings {
  EhPointerEncoding personalityEncoding;
  EhPointerEncoding fdeEncoding;
  EhPointerEncoding lsdaEncoding;
};

EhPointerEncodings getEhPointerEncodings(const EhSectionPiece &cie);
bool hasLSDA(const EhSectionPiece &cie);

} // namespace lld::elf

// OHOS_LOCAL end

#endif
