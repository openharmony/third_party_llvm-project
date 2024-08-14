//=- LoongArchCallingConv.td - Calling Conventions LoongArch -*- tablegen -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the LoongArch architecture.
//
//===----------------------------------------------------------------------===//

#ifdef ARK_GC_SUPPORT
def CSR_ILP32S_LP64S
    : CalleeSavedRegs<(add (sequence "R%u", 23, 31), R1, R22)>;

def CSR_ILP32F_LP64F
    : CalleeSavedRegs<(add (sequence "R%u", 23, 31), (sequence "F%u", 24, 31), R1, R22)>;

def CSR_ILP32D_LP64D
    : CalleeSavedRegs<(add (sequence "R%u", 23, 31), (sequence "F%u_64", 24, 31), R1, R22)>;

#else
def CSR_ILP32S_LP64S
    : CalleeSavedRegs<(add R1, (sequence "R%u", 22, 31))>;

def CSR_ILP32F_LP64F
    : CalleeSavedRegs<(add CSR_ILP32S_LP64S, (sequence "F%u", 24, 31))>;

def CSR_ILP32D_LP64D
    : CalleeSavedRegs<(add CSR_ILP32S_LP64S, (sequence "F%u_64", 24, 31))>;
#endif

// Needed for implementation of LoongArchRegisterInfo::getNoPreservedMask()
def CSR_NoRegs : CalleeSavedRegs<(add)>;

#ifdef ARK_GC_SUPPORT
// The WebKit_JS calling convention only passes the first argument (the callee)
// in register and the remaining arguments on stack. We allow 32bit stack slots,
// so that WebKit can write partial values in the stack and define the other
// 32bit quantity as undef.
def CC_LoongArch_WebKit_JS : CallingConv<[
  // Promote i8/i16 arguments to i32 like x64.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Only the first integer argument is passed in register.
  CCIfType<[i32, i64], CCAssignToReg<[R4]>>,

  // The remaining integer arguments are passed on the stack. 32bit integer and
  // floating-point arguments are aligned to 4 byte and stored in 4 byte slots.
  // 64bit integer and floating-point arguments are aligned to 8 byte and stored
  // in 8 byte stack slots.
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>,
  CCIfType<[i64, f64], CCAssignToStack<8, 8>>
]>;

// Note:
// 1. aarch64 supports 8 return value regs while x64 only supports 1.
// 2. aarch64 supports f32 and f64 while x64 doesn't.
// 3. aarch64 returns i32/i64/f32/f64 via different regs while x64 only use RAX.
def RetCC_LoongArch_WebKit_JS : CallingConv<[
  // Promote all types to i64 like x64.
  CCIfType<[i8, i16, i32], CCPromoteToType<i64>>,

  CCIfType<[i64], CCAssignToReg<[R4]>>,
  CCIfType<[f32], CCAssignToReg<[F0]>>,
  CCIfType<[f64], CCAssignToReg<[F0_64]>>,
]>;
#endif
