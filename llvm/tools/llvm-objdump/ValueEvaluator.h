#ifdef OHOS_LLVM
//===-- ValueEvaluator.h - dwarf Expression Evaluation Interface ----------===//
#pragma once

#include "AArch64BackwardSlicer.h"
#include "AArch64CrashSnapshot.h"
#include "BackwardSlicer.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDie.h"
#include "llvm/DebugInfo/DWARF/DWARFUnit.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace crash_analyzer {
namespace evaluator {

enum class EvalStatus { Resolved, OptimizedOut, MemoryFault, NeedsMCAnalysis };

struct EvaluatedVariable {
  std::string Name;
  std::string TypeName;
  EvalStatus Status;
  std::string FormattedValue;
  std::string LocationType;

  struct MCAnalysisRequest {
    std::optional<uint16_t> TargetDwarfReg;
  } McRequest;
};

class ValueEvaluator {
public:
  ValueEvaluator(std::shared_ptr<ThreadContext> ThreadCtx,
                 BackwardSlicer *Slicer = nullptr);

  EvaluatedVariable evaluate(llvm::DWARFDie VarDie,
                             llvm::ArrayRef<uint8_t> ExprData, uint64_t CrashPC,
                             uint64_t FuncLowPC,
                             const std::vector<SimplifiedInst> &InstStream);

  uint64_t getVariableByteSize(llvm::DWARFDie VarDie);

private:
  std::shared_ptr<ThreadContext> ThreadCtx;
  BackwardSlicer *Slicer;

  std::string formatHex(uint64_t Val) const;

  static std::optional<uint16_t> getFrameBaseRegister(llvm::DWARFDie VarDie);
};

} // namespace evaluator
} // namespace crash_analyzer
#endif // OHOS_LLVM
