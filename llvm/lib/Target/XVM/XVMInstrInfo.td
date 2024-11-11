//===-- XVMInstrInfo.td - Target Description for XVM Target -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the XVM instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

//===---------------------------------------------------------------------===//
// XVM specific DAG Nodes
//===---------------------------------------------------------------------===//

def SDTXVMBrCC_rr : SDTypeProfile<0, 4, []>;
def SDTXVMBrCC_ri : SDTypeProfile<0, 4, []>;
def SDTXVMSelectCC: SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                         SDTCisSameAs<0, 4>,
                                         SDTCisSameAs<4, 5>]>;

def XVMbrccrr      : SDNode<"XVMISD::BR_CC", SDTXVMBrCC_rr, [SDNPHasChain]>;
def XVMbrccri      : SDNode<"XVMISD::BR_CC", SDTXVMBrCC_ri, [SDNPHasChain]>;
def XVMretflag     : SDNode<"XVMISD::RET_FLAG", SDTNone,
                            [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def XVMselectcc     : SDNode<"XVMISD::SELECT_CC", SDTXVMSelectCC, [SDNPInGlue]>;

//===----------------------------------------------------------------------===//
// XVM specific Operands
//===----------------------------------------------------------------------===//

def bb_op : Operand<OtherVT>;

def uimm6 : Operand<i64>, ImmLeaf<i64, [{return Imm >= 0 && Imm < 64;}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    return MCOp.evaluateAsConstantImm(Imm) && isUInt<6>(Imm);
  }];
}
def uimm16 : Operand<i64>, ImmLeaf<i64, [{return Imm >= 0 && Imm < 65536;}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    return MCOp.evaluateAsConstantImm(Imm) && isUInt<16>(Imm);
  }];
}
def simm16 : Operand<i64>, ImmLeaf<i64, [{return Imm >= -32768 && Imm < 32768;}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    return MCOp.evaluateAsConstantImm(Imm) && isInt<16>(Imm);
  }];
}
def simm14 : Operand<i64>, ImmLeaf<i64, [{return Imm >= -8192 && Imm < 8192;}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    return MCOp.evaluateAsConstantImm(Imm) && isInt<14>(Imm);
  }];
}
def uimm14 : Operand<i64>, ImmLeaf<i64, [{return Imm >= 0 && Imm < 16384;}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    return MCOp.evaluateAsConstantImm(Imm) && isUInt<14>(Imm);
  }];
}


//===----------------------------------------------------------------------===//
// XVM Instruction
//===----------------------------------------------------------------------===//

def i64immSExt32 : PatLeaf<(i64 imm),
                [{return isInt<32>(N->getSExtValue()); }]>;
def u64imm     : Operand<i64>;
def calltarget : Operand<i64>;
class XVMOpcode<bits<8> val> {
  bits<8> Value = val;
}
def XVM_CALL_R : XVMOpcode<0b00000010>;
def XVM_CALL_I : XVMOpcode<0b00000011>;
def XVM_MOV_RR : XVMOpcode<0b00110000>;
def XVM_MOV_RI : XVMOpcode<0b00110001>;

def SDT_XVMCallSeqStart : SDCallSeqStart<[SDTCisVT<0, iPTR>,
                                          SDTCisVT<1, iPTR>]>;
