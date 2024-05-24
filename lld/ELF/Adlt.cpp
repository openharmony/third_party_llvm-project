//===- Adlt.cpp -------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// OHOS_LOCAL begin
#include "Adlt.h"
#include "llvm/Support/Casting.h"

#include "InputFiles.h"

using namespace llvm;

using namespace lld;
using namespace lld::elf;
using namespace llvm::object;

template <class ELFT>
SharedFileExtended<ELFT> *AdltCtx::getSoExt(InputFile *file) {
  assert(file);
  return cast<SharedFileExtended<ELFT>>(file);
}

template <class ELFT>
SharedFileExtended<ELFT> *AdltCtx::getSoExt(unsigned orderId) {
  assert(orderId < sharedFilesExtended.size());
  return sharedFilesExtended[orderId];
}

void AdltCtx::checkDuplicatedSymbols() {
  assert(!symNamesHist.empty());
  for (auto entry : symNamesHist)
    if (entry.second > 1)
      duplicatedSymNames.insert(entry.first);
  symNamesHist.clear();
}

template SharedFileExtended<ELF32LE> *
AdltCtx::getSoExt<ELF32LE>(InputFile *file);
template SharedFileExtended<ELF32BE> *
AdltCtx::getSoExt<ELF32BE>(InputFile *file);
template SharedFileExtended<ELF64LE> *
AdltCtx::getSoExt<ELF64LE>(InputFile *file);
template SharedFileExtended<ELF64BE> *
AdltCtx::getSoExt<ELF64BE>(InputFile *file);

// OHOS_LOCAL end
