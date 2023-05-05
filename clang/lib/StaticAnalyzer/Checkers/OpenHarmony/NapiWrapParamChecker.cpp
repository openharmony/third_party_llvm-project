//== NapiWrapParamChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines NapiWrapParamChecker, which is a path-sensitive check
// looking for the function return args is not null.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/Store.h"

using namespace clang;
using namespace ento;
namespace {

// Checker class
class NapiWrapParamChecker : public Checker<check::PreCall> {
  // Constant definitions
  constexpr static int NAPI_WRAP_AYGS_NUM = 6;
  constexpr static int LAST_ARG_POS = 5;

  // Function wrapper class definitions
  CallDescription NapiWrap;

  // BugType definitions
  std::unique_ptr<BugType> LastParamNotNullBugType;

  // Report warning function
  void reportParamNotNull(const CallEvent &Call, CheckerContext &C,
                          const SymbolRef NapiSym) const;

public:
  NapiWrapParamChecker();

  // Checker function
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};

} // end anonymous namespace

// Constructor
NapiWrapParamChecker::NapiWrapParamChecker()
    : NapiWrap("napi_wrap", NAPI_WRAP_AYGS_NUM) {
  // Initialize the bug types.
  LastParamNotNullBugType.reset(new BugType(
      this, "last parameter of 'napi_wrap' not null", "OHOS Napi API Error"));
}

// A callback before the function is executed to track napi_wrap
void NapiWrapParamChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  if (!this->NapiWrap.matches(Call)) {
    return;
  }
  ProgramStateRef State = C.getState();
  SVal S = Call.getArgSVal(LAST_ARG_POS);
  Optional<DefinedOrUnknownSVal> NapiSVal = S.getAs<DefinedOrUnknownSVal>();
  if (!NapiSVal) {
    return;
  }
  ConditionTruthVal Nullness = State->isNull(*NapiSVal);
  if (Nullness.isConstrainedFalse()) {
    reportParamNotNull(Call, C, S.getAsSymbol());
  }
}

// Report if the third args of napi_wrap is not null
void NapiWrapParamChecker::reportParamNotNull(const CallEvent &Call,
                                              CheckerContext &C,
                                              const SymbolRef NapiSym) const {
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode(C.getState());
  if (!ErrNode) {
    return;
  }

  auto R = std::make_unique<PathSensitiveBugReport>(
      *LastParamNotNullBugType, "The last parameter of napi_wrap is not null",
      ErrNode);
  R->addRange(Call.getSourceRange());
  R->markInteresting(NapiSym);
  C.emitReport(std::move(R));
}

void ento::registerNapiWrapParamChecker(CheckerManager &mgr) {
  mgr.registerChecker<NapiWrapParamChecker>();
}

bool ento::shouldRegisterNapiWrapParamChecker(const CheckerManager &mgr) {
  return true;
}