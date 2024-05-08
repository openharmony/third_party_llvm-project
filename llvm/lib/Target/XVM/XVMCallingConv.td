//===-- XVMCallingConv.td - Calling Conventions XVM --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the XVM architecture.
//
//===----------------------------------------------------------------------===//

// XVM 64-bit C return-value convention.
def RetCC_XVM64 : CallingConv<[
  CCIfType<[i64], CCAssignToReg<[R0]>>
  ]>;

// XVM 64-bit C Calling convention.
def CC_XVM64 : CallingConv<[
  CCIfInReg<CCIfType<[i64], CCIfSRet<CCIfType<[i64], CCAssignToReg<[R0]>>>>>,

  // Return value Register for Structs
  CCIfSRet<CCIfType<[i64], CCAssignToReg<[R6]>>>,

  // Promote i8/i16/i32 args to i64
  CCIfType<[ i8, i16, i32 ], CCPromoteToType<i64>>,

  // All arguments get passed in integer registers if there is space.
  CCIfType<[i64], CCAssignToReg<[R0, R1, R2, R3, R4, R5 ]>>,

  // Could be assigned to the stack in 8-byte aligned units, but unsupported
  CCAssignToStack<8, 8>
]>;

def CSR : CalleeSavedRegs<(add R12, R13, R14, R15, R16, R17)>;
