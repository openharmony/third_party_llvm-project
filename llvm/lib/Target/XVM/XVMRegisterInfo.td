//===-- XVMRegisterInfo.td - XVM Register defs -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the XVM register file
//===----------------------------------------------------------------------===//


// Registers are identified with 4-bit ID numbers.
// Ri - 64-bit integer registers
class Ri<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "XVM";
}

foreach I = 0-17 in {
  // 64-bit Integer registers
  def R#I  : Ri<I,  "r"#I>,  DwarfRegNum<[I]>;
}

def SP : Ri<18, "sp">, DwarfRegNum<[31]>;

def XVMGPR : RegisterClass<"XVM", [i64], 64, (add
  R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17,
  R0 // Return value
)>;

def XVMRR : RegisterClass<"XVM", [i64], 64, (add SP)>;