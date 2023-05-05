//== NapiScopeManagerChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines NapiScopeManagerChecker, which is a path-sensitive check
// looking for Memory leak issues due to no napi_handle_scope is used.
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
#include "llvm/ADT/StringRef.h"
#include <string>

using namespace clang;
using namespace ento;
namespace {
using SymbolVector = SmallVector<SymbolRef, 2>;

// String packaging class
class StringWarpper {
  const std::string Str;

public:
  StringWarpper(const std::string &S) : Str(S) {}
  const std::string &get() { return Str; }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddString(Str); }
  bool operator==(const StringWarpper &rhs) const { return Str == rhs.Str; }
  bool operator!=(const StringWarpper &rhs) const { return Str != rhs.Str; }
  bool operator<(const StringWarpper &rhs) const { return Str < rhs.Str; }
};

// napi_handle_scope state class
struct NapiScopeManagerState {
private:
  enum Kind { Opened, Closed } K;
  NapiScopeManagerState(Kind InK) : K(InK) {}

public:
  bool isOpened() const { return K == Opened; }
  bool isClosed() const { return K == Closed; }

  static NapiScopeManagerState getOpened() {
    return NapiScopeManagerState(Opened);
  }
  static NapiScopeManagerState getClosed() {
    return NapiScopeManagerState(Closed);
  }

  bool operator==(const NapiScopeManagerState &X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddInteger(K); }
};

// uv_queue_work callback state class
struct UvQueueWorkCallBackState {
private:
  enum Kind { Safe, Dangerous } K;
  UvQueueWorkCallBackState(Kind InK) : K(InK) {}

public:
  bool isSafe() const { return K == Safe; }
  bool isDangerous() const { return K == Dangerous; }

  static UvQueueWorkCallBackState getSafe() {
    return UvQueueWorkCallBackState(Safe);
  }
  static UvQueueWorkCallBackState getDangerous() {
    return UvQueueWorkCallBackState(Dangerous);
  }

  bool operator==(const UvQueueWorkCallBackState &X) const { return K == X.K; }
  void Profile(llvm::FoldingSetNodeID &ID) const { ID.AddInteger(K); }
};

// Checker class
class NapiScopeManagerChecker
    : public Checker<check::PostCall, check::PreCall, check::DeadSymbols,
                     check::BeginFunction, check::EndFunction,
                     check::PreStmt<DeclStmt>> {
  // Constant definitions
  constexpr static int NAPI_SCOPE_AYGS_NUM = 2;
  constexpr static int NAPI_SCOPE_HANDLE_POS = 1;
  constexpr static int UV_QUEUE_WORK_CALLBACK_AYGS_NUM = 2;

  // Function wrapper class definitions
  CallDescription OpenHandleScope, CloseHandleScope;

  // BugType definitions
  std::unique_ptr<BugType> DoubleCloseBugType;
  std::unique_ptr<BugType> LeakBugType;
  std::unique_ptr<BugType> UvLeakBugType;

  // Report warning functions

  void reportDoubleClose(SymbolRef NapiDescSym, const CallEvent &Call,
                         CheckerContext &C) const;

  void reportLeaks(ArrayRef<SymbolRef> LeakedStreams, CheckerContext &C,
                   ExplodedNode *ErrNode) const;

  void reportUvCallBackLeaks(CheckerContext &C, const SourceRange &SR) const;

  // Tool functions

  void UvCallbackDetect(CheckerContext &C, const bool &isBegin) const;

  bool isLeaked(SymbolRef Sym, const NapiScopeManagerState &SS, bool IsSymDead,
                ProgramStateRef State) const;

public:
  NapiScopeManagerChecker();

  // Checker functions

  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

  void checkBeginFunction(CheckerContext &C) const;

  void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;

  void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;
};

} // end anonymous namespace

// Register map to save necessary data
REGISTER_MAP_WITH_PROGRAMSTATE(HandleScopeMap, SymbolRef, NapiScopeManagerState)
REGISTER_MAP_WITH_PROGRAMSTATE(UvQueueWorkMap, StringWarpper,
                               UvQueueWorkCallBackState)

