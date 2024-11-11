//===-- XVMISelLowering.cpp - XVM DAG Lowering Implementation  ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that XVM uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVM_def.h"
#include "XVMISelLowering.h"
#include "XVM.h"
#include "XVMSubtarget.h"
#include "XVMTargetMachine.h"
#include "XVMMachineFunctionInfo.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace llvm;

#define DEBUG_TYPE "xvm-lower"
#define ALIGN_NUM_BYTES 8
#define INIT_SMALL_VECTOR_SIZE 16
#define INIT_SMALL_VECTOR_REGS_SIZE 6
#define INIT_SMALL_VECTOR_OP_CHAIN_SIZE 10
#define INIT_SMALL_VECTOR_OPS_SIZE 8
#define INIT_SMALL_VECTOR_RETOP_SIZE 4

#define NUM_BITS_PER_BYTE 8
#define SIZE_REF_IN_BYTES 8
#define XVM_MAX_RET_SIZE 1

#define CHAIN_FIRST  0
#define CHAIN_SECOND 1
#define CHAIN_THIRD  2

#define TEMPLATE_INT_32 32

static void fail(const SDLoc &DL, SelectionDAG &DAG, const Twine &Msg) {
  MachineFunction &MF = DAG.getMachineFunction();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(MF.getFunction(), Msg, DL.getDebugLoc()));
}

static int shiftAndGet16Bits(uint64_t num, int n) {
  return (num >> n) & 0xFFFF;
}

static bool isValidImmediateSize(int32_t imm) {
  return imm <= 0x3FFF && imm >= 0;
}

static bool hasFP(const MachineFunction &MF) {
  return false;
}

static bool needsSPForLocalFrame(const MachineFunction &MF) {
  auto &MFI = MF.getFrameInfo();
  return MFI.getStackSize() || MFI.adjustsStack() || hasFP(MF);
}

static bool needsSP(const MachineFunction &MF) {
  return needsSPForLocalFrame(MF);
}

static unsigned getBranchOpcodeFromSelectCC(MachineInstr &MI) {
  assert((MI.getOpcode() == XVM::PseudoSelectCC_ri ||
          MI.getOpcode() == XVM::PseudoSelectCC_rr) &&
          "The instruction should be a pseudo select cc!");
  bool IsRROp = MI.getOpcode() == XVM::PseudoSelectCC_rr;
  if (!IsRROp) {
    int64_t imm32 = MI.getOperand(MO_THIRD).getImm();
    IsRROp = !(isValidImmediateSize(imm32));
  }
  unsigned Cond = MI.getOperand(MO_FOURTH).getImm();
  unsigned NewCond;
  switch (Cond) {
#define SET_NEWCC(X, Y) \
  case ISD::X: \
    NewCond = IsRROp ? XVM::Y##_rr : XVM::Y##_ri; \
    break
  SET_NEWCC(SETGT, BSGT);
  SET_NEWCC(SETUGT, BUGT);
  SET_NEWCC(SETGE, BSGE);
  SET_NEWCC(SETUGE, BUGE);
  SET_NEWCC(SETEQ, BUEQ);
  SET_NEWCC(SETNE, BSNEQ);
  SET_NEWCC(SETLT, BSLT);
  SET_NEWCC(SETULT, BULT);
  SET_NEWCC(SETLE, BSLE);
  SET_NEWCC(SETULE, BULE);
  default:
    report_fatal_error("unimplemented select CondCode " + Twine(Cond));
  }
  return NewCond;
}

XVMTargetLowering::XVMTargetLowering(const TargetMachine &TM,
                                     const XVMSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Set up the register classes.
  addRegisterClass(MVT::i64, &XVM::XVMGPRRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(XVM::R11);

  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  setOperationAction(ISD::BR_CC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT, MVT::i64, Expand);
  setOperationAction(ISD::SETCC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i64, Custom);

  setOperationAction(ISD::SREM, MVT::i64, Expand);
  setOperationAction(ISD::UREM, MVT::i64, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Custom);

  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Custom);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  setOperationAction(ISD::CTPOP, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i64, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);

  setOperationAction({ISD::VASTART, ISD::VAARG}, MVT::Other, Custom);
  setOperationAction({ISD::VACOPY, ISD::VAEND}, MVT::Other, Expand);

  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);

  setOperationAction(ISD::BSWAP, MVT::i64, Expand);

  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);

  setOperationAction(ISD::MULHU, MVT::i64, Custom);
  setOperationAction(ISD::MULHS, MVT::i64, Custom);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Custom);
  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Custom);

  setOperationAction(ISD::ATOMIC_LOAD_ADD, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_LOAD_AND, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_LOAD_OR, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_LOAD_XOR, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_SWAP, MVT::i64, Custom);
  setOperationAction(ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS, MVT::i64, Custom);

  setOperationAction(ISD::SREM, MVT::i64, Expand);
  setOperationAction(ISD::UREM, MVT::i64, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);

  // Extended load operations for i1 types must be promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }
  setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i16, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i32, Expand);

  setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i16, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i32, Expand);

  addRegisterClass(MVT::i64, &XVM::XVMGPRRegClass);

  setBooleanContents(ZeroOrOneBooleanContent);

  // Function alignments
  setMinFunctionAlignment(Align(ALIGN_NUM_BYTES));
  setPrefFunctionAlignment(Align(ALIGN_NUM_BYTES));

  unsigned CommonMaxStores = (unsigned) 0xFFFFFFFF;
  MaxStoresPerMemset = MaxStoresPerMemsetOptSize = CommonMaxStores;
  MaxStoresPerMemcpy = MaxStoresPerMemcpyOptSize = CommonMaxStores;
  MaxStoresPerMemmove = MaxStoresPerMemmoveOptSize = CommonMaxStores;
  MaxLoadsPerMemcmp = MaxLoadsPerMemcmpOptSize = CommonMaxStores;
}

