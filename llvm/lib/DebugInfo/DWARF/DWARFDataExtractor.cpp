//===- DWARFDataExtractor.cpp ---------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/DWARF/DWARFDataExtractor.h"
#include "llvm/DebugInfo/DWARF/DWARFObject.h"
#include "llvm/DebugInfo/DWARF/DWARFRelocMap.h"
#include "llvm/Support/Errc.h"

using namespace llvm;

std::pair<uint64_t, dwarf::DwarfFormat>
DWARFDataExtractor::getInitialLength(uint64_t *Off, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  if (Err && *Err)
    return {0, dwarf::DWARF32};

  Cursor C(*Off);
  uint64_t Length = getRelocatedValue(C, 4);
  dwarf::DwarfFormat Format = dwarf::DWARF32;
  if (Length == dwarf::DW_LENGTH_DWARF64) {
    Length = getRelocatedValue(C, 8);
    Format = dwarf::DWARF64;
  } else if (Length >= dwarf::DW_LENGTH_lo_reserved) {
    cantFail(C.takeError());
    if (Err)
      *Err = createStringError(
          errc::invalid_argument,
          "unsupported reserved unit length of value 0x%8.8" PRIx64, Length);
    return {0, dwarf::DWARF32};
  }

  if (C) {
    *Off = C.tell();
    return {Length, Format};
  }
  if (Err)
    *Err = C.takeError();
  else
    consumeError(C.takeError());
  return {0, dwarf::DWARF32};
}

uint64_t DWARFDataExtractor::getRelocatedValue(uint32_t Size, uint64_t *Off,
                                               uint64_t *SecNdx,
                                               Error *Err) const {
  if (SecNdx)
    *SecNdx = object::SectionedAddress::UndefSection;
  if (!Section)
    return getUnsigned(Off, Size, Err);

  ErrorAsOutParameter ErrAsOut(Err);
  Optional<RelocAddrEntry> E = Obj->find(*Section, *Off);
  uint64_t LocData = getUnsigned(Off, Size, Err);
  if (!E || (Err && *Err))
    return LocData;
  if (SecNdx)
    *SecNdx = E->SectionIndex;

  uint64_t R =
      object::resolveRelocation(E->Resolver, E->Reloc, E->SymbolValue, LocData);
  if (E->Reloc2)
    R = object::resolveRelocation(E->Resolver, *E->Reloc2, E->SymbolValue2, R);
  return R;
}


// OHOS_LOCAL begin

Optional<uint64_t>
DWARFDataExtractor::getRawEncodedPointer(Cursor &C, uint8_t Encoding) const {
  if (Encoding == dwarf::DW_EH_PE_omit)
    return None;

  switch (Encoding & 0x0F) {
  case dwarf::DW_EH_PE_absptr:
  case dwarf::DW_EH_PE_signed:
    switch (getAddressSize()) {
    case 2:
    case 4:
    case 8:
      return dwarf::DW_EH_PE_absptr == (Encoding & 0x0F)
                 ? getUnsigned(C, getAddressSize())
                 : getSigned(C, getAddressSize());
    default:
      return None;
    }
  case dwarf::DW_EH_PE_uleb128:
    return getULEB128(C);
  case dwarf::DW_EH_PE_sleb128:
    return getSLEB128(C);
  case dwarf::DW_EH_PE_udata2:
    return getUnsigned(C, 2);
  case dwarf::DW_EH_PE_udata4:
    return getUnsigned(C, 4);
  case dwarf::DW_EH_PE_udata8:
    return getUnsigned(C, 8);
  case dwarf::DW_EH_PE_sdata2:
    return getSigned(C, 2);
  case dwarf::DW_EH_PE_sdata4:
    return SignExtend64<32>(getRelocatedValue(C, 4));
  case dwarf::DW_EH_PE_sdata8:
    return getRelocatedValue(C, 8);
  default:
    return None;
  }
}

Optional<uint64_t>
DWARFDataExtractor::getRawEncodedPointer(uint64_t *Offset,
                                         uint8_t Encoding) const {
  DWARFDataExtractor::Cursor C(*Offset);
  Optional<uint64_t> Ret = getRawEncodedPointer(C, Encoding);
  if (!C)
    return None;
  *Offset = C.tell();
  return Ret;
}

Optional<uint64_t>
DWARFDataExtractor::getEncodedPointer(uint64_t *Offset, uint8_t Encoding,
                                      uint64_t PCRelOffset) const {
  uint64_t OldOffset = *Offset;

  // First get value
  Optional<uint64_t> Result = getRawEncodedPointer(Offset, Encoding);
  if (Result == None)
    return None;

  // Then add relative offset, if required
  switch (Encoding & 0x70) {
  case dwarf::DW_EH_PE_absptr:
    // do nothing
    break;
  case dwarf::DW_EH_PE_pcrel:
    *Result += PCRelOffset;
    break;
  case dwarf::DW_EH_PE_datarel:
  case dwarf::DW_EH_PE_textrel:
  case dwarf::DW_EH_PE_funcrel:
  case dwarf::DW_EH_PE_aligned:
  default:
    *Offset = OldOffset;
    return None;
  }

  return Result;
}

// OHOS_LOCAL end
