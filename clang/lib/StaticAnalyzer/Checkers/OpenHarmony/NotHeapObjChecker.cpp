//== NotHeapObjChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines NotHeapObjChecker, which is a path-sensitive checker
// looking for sptr or wptr try to manage non-heap object.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include <string>

using namespace clang;
using namespace ento;
using namespace llvm;

namespace {
class NotHeapObjChecker : public Checker<check::PreCall> {
  mutable std::unique_ptr<BugType> BT;

private:
  void reportBug(const CallEvent &Call, CheckerContext &C) const {
    if (!BT) {
      BT.reset(
          new BugType(this, "sptr/wptr cannot manage non-heap objects", "OH"));
    }
    ExplodedNode *N = C.generateNonFatalErrorNode();
    if (!N) {
      return;
    }
    auto report =
        std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);
    report->addRange(Call.getSourceRange());
    C.emitReport(std::move(report));
  }
  bool isHeapObject(SVal Val) const {
    /**
     * Detect whether the object is on the heap, due to the existence of
     * UnKnownSpaceRegion has caused a great false positive rate, so reversely
     * detect whether the object is on the stack and the global data area
     */
    const MemRegion *Region = Val.getAsRegion();
    if (Region && (isa<StackSpaceRegion>(Region->getMemorySpace()) ||
                   isa<GlobalsSpaceRegion>(Region->getMemorySpace()))) {
      return false;
    } else {
      return true;
    }
  }
  bool isOHSmartPtrCall(const CallEvent &Call) const {
    /**
     * Detect if the function call invoked by OH smart pointer(sptr/wptr)
     */
    const auto *MethodDecl = dyn_cast_or_null<CXXMethodDecl>(Call.getDecl());
    if (!MethodDecl || !MethodDecl->getParent()) {
      return false;
    }
    const auto *RecordDecl = MethodDecl->getParent();
    if (RecordDecl && RecordDecl->getDeclName().isIdentifier()) {
      StringRef Name = RecordDecl->getName();
      return Name == "sptr" || Name == "wptr";
    }
    return false;
  }

public:
  /**
   * When the object is assigned to the OH smart pointer, detect whether the
   * object is on the heap.
   */
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};
} // end anonymous namespace

void NotHeapObjChecker::checkPreCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  if (!isOHSmartPtrCall(Call)) {
    return;
  }
  if (Call.getNumArgs() < 1) {
    return;
  }
  SVal Val = Call.getArgSVal(0);
  // the argument is null pointer
  if (C.getState()->isNull(Val).isConstrainedTrue()) {
    return;
  }
  // it is allowed to assigne one sptr/wptr to another
  if (Call.getArgExpr(0)->getType().getAsString().find("sptr") !=
          std::string::npos ||
      Call.getArgExpr(0)->getType().getAsString().find("wptr") !=
          std::string::npos) {
    return;
  }
  std::string forceStr("ForceSetRefPtr");
  /**
   * if the function is ForceSetRefPtr or the constructor of sptr/wptr
   * detect if the argument is on the heap
   */
  if ((dyn_cast<AnyCXXConstructorCall>(&Call) &&
       dyn_cast<AnyCXXConstructorCall>(&Call)->getKind() ==
           CE_CXXConstructor) || // if a constructor
      (Call.getCalleeIdentifier() &&
       forceStr.compare(Call.getCalleeIdentifier()->getName().data()) == 0)) {
    if (!isHeapObject(Val)) { // if the object is not on the heap, report bugs
      reportBug(Call, C);
    }
  }
}

void ento::registerNotHeapObjChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<NotHeapObjChecker>();
}

bool ento::shouldRegisterNotHeapObjChecker(const CheckerManager &mgr) {
  return true;
}