// Constructor
NapiScopeManagerChecker::NapiScopeManagerChecker()
    : OpenHandleScope("napi_open_handle_scope", NAPI_SCOPE_AYGS_NUM),
      CloseHandleScope("napi_close_handle_scope", NAPI_SCOPE_AYGS_NUM) {
  // Initialize the bug types.
  DoubleCloseBugType.reset(new BugType(this, "Double close 'napi_handle_scope'",
                                       "OHOS Napi API Error"));

  LeakBugType.reset(new BugType(this, "Resource 'napi_handle_scope' Leaked",
                                "OHOS Napi API Error",
                                /*SuppressOnSink=*/false));

  UvLeakBugType.reset(new BugType(
      this, "The callback of uv_queue_work no use 'napi_handle_scope'",
      "OHOS Napi API Error"));
}

// Determine whether napi_handle_scope memory leak
bool NapiScopeManagerChecker::isLeaked(SymbolRef Sym,
                                       const NapiScopeManagerState &SS,
                                       bool IsSymDead,
                                       ProgramStateRef State) const {
  if (IsSymDead && SS.isOpened()) {
    ConstraintManager &CMgr = State->getConstraintManager();
    ConditionTruthVal OpenFailed = CMgr.isNull(State, Sym);
    return !OpenFailed.isConstrainedTrue();
  }
  return false;
}

// A callback function that determines whether it is uv_queue_work and stores
// the status in Map
void NapiScopeManagerChecker::UvCallbackDetect(CheckerContext &C,
                                               const bool &isBegin) const {
  const FunctionDecl *checkFun =
      C.getCurrentAnalysisDeclContext()->getDecl()->getAsFunction();
  if (checkFun->getNumParams() != UV_QUEUE_WORK_CALLBACK_AYGS_NUM) {
    return;
  }
  std::string param1Type =
      checkFun->getParamDecl(0)->getOriginalType().getAsString();
  std::string param2Type =
      checkFun->getParamDecl(1)->getOriginalType().getAsString();
  if (param1Type != "uv_work_t *") {
    return;
  }
  if ((param2Type != "int32_t" && param2Type != "int")) {
    return;
  }

  ProgramStateRef State = C.getState();
  StringWarpper FunName = StringWarpper(checkFun->getNameAsString());
  if (isBegin) {
    State = State->set<UvQueueWorkMap>(
        FunName, UvQueueWorkCallBackState::getDangerous());
  } else {
    State = State->remove<UvQueueWorkMap>(FunName);
  }
  C.addTransition(State);
}

// A callback before the function is executed to track the
// napi_close_handle_scope
void NapiScopeManagerChecker::checkPreCall(const CallEvent &Call,
                                           CheckerContext &C) const {
  if (!this->CloseHandleScope.matches(Call)) {
    return;
  }
  ProgramStateRef State = C.getState();
  SymbolRef Napisym = Call.getArgSVal(NAPI_SCOPE_HANDLE_POS).getAsSymbol();
  if (!Napisym) {
    return;
  }

  const NapiScopeManagerState *NS = State->get<HandleScopeMap>(Napisym);
  if (!NS) {
    return;
  }

  if (NS->isClosed()) {
    reportDoubleClose(Napisym, Call, C);
    return;
  }

  State =
      State->set<HandleScopeMap>(Napisym, NapiScopeManagerState::getClosed());
  C.addTransition(State);
}

// A callback after the function is executed to track the napi_open_handle_scope
void NapiScopeManagerChecker::checkPostCall(const CallEvent &Call,
                                            CheckerContext &C) const {
  if (!this->OpenHandleScope.matches(Call)) {
    return;
  }
  ProgramStateRef State = C.getState();
  UvQueueWorkMapTy CBS = State->get<UvQueueWorkMap>();
  if (CBS.isEmpty())
    return;
  for (auto cb : CBS) {
    if (cb.second.isDangerous()) {
      State = State->set<UvQueueWorkMap>(cb.first,
                                         UvQueueWorkCallBackState::getSafe());
    }
  }

  SVal ScopeSVal = Call.getArgSVal(NAPI_SCOPE_HANDLE_POS);
  Optional<loc::MemRegionVal> X = ScopeSVal.getAs<loc::MemRegionVal>();
  StoreManager &SM = C.getStoreManager();
  SymbolRef NapiSym = SM.getBinding(State->getStore(), *X).getAsLocSymbol();
  if (!NapiSym) {
    return;
  }

  State =
      State->set<HandleScopeMap>(NapiSym, NapiScopeManagerState::getOpened());

  SVal StatusVal = Call.getReturnValue();
  Optional<DefinedOrUnknownSVal> Dval = StatusVal.getAs<DefinedOrUnknownSVal>();
  if (!Dval) {
    return;
  }
  State = State->assume(*Dval, false);
  C.addTransition(State);
}

