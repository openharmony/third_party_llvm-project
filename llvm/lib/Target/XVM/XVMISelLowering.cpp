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
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace llvm;

#define DEBUG_TYPE "xvm-lower"

static void fail(const SDLoc &DL, SelectionDAG &DAG, const Twine &Msg) {
  MachineFunction &MF = DAG.getMachineFunction();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(MF.getFunction(), Msg, DL.getDebugLoc()));
}

static int ShiftAndGet16Bits(uint64_t num, int n) {
  return (num >> n) & 0xFFFF;
}

static bool is_valid_immediate_size(int32_t imm)
{
  return imm <= 0x3FFF && imm >= 0;
}

static void fail(const SDLoc &DL, SelectionDAG &DAG, const char *Msg,
                 SDValue Val) {
  MachineFunction &MF = DAG.getMachineFunction();
  std::string Str;
  raw_string_ostream OS(Str);
  OS << Msg;
  Val->print(OS);
  OS.flush();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(MF.getFunction(), Str, DL.getDebugLoc()));
}

static bool hasFP(const MachineFunction &MF) {
  return false;
}

static bool needsSPForLocalFrame(
    const MachineFunction &MF) {
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
    int64_t imm32 = MI.getOperand(2).getImm();
    IsRROp = !(is_valid_immediate_size(imm32));
  }
  unsigned Cond = MI.getOperand(3).getImm();
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
    : TargetLowering(TM), Subtarget(&STI) {

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

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);

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
  setMinFunctionAlignment(Align(8));
  setPrefFunctionAlignment(Align(8));

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
XVMTargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'w':
      return C_RegisterClass;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
XVMTargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef Constraint,
                                                MVT VT) const {
  if (Constraint.size() == 1)
    // GCC Constraint Letters
    switch (Constraint[0]) {
    case 'r': // GENERAL_REGS
      return std::make_pair(0U, &XVM::XVMGPRRegClass);
    default:
      break;
    }

  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

void XVMTargetLowering::ReplaceNodeResults(
  SDNode *N, SmallVectorImpl<SDValue> &Results, SelectionDAG &DAG) const {
  const char *err_msg;
  uint32_t Opcode = N->getOpcode();
  switch (Opcode) {
  default:
    report_fatal_error("Unhandled custom legalization");
  case ISD::ATOMIC_LOAD_ADD:
  case ISD::ATOMIC_LOAD_AND:
  case ISD::ATOMIC_LOAD_OR:
  case ISD::ATOMIC_LOAD_XOR:
  case ISD::ATOMIC_SWAP:
  case ISD::ATOMIC_CMP_SWAP_WITH_SUCCESS:
    break;
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
  case ISD::DYNAMIC_STACKALLOC:
    report_fatal_error("Unsupported dynamic stack allocation");
  default:
    llvm_unreachable("unimplemented operand");
  }
}

// Calling Convention Implementation
#include "XVMGenCallingConv.inc"

SDValue XVMTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_XVM64);
  bool doesNeedSP = needsSP(MF);

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      MVT::SimpleValueType SimpleTy = RegVT.getSimpleVT().SimpleTy;
      switch (SimpleTy) {
      default: {
        errs() << "LowerFormalArguments Unhandled argument type: "
               << RegVT.getEVTString() << '\n';
        llvm_unreachable(nullptr);
      }
      case MVT::i32:
      case MVT::i64:
        Register VReg = RegInfo.createVirtualRegister(&XVM::XVMGPRRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        // If this is an value that has been promoted to wider types, insert an
        // assert[sz]ext to capture this, then truncate to the right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);

	      break;
      }
    } else {
        MVT LocVT = VA.getLocVT();
        MachineFrameInfo &MFI = MF.getFrameInfo();
        EVT ValVT = VA.getValVT();
        // sanity check
        assert(VA.isMemLoc());
        /* The stack pointer offset is relative to the caller stack frame.
         * we also need to add the offset created in in callee for saving callee saved regs
         * we do not need to consider further callee stack offset,
         * it will be handled later in eliminateFrameIndex
         */
        int FI = 0;
        if (doesNeedSP) {
          const MCPhysReg *CSRegs = MF.getRegInfo().getCalleeSavedRegs();
          unsigned CSRcounter = 0;
          for ( ; CSRegs[CSRcounter]; ++CSRcounter);
          FI = MFI.CreateFixedObject(ValVT.getSizeInBits()/8,
                                     VA.getLocMemOffset() + CSRcounter*8, true);
        } else {
          FI = MFI.CreateFixedObject(ValVT.getSizeInBits()/8,
                                     VA.getLocMemOffset(), true);
        }

        // Create load nodes to retrieve arguments from the stack
        SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
        SDValue Load = DAG.getLoad(
            LocVT, DL, Chain, FIN,
            MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
        InVals.push_back(Load);
    }
  }

  std::vector<SDValue> OutChains;
  if (IsVarArg) {
    const TargetRegisterClass *RC = &XVM::XVMGPRRegClass;
    MachineFrameInfo &MFI = MF.getFrameInfo();
    MachineRegisterInfo &RegInfo = MF.getRegInfo();
    XVMMachineFunctionInfo *XFI = MF.getInfo<XVMMachineFunctionInfo>();

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
      VaArgsSaveSize = 8 * (ArgRegs.size() - FirstRegIdx);
      VaArgOffset = -VaArgsSaveSize;
    }

    XFI->SetVarArgsFrameIndex(
      MFI.CreateFixedObject(8, // size
                            VaArgOffset, // SPOffset
                            true)); // IsImmutable
    // Copy the registers that have not been used for var argument passing
    // assume per size is always 8
    for (unsigned I = FirstRegIdx; I < ArgRegs.size(); I++, VaArgOffset += 8) {
      const Register Reg = RegInfo.createVirtualRegister(RC);
      RegInfo.addLiveIn(ArgRegs[I], Reg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, MVT::i64);
      int FI = MFI.CreateFixedObject(8, VaArgOffset, true);
      SDValue FrameIndex = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Store = DAG.getStore(Chain, DL, ArgValue, FrameIndex, MachinePointerInfo::getFixedStack(MF, FI));
      // Init the mem operand always.
      cast<StoreSDNode>(Store.getNode())->getMemOperand()->setValue((Value*)nullptr);
      OutChains.push_back(Store);
    }
  }

  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
  }

  return Chain;
}