def SDT_XVMCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, iPTR>, SDTCisVT<1, iPTR>]>;
def SDT_XVMCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def XVMcall             : SDNode<"XVMISD::CALL", SDT_XVMCall,
                                 [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def XVMcallseq_start    : SDNode<"ISD::CALLSEQ_START", SDT_XVMCallSeqStart,
                                 [SDNPHasChain, SDNPOutGlue]>;
def XVMcallseq_end      : SDNode<"ISD::CALLSEQ_END",   SDT_XVMCallSeqEnd,
                                 [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

include "XVMInstrFormats.td"

//----------------------------------------------------------------

let isCall=1,
    hasDelaySlot=0,
    Uses = [SP],
    Defs = [R0, R1, R2, R3, R4, R5] in {
      def CALL_IMM : CALL_imm<"call">;
      def CALL_REG : CALL_REG<"call">;
}
let Defs = [SP], Uses = [SP], isCodeGenOnly = 1 in {
  def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                "#ADJCALLSTACKDOWN $amt1 $amt2",
                                [(XVMcallseq_start timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : Pseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                                "#ADJCALLSTACKUP $amt1 $amt2",
                                [(XVMcallseq_end timm:$amt1, timm:$amt2)]>;
}

def : Pat<(XVMcall tglobaladdr:$dst), (CALL_IMM tglobaladdr:$dst)>;
def : Pat<(XVMcall texternalsym:$dst), (CALL_IMM texternalsym:$dst)>;
def : Pat<(XVMcall imm:$dst), (CALL_IMM imm:$dst)>;
def : Pat<(XVMcall XVMGPR:$dst), (CALL_REG XVMGPR:$dst)>;

let usesCustomInserter = 1, isCodeGenOnly = 1 in {
def PseudoSelectCC_rr : Pseudo<(outs XVMGPR:$dst),
                      (ins XVMGPR:$lhs, XVMGPR:$rhs, i64imm:$imm, XVMGPR:$src, XVMGPR:$src2),
                      "; Select PSEUDO $dst = $lhs $imm $rhs ? $src : $src2",
                      [(set i64:$dst,
                       (XVMselectcc i64:$lhs, i64:$rhs, (i64 imm:$imm), i64:$src, i64:$src2))]>;

def PseudoSelectCC_ri : Pseudo<(outs XVMGPR:$dst),
                    (ins XVMGPR:$lhs, i64imm:$rhs, i64imm:$imm, XVMGPR:$src, XVMGPR:$src2),
                    "# Select PSEUDO $dst = $lhs $imm $rhs ? $src : $src2",
                    [(set i64:$dst,
                     (XVMselectcc i64:$lhs, (i64immSExt32:$rhs), (i64 imm:$imm), i64:$src, i64:$src2))]>;
}

let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
  def LD_global_imm64 : MOV_GLOBAL_IMM64<"mov">;
  def PEI_PROLOGUE_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src1, XVMGPR:$src2, u64imm:$imm),
                      "",
                      []>;
  def PEI_EPILOGUE_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src1, XVMGPR:$src2, u64imm:$imm),
                      "",
                      []>;
  def SubRef_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src, uimm14:$imm),
                      "subref $dst, $src, #$imm",
                      []>;
  def AddRef_ri  : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src, uimm14:$imm),
                      "addref $dst, $src, #$imm",
                      []>;
  def OrRef_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src, uimm14:$imm),
                      "orref $dst, $src, #$imm",
                      []>;
  def XorRef_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src, uimm14:$imm),
                      "xorref $dst, $src, #$imm",
                      []>;
  def AndRef_ri : XVMInstr<(outs XVMGPR:$dst),
                      (ins XVMGPR:$src, uimm14:$imm),
                      "andref $dst, $src, #$imm",
                      []>;
}
def FI_ri
    : XVMInstr<(outs XVMGPR:$dst),
               (ins MEMri:$addr),
               "",
               [(set i64:$dst, FIri:$addr)]> {
  // This is a tentative instruction, and will be replaced
  // with MOV_rr and ADD_ri in PEI phase
}
def SDT_XVMWrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def XVMWrapper     : SDNode<"XVMISD::Wrapper", SDT_XVMWrapper>;
/* load 64-bit global addr into register: such as function ptr as parameters */
def : Pat<(XVMWrapper tglobaladdr:$in), (LD_global_imm64 tglobaladdr:$in)>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RETURN : ControFlowNoArgs<0b00000001, (outs), (ins), "ret", [(XVMretflag)]>;
}

def : InstAlias<"ret", (RETURN)>;

let isBranch = 1, hasCtrlDep = 1 in {
  def LOOP : ControFlowNoArgs<0b00000000, (outs), (ins), "(loop", []>;
  def END_LOOP : ControFlowNoArgs<0b00000000, (outs), (ins), ")", []>;
  def BLOCK : ControFlowNoArgs<0b00000000, (outs), (ins), "(block", []>;
  def END_BLOCK : ControFlowNoArgs<0b00000000, (outs), (ins), ")", []>;
  def BREAK : ControFlowNoArgs<0b00000000, (outs), (ins), "break", []>;
  def BREAK_IMM : ControFlowNoArgs<0b00001000, (outs), (ins uimm6:$imm), "break #$imm", []>;
  def CONTINUE : ControFlowNoArgs<0b00000000, (outs), (ins), "continue", []>;
  def THEN : ControFlowNoArgs<0b00000000, (outs), (ins), "(then", []>;
  def END_THEN : ControFlowNoArgs<0b00000000, (outs), (ins), ") ;; end_then", []>;
  def ELSE : ControFlowNoArgs<0b00000000, (outs), (ins), "(else", []>;
  def END_ELSE : ControFlowNoArgs<0b00000000, (outs), (ins), ")", []>;
  def END_IF : ControFlowNoArgs<0b00000000, (outs), (ins), ")", []>;
}

