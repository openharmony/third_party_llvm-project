//== NapiGetArrBufferDataChecker.cpp ----------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines NapiGetArrBufferDataChecker, which is a path-sensitive
// check looking for JS data is released by mistake.
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
class NapiGetArrBufferDataChecker
    : public Checker<check::PreCall, check::PostCall,
                     check::PreStmt<CXXDeleteExpr>> {
  // Constant definitions
  constexpr static int NAPI_GET_ARRAYBUFFER_INFO_AYGS_NUM = 4;
  constexpr static int FREE_AYGS_NUM = 1;
  constexpr static int NAPI_DATA_POS = 2;
  constexpr static int FREE_CONTENT_POS = 0;

  // Function wrapper class definitions
  CallDescription NapiGetArrBuffer, Free;

  // BugType definitions
  std::unique_ptr<BugType> DataReleaseBugType;

  // Report warning function
  void reportDataRelease(CheckerContext &C, const SymbolRef NapiSym) const;

public:
  NapiGetArrBufferDataChecker();

  // Checker function
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkPreStmt(const CXXDeleteExpr *DS, CheckerContext &C) const;
};

} // end anonymous namespace

// Register set to save necessary data
REGISTER_SET_WITH_PROGRAMSTATE(DataSet, SymbolRef)

// Constructor
NapiGetArrBufferDataChecker::NapiGetArrBufferDataChecker()
    : NapiGetArrBuffer("napi_get_arraybuffer_info",
                       NAPI_GET_ARRAYBUFFER_INFO_AYGS_NUM),
      Free("free", FREE_AYGS_NUM) {
  // Initialize the bug types.
  DataReleaseBugType.reset(new BugType(
      this, "The third parameter illegal release", "OHOS Napi API Error"));
}

// A callback after the function is executed to track the
// napi_get_arraybuffer_info
void NapiGetArrBufferDataChecker::checkPostCall(const CallEvent &Call,
                                                CheckerContext &C) const {
  if (!this->NapiGetArrBuffer.matches(Call)) {
    return;
  }
  ProgramStateRef State = C.getState();
  SVal S = Call.getArgSVal(NAPI_DATA_POS);
  Optional<loc::MemRegionVal> X = S.getAs<loc::MemRegionVal>();
  StoreManager &SM = C.getStoreManager();
  SymbolRef DataSym = SM.getBinding(State->getStore(), *X).getAsLocSymbol();
  if (!DataSym) {
    return;
  }
  State = State->add<DataSet>(DataSym);
  C.addTransition(State);
}

// A callback before the function is executed to track free
void NapiGetArrBufferDataChecker::checkPreCall(const CallEvent &Call,
                                               CheckerContext &C) const {
  if (!this->Free.matches(Call)) {
    return;
  }
  ProgramStateRef State = C.getState();
  SymbolRef PtrSym = Call.getArgSVal(FREE_CONTENT_POS).getAsSymbol();
  if (!PtrSym) {
    return;
  }
  if (State->contains<DataSet>(PtrSym)) {
    State = State->remove<DataSet>(PtrSym);
    reportDataRelease(C, PtrSym);
  }
}

// A callback before the delete statement is used to capture the delete
// behavior
void NapiGetArrBufferDataChecker::checkPreStmt(const CXXDeleteExpr *DE,
                                               CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SymbolRef ArgSym = C.getSVal(DE->getArgument()).getAsSymbol();
  if (!ArgSym) {
    return;
  }
  if (State->contains<DataSet>(ArgSym)) {
    State = State->remove<DataSet>(ArgSym);
    reportDataRelease(C, ArgSym);
  }
}

// Report if the data is released
void NapiGetArrBufferDataChecker::reportDataRelease(
    CheckerContext &C, const SymbolRef NapiSym) const {
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode(C.getState());
  if (!ErrNode) {
    return;
  }
  auto R = std::make_unique<PathSensitiveBugReport>(
      *DataReleaseBugType, "The variable is not allowed to be released",
      ErrNode);
  R->markInteresting(NapiSym);
  C.emitReport(std::move(R));
}

void ento::registerNapiGetArrBufferDataChecker(CheckerManager &mgr) {
  mgr.registerChecker<NapiGetArrBufferDataChecker>();
}

bool ento::shouldRegisterNapiGetArrBufferDataChecker(
    const CheckerManager &mgr) {
  return true;
}