bool XVMTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  return false;
}

bool XVMTargetLowering::isTruncateFree(Type *Ty1, Type *Ty2) const {
  if (!Ty1->isIntegerTy() || !Ty2->isIntegerTy())
    return false;
  unsigned NumBits1 = Ty1->getPrimitiveSizeInBits();
  unsigned NumBits2 = Ty2->getPrimitiveSizeInBits();
  return NumBits1 > NumBits2;
}

bool XVMTargetLowering::isTruncateFree(EVT VT1, EVT VT2) const {
  if (!VT1.isInteger() || !VT2.isInteger())
    return false;
  unsigned NumBits1 = VT1.getSizeInBits();
  unsigned NumBits2 = VT2.getSizeInBits();
  return NumBits1 > NumBits2;
}

bool XVMTargetLowering::isZExtFree(Type *Ty1, Type *Ty2) const {
  // because we only have 64 bit registers and 64 bit ops
  // extend must always be performed
  return false;
}

bool XVMTargetLowering::isZExtFree(EVT VT1, EVT VT2) const {
  return false;
}

XVMTargetLowering::ConstraintType
XVMTargetLowering::getConstraintType(StringRef CType) const {
  if (CType.size() == 1)
    if (CType[0] == 'w')
      return C_RegisterClass;

  return TargetLowering::getConstraintType(CType);
}

std::pair<unsigned, const TargetRegisterClass *>
XVMTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef CType, MVT VT) const {
  if (CType.size() == 1) // GCC Constraint Letters
    if (CType[0] == 'r') // GENERAL_REGS
      return std::make_pair(0U, &XVM::XVMGPRRegClass);

  return TargetLowering::getRegForInlineAsmConstraint(TRI, CType, VT);
}

void XVMTargetLowering::ReplaceNodeResults(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const {
  const char *err_msg = nullptr;
  uint32_t Op = N->getOpcode();
  switch (Op) {
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
  case ISD::ATOMIC_SWAP:
    break;
  default:
    report_fatal_error("Unhandled custom legalization");
  }

  SDLoc DL(N);
  fail(DL, DAG, err_msg);
}

SDValue XVMTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BRCOND:
    return LowerBRCOND(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::VAARG:
    return LowerVAARG(Op, DAG);
  case ISD::DYNAMIC_STACKALLOC: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "Unsupported dynamic stack allocation");
    exit(1);
  }
  case ISD::BRIND: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "Unsupported Indirect Branch");
    exit(1);
  }
  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_SWAP:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "Unsupported Atomic Instructions");
    exit(1);
  }
  case ISD::MULHU:
  case ISD::MULHS:
  case ISD::SMUL_LOHI:
  case ISD::UMUL_LOHI: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "Unsupported multiplication overflow");
    exit(1);
  }
  case ISD::BlockAddress: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "BlockAddress currently unsupported");
    exit(1);
  }
  default: {
    SDNode *N = Op.getNode();
    SDLoc DL(N);
    fail(DL, DAG, "unimplemented operand");
    break;
  }
  }
}

// Calling Convention Implementation
#include "XVMGenCallingConv.inc"

