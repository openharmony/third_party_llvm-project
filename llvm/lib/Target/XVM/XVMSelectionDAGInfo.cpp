//===-- XVMSelectionDAGInfo.cpp - XVM SelectionDAG Info -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the XVMSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVMTargetMachine.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/DerivedTypes.h"
using namespace llvm;

#define DEBUG_TYPE "XVM-selectiondag-info"

SDValue XVMSelectionDAGInfo::EmitTargetCodeForMemcpy(
    SelectionDAG &DAG, const SDLoc &dl, SDValue Chain, SDValue Dst, SDValue Src,
    SDValue Size, Align Alignment, bool isVolatile, bool AlwaysInline,
    MachinePointerInfo DstPtrInfo, MachinePointerInfo SrcPtrInfo) const {
  SDVTList VTs = DAG.getVTList(MVT::Other, MVT::Glue);
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Size);
  if (ConstantSize) {
    uint64_t CopyLen = ConstantSize->getZExtValue();
    Dst = DAG.getNode(XVMISD::MEMCPY, dl, VTs, Chain, Dst, Src,
                      DAG.getConstant(CopyLen, dl, MVT::i64));
  } else {
    Dst = DAG.getNode(XVMISD::MEMCPY, dl, VTs, Chain, Dst, Src,
                      Size);
  }
  return Dst.getValue(0);
}

SDValue XVMSelectionDAGInfo::EmitTargetCodeForMemmove(
            SelectionDAG &DAG, const SDLoc &dl, SDValue Chain,
              SDValue Op1, SDValue Op2, SDValue Op3,
              Align Alignment, bool isVolatile,
              MachinePointerInfo DstPtrInfo,
              MachinePointerInfo SrcPtrInfo) const {
  SDVTList VTs = DAG.getVTList(MVT::Other, MVT::Glue);
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Op3);
  if (ConstantSize) {
    uint64_t CopyLen = ConstantSize->getZExtValue();
    Op1 = DAG.getNode(XVMISD::MEMMOV, dl, VTs, Chain, Op1, Op2,
                      DAG.getConstant(CopyLen, dl, MVT::i64));
  } else {
    Op1 = DAG.getNode(XVMISD::MEMMOV, dl, VTs, Chain, Op1, Op2,
                      Op3);
  }
  return Op1.getValue(0);
}

SDValue XVMSelectionDAGInfo::EmitTargetCodeForMemset(
                                SelectionDAG &DAG, const SDLoc &DL,
                                SDValue Chain, SDValue Op1, SDValue Op2,
                                SDValue Op3, Align Alignment, bool IsVolatile,
                                bool AlwaysInline,
                                MachinePointerInfo DstPtrInfo) const {
  SDVTList VTs = DAG.getVTList(MVT::Other, MVT::Glue);
  ConstantSDNode *ConstantSize = dyn_cast<ConstantSDNode>(Op3);
  if (ConstantSize) {
    uint64_t CopyLen = ConstantSize->getZExtValue();
    Op1 = DAG.getNode(XVMISD::MEMSET, DL, VTs, Chain, Op1, DAG.getAnyExtOrTrunc(Op2, DL, MVT::i64),
                      DAG.getZExtOrTrunc(Op3, DL, MVT::i64));
  } else {
    Op1 = DAG.getNode(XVMISD::MEMSET, DL, VTs, Chain, Op1, DAG.getAnyExtOrTrunc(Op2, DL, MVT::i64),
                      Op3);
  }
  return Op1.getValue(0);
}
#endif
