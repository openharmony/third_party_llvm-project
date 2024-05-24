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
  return cast<SharedFileExtended<ELFT>>(sharedFilesExtended[orderId]);
}

template <class ELFT>
void AdltCtx::buildSymbolsHist(std::vector<InputFile *> &files) {
  for (auto *file : files)
    if (auto *soExt = getSoExt<ELFT>(file))
      soExt->buildSymbolsHist();
  assert(!symNamesHist.empty());
}

void AdltCtx::checkDuplicatedSymbols() {
  assert(!symNamesHist.empty());
  for (auto entry : symNamesHist)
    if (entry.second > 1)
      duplicatedSymNames.insert(CachedHashStringRef(entry.first));
  symNamesHist.clear();
}

// TODO: inherit from SharedFile
template SharedFileExtended<ELF32LE> *AdltCtx::getSoExt<ELF32LE>(InputFile *);
template SharedFileExtended<ELF32BE> *AdltCtx::getSoExt<ELF32BE>(InputFile *);
template SharedFileExtended<ELF64LE> *AdltCtx::getSoExt<ELF64LE>(InputFile *);
template SharedFileExtended<ELF64BE> *AdltCtx::getSoExt<ELF64BE>(InputFile *);

template SharedFileExtended<ELF32LE> *AdltCtx::getSoExt<ELF32LE>(unsigned);
template SharedFileExtended<ELF32BE> *AdltCtx::getSoExt<ELF32BE>(unsigned);
template SharedFileExtended<ELF64LE> *AdltCtx::getSoExt<ELF64LE>(unsigned);
template SharedFileExtended<ELF64BE> *AdltCtx::getSoExt<ELF64BE>(unsigned);

template void AdltCtx::buildSymbolsHist<ELF32LE>(std::vector<InputFile *> &);
template void AdltCtx::buildSymbolsHist<ELF32BE>(std::vector<InputFile *> &);
template void AdltCtx::buildSymbolsHist<ELF64LE>(std::vector<InputFile *> &);
template void AdltCtx::buildSymbolsHist<ELF64BE>(std::vector<InputFile *> &);

// OHOS_LOCAL end