SDValue XVMTargetLowering::LowerFormalArguments(SDValue Nodes, CallingConv::ID Call, bool IsVar,
                                                const SmallVectorImpl<ISD::InputArg> &In, const SDLoc &DLoc,
                                                SelectionDAG &DAGraph, SmallVectorImpl<SDValue> &Vals) const {
  switch (Call) {
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  default:
    report_fatal_error("Calling convention unsupported");
  }

  MachineFunction &MFunction = DAGraph.getMachineFunction();
  MachineRegisterInfo &RInfo = MFunction.getRegInfo();

  // All incoming arguments are assigned locations.
  SmallVector<CCValAssign, INIT_SMALL_VECTOR_SIZE> ArgLoc;
  CCState CCInfo(Call, IsVar, MFunction, ArgLoc, *DAGraph.getContext());
  CCInfo.AnalyzeFormalArguments(In, CC_XVM64);
  bool doesNeedSP = needsSP(MFunction);

  for (auto &LocIterator : ArgLoc) {
    if (LocIterator.isRegLoc()) {
      // Register passed args
      EVT VT = LocIterator.getLocVT();
      MVT::SimpleValueType SVT = VT.getSimpleVT().SimpleTy;
      switch (SVT) {
      default:
        errs() << "LowerFormalArguments Unhandled argument type: " << VT.getEVTString() << '\n';
        llvm_unreachable(nullptr);
      case MVT::i32:
      case MVT::i64:
        Register VRegister = RInfo.createVirtualRegister(&XVM::XVMGPRRegClass);
        RInfo.addLiveIn(LocIterator.getLocReg(), VRegister);
        SDValue ArgV = DAGraph.getCopyFromReg(Nodes, DLoc, VRegister, VT);
        //insert an assert[sz]ext tp capture a value being promoted to a wider type
        if (LocIterator.getLocInfo() == CCValAssign::SExt)
          ArgV = DAGraph.getNode(ISD::AssertSext, DLoc, VT, ArgV,
                                 DAGraph.getValueType(LocIterator.getValVT()));
        else if (LocIterator.getLocInfo() == CCValAssign::ZExt)
          ArgV = DAGraph.getNode(ISD::AssertZext, DLoc, VT, ArgV,
                                 DAGraph.getValueType(LocIterator.getValVT()));

        if (LocIterator.getLocInfo() != CCValAssign::Full)
          ArgV = DAGraph.getNode(ISD::TRUNCATE, DLoc, LocIterator.getValVT(), ArgV);

        Vals.push_back(ArgV);
        break;
      }
    } else {
      MVT LocVT = LocIterator.getLocVT();
      MachineFrameInfo &MFI = MFunction.getFrameInfo();
      EVT ValVT = LocIterator.getValVT();
      // sanity check
      assert(LocIterator.isMemLoc());
      /* The stack pointer offset is relative to the caller stack frame.
       * we also need to add the offset created in in callee for saving callee saved regs
       * we do not need to consider further callee stack offset,
       * it will be handled later in eliminateFrameIndex
       */
      int FI = 0;
      if (doesNeedSP) {
        const MCPhysReg *CSRegs = MFunction.getRegInfo().getCalleeSavedRegs();
        unsigned CSRcounter = 0;
        for (; CSRegs[CSRcounter]; ++CSRcounter);
          FI = MFI.CreateFixedObject(ValVT.getSizeInBits()/NUM_BITS_PER_BYTE,
                                     LocIterator.getLocMemOffset() + CSRcounter*SIZE_REF_IN_BYTES, true);
      } else {
        FI = MFI.CreateFixedObject(ValVT.getSizeInBits()/NUM_BITS_PER_BYTE,
                                   LocIterator.getLocMemOffset(), true);
      }

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAGraph.getFrameIndex(FI, getPointerTy(DAGraph.getDataLayout()));
      SDValue Load = DAGraph.getLoad(
            LocVT, DLoc, Nodes, FIN,
            MachinePointerInfo::getFixedStack(DAGraph.getMachineFunction(), FI));
      Vals.push_back(Load);
    }
  }

  std::vector<SDValue> OutChains;
  if (IsVar) {
    const TargetRegisterClass *RC = &XVM::XVMGPRRegClass;
    MachineFrameInfo &MFI = MFunction.getFrameInfo();
    MachineRegisterInfo &RegInfo = MFunction.getRegInfo();
    XVMMachineFunctionInfo *XFI = MFunction.getInfo<XVMMachineFunctionInfo>();

    static const MCPhysReg ArgGPRs[] = {XVM::R0, XVM::R1, XVM::R2, XVM::R3, XVM::R4, XVM::R5};
    ArrayRef<MCPhysReg> ArgRegs = makeArrayRef(ArgGPRs);
    // The first register index: for example,
    // in foo(int t1, int t2, ...), FirstRegIdx is 2 (R0 and R1 are used for t1 and t2);
    unsigned FirstRegIdx = CCInfo.getFirstUnallocated(ArgRegs);

    int VaArgOffset, VaArgsSaveSize;
    if (FirstRegIdx == ArgRegs.size()) {
      VaArgOffset = CCInfo.getNextStackOffset();
      VaArgsSaveSize = 0;
    } else {
      VaArgsSaveSize = SIZE_REF_IN_BYTES * (ArgRegs.size() - FirstRegIdx);
      VaArgOffset = -VaArgsSaveSize;
    }

    XFI->SetVarArgsFrameIndex(
        MFI.CreateFixedObject(SIZE_REF_IN_BYTES, // size
                              VaArgOffset, // SPOffset
                              true)); // IsImmutable
    // Copy the registers that have not been used for var argument passing
    // assume per size is always 8
    for (unsigned I = FirstRegIdx; I < ArgRegs.size(); I++, VaArgOffset += 8) {
      const Register Reg = RegInfo.createVirtualRegister(RC);
      RegInfo.addLiveIn(ArgRegs[I], Reg);
      SDValue ArgV = DAGraph.getCopyFromReg(Nodes, DLoc, Reg, MVT::i64);
      int FI = MFI.CreateFixedObject(8, VaArgOffset, true);
      SDValue FrameIndex = DAGraph.getFrameIndex(FI, getPointerTy(DAGraph.getDataLayout()));
      SDValue Store = DAGraph.getStore(Nodes, DLoc, ArgV, FrameIndex, MachinePointerInfo::getFixedStack(MFunction, FI));
      // Init the mem operand always.
      cast<StoreSDNode>(Store.getNode())->getMemOperand()->setValue((Value*)nullptr);
      OutChains.push_back(Store);
    }
  }

  if (!OutChains.empty()) {
    OutChains.push_back(Nodes);
    Nodes = DAGraph.getNode(ISD::TokenFactor, DLoc, MVT::Other, OutChains);
  }

  return Nodes;
}