SDValue XVMTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  auto &Outs = CLI.Outs;
  auto &OutVals = CLI.OutVals;
  auto &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  MachineFunction &MF = DAG.getMachineFunction();

  // XVM target does not support tail call optimization.
  IsTailCall = false;

  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    break;
  }
  const XVMRegisterInfo *TRI = Subtarget->getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_XVM64);

  unsigned NumBytes = CCInfo.getNextStackOffset();

  for (auto &Arg : Outs) {
    ISD::ArgFlagsTy Flags = Arg.Flags;
    if (!Flags.isByVal())
      continue;
  }

  auto PtrVT = getPointerTy(MF.getDataLayout());
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, CLI.DL);

  SmallVector<std::pair<unsigned, SDValue>, 6> RegsToPass;
  SmallVector<SDValue, 10> MemOpChains;

  SDValue StackPtr = DAG.getCopyFromReg(Chain, CLI.DL, XVM::SP, PtrVT);

  // Walk arg assignments
  for (unsigned i = 0, e = Outs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, CLI.DL, VA.getLocVT(), Arg);
      break;
    }

    // Push arguments into RegsToPass vector
    if (VA.isRegLoc())
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    else{
      assert(VA.isMemLoc());

      int32_t Offset = VA.getLocMemOffset();
      SDValue PtrOff = DAG.getIntPtrConstant(Offset, CLI.DL);

      SDValue DstAddr = DAG.getNode(ISD::ADD, CLI.DL, PtrVT, StackPtr, PtrOff);
      MachinePointerInfo DstInfo = MachinePointerInfo::getStack(MF, Offset);

      SDValue Store = DAG.getStore(Chain, CLI.DL, Arg, DstAddr, DstInfo);
      MemOpChains.push_back(Store);
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, CLI.DL, MVT::Other, MemOpChains);

  SDValue InFlag;

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, CLI.DL, Reg.first, Reg.second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), CLI.DL, PtrVT,
                                        G->getOffset(), 0);
  } else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), PtrVT, 0);
    fail(CLI.DL, DAG, Twine("A call to built-in function '"
                            + StringRef(E->getSymbol())
                            + "' is not supported."));
  }

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(XVMISD::CALL, CLI.DL, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(
      Chain, DAG.getConstant(NumBytes, CLI.DL, PtrVT, true),
      DAG.getConstant(0, CLI.DL, PtrVT, true), InFlag, CLI.DL);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, IsVarArg, Ins, CLI.DL, DAG,
                         InVals);
}

