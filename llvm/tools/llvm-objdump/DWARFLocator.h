//===-- DWARFLocator.h - dwarf Variable Locator Interface -*- C++ -*-===//
//
// This file declares the DWARFVariableLocator class, which is responsible for
// locating the function containing a given crash PC, traversing the dwarf DIE
// tree to identify variables and formal parameters that are alive at that PC,
// and dispatching them for value evaluation.
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "AArch64CrashSnapshot.h"
#include "ValueEvaluator.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDie.h"
#include <cstdint>
#include <vector>

namespace crash_analyzer {

class DWARFVariableLocator {
public:
  static void locateVariablesAtCrashPC(
      llvm::DWARFContext &DICtx, uint64_t CrashPC,
      const std::vector<evaluator::SimplifiedInst> &InstStream,
      const std::shared_ptr<ThreadContext> &Ctx = nullptr,
      bool IsCrashPC = true, evaluator::BackwardSlicer *Slicer = nullptr);

private:
  static void extractAndTrackVariables(
      llvm::DWARFDie ScopeDie, uint64_t CrashPC, uint64_t FuncLowPC,
      const std::vector<evaluator::SimplifiedInst> &InstStream,
      const std::shared_ptr<ThreadContext> &Ctx, bool IsCrashPC = true,
      evaluator::BackwardSlicer *Slicer = nullptr);
};

} // namespace crash_analyzer