SDValue XVMTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLoweringInfo,
                                     SmallVectorImpl<SDValue> &Vals) const {
  auto &In = CLoweringInfo.Ins;
  auto &Out = CLoweringInfo.Outs;
  SDValue Nodes = CLoweringInfo.Chain;
  auto &OutValues = CLoweringInfo.OutVals;
  SDValue Called = CLoweringInfo.Callee;
  bool IsVArg = CLoweringInfo.IsVarArg;
  SelectionDAG &DAGraph = CLoweringInfo.DAG;
  bool &IsTail = CLoweringInfo.IsTailCall;
  CallingConv::ID CallConvention = CLoweringInfo.CallConv;
  MachineFunction &MFunction = DAGraph.getMachineFunction();

  // XVM target does not support tail call optimization.
  IsTail = false;

  switch (CallConvention) {
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  default:
    report_fatal_error("calling convention unsupported");
  }

  const XVMRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MFunction, CallConvention);
  assert(Mask && "Missing call preserved mask for calling convention");

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, INIT_SMALL_VECTOR_SIZE> ArgLocs;
  CCState CCInfo(CallConvention, IsVArg, MFunction, ArgLocs, *DAGraph.getContext());

  CCInfo.AnalyzeCallOperands(Out, CC_XVM64);

  unsigned NumBytes = CCInfo.getNextStackOffset();

  for (auto &Arg : Out) {
    ISD::ArgFlagsTy Flags = Arg.Flags;
    if (!Flags.isByVal())
      continue;
  }

  auto PtrVT = getPointerTy(MFunction.getDataLayout());
  Nodes = DAGraph.getCALLSEQ_START(Nodes, NumBytes, 0, CLoweringInfo.DL);

  SmallVector<std::pair<unsigned, SDValue>, INIT_SMALL_VECTOR_REGS_SIZE> PassingRegs;
  SmallVector<SDValue, INIT_SMALL_VECTOR_OP_CHAIN_SIZE> MemOpNodeChains;

  SDValue StackP = DAGraph.getCopyFromReg(Nodes, CLoweringInfo.DL, XVM::SP, PtrVT);

  // Walk arg assignments
  for (unsigned argnum = 0, e = Out.size(); argnum != e; ++argnum) {
    SDValue Arg = OutValues[argnum];
    CCValAssign &CurrentArgloc = ArgLocs[argnum];

    // Promote the value if needed.
    switch (CurrentArgloc.getLocInfo()) {
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAGraph.getNode(ISD::SIGN_EXTEND, CLoweringInfo.DL, CurrentArgloc.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAGraph.getNode(ISD::ANY_EXTEND, CLoweringInfo.DL, CurrentArgloc.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAGraph.getNode(ISD::ZERO_EXTEND, CLoweringInfo.DL, CurrentArgloc.getLocVT(), Arg);
      break;
    default:
      llvm_unreachable("Unknown loc info");
    }

    // Push arguments into Passingregs vector
    if (CurrentArgloc.isRegLoc())
      PassingRegs.push_back(std::make_pair(CurrentArgloc.getLocReg(), Arg));
    else {
      assert(CurrentArgloc.isMemLoc());

      int32_t Offset = CurrentArgloc.getLocMemOffset();
      SDValue PtrOff = DAGraph.getIntPtrConstant(Offset, CLoweringInfo.DL);

      SDValue DstAddr = DAGraph.getNode(ISD::ADD, CLoweringInfo.DL, PtrVT, StackP, PtrOff);
      MachinePointerInfo DstInfo = MachinePointerInfo::getStack(MFunction, Offset);

      SDValue Store = DAGraph.getStore(Nodes, CLoweringInfo.DL, Arg, DstAddr, DstInfo);
      MemOpNodeChains.push_back(Store);
    }
  }

  if (!MemOpNodeChains.empty())
    Nodes = DAGraph.getNode(ISD::TokenFactor, CLoweringInfo.DL, MVT::Other, MemOpNodeChains);

  SDValue IFlag;

  // build a node dependency chain and flag operands that copy outgoing args to regs
  // All emitted instructions have to be grouped togeher, hence IFLAG
  for (auto &CurrentReg : PassingRegs) {
    Nodes = DAGraph.getCopyToReg(Nodes, CLoweringInfo.DL, CurrentReg.first, CurrentReg.second, IFlag);
    IFlag = Nodes.getValue(1);
  }

  // if a GlobalAddress node is the "called" (every direct call is)
  // change it to a TargetGlobalAddress so legalize doesnt hack it
  // ExternelSymbol -> TargetExternelSymbol
  if (GlobalAddressSDNode *GlblAddNode = dyn_cast<GlobalAddressSDNode>(Called)) {
    Called = DAGraph.getTargetGlobalAddress(GlblAddNode->getGlobal(), CLoweringInfo.DL, PtrVT,
                                            GlblAddNode->getOffset(), 0);
  } else if (ExternalSymbolSDNode *ExtSymbNode = dyn_cast<ExternalSymbolSDNode>(Called)) {
    Called = DAGraph.getTargetExternalSymbol(ExtSymbNode->getSymbol(), PtrVT, 0);
    if (strcmp(ExtSymbNode->getSymbol(), "__fixsfdi") == 0) {
      fail(CLoweringInfo.DL, DAGraph, Twine("Floats are unsupported in XVM"));
      exit(1);
    } else {
      fail(CLoweringInfo.DL, DAGraph, Twine("A call to built-in function '"
                              + StringRef(ExtSymbNode->getSymbol())
                              + "' is not supported."));
      exit(1);
    }
  }

  // Return a chain of nodes and a flag that retval copy can use.
  SDVTList NodeTypes = DAGraph.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, INIT_SMALL_VECTOR_OPS_SIZE> Ops;
  Ops.push_back(Nodes);
  Ops.push_back(Called);
  Ops.push_back(DAGraph.getRegisterMask(Mask));

  // Make argument registers line into the call by appending them to the list
  for (auto &RegIter : PassingRegs)
    Ops.push_back(DAGraph.getRegister(RegIter.first, RegIter.second.getValueType()));

  if (IFlag.getNode())
    Ops.push_back(IFlag);

  Nodes = DAGraph.getNode(XVMISD::CALL, CLoweringInfo.DL, NodeTypes, Ops);
  IFlag = Nodes.getValue(1);

  // CALLSEQ_END node created.
  Nodes = DAGraph.getCALLSEQ_END(Nodes,
                                 DAGraph.getConstant(NumBytes, CLoweringInfo.DL, PtrVT, true),
                                 DAGraph.getConstant(0, CLoweringInfo.DL, PtrVT, true),
                                 IFlag, CLoweringInfo.DL);
  IFlag = Nodes.getValue(1);

  // copt result values from pregs to the returned vregs
  return LowerCallResult(Nodes, IFlag, CallConvention, IsVArg, In, CLoweringInfo.DL, DAGraph,
                         Vals);
}

SDValue XVMTargetLowering::LowerReturn(SDValue Nodes, CallingConv::ID CallConv,
                                       bool IsVarArg,
                                       const SmallVectorImpl<ISD::OutputArg> &Outs,
                                       const SmallVectorImpl<SDValue> &OutVals,
                                       const SDLoc &DLoc, SelectionDAG &DAGraph) const {
  unsigned Opc = XVMISD::RET_FLAG;

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, INIT_SMALL_VECTOR_SIZE> RVLocs;
  MachineFunction &MFunction = DAGraph.getMachineFunction();

  // CCState - register and stack slot info.
  CCState CCInfo(CallConv, IsVarArg, MFunction, RVLocs, *DAGraph.getContext());

  if (MFunction.getFunction().getReturnType()->isAggregateType()) {
    fail(DLoc, DAGraph, "only integer returns supported");
    return DAGraph.getNode(Opc, DLoc, MVT::Other, Nodes);
  }

  // return value analysis
  CCInfo.AnalyzeReturn(Outs, RetCC_XVM64);

  SDValue IFlag;
  SmallVector<SDValue, INIT_SMALL_VECTOR_RETOP_SIZE> RetOps(1, Nodes);

  // insert our results into the output reg.
  for (unsigned counter = 0; counter != RVLocs.size(); ++counter) {
    CCValAssign &ValIter = RVLocs[counter];
    assert(ValIter.isRegLoc() && "Can only return in registers!");

    Nodes = DAGraph.getCopyToReg(Nodes, DLoc, ValIter.getLocReg(), OutVals[counter], IFlag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    IFlag = Nodes.getValue(1);
    RetOps.push_back(DAGraph.getRegister(ValIter.getLocReg(), ValIter.getLocVT()));
  }

  RetOps[0] = Nodes; // Update chain.

  // Add the flag if we have it.
  if (IFlag.getNode())
    RetOps.push_back(IFlag);

  return DAGraph.getNode(Opc, DLoc, MVT::Other, RetOps);
}

SDValue XVMTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, INIT_SMALL_VECTOR_SIZE> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  if (Ins.size() >= XVM_MAX_RET_SIZE + 1) {
    fail(DL, DAG, "only small returns supported");
    for (unsigned i = 0, e = Ins.size(); i != e; ++i)
      InVals.push_back(DAG.getConstant(0, DL, Ins[i].VT));
    return DAG.getCopyFromReg(Chain, DL, 1, Ins[0].VT, InFlag).getValue(1);
  }

  CCInfo.AnalyzeCallResult(Ins, RetCC_XVM64);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Val : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, DL, Val.getLocReg(),
                               Val.getValVT(), InFlag).getValue(CHAIN_SECOND);
    InFlag = Chain.getValue(CHAIN_THIRD);
    InVals.push_back(Chain.getValue(CHAIN_FIRST));
  }

  return Chain;
}