SDValue
XVMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  unsigned Opc = XVMISD::RET_FLAG;

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  if (MF.getFunction().getReturnType()->isAggregateType()) {
    fail(DL, DAG, "only integer returns supported");
    return DAG.getNode(Opc, DL, MVT::Other, Chain);
  }

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_XVM64);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(Opc, DL, MVT::Other, RetOps);
}

SDValue XVMTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  if (Ins.size() >= 2) {
    fail(DL, DAG, "only small returns supported");
    for (unsigned i = 0, e = Ins.size(); i != e; ++i)
      InVals.push_back(DAG.getConstant(0, DL, Ins[i].VT));
    return DAG.getCopyFromReg(Chain, DL, 1, Ins[0].VT, InFlag).getValue(1);
  }

  CCInfo.AnalyzeCallResult(Ins, RetCC_XVM64);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Val : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, DL, Val.getLocReg(),
                               Val.getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

SDValue XVMTargetLowering::LowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  SDValue CondV = Op.getOperand(1);
  SDLoc DL(Op);

  if (CondV.getOpcode() == ISD::SETCC &&
      CondV.getOperand(0).getValueType() == MVT::i64) {
    SDValue LHS = CondV.getOperand(0);
    SDValue RHS = CondV.getOperand(1);

    SDValue TargetCC = CondV.getOperand(2);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(0), LHS, RHS, TargetCC, Op.getOperand(2));
  } else if (CondV.getOpcode() == ISD::AssertZext &&
             CondV.getOperand(0).getValueType() == MVT::i64) {
    SDValue LHS = CondV.getOperand(0);
    SDValue RHS = DAG.getConstant(1, DL, MVT::i64);

    SDValue TargetCC = DAG.getCondCode(ISD::SETEQ);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(0), LHS, RHS, TargetCC, Op.getOperand(2));
  } else if (CondV.getOpcode() == ISD::AND || CondV.getOpcode() == ISD::OR ||
             CondV.getOpcode() == ISD::XOR || CondV.getOpcode() == ISD::Constant) {
    SDValue LHS = CondV;
    if (CondV.getNumOperands()>0) {
      LHS = CondV.getOperand(0);
    }
    SDValue RHS = DAG.getConstant(0, DL, MVT::i64);
    SDValue TargetCC = DAG.getCondCode(ISD::SETNE);

    return DAG.getNode(XVMISD::BR_CC, DL, Op.getValueType(),
                       Op.getOperand(0), LHS, RHS, TargetCC, Op.getOperand(2));
  }
  //FIXME: complete the lowering for other cases
  return Op;
}

SDValue XVMTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueV = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
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

  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  SDValue RetSDValue = DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex, Op.getOperand(1),
                      MachinePointerInfo() // unsigned AddressSpace = 0, int64_t offset = 0
                     );
  return RetSDValue;
}

/// VAARG - VAARG has four operands: an input chain, a pointer, a SRCVALUE,
/// and the alignment. It returns a pair of values: the vaarg value and a
/// new chain.
SDValue XVMTargetLowering::LowerVAARG(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  MachinePointerInfo MPI(0U, 0L);
  EVT PointerType = getPointerTy(DAG.getMachineFunction().getDataLayout());
  SDValue LoadPointer = DAG.getLoad(PointerType, DL, Op.getOperand(0), Op.getOperand(1), MPI);
  SDValue Offset = DAG.getConstant(8, DL, MVT::i64);
  SDValue AddPointer = DAG.getNode(ISD::ADD, DL, PointerType, LoadPointer, Offset);
  SDValue StorePointer = DAG.getStore(LoadPointer.getValue(1), DL, AddPointer, Op.getOperand(1), MPI);
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
  }
  return nullptr;
}

