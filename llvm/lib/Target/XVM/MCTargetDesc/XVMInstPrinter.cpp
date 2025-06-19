//===-- XVMInstPrinter.cpp - Convert XVM MCInst to asm syntax -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an XVM MCInst to a .s file.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "../XVM_def.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#define DEBUG_TYPE "asm-printer"
#define GET_INSTRINFO_CTOR_DTOR

// Include the auto-generated portion of the assembly writer.
#include "XVMGenAsmWriter.inc"

using namespace llvm;
using namespace std;

#define MIN_NUM_MO_CALL_INSTR 2
#define MIN_NUM_MO_DATA_REF_INSTR 3

void XVMInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                               StringRef Annot, const MCSubtargetInfo &STI,
                               raw_ostream &O) {
  unsigned int MCInstFlag = MI->getFlags();
  MCOperand MCOp = MI->getOperand(MI->getNumOperands() - 1);
  if (MCOp.isImm()) {
    unsigned Indent = MCOp.getImm();
    for (unsigned i = 0; i <= Indent; ++i) {
      O << "\t";
    }
  }
  switch (MCInstFlag) {
    default:
      printInstruction(MI, Address, O);
      break;
    case FUNC_CALL_FLAG_MC_INST_IMM:
      printCallInstructionImm(MI, O);
      break;
    case FUNC_CALL_FLAG_MC_INST_REG:
      printCallInstructionReg(MI, O);
      break;
    case FUNC_ID_FLAG_MC_INST_REG:
      printMovWithFuncID(MI, O);
      break;
    case GLOBAL_DATAREF_FLAG_MC_INST:
      printDataRefWithGlobalID(MI, O);
      break;
  }
  printAnnotation(O, Annot);
}

void XVMInstPrinter::printCallInstructionImm(const MCInst *MI, raw_ostream &O) {
  assert(MI->getNumOperands() >= MIN_NUM_MO_CALL_INSTR);
  O << "\t";
  auto MnemonicInfo = getMnemonic(MI);
  O << MnemonicInfo.first;

  assert(MI->getOperand(1).isImm());
  O << MI->getOperand(1).getImm();
}

void XVMInstPrinter::printMovWithFuncID(const MCInst *MI, raw_ostream &O) {
  const MCOperand &Op0 = MI->getOperand(0);
  assert(Op0.isReg());
  const MCOperand &Op1 = MI->getOperand(1);
  assert(Op1.isExpr());
  const MCOperand &Op2 = MI->getOperand(2);
  assert(Op2.isImm());
  O << "\t";
  O << "mov " << getRegisterName(Op0.getReg()) << ", #" << Op2.getImm();
}

void XVMInstPrinter::printDataRefWithGlobalID(const MCInst *MI, raw_ostream &O) {
  assert(MI->getNumOperands() >= MIN_NUM_MO_DATA_REF_INSTR);
  const MCOperand &Op0 = MI->getOperand(0);
  assert(Op0.isReg());
  const MCOperand &Op1 = MI->getOperand(1);
  assert(Op1.isExpr());
  const MCOperand &Op2 = MI->getOperand(2);
  assert(Op2.isImm());
  O << "\t";
  O << "dataref " << getRegisterName(Op0.getReg()) << ", #" << Op2.getImm();
}

void XVMInstPrinter::printCallInstructionReg(const MCInst *MI, raw_ostream &O) {
  assert(MI->getNumOperands() >= MIN_NUM_MO_CALL_INSTR);
  O << "\t";
  auto MnemonicInfo = getMnemonic(MI);
  O << MnemonicInfo.first;

  assert(MI->getOperand(1).isReg());
  O << getRegisterName(MI->getOperand(1).getReg());
}

static void printExpr(const MCExpr *Expression, raw_ostream &OStream) {
#ifndef NDEBUG
  const MCSymbolRefExpr *SRExpr;

  if (const MCBinaryExpr *BExpr = dyn_cast<MCBinaryExpr>(Expression))
    SRExpr = dyn_cast<MCSymbolRefExpr>(BExpr->getLHS());
  else
    SRExpr = dyn_cast<MCSymbolRefExpr>(Expression);
  assert(SRExpr && "Unexpected MCExpr type.");

  MCSymbolRefExpr::VariantKind VarKind = SRExpr->getKind();

  assert(VarKind == MCSymbolRefExpr::VK_None);
#endif
  OStream << *Expression;
}

void XVMInstPrinter::printOperand(const MCInst *MInst, unsigned OpNum,
                                  raw_ostream &OStream, const char *Mod) {
  assert((Mod == nullptr || Mod[0] == 0) && "No modifiers supported");
  const MCOperand &Oper = MInst->getOperand(OpNum);
  if (Oper.isReg()) {
    OStream << getRegisterName(Oper.getReg());
  } else if (Oper.isImm()) {
    OStream << formatImm((int32_t)Oper.getImm());
  } else {
    assert(Oper.isExpr() && "Expected an expression");
    printExpr(Oper.getExpr(), OStream);
  }
}

void XVMInstPrinter::printMemOperand(const MCInst *MInst, int OpNum,
                                     raw_ostream &OStream, const char *Mod) {
  const MCOperand &ROp = MInst->getOperand(OpNum);
  const MCOperand &OffOp = MInst->getOperand(OpNum + 1);

  // register
  assert(ROp.isReg() && "Register operand not a register");
  OStream << getRegisterName(ROp.getReg());

  // offset
  if (OffOp.isImm()) {
    auto I = OffOp.getImm();
    if (I == 0)
      OStream << ", #" << formatImm(I);
    else
      OStream << ", #" << formatImm(I);
  } else {
    assert(0 && "Expected an immediate");
  }
}

void XVMInstPrinter::printImm64Operand(const MCInst *MInst, unsigned OpNum, raw_ostream &OStream) {
  const MCOperand &Oper = MInst->getOperand(OpNum);
  if (Oper.isImm())
    OStream << formatImm(Oper.getImm());
  else if (Oper.isExpr())
    printExpr(Oper.getExpr(), OStream);
  else
    OStream << Oper;
}

void XVMInstPrinter::printBrTargetOperand(const MCInst *MInst, unsigned OpNum, raw_ostream &OStream) {
  const MCOperand &Oper = MInst->getOperand(OpNum);
  if (Oper.isImm()) {
    int16_t I = Oper.getImm();
    OStream << ((I >= 0) ? "+" : "") << formatImm(I);
  } else if (Oper.isExpr()) {
    printExpr(Oper.getExpr(), OStream);
  } else {
    OStream << Oper;
  }
}

#endif
