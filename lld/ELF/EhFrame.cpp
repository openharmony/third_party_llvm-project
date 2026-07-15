#ifndef OHOS_LLVM
//===- EhFrame.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// .eh_frame section contains information on how to unwind the stack when
// an exception is thrown. The section consists of sequence of CIE and FDE
// records. The linker needs to merge CIEs and associate FDEs to CIEs.
// That means the linker has to understand the format of the section.
//
// This file contains a few utility functions to read .eh_frame contents.
//
//===----------------------------------------------------------------------===//

#include "EhFrame.h"
#include "Config.h"
#include "InputFiles.h"
#include "InputSection.h"
#include "Relocations.h"
#include "Target.h"
#include "llvm/BinaryFormat/Dwarf.h"

using namespace llvm;
using namespace llvm::ELF;
using namespace llvm::dwarf;
using namespace llvm::object;
using namespace lld;
using namespace lld::elf;

namespace {
class EhReader {
public:
  EhReader(InputSectionBase *s, ArrayRef<uint8_t> d) : isec(s), d(d) {}
  uint8_t getFdeEncoding();
  bool hasLSDA();

private:
  template <class P> void errOn(const P *loc, const Twine &msg) {
    Ctx &ctx = isec->file->ctx;
    Err(ctx) << "corrupted .eh_frame: " << msg << "\n>>> defined in "
             << isec->getObjMsg((const uint8_t *)loc - isec->content().data());
  }

  uint8_t readByte();
  void skipBytes(size_t count);
  StringRef readString();
  void skipLeb128();
  void skipAugP();
  StringRef getAugmentation();

  InputSectionBase *isec;
  ArrayRef<uint8_t> d;
};
}

// Read a byte and advance D by one byte.
uint8_t EhReader::readByte() {
  if (d.empty()) {
    errOn(d.data(), "unexpected end of CIE");
    return 0;
  }
  uint8_t b = d.front();
  d = d.slice(1);
  return b;
}

void EhReader::skipBytes(size_t count) {
  if (d.size() < count)
    errOn(d.data(), "CIE is too small");
  else
    d = d.slice(count);
}

// Read a null-terminated string.
StringRef EhReader::readString() {
  const uint8_t *end = llvm::find(d, '\0');
  if (end == d.end()) {
    errOn(d.data(), "corrupted CIE (failed to read string)");
    return {};
  }
  StringRef s = toStringRef(d.slice(0, end - d.begin()));
  d = d.slice(s.size() + 1);
  return s;
}

// Skip an integer encoded in the LEB128 format.
// Actual number is not of interest because only the runtime needs it.
// But we need to be at least able to skip it so that we can read
// the field that follows a LEB128 number.
void EhReader::skipLeb128() {
  const uint8_t *errPos = d.data();
  while (!d.empty()) {
    uint8_t val = d.front();
    d = d.slice(1);
    if ((val & 0x80) == 0)
      return;
  }
  errOn(errPos, "corrupted CIE (failed to read LEB128)");
}

static size_t getAugPSize(Ctx &ctx, unsigned enc) {
  switch (enc & 0x0f) {
  case DW_EH_PE_absptr:
  case DW_EH_PE_signed:
    return ctx.arg.wordsize;
  case DW_EH_PE_udata2:
  case DW_EH_PE_sdata2:
    return 2;
  case DW_EH_PE_udata4:
  case DW_EH_PE_sdata4:
    return 4;
  case DW_EH_PE_udata8:
  case DW_EH_PE_sdata8:
    return 8;
  }
  return 0;
}

void EhReader::skipAugP() {
  uint8_t enc = readByte();
  if ((enc & 0xf0) == DW_EH_PE_aligned)
    return errOn(d.data() - 1, "DW_EH_PE_aligned encoding is not supported");
  size_t size = getAugPSize(isec->getCtx(), enc);
  if (size == 0)
    return errOn(d.data() - 1, "unknown FDE encoding");
  if (size >= d.size())
    return errOn(d.data() - 1, "corrupted CIE");
  d = d.slice(size);
}

