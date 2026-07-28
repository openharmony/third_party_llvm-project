#ifdef OHOS_LLVM
//===-- AArch64BackwardSlicer.h - AArch64 Specific Backward Slicing -------===//
#pragma once

#include "BackwardSlicer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include <optional>

namespace crash_analyzer {
namespace evaluator {

class AArch64BackwardSlicer : public BackwardSlicer {
public:
  AArch64BackwardSlicer(const llvm::MCRegisterInfo *MRI,
                        const llvm::MCInstrInfo *MII);

  std::optional<uint64_t>
  sliceAndRecover(uint16_t TargetDwarfReg, uint64_t CrashPC, uint64_t FuncLowPC,
                  const std::vector<SimplifiedInst> &InstStream,
                  uint64_t InitialValue) override;

private:
  const llvm::MCRegisterInfo *MRI;
  const llvm::MCInstrInfo *MII;

  unsigned getLLVMRegFromDwarf(uint16_t DwarfReg) const;
};

} // namespace evaluator
} // namespace crash_analyzer
#endif // OHOS_LLVM
