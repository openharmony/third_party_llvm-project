//===-- DWARFLocator.cpp - DWARF Variable Location & Lifetime Analysis ---===//
//
// This file implements the DWARF positioning module, which locates the function
// containing the crash PC, traverses the DIE tree to find all variables and
// formal parameters alive at that crash moment, and dispatches them for value
// evaluation.
//
//
//===----------------------------------------------------------------------===//
#include "DWARFLocator.h"
#include "AArch64BackwardSlicer.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DWARF/DWARFCompileUnit.h"
#include "llvm/DebugInfo/DWARF/DWARFExpression.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace crash_analyzer;

// Locates and evaluates all active variables at the specific address where the
// crash occurred. This function acts as the entry point for the DWARF
// Positioning Module. It identifies the Compile Unit and the specific Function
// covering the CrashPC, then initiates the variable tracking and evaluation
// process.

void DWARFVariableLocator::locateVariablesAtCrashPC(
    DWARFContext &DICtx, uint64_t CrashPC,
    const std::vector<evaluator::SimplifiedInst> &InstStream,
    const std::shared_ptr<ThreadContext> &Ctx, bool IsCrashPC,
    evaluator::BackwardSlicer *Slicer) {

  outs() << "========== DWARF POSITIONING MODULE ==========\n";
  outs() << "Target Crash PC: 0x" << Twine::utohexstr(CrashPC) << "\n";

  DWARFCompileUnit *CU = DICtx.getCompileUnitForAddress(CrashPC);
  if (!CU) {
    outs() << "No Compile Unit found for Crash PC.\n";
    return;
  }

  DWARFDie FuncDie = CU->getSubroutineForAddress(CrashPC);
  if (!FuncDie.isValid()) {
    outs() << "No Subprogram found for Crash PC.\n";
    return;
  }

  // Obtains the start address->LowPC of the function, which is used as the end
  // boundary of the reverse slicing.
  uint64_t FuncLowPC = 0;
  if (auto LowPCVal =
          llvm::dwarf::toAddress(FuncDie.find(dwarf::DW_AT_low_pc))) {
    FuncLowPC = *LowPCVal;
  }

  const char *FuncName = FuncDie.getName(DINameKind::ShortName);
  outs() << "Found Function: " << (FuncName ? FuncName : "<unknown>") << "\n";
  outs() << "Active Variables at Crash Moment:\n";
  extractAndTrackVariables(FuncDie, CrashPC, FuncLowPC, InstStream, Ctx,
                           IsCrashPC, Slicer);
  outs() << "==============================================\n";
}

// Recursively traverses the DWARF DIE tree to find
// and evaluate all variables and parameters that are active at the CrashPC.
void DWARFVariableLocator::extractAndTrackVariables(
    DWARFDie ScopeDie, uint64_t CrashPC, uint64_t FuncLowPC,
    const std::vector<evaluator::SimplifiedInst> &InstStream,
    const std::shared_ptr<ThreadContext> &Ctx, bool IsCrashPC,
    evaluator::BackwardSlicer *Slicer) {

  for (DWARFDie Child : ScopeDie) {
    auto Tag = Child.getTag();

    // If the child is a lexical block (e.g., code inside { ... }), should check
    // if the CrashPC falls within its address range before diving deeper.
    if (Tag == dwarf::DW_TAG_lexical_block) {
      if (auto RangesOrErr = Child.getAddressRanges()) {
        bool containsPC = false;
        for (auto &Range : *RangesOrErr) {
          if (Range.LowPC <= CrashPC && CrashPC < Range.HighPC) {
            containsPC = true;
            break;
          }
        }
        if (containsPC) {
          extractAndTrackVariables(Child, CrashPC, FuncLowPC, InstStream, Ctx,
                                   IsCrashPC, Slicer);
        }
      } else {
        consumeError(RangesOrErr.takeError());
      }
      continue;
    }

    // Look for formal parameters and local variables.
    if (Tag == dwarf::DW_TAG_variable ||
        Tag == dwarf::DW_TAG_formal_parameter) {
      const char *VarName = Child.getName(DINameKind::ShortName);
      if (!VarName)
        VarName = "<anonymous>";

      // Retrieve the location expression DW_AT_location.
      // This tells us where the variable is stored across different PC range.
      auto LocsOrErr = Child.getLocations(dwarf::DW_AT_location);
      if (!LocsOrErr) {
        consumeError(LocsOrErr.takeError());
        continue;
      }

      bool isAlive = false;
      ArrayRef<uint8_t> ActiveDWARFExpr;

      for (const DWARFLocationExpression &Loc : *LocsOrErr) {
        if (!Loc.Range ||
            (Loc.Range->LowPC <= CrashPC && CrashPC < Loc.Range->HighPC)) {
          isAlive = true;
          ActiveDWARFExpr = Loc.Expr;
          break;
        }
      }
      // Evaluate active variables
      if (isAlive) {
        StringRef TagType =
            (Tag == dwarf::DW_TAG_formal_parameter) ? "Parameter" : "Local Var";
        evaluator::ValueEvaluator val_evaluator(Ctx, Slicer);
        evaluator::EvaluatedVariable evalres = val_evaluator.evaluate(
            Child, ActiveDWARFExpr, CrashPC, FuncLowPC, InstStream);

        if (IsCrashPC) {
          // Directly output the description returned by ValueEvaluator.
          outs() << "    [" << TagType << "] "
                 << "Name: " << VarName << " \t"
                 << "Location: " << evalres.LocationType << " \t"
                 << "Value: " << evalres.FormattedValue << "\n";
        } else {
          outs() << "    [" << TagType << "] "
                 << "Name: " << VarName << " \t"
                 << "Location: " << evalres.LocationType << "\n";
        }
      }
    }
  }
}