// A callbacks for symbol death, determine whether the scope has died
void NapiScopeManagerChecker::checkDeadSymbols(SymbolReaper &SymReaper,
                                               CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  SymbolVector LeakedStreams;
  HandleScopeMapTy TrackedStreams = State->get<HandleScopeMap>();
  for (HandleScopeMapTy::iterator I = TrackedStreams.begin(),
                                  E = TrackedStreams.end();
       I != E; ++I) {
    SymbolRef Sym = I->first;

    bool IsSymDead = SymReaper.isDead(Sym);

    if (isLeaked(Sym, I->second, IsSymDead, State)) {
      LeakedStreams.push_back(Sym);
    }

    if (IsSymDead) {
      State = State->remove<HandleScopeMap>(Sym);
    }
  }

  ExplodedNode *N = C.generateNonFatalErrorNode(State);
  if (!N) {
    return;
  }
  reportLeaks(LeakedStreams, C, N);
}

// A callback from which the function starts, which tracks the start of the
// callback function of uv_queue_work
void NapiScopeManagerChecker::checkBeginFunction(CheckerContext &C) const {
  UvCallbackDetect(C, true);
}

// A callback from which the function ends, which tracks the end of the callback function of
// uv_queue_work
void NapiScopeManagerChecker::checkEndFunction(const ReturnStmt *RS,
                                               CheckerContext &C) const {
  UvCallbackDetect(C, false);
}

// A callback before the declaration statement is used to capture the JSValue
// declaration
void NapiScopeManagerChecker::checkPreStmt(const DeclStmt *DS,
                                           CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  UvQueueWorkMapTy CBS = State->get<UvQueueWorkMap>();
  if (CBS.isEmpty()) {
    return;
  }

  if (!DS->isSingleDecl()) {
    return;
  }

  const VarDecl *VD = dyn_cast<VarDecl>(DS->getSingleDecl());
  if (!VD) {
    return;
  }

  const std::string DeclType = VD->getType().getAsString();
  if (DeclType.find("napi_value") == std::string::npos) {
    return;
  }

  for (auto cb : CBS) {
    if (cb.second.isDangerous()) {

      reportUvCallBackLeaks(C, DS->getSourceRange());
    }
  }
}

// Report if the scope is open but not closed
void NapiScopeManagerChecker::reportLeaks(ArrayRef<SymbolRef> LeakedStreams,
                                          CheckerContext &C,
                                          ExplodedNode *ErrNode) const {
  for (SymbolRef LeakedStream : LeakedStreams) {
    auto R = std::make_unique<PathSensitiveBugReport>(
        *LeakBugType, "Opened Napi is never closed; potential resource leak",
        ErrNode);
    R->markInteresting(LeakedStream);
    C.emitReport(std::move(R));
  }
}

// Report if the scope is double closed
void NapiScopeManagerChecker::reportDoubleClose(SymbolRef NapiSym,
                                                const CallEvent &Call,
                                                CheckerContext &C) const {
  ExplodedNode *ErrNode = C.generateErrorNode(C.getState());
  if (!ErrNode) {
    return;
  }

  auto R = std::make_unique<PathSensitiveBugReport>(
      *DoubleCloseBugType, "Closing a previously closed napi_handle_scope",
      ErrNode);
  R->addRange(Call.getSourceRange());
  R->markInteresting(NapiSym);
  C.emitReport(std::move(R));
}

// Report if the callback of uv_queue_work use JSValue but no use napi_handle_scope
void NapiScopeManagerChecker::reportUvCallBackLeaks(
    CheckerContext &C, const SourceRange &SR) const {
  ExplodedNode *ErrNode = C.generateNonFatalErrorNode(C.getState());
  if (!ErrNode) {
    return;
  }

  auto R = std::make_unique<PathSensitiveBugReport>(
      *UvLeakBugType,
      "Illegal access to JSValue, access to JSValue must be within the napi "
      "handle scope",
      ErrNode);
  R->addRange(SR);
  C.emitReport(std::move(R));
}

void ento::registerNapiScopeManagerChecker(CheckerManager &mgr) {
  mgr.registerChecker<NapiScopeManagerChecker>();
}

bool ento::shouldRegisterNapiScopeManagerChecker(const CheckerManager &mgr) {
  return true;
}