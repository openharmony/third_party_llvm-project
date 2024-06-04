//=====- XVMTargetStreamer.cpp - XVMTargetStreamer class ------------=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the XVMTargetStreamer class.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVMTargetStreamer.h"

using namespace llvm;

//
// XVMTargetStreamer Implemenation
//
XVMTargetStreamer::XVMTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

XVMTargetStreamer::~XVMTargetStreamer() = default;

void XVMTargetStreamer::changeSection(const MCSection *CurSection,
                                      MCSection *Section,
                                      const MCExpr *SubSection,
                                      raw_ostream &OS) {
}

#endif
