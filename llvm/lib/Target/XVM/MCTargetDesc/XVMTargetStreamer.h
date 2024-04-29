//=====-- XVMTargetStreamer.h - XVM Target Streamer ------*- C++ -*--=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMTARGETSTREAMER_H
#define LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {
class MCSection;

/// Implments XVM-specific streamer.
class XVMTargetStreamer : public MCTargetStreamer {

public:
  XVMTargetStreamer(MCStreamer &S);
  ~XVMTargetStreamer() override;

  void changeSection(const MCSection *CurSection, MCSection *Section,
                     const MCExpr *SubSection, raw_ostream &OS) override;
};

} // end namespace llvm

#endif
