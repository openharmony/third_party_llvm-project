#ifdef OHOS_LLVM
//===-- AArch64BackwardSlicer.cpp - AArch64 backward slicing analysis -----===//
//
// This file implements the AArch64-specific backward slicing analysis.
// It provides functionality to recover the value of a register at a previous
// program point by walking backwards through a stream of simplified MCInsts.
// It uses LLVM MC layer interfaces (MCRegisterInfo, MCInstrInfo) to process
// target-dependent information such as register aliasing and instruction
// descriptions.
//
//===----------------------------------------------------------------------===//
#include "AArch64BackwardSlicer.h"
#include "MCTargetDesc/AArch64MCTargetDesc.h"
#include <optional>

using namespace llvm;

namespace crash_analyzer {
namespace evaluator {

AArch64BackwardSlicer::AArch64BackwardSlicer(const llvm::MCRegisterInfo *MRI,
                                             const llvm::MCInstrInfo *MII)
    : MRI(MRI), MII(MII) {}

unsigned AArch64BackwardSlicer::getLLVMRegFromDwarf(uint16_t DwarfReg) const {
  auto LLVMReg = MRI->getLLVMRegNum(DwarfReg, true);

  // If the DWARF number cannot be found, return NoRegister (0).
  if (!LLVMReg)
    return llvm::AArch64::NoRegister;

  return *LLVMReg;
}

std::optional<uint64_t> AArch64BackwardSlicer::sliceAndRecover(
    uint16_t TargetDwarfReg, uint64_t CrashPC, uint64_t FuncLowPC,
    const std::vector<SimplifiedInst> &InstStream, uint64_t InitialValue) {

  uint64_t RunningValue = InitialValue;
  unsigned CurrentTargetReg = getLLVMRegFromDwarf(TargetDwarfReg);

  if (!CurrentTargetReg)
    return std::nullopt;

  for (auto It = InstStream.rbegin(); It != InstStream.rend(); ++It) {
    if (It->Address >= CrashPC)
      continue;
    if (It->Address < FuncLowPC)
      break;

    const llvm::MCInst &Inst = It->Inst;
    unsigned Opcode = Inst.getOpcode();

    // Use MCInstrDesc to detect control-flow altering instructions
    const llvm::MCInstrDesc &Desc = MII->get(Opcode);
    if (Desc.isBranch() || Desc.isCall() || Desc.isReturn())
      return std::nullopt;

    // Ensure the instruction has at least one operand and the first operand
    // is a register (the typical destination register).
    if (Inst.getNumOperands() == 0 || !Inst.getOperand(0).isReg())
      continue;

    unsigned DstReg = Inst.getOperand(0).getReg();

    // Check if the instruction modifies the register we are tracking.
    // isSubRegisterEq correctly handles sub-register aliases (e.g., W0 vs X0).
    if (!MRI->isSubRegisterEq(CurrentTargetReg, DstReg))
      continue;

    // Match register-to-register MOV instructions.
    // In AArch64, MOV Xd, Xm is encoded as ORR Xd, XZR, Xm.
    if (Opcode == llvm::AArch64::ORRXrs && Inst.getNumOperands() >= 3 &&
        Inst.getOperand(1).isReg() &&
        Inst.getOperand(1).getReg() == llvm::AArch64::XZR) {

      // The source of the move is operand 2. Switch tracking to that register.
      CurrentTargetReg = Inst.getOperand(2).getReg();
      continue;
    }

    // Match ADD/SUB immediate instructions.
    if ((Opcode == llvm::AArch64::ADDXri || Opcode == llvm::AArch64::SUBXri) &&
        Inst.getNumOperands() >= 3) {

      // Obtain the immediate value directly from the MCInst operand.
      if (Inst.getOperand(2).isImm()) {
        int64_t Imm = Inst.getOperand(2).getImm();

        // When slicing backwards:
        // ADD Xd, Xn, #imm  ->  Xn = Xd - imm
        if (Opcode == llvm::AArch64::ADDXri)
          RunningValue -= Imm;
        else
          RunningValue += Imm;

        continue;
      }
    }

    // Any other modification to the tracked register that we cannot invert
    // leads to failure.
    return std::nullopt;
  }

  return RunningValue;
}

} // namespace evaluator
} // namespace crash_analyzer
#endif // OHOS_LLVM
