//===-- XVMInstPrinter.h - Convert XVM MCInst to asm syntax -------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints a XVM MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMINSTPRINTER_H
#define LLVM_LIB_TARGET_XVM_MCTARGETDESC_XVMINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {
class XVMInstPrinter : public MCInstPrinter {
public:
  XVMInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                 const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;
  void printOperand(const MCInst *MInst, unsigned OpNum, raw_ostream &OStream,
                    const char *Mod = nullptr);
  void printMemOperand(const MCInst *MInst, int OpNum, raw_ostream &OStream,
                       const char *Mod  = nullptr);
  void printImm64Operand(const MCInst *MInst, unsigned OpNum, raw_ostream &OStream);
  void printBrTargetOperand(const MCInst *MInst, unsigned OpNum, raw_ostream &OStream);

  void printCallInstructionImm(const MCInst *MI, raw_ostream &O);
  void printCallInstructionReg(const MCInst *MI, raw_ostream &O);
  void printMovWithFuncID(const MCInst *MI, raw_ostream &O);
  void printDataRefWithGlobalID(const MCInst *MI, raw_ostream &O);

  // Autogenerated by tblgen.
  std::pair<const char *, uint64_t> getMnemonic(const MCInst *MI) override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  static const char *getRegisterName(unsigned RegNo);
};
}

#endif