let mayStore = 1 in {
  def STW : STOREi64<XVM_W, "w", truncstorei32>;
  def STH : STOREi64<XVM_H, "h", truncstorei16>;
  def STB : STOREi64<XVM_B, "b", truncstorei8>;
  def STD : STOREi64<XVM_D, "", store>;
}

let mayLoad = 1 in {
  def LDW_z : LOADi64<XVM_W, "w", zextloadi32>;
  def LDW : LOADi64<XVM_W, "w", extloadi32>;
  def LDH_z : LOADi64<XVM_H, "h", zextloadi16>;
  def LDH : LOADi64<XVM_H, "h", extloadi16>;
  def LDB_z : LOADi64<XVM_B, "b",  zextloadi8>;
  def LDB : LOADi64<XVM_B, "b",  extloadi8>;
  def LDD : LOADi64<XVM_D, "", load>;
}

def AddRef_rr : Ref_rr<"addref">;
def SubRef_rr : Ref_rr<"subref">;
def DifRef_rr : Ref_rr<"difref">;
def OrRef_rr  : Ref_rr<"orref">;
def XorRef_rr : Ref_rr<"xorref">;
def AndRef_rr : Ref_rr<"andref">;

def LoadRef_ri : Ref_ri<"ldrref">;

def StoreRef_ri : StrRef_ri<"strref">;

let isAsCheapAsAMove = 1 in {
  def ADD_rr : ALU_rr<0b00110100, "add", [(set XVMGPR:$res, (add i64:$src1, i64:$src2))]>;
  def ADD_ri : ALU_ri<0b00110101, "add", [(set XVMGPR:$res, (add i64:$src1, uimm14:$src2))]>;
  def SUB_rr : ALU_rr<0b00110110, "sub", [(set XVMGPR:$res, (sub i64:$src1, i64:$src2))]>;
  def SUB_ri : ALU_ri<0b00110111, "sub", [(set XVMGPR:$res, (sub i64:$src1, uimm14:$src2))]>;
  def OR_rr  : ALU_rr<0b01000000, "or",  [(set XVMGPR:$res, (or  i64:$src1, i64:$src2))]>;
  def OR_ri  : ALU_ri<0b01000001, "or",  [(set XVMGPR:$res, (or  i64:$src1, uimm14:$src2))]>;
  def XOR_rr : ALU_rr<0b01000010, "xor", [(set XVMGPR:$res, (xor i64:$src1, i64:$src2))]>;
  def XOR_ri : ALU_ri<0b01000011, "xor", [(set XVMGPR:$res, (xor i64:$src1, uimm14:$src2))]>;
  def AND_rr : ALU_rr<0b01000100, "and", [(set XVMGPR:$res, (and i64:$src1, i64:$src2))]>;
  def AND_ri : ALU_ri<0b01000101, "and", [(set XVMGPR:$res, (and i64:$src1, uimm14:$src2))]>;
  def ASR_rr : ALU_rr<0b01000110, "asr", [(set XVMGPR:$res, (sra i64:$src1, i64:$src2))]>;
  def ASR_ri : ALU_ri<0b01000111, "asr", [(set XVMGPR:$res, (sra i64:$src1, uimm6:$src2))]>;
  def LSR_rr : ALU_rr<0b01001000, "lsr", [(set XVMGPR:$res, (srl i64:$src1, i64:$src2))]>;
  def LSR_ri : ALU_ri<0b01001001, "lsr", [(set XVMGPR:$res, (srl i64:$src1, uimm6:$src2))]>;
  def LSL_rr : ALU_rr<0b01001010, "lsl", [(set XVMGPR:$res, (shl i64:$src1, i64:$src2))]>;
  def LSL_ri : ALU_ri<0b01001011, "lsl", [(set XVMGPR:$res, (shl i64:$src1, uimm6:$src2))]>;
let isMoveReg = 1 in {
  def MOV_rr : ALU_r<0b00110000, "mov", []>;
  def MOVK_rr : ALU_ri<0b00110010, "movk", []>;
}
let isMoveImm = 1 in {
  def MOV_ri : ALU_i<0b00110001, "mov", [(set XVMGPR:$res, simm16:$src1)]>;
  def MOVK_ri : ALU_rii<0b00110011, "movk", []>;
}
}
  def MUL_rr  : ALU_rr<0b00111000, "mul",  [(set XVMGPR:$res, (mul  i64:$src1, i64:$src2))]>;
  def UDIV_rr : ALU_rr<0b00111001, "udiv", [(set XVMGPR:$res, (udiv i64:$src1, i64:$src2))]>;
  def SDIV_rr : ALU_rr<0b00111010, "sdiv", [(set XVMGPR:$res, (sdiv i64:$src1, i64:$src2))]>;
  def UMOD_rr : ALU_rr<0b00111011, "umod", [(set XVMGPR:$res, (urem i64:$src1, i64:$src2))]>;
  def SMOD_rr : ALU_rr<0b00111100, "smod", [(set XVMGPR:$res, (srem i64:$src1, i64:$src2))]>;


