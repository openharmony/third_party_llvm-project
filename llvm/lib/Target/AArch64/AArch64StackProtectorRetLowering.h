//===-- AArch64StackProtectorRetLowering.h ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

/// OHOS_LOCAL begin
#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64STACKPROTECTORRETLOWERING_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64STACKPROTECTORRETLOWERING_H

#include "llvm/CodeGen/StackProtectorRetLowering.h"

namespace llvm {

class AArch64StackProtectorRetLowering : public StackProtectorRetLowering {
public:
  /// insert stack-protector-ret instrumentation in prologue or epilogue.
  virtual void
  insertStackProtectorRetPrologue(MachineFunction &MF, MachineBasicBlock &MBB,
                                  GlobalVariable *cookie) const override;
  virtual void
  insertStackProtectorRetEpilogue(MachineFunction &MF, MachineInstr &MI,
                                  GlobalVariable *cookie) const override;

  /// Check if it is a return instruction.
  virtual bool instrIsRet(unsigned opcode) const override;

  /// Get a caller saved temporary register for the target architecture.
  virtual unsigned getTargetReg(void) const override;

  /// Set SSPRetReg as a callee saved register to push the computed stack guard value onto the stack.
  virtual void saveStackProtectorRetRegister(MachineFunction &MF,
                                             std::vector<CalleeSavedInfo> &CSI) const override;
};

} // namespace llvm

#endif
/// OHOS_LOCAL end