SDValue XVMTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  auto N = cast<GlobalAddressSDNode>(Op);
  assert(N->getOffset() == 0 && "Invalid offset for global address");

  SDLoc DL(Op);
  const GlobalValue *GV = N->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, DL, MVT::i64);

  return DAG.getNode(XVMISD::Wrapper, DL, MVT::i64, GA);
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
  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = ++BB->getIterator();

  // ThisMBB:
  // ...
  //  TrueVal = ...
  //  jmp_XX r1, r2 goto Copy1MBB
  //  fallthrough --> Copy0MBB
  MachineBasicBlock *ThisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *Copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *Copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, Copy0MBB);
  F->insert(I, Copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  Copy1MBB->splice(Copy1MBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  Copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(Copy0MBB);
  BB->addSuccessor(Copy1MBB);

  // Insert Branch if Flag
  int NewCC = getBranchOpcodeFromSelectCC(MI);

  Register LHS = MI.getOperand(1).getReg();
  if (isSelectRROp) {
    Register RHS = MI.getOperand(2).getReg();
    BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBB).addReg(LHS).addReg(RHS);
  } else {
    int64_t imm32 = MI.getOperand(2).getImm();
    // Check before we build J*_ri instruction.
    assert (isInt<32>(imm32));
    if (is_valid_immediate_size(imm32)) {
        BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBB).addReg(LHS).addImm(imm32);
    } else {
        Register ScratchReg;
        MachineRegisterInfo &MRI = F->getRegInfo();
        ScratchReg = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
        uint64_t MostSignificantBits = ShiftAndGet16Bits(imm32, 48);
        uint64_t UpperMiddleBits = ShiftAndGet16Bits(imm32, 32);
        uint64_t LowerMiddleBits = ShiftAndGet16Bits(imm32, 16);
        uint64_t LeastSignificantBits = ShiftAndGet16Bits(imm32, 0);

        Register VRegForMov = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
        MachineInstr * MovMI = BuildMI(BB, DL, TII.get(XVM::MOV_ri), VRegForMov)
                            .addImm(0);
        Register PrevReg = VRegForMov;
        if (LeastSignificantBits) {
          Register VRegForMovk1 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk1)
              .addReg(PrevReg).addImm(LeastSignificantBits).addImm(0);
          PrevReg = VRegForMovk1;
        }
        if (LowerMiddleBits) {
          Register VRegForMovk2 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk2)
              .addReg(PrevReg).addImm(LowerMiddleBits).addImm(0);
          PrevReg = VRegForMovk2;
        }
        if (UpperMiddleBits) {
          Register VRegForMovk3 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk3)
              .addReg(PrevReg).addImm(UpperMiddleBits).addImm(0);
          PrevReg = VRegForMovk3;
        }
        if (MostSignificantBits) {
          Register VRegForMovk4 = MRI.createVirtualRegister(&XVM::XVMGPRRegClass);
          BuildMI(BB, DL, TII.get(XVM::MOVK_ri), VRegForMovk4)
              .addReg(PrevReg).addImm(MostSignificantBits).addImm(0);
        }
        BuildMI(BB, DL, TII.get(NewCC)).addMBB(Copy1MBB).addReg(LHS).addReg(PrevReg);
    }
  }

  // Copy0MBB:
  //  %FalseValue = ...
  //  # fallthrough to Copy1MBB
  BB = Copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(Copy1MBB);

  // Copy1MBB:
  //  %Result = phi [ %FalseValue, Copy0MBB ], [ %TrueValue, ThisMBB ]
  // ...
  BB = Copy1MBB;
  BuildMI(*BB, BB->begin(), DL, TII.get(XVM::PHI), MI.getOperand(0).getReg())
      .addReg(MI.getOperand(5).getReg())
      .addMBB(Copy0MBB)
      .addReg(MI.getOperand(4).getReg())
      .addMBB(ThisMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return BB;
}

MachineBasicBlock *
XVMTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  unsigned Opc = MI.getOpcode();
  switch(Opc) {
    case XVM::PseudoSelectCC_rr:
    case XVM::PseudoSelectCC_ri:
      return EmitInstrWithCustomInserterSelectCC(MI, BB);
  }
  return BB;
}

EVT XVMTargetLowering::getSetCCResultType(const DataLayout &, LLVMContext &,
                                          EVT VT) const {
  return MVT::i64;
}

MVT XVMTargetLowering::getScalarShiftAmountTy(const DataLayout &DL,
                                              EVT VT) const {
  return MVT::i64;
}

bool XVMTargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM, Type *Ty,
                                              unsigned AS,
                                              Instruction *I) const {
  // No global is ever allowed as a base.
  if (AM.BaseGV)
    return false;

  switch (AM.Scale) {
  case 0: // "r+i" or just "i", depending on HasBaseReg.
    break;
  case 1:
    if (!AM.HasBaseReg) // allow "r+i".
      break;
    return false; // disallow "r+r" or "r+r+i".
  default:
    return false;
  }

  return true;
}

#endif