SDValue XVMTargetLowering::LowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  SDValue CondV = Op.getOperand(MO_SECOND);
  SDLoc DL(Op);

  if (CondV.getOpcode() == ISD::SETCC &&
      CondV.getOperand(MO_FIRST).getValueType() == MVT::i64) {
    SDValue LHS = CondV.getOperand(MO_FIRST);
    SDValue RHS = CondV.getOperand(MO_SECOND);

    SDValue TargetCC = CondV.getOperand(MO_THIRD);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(MO_FIRST), LHS, RHS, TargetCC, Op.getOperand(MO_THIRD));
  } else if (CondV.getOpcode() == ISD::AssertZext &&
             CondV.getOperand(MO_FIRST).getValueType() == MVT::i64) {
    SDValue LHS = CondV.getOperand(MO_FIRST);
    SDValue RHS = DAG.getConstant(1, DL, MVT::i64);

    SDValue TargetCC = DAG.getCondCode(ISD::SETEQ);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(MO_FIRST), LHS, RHS, TargetCC, Op.getOperand(MO_THIRD));
  } else if (CondV.getOpcode() == ISD::AND || CondV.getOpcode() == ISD::OR ||
             CondV.getOpcode() == ISD::XOR || CondV.getOpcode() == ISD::Constant ||
             CondV.getOpcode() == ISD::UADDO || CondV.getOpcode() == ISD::SELECT_CC ||
             CondV.getOpcode() == ISD::UREM || CondV.getOpcode() == ISD::SRL ||
             CondV.getOpcode() == ISD::SELECT) {
    SDValue LHS = CondV;
    SDValue RHS = DAG.getConstant(0, DL, MVT::i64);
    SDValue TargetCC = DAG.getCondCode(ISD::SETNE);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(MO_FIRST), LHS, RHS, TargetCC, Op.getOperand(MO_THIRD));
  }
  //Note: complete the lowering for other cases later
  return Op;
}

