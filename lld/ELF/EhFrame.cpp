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

// OHOS_LOCAL begin

#include "EhFrame.h"
#include "Config.h"
#include "InputSection.h"
#include "lld/Common/ErrorHandler.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/DebugInfo/DWARF/DWARFDataExtractor.h"

using namespace llvm;

namespace lld {
namespace elf {
namespace {

// The reason for having a class for parsing a EH CIE frame instead of a single
// function is need for hasLSDA() used by ICF<ELFT>::handleLSDA(). We'd better
// parse each CIE only once but at this point the EhFrameSection::cieRecords
// vector is not filled yet (it's done in EhFrameSection::finalizeContents()
// later). So, to avoid parsing unneeded CIE parts, we split parsing into 2
// phases, and use a class just to save internal parsing state between them.
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
  DWARFDataExtractor dataExtractor;
  DWARFDataExtractor::Cursor cursor = DWARFDataExtractor::Cursor(0);

  uint8_t version;
  StringRef augmentationString;

  EhPointerEncodings encodings;
};

EhCieReader::EhCieReader(const EhSectionPiece &cie)
    : cie(cie), dataExtractor(
                    /* Data = */ cie.data(),
                    /* IsLittleEndian = */ config->isLE,
                    /* AddressSize = */ config->wordsize) {}

bool EhCieReader::hasLSDA() {
  parseUntilAugmentationString();
  return augmentationString.contains('L');
}

EhPointerEncodings EhCieReader::getEhPointerEncodings() {
  parseAll();
  return encodings;
}

void EhCieReader::failOnCursorPos(const Twine &msg) {
  fatal("malformed CIE in .eh_frame: " + msg + "\n>>> defined in " +
        cie.sec->getObjMsg(cie.data().data() + cursor.tell() -
                           cie.sec->data().data()));
}

void EhCieReader::parseUntilAugmentationString() {
  // Parsing is intended to be run only once.
  assert(cursor.tell() == 0);

  /* length = */ dataExtractor.getU32(cursor);
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

  // Skip ret address reg
  if (version == 1)
    dataExtractor.getU8(cursor);
  else
    dataExtractor.getULEB128(cursor);
  if (!cursor)
    failOnCursorPos("corrupted ret address reg");

  if (augmentationString.empty())
    return;

  uint64_t augmentationSectionBegin = 0;
  uint64_t augmentationSectionExpectedSize;
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
      if (dataExtractor.getRawEncodedPointer(
              cursor, encodings.personalityEncoding.encoding) == None)
        failOnCursorPos(
            "cannot get personality pointer for personality encoding " +
            Twine(static_cast<int>(encodings.personalityEncoding.encoding)));
      if ((encodings.personalityEncoding.encoding & 0xf0) ==
          dwarf::DW_EH_PE_aligned)
        failOnCursorPos(
            "DW_EH_PE_aligned personality encoding is not supported");
      break;
    case 'B':
      // B-Key is used for signing functions associated with this
      // augmentation string
    case 'S':
      // Current frame is a signal trampoline.
    case 'G':
      // This stack frame contains MTE tagged data, so needs to be
      // untagged on unwind.
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
                    "does not match the actual size " +
                    Twine(augmentationSectionActualSize));
}

} // namespace

EhPointerEncodings getEhPointerEncodings(const EhSectionPiece &cie) {
  return EhCieReader(cie).getEhPointerEncodings();
}

bool hasLSDA(const EhSectionPiece &cie) { return EhCieReader(cie).hasLSDA(); }

} // namespace elf
} // namespace lld

// OHOS_LOCAL end
