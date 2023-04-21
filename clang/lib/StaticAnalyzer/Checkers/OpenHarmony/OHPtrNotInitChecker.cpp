//== OHPtrNotInitChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines OHPtrNotInitChecker, which is a path-sensitive checker
// looking for the usuage of sptr or wptr which is not init.
// write or execute permission
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"

using namespace clang;
using namespace ento;
using namespace llvm;

namespace {
class OHPtrNotInitChecker : public Checker<check::PreCall> {
public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  /**
   * A CallBack to iterate all bindings of region store
   *
   * sptr/wptr has only one attribute, and it should be null pointer
   * if sptr/wptr not init.
   */
  class CallBack : public StoreManager::BindingsHandler {
  private:
    ProgramStateRef &State;
    bool isNull = false;

  public:
    CallBack(ProgramStateRef &S) : State(S) {}
    bool HandleBinding(StoreManager &SMgr, Store S, const MemRegion *Region,
                       SVal Val) override {
      if (State->isNull(Val).isConstrainedTrue()) {
        isNull = true;
        return false;
      }
      return true;
    }
    bool isNullPtr() const { return isNull; }
  };

private:
  // if the function call is invoked by sptr/wptr
  bool isSmartPointer(const CallEvent &Call) const;
  // detect wheather the namespace is OHOS
  bool isOHOSNameSpace(const DeclContext *DC) const;
  void bugReport(CheckerContext &C, const ProgramStateRef &State) const;
  BugType NON_FATAL_BUG{this, "NON_FATAL_BUG", "NON_FATAL_BUG"};
};
} // end anonymous namespace

// regist a map to trace the state of sptr/wptr
REGISTER_MAP_WITH_PROGRAMSTATE(InitMap, const MemRegion *,
                               bool) // true: don't warning
void OHPtrNotInitChecker::checkPreCall(const CallEvent &Call,
                                       CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (!isSmartPointer(Call)) {
    return;
  }

  /**
   * Let wptr avoid false positives, which still report bugs after initialized
   * confusing... @_@
   */
  const auto *CCC = dyn_cast<AnyCXXConstructorCall>(&Call);
  if (CCC && (Call.getNumArgs() > 0)) {
    SVal Val = CCC->getCXXThisVal();
    const MemRegion *Region = Val.getAsRegion();
    State = State->set<InitMap>(Region, true);
    C.addTransition(State);
    return;
  }
  const auto *CMOC = dyn_cast<CXXMemberOperatorCall>(&Call);
  if (!CMOC) {
    return;
  }
  SVal Val = CMOC->getCXXThisVal();
  // if sptr/wptr itself is a null pointer
  if (State->isNull(Val).isConstrainedTrue()) {
    bugReport(C, State);
  }
  /**
   * Because clang sometimes cannot infer the value of sptr/wptr from the
   * context, there will be false positives.
   *
   * So we reduce false positives by tracking the state of sptr/wptr. When sptr
   * and wptr have been compared with the null pointer or performed a
   * not operation, it is considered impossible to be empty
   */
  if (CMOC->getOverloadedOperator() == OO_Exclaim) { // !
    const MemRegion *Region = Val.getAsRegion();
    State = State->set<InitMap>(Region, true);
    C.addTransition(State);
  }
  if ((CMOC->getOverloadedOperator() == OO_EqualEqual) ||
      (CMOC->getOverloadedOperator() == OO_ExclaimEqual)) { // ==/!= nullptr
    int comparedArgIndex = 0;
    SVal ArgVal = Call.getArgSVal(comparedArgIndex);
    ConditionTruthVal Nullness = State->isNull(ArgVal);
    if (Nullness.isConstrainedTrue()) {
      const MemRegion *Region = Val.getAsRegion();
      State = State->set<InitMap>(Region, true);
      C.addTransition(State);
    }
  }
  /**
   * When the operator-> or operator* has been invoked, and the
   * sptr/wptr not init, report bugs
   */
  if (CMOC->getOverloadedOperator() == OO_Arrow ||
      CMOC->getOverloadedOperator() == OO_Star) { // process -> and *
    CallBack Cb(State);
    State->getStateManager().getStoreManager().iterBindings(State->getStore(),
                                                            Cb);
    const MemRegion *Region = Val.getAsRegion();
    // Whether sptr/wptr has been manually judged empty
    if (State->get<InitMap>(Region)) {
      return;
    }
    /**
     * If the only attribute of sptr/wptr is null, then it is considered
     * uninitialized and an error is reported
     */
    if (Cb.isNullPtr()) {
      bugReport(C, State);
    }
  }
}

bool OHPtrNotInitChecker::isSmartPointer(const CallEvent &Call) const {
  const auto *MethodDecl = dyn_cast_or_null<CXXMethodDecl>(Call.getDecl());
  if (!MethodDecl) {
    return false;
  }
  const auto *RecordDecl = MethodDecl->getParent();
  if (!RecordDecl) {
    return false;
  }
  if (!isOHOSNameSpace(RecordDecl->getDeclContext())) {
    return false;
  }
  if (RecordDecl->getDeclName().isIdentifier()) {
    StringRef Name = RecordDecl->getName();
    return (Name == "sptr") || (Name == "wptr");
  }

  return false;
}

bool OHPtrNotInitChecker::isOHOSNameSpace(const DeclContext *DC) const {
  if (!DC->isNamespace())
    return false;
  const auto *ND = cast<NamespaceDecl>(DC);
  if (!ND) {
    return false;
  }
  const IdentifierInfo *II = ND->getIdentifier();
  return II && II->isStr("OHOS");
}

void OHPtrNotInitChecker::bugReport(CheckerContext &C,
                                    const ProgramStateRef &State) const {
  ExplodedNode *N = C.generateNonFatalErrorNode(State);

  auto R = std::make_unique<PathSensitiveBugReport>(NON_FATAL_BUG,
                                                    "sptr/wptr not init", N);
  C.emitReport(std::move(R));
}

void ento::registerOHPtrNotInitChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<OHPtrNotInitChecker>();
}

bool ento::shouldRegisterOHPtrNotInitChecker(const CheckerManager &mgr) {
  return true;
}