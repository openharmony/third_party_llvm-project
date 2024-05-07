//===-- XVMMCTargetDesc.h - XVM Target Descriptions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides XVM specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMMCTARGETDESC_H
#define LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMMCTARGETDESC_H

#include "llvm/Config/config.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#include <memory>

namespace llvm {
class MCContext;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

}

// Defines symbolic names for XVM registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "XVMGenRegisterInfo.inc"

// Defines symbolic names for the XVM instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "XVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "XVMGenSubtargetInfo.inc"

#endif