uint8_t elf::getFdeEncoding(EhSectionPiece *p) {
  return EhReader(p->sec, p->data()).getFdeEncoding();
}

bool elf::hasLSDA(const EhSectionPiece &p) {
  return EhReader(p.sec, p.data()).hasLSDA();
}

StringRef EhReader::getAugmentation() {
  skipBytes(8);
  int version = readByte();
  if (version != 1 && version != 3) {
    errOn(d.data() - 1,
          "FDE version 1 or 3 expected, but got " + Twine(version));
    return {};
  }

  StringRef aug = readString();

  // Skip code and data alignment factors.
  skipLeb128();
  skipLeb128();

  // Skip the return address register. In CIE version 1 this is a single
  // byte. In CIE version 3 this is an unsigned LEB128.
  if (version == 1)
    readByte();
  else
    skipLeb128();
  return aug;
}

uint8_t EhReader::getFdeEncoding() {
  // We only care about an 'R' value, but other records may precede an 'R'
  // record. Unfortunately records are not in TLV (type-length-value) format,
  // so we need to teach the linker how to skip records for each type.
  StringRef aug = getAugmentation();
  for (char c : aug) {
    if (c == 'R')
      return readByte();
    if (c == 'z')
      skipLeb128();
    else if (c == 'L')
      readByte();
    else if (c == 'P')
      skipAugP();
    else if (c != 'B' && c != 'S' && c != 'G') {
      errOn(aug.data(), "unknown .eh_frame augmentation string: " + aug);
      break;
    }
  }
  return DW_EH_PE_absptr;
}

bool EhReader::hasLSDA() {
  StringRef aug = getAugmentation();
  for (char c : aug) {
    if (c == 'L')
      return true;
    if (c == 'z')
      skipLeb128();
    else if (c == 'P')
      skipAugP();
    else if (c == 'R')
      readByte();
    else if (c != 'B' && c != 'S' && c != 'G') {
      errOn(aug.data(), "unknown .eh_frame augmentation string: " + aug);
      break;
    }
  }
  return false;
}
#else /* OHOS_LLVM */
//===- EhFrame.cpp -------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//


#include "EhFrame.h"
#include "InputFiles.h"
#include "InputSection.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/DebugInfo/DWARF/LowLevel/DWARFDataExtractorSimple.h"

using namespace llvm;
using namespace llvm::dwarf;

namespace lld {
namespace elf {

namespace {
class EhCieReader {
public:
  EhCieReader(const EhSectionPiece &cie);

  bool hasLSDA();
  EhPointerEncodings getEhPointerEncodings();

private:
  void failOnCursorPos(const Twine &msg);

  void parseUntilAugmentationString();
  void parseAll();

  const EhSectionPiece &cie;
  Ctx &ctx;
  DWARFDataExtractorSimple dataExtractor;
  DWARFDataExtractorSimple::Cursor cursor = DWARFDataExtractorSimple::Cursor(0);

  uint8_t version;
  StringRef augmentationString;

