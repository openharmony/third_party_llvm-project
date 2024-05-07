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
using namespace llvm;
using namespace std;
#define DEBUG_TYPE "asm-printer"
#define GET_INSTRINFO_CTOR_DTOR

// Include the auto-generated portion of the assembly writer.
#include "XVMGenAsmWriter.inc"

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
  switch(MCInstFlag) {
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
  assert(MI->getNumOperands() >= 2);
  O << "\t";
  auto MnemonicInfo = getMnemonic(MI);
  O << MnemonicInfo.first;

  assert(MI->getOperand(1).isImm());
  O << MI->getOperand(1).getImm();
}

void XVMInstPrinter::printMovWithFuncID(const MCInst *MI, raw_ostream &O) {
  const MCOperand & Op0 = MI->getOperand(0);
  assert(Op0.isReg());
  const MCOperand & Op1 = MI->getOperand(1);
  assert(Op1.isExpr());
  const MCOperand & Op2 = MI->getOperand(2);
  assert(Op2.isImm());
  O << "\t";
  O << "mov " << getRegisterName(Op0.getReg()) << ", #" << Op2.getImm();
}

void XVMInstPrinter::printDataRefWithGlobalID(const MCInst *MI, raw_ostream &O) {
  assert(MI->getNumOperands() >= 3);
  const MCOperand & Op0 = MI->getOperand(0);
  assert(Op0.isReg());
  const MCOperand & Op1 = MI->getOperand(1);
  assert(Op1.isExpr());
  const MCOperand & Op2 = MI->getOperand(2);
  assert(Op2.isImm());
  O << "\t";
  O << "dataref " << getRegisterName(Op0.getReg()) << ", #" << Op2.getImm();
}

void XVMInstPrinter::printCallInstructionReg(const MCInst *MI, raw_ostream &O) {
  assert(MI->getNumOperands() >= 2);
  O << "\t";
  auto MnemonicInfo = getMnemonic(MI);
  O << MnemonicInfo.first;

  assert(MI->getOperand(1).isReg());
  O << getRegisterName(MI->getOperand(1).getReg());
}

static void printExpr(const MCExpr *Expr, raw_ostream &O) {
#ifndef NDEBUG
  const MCSymbolRefExpr *SRE;

  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr))
    SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
  else
    SRE = dyn_cast<MCSymbolRefExpr>(Expr);
  assert(SRE && "Unexpected MCExpr type.");

  MCSymbolRefExpr::VariantKind Kind = SRE->getKind();

  assert(Kind == MCSymbolRefExpr::VK_None);
#endif
  O << *Expr;
}

void XVMInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    O << formatImm((int32_t)Op.getImm());
  } else {
    assert(Op.isExpr() && "Expected an expression");
    printExpr(Op.getExpr(), O);
  }
}

void XVMInstPrinter::printMemOperand(const MCInst *MI, int OpNo, raw_ostream &O,
                                     const char *Modifier) {
  const MCOperand &RegOp = MI->getOperand(OpNo);
  const MCOperand &OffsetOp = MI->getOperand(OpNo + 1);

  // register
  assert(RegOp.isReg() && "Register operand not a register");
  O << getRegisterName(RegOp.getReg());

  // offset
  if (OffsetOp.isImm()) {
    auto Imm = OffsetOp.getImm();
    if (Imm == 0)
      O << ", #" << formatImm(Imm);
    else
      O << ", #" << formatImm(Imm);
  } else {
    assert(0 && "Expected an immediate");
  }
}

void XVMInstPrinter::printImm64Operand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm())
    O << formatImm(Op.getImm());
  else if (Op.isExpr())
    printExpr(Op.getExpr(), O);
  else
    O << Op;
}

void XVMInstPrinter::printBrTargetOperand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    int16_t Imm = Op.getImm();
    O << ((Imm >= 0) ? "+" : "") << formatImm(Imm);
  } else if (Op.isExpr()) {
    printExpr(Op.getExpr(), O);
  } else {
    O << Op;
  }
}

#endif