def NEG_r : ALU_r<0b00110011, "neg", []>;

def REV16 : XVMInstr<(outs XVMGPR:$dst),
                           (ins XVMGPR:$src1, i64imm:$imm1, i64imm:$imm2),
                           "rev16 $dst, $src1",
                           [(set i64:$dst,
                           (or (shl i64:$src1, (i64 imm:$imm1)), (srl (and i64:$src1, (i64 imm:$imm2)), (i64 imm:$imm1))))]>;

def REV32 : XVMInstr<(outs XVMGPR:$dst),
                            (ins XVMGPR:$src1,
                            i64imm:$imm1, i64imm:$imm2, i64imm:$imm3, i64imm:$imm4, i64imm:$imm5),
                            "rev32 $dst, $src1",
                            [(set i64:$dst,
                              (or (or (shl i64:$src1, (i64 imm:$imm1)), (and (shl i64:$src1, (i64 imm:$imm2)), (i64 imm:$imm3))), 
                                  (or (and (srl (and i64:$src1, (i64 imm:$imm4)), (i64 imm:$imm2)), (i64 imm:$imm5)), (srl (and i64:$src1, (i64 imm:$imm4)), (i64 imm:$imm1)))))]>;

def REV64 : XVMInstr<(outs XVMGPR:$dst),
                           (ins XVMGPR:$src1,
                            i64imm:$imm1, i64imm:$imm2, i64imm:$imm3, i64imm:$imm4, i64imm:$imm5,
                            i64imm:$imm6, i64imm:$imm7, i64imm:$imm8, i64imm:$imm9, i64imm:$imm10),
                            "rev64 $dst, $src1",
                            [(set i64:$dst,
                             (or (or (or (shl i64:$src1, (i64 imm:$imm2)), (and (shl i64:$src1, (i64 imm:$imm3)), (i64 imm:$imm4))), 
                                     (or (and (shl i64:$src1, (i64 imm:$imm5)), (i64 imm:$imm6)), (and (shl i64:$src1, (i64 imm:$imm1)), (i64 imm:$imm7)))), 
                                  (or (or (and (srl i64:$src1, (i64 imm:$imm1)), (i64 imm:$imm8)), 
                                      (and (srl i64:$src1, (i64 imm:$imm5)), (i64 imm:$imm9))), (or (and (srl i64:$src1, (i64 imm:$imm3)), (i64 imm:$imm10)), (srl i64:$src1, (i64 imm:$imm2))))))]>;

let AddedComplexity = 1 in
def : Pat < (sub 0, i64:$src2), (NEG_r i64:$src2)>;

// mov combinations
def GetMovImm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant( (N->getSExtValue() & 0x7fff), SDLoc(N), MVT::i64);
}]>;
def GetMovk0Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant( (N->getSExtValue() & 0xffff), SDLoc(N), MVT::i64);
}]>;
def GetMovk1Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant( ( (N->getSExtValue() >> 16 ) & 0xffff), SDLoc(N), MVT::i64);
}]>;
def GetMovk2Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant( ( (N->getSExtValue() >> 32 ) & 0xffff), SDLoc(N), MVT::i64);
}]>;
def GetMovk3Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant( ( (N->getSExtValue() >> 48 ) & 0xffff), SDLoc(N), MVT::i64);
}]>;

