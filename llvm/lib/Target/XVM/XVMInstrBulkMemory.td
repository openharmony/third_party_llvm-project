// XVMInstrBulkMemory.td - bulk memory codegen support --*- tablegen -*-
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// XVM bulk memory codegen constructs.
///
//===----------------------------------------------------------------------===//

def SDT_XVMMEMCPY_ri       : SDTypeProfile<0, 3, []>;
def SDT_XVMMEMCPY_rr       : SDTypeProfile<0, 3, []>;
def SDT_XVMMEMMOV_ri	   : SDTypeProfile<0, 3, []>;
def SDT_XVMMEMMOV_rr       : SDTypeProfile<0, 3, []>;
def SDT_XVMMEMSET_ri	   : SDTypeProfile<0, 3, []>;
def SDT_XVMMEMSET_rr       : SDTypeProfile<0, 3, []>;

def XVMmemcpy_ri       : SDNode<"XVMISD::MEMCPY", SDT_XVMMEMCPY_ri,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;
def XVMmemcpy_rr       : SDNode<"XVMISD::MEMCPY", SDT_XVMMEMCPY_rr,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;

def XVMmemmov_ri       : SDNode<"XVMISD::MEMMOV", SDT_XVMMEMMOV_ri,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;
def XVMmemmov_rr       : SDNode<"XVMISD::MEMMOV", SDT_XVMMEMMOV_rr,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;
def XVMmemset_ri       : SDNode<"XVMISD::MEMSET", SDT_XVMMEMSET_ri,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;
def XVMmemset_rr       : SDNode<"XVMISD::MEMSET", SDT_XVMMEMSET_rr,
                             [SDNPHasChain, SDNPInGlue, SDNPOutGlue,
                              SDNPMayStore, SDNPMayLoad]>;


let isCall=1,
    hasDelaySlot=0,
    Uses = [SP],
    Defs = [R0, R1, R2, R3, R4, R5] in {
      def MEMCPY_ri : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, i64imm:$len, variable_ops),
        "#memcpy dst: $dst, src: $src, len: $len",
        [(XVMmemcpy_ri XVMGPR:$dst, XVMGPR:$src, imm:$len)]>;
      def MEMCPY_rr : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len, variable_ops),
        "#memcpy dst: $dst, src: $src, len: $len",
        [(XVMmemcpy_rr XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len)]>;
      def MEMMOV_ri : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, i64imm:$len, variable_ops),
        "#memmov dst: $dst, src: $src, len: $len",
        [(XVMmemmov_ri XVMGPR:$dst, XVMGPR:$src, imm:$len)]>;
      def MEMMOV_rr : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len, variable_ops),
        "#memmov dst: $dst, src: $src, len: $len",
        [(XVMmemmov_rr XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len)]>;
      
      def MEMSET_ri : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, i64imm:$len, variable_ops),
        "#memset dst: $dst, src: $src, len: $len",
        [(XVMmemset_ri XVMGPR:$dst, XVMGPR:$src, imm:$len)]>;
      def MEMSET_rr : Pseudo<
        (outs),
        (ins XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len, variable_ops),
        "#memset dst: $dst, src: $src, len: $len",
        [(XVMmemset_rr XVMGPR:$dst, XVMGPR:$src, XVMGPR:$len)]>;
}