SDValue XVMTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(MO_FIRST);
  SDValue RHS = Op.getOperand(MO_SECOND);
  SDValue TrueV = Op.getOperand(MO_THIRD);
  SDValue FalseV = Op.getOperand(MO_FOURTH);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(MO_FIFTH))->get();
  SDLoc DL(Op);

  SDValue TargetCC = DAG.getConstant(CC, DL, LHS.getValueType());
  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SDValue Ops[] = {LHS, RHS, TargetCC, TrueV, FalseV};

  return DAG.getNode(XVMISD::SELECT_CC, DL, VTs, Ops);
}

/// VASTART - have three operands: an input chain,
/// pointer, and a SRCVALUE.
SDValue XVMTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  int VarArgsFrameIndex = MF.getInfo<XVMMachineFunctionInfo>()->GetVarArgsFrameIndex();
  SDValue FrameIndex = DAG.getFrameIndex(VarArgsFrameIndex, getPointerTy(MF.getDataLayout()));

  SDValue RetSDValue = DAG.getStore(Op.getOperand(MO_FIRST), SDLoc(Op), FrameIndex, Op.getOperand(MO_SECOND),
                                    MachinePointerInfo()); // unsigned AddressSpace = 0, int64_t offset = 0

  return RetSDValue;
}

/// VAARG - VAARG has four operands: an input chain, a pointer, a SRCVALUE,
/// and the alignment. It returns a pair of values: the vaarg value and a
/// new chain.
SDValue XVMTargetLowering::LowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  MachinePointerInfo MPI(0U, 0L);
  EVT PointerType = getPointerTy(DAG.getMachineFunction().getDataLayout());
  SDValue LoadPointer = DAG.getLoad(PointerType, DL, Op.getOperand(MO_FIRST), Op.getOperand(MO_SECOND), MPI);
  SDValue Offset = DAG.getConstant(8, DL, MVT::i64);
  SDValue AddPointer = DAG.getNode(ISD::ADD, DL, PointerType, LoadPointer, Offset);
  SDValue StorePointer = DAG.getStore(LoadPointer.getValue(1), DL, AddPointer, Op.getOperand(MO_SECOND), MPI);
  SDValue RetSDValue = DAG.getLoad(Op.getValueType(), DL, StorePointer, LoadPointer, MPI);
  return RetSDValue;
}