def IsLess15Bits : ImmLeaf<i64, [{
  return (long long)Imm <= 32767;
}], GetMovImm>;
def IsLess16Bits : ImmLeaf<i64, [{
  return (long long)Imm > 32767 && (long long)Imm <= 65535;
}], GetMovk0Imm>;
def IsOver16Bits : ImmLeaf<i64, [{
  return (long long)Imm > 65535 && (long long)Imm <= 4294967295;
}], GetMovk1Imm>;
def IsOver32Bits : ImmLeaf<i64, [{
  return (long long)Imm > 4294967295 && (long long)Imm <= 281474976710655;
}], GetMovk2Imm>;
def IsOver48Bits : ImmLeaf<i64, [{
  return (long long)Imm > 281474976710655 || (long long)Imm <= -32769;
}], GetMovk3Imm>;

def : Pat<
  (i64 IsLess16Bits:$src1),
  (MOVK_ri (MOV_ri IsLess15Bits:$src1), IsLess16Bits:$src1, 0)
>;
def : Pat<
  (i64 IsOver16Bits:$src1),
  (MOVK_ri (MOVK_ri (MOV_ri IsLess15Bits:$src1), IsLess16Bits:$src1, 0), IsOver16Bits:$src1, 1)
>;
def : Pat<
  (i64 IsOver32Bits:$src1),
  (MOVK_ri (MOVK_ri (MOVK_ri (MOV_ri IsLess15Bits:$src1), IsLess16Bits:$src1, 0), IsOver16Bits:$src1, 1), IsOver32Bits:$src1, 2)
>;
def : Pat<
  (i64 IsOver48Bits:$src1),
  (MOVK_ri (MOVK_ri (MOVK_ri (MOVK_ri (MOV_ri IsLess15Bits:$src1), IsLess16Bits:$src1, 0), IsOver16Bits:$src1, 1), IsOver32Bits:$src1, 2), IsOver48Bits:$src1, 3)
>;


let isBranch = 1, isTerminator = 1, hasCtrlDep = 1 in {
  def BR: XVMInstr<(outs), (ins bb_op:$dst), "br \t$dst", [(br bb:$dst)]>;
  def LOOP_BR: XVMInstr<(outs), (ins bb_op:$dst), "br \t$dst", [(br bb:$dst)]>;
}

//TODO: all signed and unsigned comparisons are the same for the moment
def BUEQ_rr  : BranchCC_rr<"ueq">;
def BSNEQ_rr : BranchCC_rr<"une">;
def BSGE_rr  : BranchCC_rr<"sge">;
def BUGE_rr : BranchCC_rr<"uge">;
def BSLE_rr  : BranchCC_rr<"sle">;
def BULE_rr : BranchCC_rr<"ule">;
def BSGT_rr  : BranchCC_rr<"sgt">;
def BUGT_rr : BranchCC_rr<"ugt">;
def BSLT_rr  : BranchCC_rr<"slt">;
def BULT_rr : BranchCC_rr<"ult">;

def BSEQ_ri  : BranchCC_ris<"seq">;
def BUEQ_ri : BranchCC_riu<"ueq">;
def BSNEQ_ri : BranchCC_ris<"sne">;
def BUNEQ_ri: BranchCC_riu<"une">;
def BSGE_ri  : BranchCC_ris<"sge">;
def BUGE_ri : BranchCC_riu<"uge">;
def BSLE_ri  : BranchCC_ris<"sle">;
def BULE_ri : BranchCC_riu<"ule">;
def BSGT_ri  : BranchCC_ris<"sgt">;
def BUGT_ri : BranchCC_riu<"ugt">;
def BSLT_ri  : BranchCC_ris<"slt">;
def BULT_ri : BranchCC_riu<"ult">;

def LOOP_BUEQ_rr  : BranchCC_rr<"ueq">;
def LOOP_BSNEQ_rr : BranchCC_rr<"une">;
def LOOP_BSGE_rr  : BranchCC_rr<"sge">;
def LOOP_BUGE_rr : BranchCC_rr<"uge">;
def LOOP_BSLE_rr  : BranchCC_rr<"sle">;
def LOOP_BULE_rr : BranchCC_rr<"ule">;
def LOOP_BSGT_rr  : BranchCC_rr<"sgt">;
def LOOP_BUGT_rr : BranchCC_rr<"ugt">;
def LOOP_BSLT_rr  : BranchCC_rr<"slt">;
def LOOP_BULT_rr : BranchCC_rr<"ult">;

