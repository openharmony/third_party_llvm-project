//===-- XVMMCAsmInfo.h - XVM asm properties -------------------*- C++ -*--====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the XVMMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMMCASMINFO_H
#define LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMMCASMINFO_H

#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class XVMMCAsmInfo : public MCAsmInfo {
public:
  explicit XVMMCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {

    PrivateGlobalPrefix = ".L";
    WeakRefDirective = "\t.weak\t";
    CommentString = ";;";

    UsesELFSectionDirectiveForBSS = true;
    HasSingleParameterDotFile = false;
    HasDotTypeDotSizeDirective = false;

    SupportsDebugInformation = true;
    ExceptionsType = ExceptionHandling::DwarfCFI;
    MinInstAlignment = 4;
    CommentString = ";;";

    // the default is 4 and it only affects dwarf elf output
    // so if not set correctly, the dwarf data will be
    // messed up in random places by 4 bytes. .debug_line
    // section will be parsable, but with odd offsets and
    // line numbers, etc.
    CodePointerSize = 8;
  }

  void setDwarfUsesRelocationsAcrossSections(bool enable) {
    DwarfUsesRelocationsAcrossSections = enable;
  }
  bool shouldOmitSectionDirective(StringRef SectionName) const override {
    return true;
  }
};
}

#endif