  EhPointerEncodings encodings;
};

EhCieReader::EhCieReader(const EhSectionPiece &cie)
    : cie(cie), ctx(cie.sec->file->ctx),
      dataExtractor(cie.data(), ctx.arg.isLE, ctx.arg.wordsize) {}

bool EhCieReader::hasLSDA() {
  parseUntilAugmentationString();
  return augmentationString.contains('L');
}

EhPointerEncodings EhCieReader::getEhPointerEncodings() {
  parseAll();
  return encodings;
}

void EhCieReader::failOnCursorPos(const Twine &msg) {
  Err(ctx) << "malformed CIE in .eh_frame: " << msg << "\n>>> defined in "
           << cie.sec->getObjMsg(cie.data().data() + cursor.tell() -
                                 cie.data().data());
}

void EhCieReader::parseUntilAugmentationString() {
  assert(cursor.tell() == 0);

  dataExtractor.getU32(cursor);
  uint32_t id = dataExtractor.getU32(cursor);
  if (!cursor)
    failOnCursorPos("corrupted length or id");

  if (id != 0)
    failOnCursorPos("id must be 0, got " + Twine(id));

  version = dataExtractor.getU8(cursor);
  if (!cursor)
    failOnCursorPos("corrupted version");
  if (version != 1 && version != 3)
    failOnCursorPos("version must be 1 or 3, got " +
                    Twine(static_cast<int>(version)));

  augmentationString = dataExtractor.getCStrRef(cursor);
  if (!cursor)
    failOnCursorPos("corrupted augmentation string");
}

void EhCieReader::parseAll() {
  parseUntilAugmentationString();

  dataExtractor.getULEB128(cursor);
  dataExtractor.getSLEB128(cursor);
  if (!cursor)
    failOnCursorPos("corrupted code or data align factor");

  if (version == 1)
    dataExtractor.getU8(cursor);
  else
    dataExtractor.getULEB128(cursor);
  if (!cursor)
    failOnCursorPos("corrupted ret address reg");

  if (augmentationString.empty())
    return;

  uint64_t augmentationSectionBegin = 0;
  uint64_t augmentationSectionExpectedSize = 0;
  if (augmentationString.front() == 'z') {
    augmentationSectionExpectedSize = dataExtractor.getULEB128(cursor);
    if (!cursor)
      failOnCursorPos("corrupted augmentation section size");
    augmentationSectionBegin = cursor.tell();
    augmentationString = augmentationString.slice(1, StringRef::npos);
  }

  auto helper = [this](EhPointerEncoding &encoding, char c) {
    if (encoding.offsetInCie != size_t(-1))
      failOnCursorPos("duplicate occurrance of " + Twine(c) +
                      " in augmentation string \"" + augmentationString + "\"");
    encoding.offsetInCie = cursor.tell();
    encoding.encoding = dataExtractor.getU8(cursor);
  };

  for (char c : augmentationString) {
    switch (c) {
    case 'R':
      helper(encodings.fdeEncoding, c);
      break;
    case 'L':
      helper(encodings.lsdaEncoding, c);
      break;
    case 'P':
      helper(encodings.personalityEncoding, c);
      if (!dataExtractor.getRawEncodedPointer(
              cursor, encodings.personalityEncoding.encoding))
        failOnCursorPos(
            "cannot get personality pointer for personality encoding " +
            Twine(static_cast<int>(encodings.personalityEncoding.encoding)));
      if ((encodings.personalityEncoding.encoding & 0xf0) == DW_EH_PE_aligned)
        failOnCursorPos(
            "DW_EH_PE_aligned personality encoding is not supported");
      break;
    case 'B':
    case 'S':
    case 'G':
      break;
    default:
      failOnCursorPos("unexpected character in CIE augmentation string: " +
                      augmentationString);
    }
    if (!cursor)
      failOnCursorPos("corrupted CIE augmentation section");
  }

  uint64_t augmentationSectionActualSize =
      cursor.tell() - augmentationSectionBegin;

  if (augmentationSectionBegin != 0 &&
      augmentationSectionActualSize != augmentationSectionExpectedSize)
    failOnCursorPos("augmentation section size " +
                    Twine(augmentationSectionExpectedSize) +
                    " does not match the actual size " +
                    Twine(augmentationSectionActualSize));
}
} // namespace

EhPointerEncodings getEhPointerEncodings(const EhSectionPiece &cie) {
  return EhCieReader(cie).getEhPointerEncodings();
}

bool hasLSDA(const EhSectionPiece &cie) {
  return EhCieReader(cie).hasLSDA();
}

} // namespace elf
} // namespace lld

#endif /* OHOS_LLVM */