def LOOP_BSEQ_ri  : BranchCC_ris<"seq">;
def LOOP_BUEQ_ri : BranchCC_riu<"ueq">;
def LOOP_BSNEQ_ri : BranchCC_ris<"sne">;
def LOOP_BUNEQ_ri: BranchCC_riu<"une">;
def LOOP_BSGE_ri  : BranchCC_ris<"sge">;
def LOOP_BUGE_ri : BranchCC_riu<"uge">;
def LOOP_BSLE_ri  : BranchCC_ris<"sle">;
def LOOP_BULE_ri : BranchCC_riu<"ule">;
def LOOP_BSGT_ri  : BranchCC_ris<"sgt">;
def LOOP_BUGT_ri : BranchCC_riu<"ugt">;
def LOOP_BSLT_ri  : BranchCC_ris<"slt">;
def LOOP_BULT_ri : BranchCC_riu<"ult">;

def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETNE, bb:$dst),
          (BSNEQ_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETEQ, bb:$dst),
          (BUEQ_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETGE, bb:$dst),
          (BSGE_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETUGE, bb:$dst),
          (BUGE_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETLE, bb:$dst),
          (BSLE_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETULE, bb:$dst),
          (BULE_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETGT, bb:$dst),
          (BSGT_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETUGT, bb:$dst),
          (BUGT_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETLT, bb:$dst),
          (BSLT_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;
def : Pat<(XVMbrccrr i64:$op1, i64:$op2, SETULT, bb:$dst),
          (BULT_rr bb:$dst, XVMGPR:$op1, XVMGPR:$op2)>;

def DoesNotFitIn15Bits : ImmLeaf<i64, [{
  return !( (long long)Imm <= 32767 && (long long)Imm >= 0 );
}]>;
class BigImmediateBRCC<CondCode condcode, BranchCC_rr outnode>
    : Pat<(XVMbrccri i64:$op1, DoesNotFitIn15Bits:$op2, condcode, bb:$dst),
          (outnode bb:$dst, XVMGPR:$op1, i64:$op2)>;

def SignedDoesNotFitIn15Bits : ImmLeaf<i64, [{
  return !( (long long)Imm <= 16383  && (long long)Imm >= -16383  );
}]>;
class SignedBigImmediateBRCC<CondCode condcode, BranchCC_rr outnode>
    : Pat<(XVMbrccri i64:$op1, SignedDoesNotFitIn15Bits:$op2, condcode, bb:$dst),
          (outnode bb:$dst, XVMGPR:$op1, i64:$op2)>;

def : SignedBigImmediateBRCC<SETNE, BSNEQ_rr>;
def : SignedBigImmediateBRCC<SETEQ, BUEQ_rr>;
def : SignedBigImmediateBRCC<SETGE, BSGE_rr>;
def : BigImmediateBRCC<SETUGE, BUGE_rr>;
def : SignedBigImmediateBRCC<SETLE, BSLE_rr>;
def : BigImmediateBRCC<SETULE, BULE_rr>;
def : SignedBigImmediateBRCC<SETGT, BSGT_rr>;
def : BigImmediateBRCC<SETUGT, BUGT_rr>;
def : SignedBigImmediateBRCC<SETLT, BSLT_rr>;
def : BigImmediateBRCC<SETULT, BULT_rr>;

def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETNE, bb:$dst),
          (BSNEQ_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETEQ, bb:$dst),
          (BSEQ_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETGE, bb:$dst),
          (BSGE_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETUGE, bb:$dst),
          (BUGE_ri bb:$dst, XVMGPR:$op1, uimm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETLE, bb:$dst),
          (BSLE_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETULE, bb:$dst),
          (BULE_ri bb:$dst, XVMGPR:$op1, uimm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETGT, bb:$dst),
          (BSGT_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETUGT, bb:$dst),
          (BUGT_ri bb:$dst, XVMGPR:$op1, uimm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETLT, bb:$dst),
          (BSLT_ri bb:$dst, XVMGPR:$op1, simm16:$op2)>;
def : Pat<(XVMbrccri i64:$op1, imm:$op2, SETULT, bb:$dst),
          (BULT_ri bb:$dst, XVMGPR:$op1, uimm16:$op2)>;

include "XVMInstrBulkMemory.td"