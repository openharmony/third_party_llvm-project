//===-- XVMErrorMsg.h - Error Message interface for XVM Backend ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------------===//

#ifndef __XVM__ERROR_MSG__HH__
#define __XVM__ERROR_MSG__HH__

#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace llvm;

static inline void ExportFailMsg(const Function &Func, DebugLoc DL, const char *Msg, void *Val) {
  std::string Str;
  raw_string_ostream OS(Str);
  OS << Msg;
  if (Val != NULL) {
    uint64_t *ValToPrint = (uint64_t *)Val;
    OS << ":" << *ValToPrint;
  }
  Func.getContext().diagnose(DiagnosticInfoUnsupported(Func, Str, DL));
}

#endif // __XVM__ERROR_MSG__HH__
