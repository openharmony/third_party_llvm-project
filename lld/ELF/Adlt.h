//===- Adlt.h -------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// OHOS_LOCAL begin
#ifndef LLD_ELF_ADLT_H
#define LLD_ELF_ADLT_H

#include "llvm/ADT/CachedHashString.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/Endian.h"
#include <memory>

namespace lld {
namespace elf {

class ELFFileBase;
class InputFile;
class Symbol;
struct PhdrEntry;

template <typename ELFT> class SharedFileExtended;
struct PhdrEntry;

class AdltCtx {
public:
  llvm::SmallVector<ELFFileBase *> sharedFilesExtended;

  void scanDuplicatedSymbols();

  template <class ELFT> void buildSymbolsHist(std::vector<InputFile *> &files);

  template <class ELFT> SharedFileExtended<ELFT> *getSoExt(InputFile *file);
  template <class ELFT> SharedFileExtended<ELFT> *getSoExt(size_t orderId);

  llvm::SetVector<const PhdrEntry *> commonProgramHeaders;

  bool withCfi = false;

  // From input .rela.dyn, .rela.plt:
  // Keep input library indexes that are needed for got/plt symbol
  llvm::DenseMap<const Symbol *, llvm::SmallVector<unsigned, 0>>
      gotPltInfo; // sym, soFile->orderIdx array;

  llvm::DenseMap<llvm::CachedHashStringRef, unsigned>
      symNamesHist; // hash, count usages
  llvm::DenseSet<llvm::CachedHashStringRef> duplicatedSymNames;
};

// The only instance of Ctx struct.
extern std::unique_ptr<AdltCtx> adltCtx;

} // namespace elf
} // namespace lld

#endif // LLD_ELF_ADLT_H
// OHOS_LOCAL end
