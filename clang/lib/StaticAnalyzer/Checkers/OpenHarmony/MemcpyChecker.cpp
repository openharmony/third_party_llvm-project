#ifdef OHOS_LLVM
//===-- MemcpyChecker.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines MemcpyChecker, which is a path-sensitive check
// looking for mismatch src and dest buffer length may cause buffer overflow.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicExtent.h"
#include <optional>

using namespace clang;
using namespace ento;

namespace {
class MemcpyChecker : public Checker<check::PreCall> {
  const CallDescription MemcpyS{CallDescription::Mode::SimpleFunc, {"memcpy_s"},
                                4};
  const BugType OverflowBugType{this, "Unsafe buffer operation",
                                categories::UnixAPI};

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};

void MemcpyChecker::checkPreCall(const CallEvent &Call,
                                 CheckerContext &C) const {
  if (!MemcpyS.matches(Call))
    return;

  SValBuilder &SVB = C.getSValBuilder();
  ProgramStateRef State = C.getState();
  SVal DstAddrSVal = Call.getArgSVal(0);
  SVal SrcLengthSVal = Call.getArgSVal(3);

  const MemRegion *DstAddrMR = DstAddrSVal.getAsRegion();
  if (!DstAddrMR)
    return;

  const ElementRegion *DstAddrER = dyn_cast<ElementRegion>(DstAddrMR);
  if (!DstAddrER)
    return;

  DefinedOrUnknownSVal Idx = DstAddrER->getIndex().castAs<DefinedOrUnknownSVal>();
  std::optional<DefinedSVal> IdxSVal = Idx.getAs<DefinedSVal>();
  if (!IdxSVal)
    return;

  DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      State, DstAddrER->getSuperRegion(), C.getSValBuilder(),
      DstAddrER->getValueType());

  std::optional<DefinedSVal> DstAddrLenSVal = ElementCount.getAs<DefinedSVal>();
  if (!DstAddrLenSVal)
    return;

  std::optional<DefinedSVal> SrcLengthDSVal = SrcLengthSVal.getAs<DefinedSVal>();
  if (!SrcLengthDSVal)
    return;

  SVal SrcLenDSval =
      SVB.evalBinOp(State, BO_Add, *SrcLengthDSVal, *IdxSVal,
                    SVB.getArrayIndexType());

  SVal DstLessThanSrcLength = SVB.evalBinOp(
      State, BO_LT, *DstAddrLenSVal, SrcLenDSval, SVB.getConditionType());

  std::optional<DefinedSVal> DstLessThanSrcLengthDVal =
      DstLessThanSrcLength.getAs<DefinedSVal>();
  if (!DstLessThanSrcLengthDVal)
    return;

  if (State->assume(*DstLessThanSrcLengthDVal, true)) {
    ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
    if (!ErrNode)
      return;

    auto R = std::make_unique<PathSensitiveBugReport>(
        OverflowBugType,
        "memcpy_s(): src length may be larger than dst length", ErrNode);
    R->addRange(Call.getSourceRange());
    C.emitReport(std::move(R));
  }
}
} // namespace

void ento::registerMemcpyChecker(CheckerManager &mgr) {
  mgr.registerChecker<MemcpyChecker>();
}

bool ento::shouldRegisterMemcpyChecker(const CheckerManager &mgr) {
  return true;
}
#endif /* OHOS_LLVM */
