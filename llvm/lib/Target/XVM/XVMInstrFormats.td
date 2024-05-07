//===-- XVMInstrFormats.td - XVM Instruction Formats ---*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

class XVMInstr<dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin = NoItinerary> : Instruction {
    field bits<32> Inst;
    let Namespace = "XVM";
    let DecoderNamespace = "XVM";
    let Size = 4;

    bits<8> opcode;
    let Inst{31-24} = opcode;

    dag OutOperandList = outs;
    dag InOperandList = ins;
    let AsmString = asmstr;
    let Pattern = pattern;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : XVMInstr<outs, ins, asmstr, pattern> {
  let Inst{31-0} = 0;
  let isPseudo = 1;
}

class ControFlowNoArgs<bits<8>opc, dag outs, dag ins, string asmstr, list<dag> pattern,
                       InstrItinClass itin = NoItinerary>
  : XVMInstr<outs, ins, asmstr, pattern, itin> {
  let opcode = opc;
  let Inst{23-0} = 0;
}

// Addressing modes.
def ADDRri : ComplexPattern<i64, 2, "SelectAddr", [], []>;
def FIri : ComplexPattern<i64, 2, "SelectFIAddr", [add, or], []>;

// Address operands
def MEMri : Operand<i64> {
  let PrintMethod = "printMemOperand";
  let EncoderMethod = "getMemoryOpValue";
  let DecoderMethod = "decodeMemoryOpValue";
  let MIOperandInfo = (ops XVMGPR, i16imm);
}

class XVMWidthModifier<bits<2> val> {
  bits<2> Value = val;
}

def XVM_W  : XVMWidthModifier<0x0>;
def XVM_H  : XVMWidthModifier<0x1>;
def XVM_B  : XVMWidthModifier<0x2>;
def XVM_D  : XVMWidthModifier<0x3>;

class LD_ST<bits<2> size, dag outs, dag ins, string asmstr, list<dag> pattern>
  : XVMInstr<outs, ins, asmstr, pattern>;

class STORE<XVMWidthModifier SizeOp, string OpcodeStr, list<dag> Pattern>
  : LD_ST<SizeOp.Value, (outs), (ins XVMGPR:$src, MEMri:$addr),
          ""#OpcodeStr#" $src, $addr", Pattern>;

class STOREi64<XVMWidthModifier SizeOp, string OpcodeStr, PatFrag OpNode>
  : STORE<SizeOp, !strconcat("str", OpcodeStr), [(OpNode i64:$src, ADDRri:$addr)]>;

class LOAD<XVMWidthModifier SizeOp, string OpcodeStr, list<dag> Pattern>
  : LD_ST<SizeOp.Value, (outs XVMGPR:$dst), (ins MEMri:$addr),
          ""#OpcodeStr#" $dst, $addr", Pattern>;

class LOADi64<XVMWidthModifier SizeOp, string OpcodeStr, PatFrag OpNode>
  : LOAD<SizeOp, !strconcat("ldr", OpcodeStr), [(set i64:$dst, (OpNode ADDRri:$addr))]>;

class ALU_rr<bits<8>opc, string asmstr, list<dag> pattern>
  : XVMInstr<(outs XVMGPR:$res), (ins XVMGPR:$src1, XVMGPR:$src2), ""#asmstr#" $res, $src1, $src2", pattern>;

class ALU_ri<bits<8>opc, string asmstr, list<dag> pattern>
  : XVMInstr<(outs XVMGPR:$res), (ins XVMGPR:$src1, simm14:$src2), ""#asmstr#" $res, $src1, #$src2", pattern>;

class ALU_r<bits<8>opc, string asmstr, list<dag> pattern>
  : XVMInstr<(outs XVMGPR:$res), (ins XVMGPR:$src1), ""#asmstr#" $res, $src1", pattern>;

class ALU_i<bits<8>opc, string asmstr, list<dag> pattern>
  : XVMInstr<(outs XVMGPR:$res), (ins uimm16:$src1), ""#asmstr#" $res, #$src1", pattern>;

class ALU_rii<bits<8>opc, string asmstr, list<dag> pattern>
  : XVMInstr<(outs XVMGPR:$res), (ins XVMGPR:$src3, uimm16:$src1, uimm16:$src2), ""#asmstr#" $res, #$src1, #$src2", pattern>{
    let Constraints = "$res = $src3";
  }

class TYPE_ALU_JMP<bits<8> Opc,
                   dag outs, dag ins, string asmstr, list<dag> pattern>
  : XVMInstr<outs, ins, asmstr, pattern> {
}

class BranchCC_rr<string cccode>
  : XVMInstr<(outs), (ins bb_op:$dst, XVMGPR: $op1, XVMGPR: $op2),
             "(if "#cccode#" $op1, $op2", []> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasCtrlDep = 1;
}

class BranchCC_ris<string cccode>
  : XVMInstr<(outs), (ins bb_op:$dst, XVMGPR: $op1, simm16: $op2),
             "(if "#cccode#" $op1, #$op2", []> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasCtrlDep = 1;
}

class BranchCC_riu<string cccode>
  : XVMInstr<(outs), (ins bb_op:$dst, XVMGPR: $op1, uimm16: $op2),
             "(if "#cccode#" $op1, #$op2", []> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasCtrlDep = 1;
}

class CALL_imm<string OpcodeStr>
    : TYPE_ALU_JMP<XVM_CALL_I.Value,
                   (outs),
                   (ins calltarget:$BrDst),
                   !strconcat(OpcodeStr, " #$BrDst"),
                   []> {
}
class CALL_REG<string OpcodeStr>
    : TYPE_ALU_JMP<XVM_CALL_R.Value,
                   (outs),
                   (ins XVMGPR:$BrDst),
                   !strconcat(OpcodeStr, " $BrDst"),
                   []> {
}

class MOV_GLOBAL_IMM64<string OpcodeStr>
    : XVMInstr<(outs XVMGPR:$dst),
               (ins u64imm:$imm),
                ""#OpcodeStr#" $dst, ${imm}",
                [(set XVMGPR:$dst, (i64 imm:$imm))]> {
}

class Ref_rr<string asmstr>
  : XVMInstr<(outs XVMGPR:$res), (ins XVMGPR:$src1, XVMGPR:$src2), ""#asmstr#" $res, $src1, $src2", []> {
}

class Ref_ri<string asmstr>
  : XVMInstr<(outs XVMGPR:$dst), (ins XVMGPR:$src1, simm14:$src2), ""#asmstr#" $dst, $src1, #$src2", []> {
}

class StrRef_ri<string asmstr>
  : XVMInstr<(outs), (ins XVMGPR:$dst, XVMGPR:$src1, simm14:$src2), ""#asmstr#" $dst, $src1, #$src2", []> {
}