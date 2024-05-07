//===-- XVM.td - Describe the XVM Target Machine -----------*- tablegen -*-===//

//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "XVMRegisterInfo.td"
include "XVMCallingConv.td"
include "XVMInstrInfo.td"

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;


def XVMInstrInfo : InstrInfo;

def XVMInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def Common : SubtargetFeature<"common", "isCommon",
                              "true", "common 64-bit subtarget">;

def XVMAsmParser : AsmParser {
  bit HasMnemonicFirst = 0;
}

def XVMAsmParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "XVM";
  string BreakCharacters = ".";
  string TokenizingCharacters = "#()[]=:.<>!+*";
}

def XVM : Target {
  let InstructionSet = XVMInstrInfo;
  let AssemblyWriters = [XVMInstPrinter];
  let AssemblyParsers = [XVMAsmParser];
  let AssemblyParserVariants = [XVMAsmParserVariant];
}
