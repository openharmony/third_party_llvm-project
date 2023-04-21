//== RefBaseDeleteChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines RefBaseDeleteChecker, which is a path-sensitive checker
// looking for delete a pointer managed by sptr or wptr.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include <string>

using namespace clang;
using namespace ento;
using namespace llvm;

// registe a map to track the state of RefBase
REGISTER_MAP_WITH_PROGRAMSTATE(RefBaseStateMap, const MemRegion *, bool)
namespace {
class RefBaseDeleteChecker
    : public Checker<check::PreCall, check::PreStmt<CXXDeleteExpr>> {
  mutable std::unique_ptr<BugType> BT;

private:
  /**
   * Detect wheather the Object is derived from RefBase
   */
  bool isDerivedFromRefBase(const CXXRecordDecl *decl) const {
    if (!decl || !(decl->hasDefinition())) {
      return false;
    }
    // iterate all parents recursively
    for (auto s = decl->bases_begin(), e = decl->bases_end(); s != e; s++) {
      QualType qt = s->getType();
      std::string base_str = qt.getAsString();
      if (base_str == "class OHOS::RefBase") {
        return true;
      }
      CXXRecordDecl *crd = qt->getAsCXXRecordDecl();
      if (isDerivedFromRefBase(crd)) {
        return true;
      }
    }
    return false;
  }
  void reportBug(const CXXDeleteExpr *DE, CheckerContext &C) const {
    if (!BT) {
      BT.reset(new BugType(
          this, "Manually deleted the RefBase pointer managed by sptr/wptr",
          "OH"));
    }
    ExplodedNode *N = C.generateNonFatalErrorNode();
    if (!N) {
      return;
    }
    auto report =
        std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);
    report->addRange(DE->getSourceRange());
    C.emitReport(std::move(report));
  }

  /**
   * Detect wheather the object is a RefBase pointer or derived from RefBase
   */
  bool isRefBasePtrOrDerived(const CXXDeleteExpr *DE, CheckerContext &C) const {
    const Expr *deletedObj = DE->getArgument();
    if (!deletedObj) {
      return false;
    }
    std::string deletedStr = deletedObj->getType().getAsString();
    if (deletedStr == "class OHOS::RefBase *") {
      return true;
    }
    const MemRegion *mr = C.getSVal(deletedObj).getAsRegion();
    if (!mr) {
      return false;
    }
    const MemRegion *baseRegion = mr->getBaseRegion();
    if (!baseRegion) {
      return false;
    }
    const auto *derivedClassRegion = baseRegion->getAs<SymbolicRegion>();
    if (!derivedClassRegion) {
      return false;
    }
    const CXXRecordDecl *derivedClass =
        derivedClassRegion->getSymbol()->getType()->getPointeeCXXRecordDecl();
    return isDerivedFromRefBase(derivedClass);
  }

  void addTransition(ProgramStateRef State,
                     const MemRegion *OtherSmartPtrRegion,
                     CheckerContext &C) const {
    /**
     * add the MemRegion of OH pointer to the map to track its state
     */
    while (OtherSmartPtrRegion != OtherSmartPtrRegion->getBaseRegion()) {
      // recursively find base region and track its state
      // mainly for: sptr<ParentClass> s(DerivedClassObj);
      OtherSmartPtrRegion = OtherSmartPtrRegion->getBaseRegion();
    }
    State = State->set<RefBaseStateMap>(OtherSmartPtrRegion, true);
    C.addTransition(State);
  }

  bool isOHOSNameSpace(const CallEvent &Call) const {
    /**
     * detect wheather the namespace is OHOS
     */
    const auto *MethodDecl = dyn_cast_or_null<CXXMethodDecl>(Call.getDecl());
    if (!MethodDecl) {
      return false;
    }
    const auto *RecordDecl = MethodDecl->getParent();
    if (!RecordDecl) {
      return false;
    }
    const DeclContext *DC = RecordDecl->getDeclContext();
    if (!DC) {
      return false;
    }
    if (!DC->isNamespace()) {
      return false;
    }
    const auto *ND = cast<NamespaceDecl>(DC);
    if (!ND) {
      return false;
    }
    const IdentifierInfo *II = ND->getIdentifier();
    return II && II->isStr("OHOS");
  }

  bool isSmartPointer(const CallEvent &Call) const {
    /**
     * Detect if the function call invoked by OH smart pointer(sptr/wptr)
     */
    const auto *MethodDecl = dyn_cast_or_null<CXXMethodDecl>(Call.getDecl());
    if (!MethodDecl) {
      return false;
    }
    const auto *RecordDecl = MethodDecl->getParent();
    if (!RecordDecl) {
      return false;
    }
    if (!isOHOSNameSpace(Call)) {
      return false;
    }
    if (RecordDecl->getDeclName().isIdentifier()) {
      StringRef Name = RecordDecl->getName();
      return (Name == "sptr") || (Name == "wptr");
    }

    return false;
  }

public:
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
  void checkPreStmt(const CXXDeleteExpr *DE, CheckerContext &C) const;
};
} // end anonymous namespace

void RefBaseDeleteChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  /**
   * after analyzing, when assign a refbase to sptr/wptr, its
   * index always 0.
   * here directely track state of argument whose inddex is 0.
   */
  if (!isSmartPointer(Call)) {
    return;
  }
  if (Call.getNumArgs() < 1) {
    return;
  }
  const MemRegion *ParamRegion = Call.getArgSVal(0).getAsRegion();
  if (!ParamRegion) {
    return;
  }
  addTransition(Call.getState(), ParamRegion, C);
}
void RefBaseDeleteChecker::checkPreStmt(const CXXDeleteExpr *DE,
                                        CheckerContext &C) const {
  /**
   * Listen to all delete statements, if it is found that the deleted object is
   * a refbase pointer and is in the map, an warning will be reported
   */
  if (!isRefBasePtrOrDerived(DE, C)) {
    return;
  }
  const Expr *DeletedObj = DE->getArgument();
  const MemRegion *MR = C.getSVal(DeletedObj).getAsRegion();
  if (!MR) {
    return;
  }
  while (MR != MR->getBaseRegion()) {
    MR = MR->getBaseRegion();
  }
  const ProgramStateRef State = C.getState();
  const bool *tracked = State->get<RefBaseStateMap>(MR);
  if (tracked) {
    reportBug(DE, C);
  }
}

// builtin
void ento::registerRefBaseDeleteChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<RefBaseDeleteChecker>();
}

bool ento::shouldRegisterRefBaseDeleteChecker(const CheckerManager &mgr) {
  return true;
}