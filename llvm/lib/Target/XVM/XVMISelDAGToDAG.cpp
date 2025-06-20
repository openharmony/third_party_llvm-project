//===-- XVMISelDAGToDAG.cpp - A dag to dag inst selector for XVM ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a DAG pattern matching instruction selector for XVM,
// converting from a legalized dag to a XVM dag.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVM.h"
#include "XVMRegisterInfo.h"
#include "XVMSubtarget.h"
#include "XVMTargetMachine.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "xvm-isel"
#define TEMPLATE_INT_16 16

// Instruction Selector Implementation
namespace {

class XVMDAGToDAGISel : public SelectionDAGISel {
  /// Subtarget - Keep a pointer to the XVMSubtarget around so that we can
  /// make the right decision when generating code for different subtargets.
  const XVMSubtarget *Subtarget;

public:
  explicit XVMDAGToDAGISel(XVMTargetMachine &TM)
      : SelectionDAGISel(TM), Subtarget(nullptr) {}

  StringRef getPassName() const override {
    return "XVM DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MFunc) override {
    LLVM_DEBUG(dbgs() << "********** XVMDAGToDAGISel **********\n"
                         "********** Function: "
                      << MFunc.getName() << '\n');
    // Reset the subtarget each time through.
    Subtarget = &MFunc.getSubtarget<XVMSubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MFunc);
  }

  void PreprocessISelDAG() override;

  bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintCode,
                                    std::vector<SDValue> &OutOps) override;

private:
// The pieces autogenerated from the target description are included.
#include "XVMGenDAGISel.inc"

  void Select(SDNode *N) override;

  // Complex Pattern for address selection.
  bool SelectAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
  bool SelectFIAddr(SDValue Addr, SDValue &Base, SDValue &Offset);
};
} // namespace

// ComplexPattern used on XVM Load/Store instructions
bool XVMDAGToDAGISel::SelectAddr(SDValue Address, SDValue &Bas, SDValue &Offs) {
  // if Address is FI, get the TargetFrameIndex.
  SDLoc DL(Address);
  if (auto *FIN = dyn_cast<FrameIndexSDNode>(Address)) {
    Bas = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
    Offs = CurDAG->getTargetConstant(0, DL, MVT::i64);
    return true;
  }

  if (Address.getOpcode() == ISD::TargetExternalSymbol ||
      Address.getOpcode() == ISD::TargetGlobalAddress)
    return false;

  // Addresses of the form Addr+const or Addr|const
  if (CurDAG->isBaseWithConstantOffset(Address)) {
    auto *CN = cast<ConstantSDNode>(Address.getOperand(1));
    if (isInt<TEMPLATE_INT_16>(CN->getSExtValue())) {
      // If the first operand is a FI, get the TargetFI Node
      if (auto *FIN = dyn_cast<FrameIndexSDNode>(Address.getOperand(0)))
        Bas = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
      else
        Bas = Address.getOperand(0);

      Offs = CurDAG->getTargetConstant(CN->getSExtValue(), DL, MVT::i64);
      return true;
    }
  }

  Bas = Address;
  Offs = CurDAG->getTargetConstant(0, DL, MVT::i64);
  return true;
}

// ComplexPattern used on XVM FI instruction
bool XVMDAGToDAGISel::SelectFIAddr(SDValue Addr, SDValue &Base, SDValue &Offs) {
  SDLoc DL(Addr);

  if (CurDAG->isBaseWithConstantOffset(Addr)) {
    auto *CN = cast<ConstantSDNode>(Addr.getOperand(1));
    if (isInt<TEMPLATE_INT_16>(CN->getSExtValue())) {
      if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr.getOperand(0))) {
        Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
        Offs = CurDAG->getTargetConstant(CN->getSExtValue(), DL, MVT::i64);
        return true;
      }
    }
  }
  return false;
}

bool XVMDAGToDAGISel::SelectInlineAsmMemoryOperand(
    const SDValue &Operand, unsigned ConstraintCode, std::vector<SDValue> &OutOps) {
  bool ret = false;
  SDValue Operand0, Operand1;
  switch (ConstraintCode) {
  case InlineAsm::Constraint_m: // memory
    if (!SelectAddr(Operand, Operand0, Operand1)) {
      ret = true;
    }
    break;
  default:
    ret = true;
  }

  if (ret == false) {
    SDLoc DL(Operand);
    SDValue AluOperand = CurDAG->getTargetConstant(ISD::ADD, DL, MVT::i32);
    OutOps.push_back(Operand0);
    OutOps.push_back(Operand1);
    OutOps.push_back(AluOperand);
  }

  return ret;
}

void XVMDAGToDAGISel::Select(SDNode *CNode) {
  unsigned Op = CNode->getOpcode();

  // We have already selected if we have a custom node
  if (CNode->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== ";
    CNode->dump(CurDAG); dbgs() << '\n');
    return;
  }

  switch (Op) { // handle table gen selection
  case ISD::FrameIndex:
  {
    int FI = cast<FrameIndexSDNode>(CNode)->getIndex();
    EVT VT = CNode->getValueType(0);
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, VT);
    unsigned Opc = XVM::MOV_rr;
    if (CNode->hasOneUse())
      CurDAG->SelectNodeTo(CNode, Opc, VT, TFI);
    else
      ReplaceNode(CNode, CurDAG->getMachineNode(Opc, SDLoc(CNode), VT, TFI));
  }
    break;
  default:
    SelectCode(CNode); // select default instr
  }
  return;
}

void XVMDAGToDAGISel::PreprocessISelDAG() {
}

FunctionPass *llvm::createXVMISelDag(XVMTargetMachine &TM) {
  return new XVMDAGToDAGISel(TM);
}

#endif