const char *XVMTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((XVMISD::NodeType)Opcode) {
  case XVMISD::FIRST_NUMBER:
    break;
  case XVMISD::RET_FLAG:
    return "XVMISD::RET_FLAG";
  case XVMISD::CALL:
    return "XVMISD::CALL";
  case XVMISD::SELECT_CC:
    return "XVMISD::SELECT_CC";
  case XVMISD::BR_CC:
    return "XVMISD::BR_CC";
  case XVMISD::Wrapper:
    return "XVMISD::Wrapper";
  case XVMISD::MEMCPY:
    return "XVMISD::MEMCPY";
  default:
    return nullptr;
  }
}

SDValue XVMTargetLowering::LowerGlobalAddress(SDValue Oper,
                                              SelectionDAG &DAGraph) const {
  auto NIter = cast<GlobalAddressSDNode>(Oper);
  assert(NIter->getOffset() == 0 && "global address offset invalid");

  SDLoc DLoc(Oper);
  const GlobalValue *GlobVal = NIter->getGlobal();
  SDValue GlobalAdd = DAGraph.getTargetGlobalAddress(GlobVal, DLoc, MVT::i64);

  return DAGraph.getNode(XVMISD::Wrapper, DLoc, MVT::i64, GlobalAdd);
}

unsigned
XVMTargetLowering::EmitSubregExt(MachineInstr &MI, MachineBasicBlock *BB,
                                 unsigned Reg, bool isSigned) const {
  return 0;
}

