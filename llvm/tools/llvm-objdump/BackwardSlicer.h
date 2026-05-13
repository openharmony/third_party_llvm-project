//===-- BackwardSlicer.h - Architecture Independent Backward Slicing Interface
//-----===//
#pragma once

#include "llvm/ADT/Optional.h"
#include "llvm/MC/MCInst.h"
#include <cstdint>
#include <vector>

namespace crash_analyzer {
namespace evaluator {

struct SimplifiedInst {
  uint64_t Address;
  llvm::MCInst Inst;
};

class BackwardSlicer {
public:
  virtual ~BackwardSlicer() = default;

  virtual llvm::Optional<uint64_t>
  sliceAndRecover(uint16_t TargetDwarfReg, uint64_t CrashPC, uint64_t FuncLowPC,
                  const std::vector<SimplifiedInst> &InstStream,
                  uint64_t InitialValue) = 0;
};

} // namespace evaluator
} // namespace crash_analyzer