MachineBasicBlock *
XVMTargetLowering::EmitInstrWithCustomInserterSelectCC(MachineInstr &MI,
                                                       MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Opc = MI.getOpcode();

  bool isSelectRROp = (Opc == XVM::PseudoSelectCC_rr);

#ifndef NDEBUG
  bool isSelectRIOp = (Opc == XVM::PseudoSelectCC_ri);

  assert((isSelectRROp || isSelectRIOp) && "Unexpected instr type to insert");
#endif
  // We insert a diamond control-flow pattern, to insert a SELECT instruction
  // The incoming instruction holds the branch opcode, the true/false values,
  // the condition code register to branch on and the dest vreg
  const BasicBlock *LLVM_BasicB = BB->getBasicBlock();
  MachineFunction::iterator Iter = ++BB->getIterator();

  // CurrentMBBlock:
  //  TrueValue = ...
  //  jmp_XX r1, r2 goto Copy1MBBlock
  //  fallthrough --> Copy0MBBlock
  MachineBasicBlock *CurrentMBBlock = BB;
  MachineFunction *MFunc = BB->getParent();
  MachineBasicBlock *Copy0MBBlock = MFunc->CreateMachineBasicBlock(LLVM_BasicB);
  MachineBasicBlock *Copy1MBBlock = MFunc->CreateMachineBasicBlock(LLVM_BasicB);

  MFunc->insert(Iter, Copy0MBBlock);
  MFunc->insert(Iter, Copy1MBBlock);
  // The Phi node for the select will be in the new block, transferring
  // all successors of the current block to the new one will update the
  // machien-CFG edges
  Copy1MBBlock->splice(Copy1MBBlock->begin(), BB,
                       std::next(MachineBasicBlock::iterator(MI)), BB->end());
  Copy1MBBlock->transferSuccessorsAndUpdatePHIs(BB);
  // Add the true/fallthrough blocks as its successors.
  BB->addSuccessor(Copy0MBBlock);
  BB->addSuccessor(Copy1MBBlock);

  // if Flag insert branch
  int NewCC = getBranchOpcodeFromSelectCC(MI);

  Register LHS = MI.getOperand(MO_SECOND).getReg();
  if (isSelectRROp) {
    Register RHS = MI.getOperand(MO_THIRD).getReg();
    BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBBlock).addReg(LHS).addReg(RHS);
  } else {
    int64_t imm32 = MI.getOperand(MO_THIRD).getImm();
    // Check before we build J*_ri instruction.
    assert (isInt<TEMPLATE_INT_32>(imm32));
    if (isValidImmediateSize(imm32)) {
        BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBBlock).addReg(LHS).addImm(imm32);
    } else {
        Register ScratchReg;
        MachineRegisterInfo &MRI = MFunc->getRegInfo();
        ScratchReg = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
        uint64_t MostSignificantBits = shiftAndGet16Bits(imm32, MOST_SIGNIFICANT);
        uint64_t UpperMiddleBits = shiftAndGet16Bits(imm32, UPPER_MIDDLE);
        uint64_t LowerMiddleBits = shiftAndGet16Bits(imm32, LOWER_MIDDLE);
        uint64_t LeastSignificantBits = shiftAndGet16Bits(imm32, LEAST_SIGNIFICANT);

        Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
        BuildMI(BB, DL, TII.get(XVM::MOV_ri), VRegForMov).addImm(0);
        Register PrevReg = VRegForMov;
        if (LeastSignificantBits) {
          Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk1)
              .addReg(PrevReg).addImm(LeastSignificantBits).addImm(MOVK_SHIFT_0);
          PrevReg = VRegForMovk1;
        }
        if (LowerMiddleBits) {
          Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk2)
              .addReg(PrevReg).addImm(LowerMiddleBits).addImm(MOVK_SHIFT_16);
          PrevReg = VRegForMovk2;
        }
        if (UpperMiddleBits) {
          Register VRegForMovk3 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk3)
              .addReg(PrevReg).addImm(UpperMiddleBits).addImm(MOVK_SHIFT_32);
          PrevReg = VRegForMovk3;
        }
        if (MostSignificantBits) {
          Register VRegForMovk4 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk4)
              .addReg(PrevReg).addImm(MostSignificantBits).addImm(MOVK_SHIFT_48);
          PrevReg = VRegForMovk4;
        }
        BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBBlock).addReg(LHS).addReg(PrevReg);
    }
  }

  // Copy0MBBlock:
  //  %FalseValue = ...
  //  # fallthrough to Copy1MBBlock
  BB = Copy0MBBlock;

  // machine-CFG edges, update
  BB->addSuccessor(Copy1MBBlock);

  // Copy1MBBlock:
  //  %Result = phi [ %FalseValue, Copy0MBBlock ], [ %TrueValue, CurrentMBBlock ]
  BB = Copy1MBBlock;
  BuildMI(*BB, BB->begin(), DL, TII.get(XVM::PHI), MI.getOperand(MO_FIRST).getReg())
      .addReg(MI.getOperand(MO_SIXTH).getReg())
      .addMBB(Copy0MBBlock)
      .addReg(MI.getOperand(MO_FIFTH).getReg())
      .addMBB(CurrentMBBlock);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}

MachineBasicBlock *
XVMTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *BB) const {
  DebugLoc DL = MI.getDebugLoc();
  unsigned Opc = MI.getOpcode();
  switch (Opc) {
  case XVM::PseudoSelectCC_rr:
  case XVM::PseudoSelectCC_ri:
    return EmitInstrWithCustomInserterSelectCC(MI, BB);
  default:
    return BB;
  }
}

EVT XVMTargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                          EVT VT) const {
  return MVT::i64;
}

MVT XVMTargetLowering::getScalarShiftAmountTy(const DataLayout &DL,
                                              EVT VT) const {
  return MVT::i64;
}

bool XVMTargetLowering::isLegalAddressingMode(const DataLayout &DLayout, const AddrMode &AMode,
                                              Type *T, unsigned AS, Instruction *Inst) const {
  // Globals are never allowed as a base
  bool ret = true;
  if (AMode.BaseGV)
    ret = false;
  else
    switch (AMode.Scale) {
    case 0: // "r+i" or "i", depends on HasBaseReg.
      break;
    case 1:
      if (!AMode.HasBaseReg) // "r+i", allowed
        break;
      ret = false; // "r+r" or "r+r+i", not allowed
    default:
      ret = false;
    }

  return ret;
}

#endif
