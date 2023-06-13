//== OHPtrImplicitConversionChecker.cpp ------------------------------*- C++
//-*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines OHPtrImplicitConversionChecker, which is a path-sensitive
// checker looking for implicit conversion from common pointer to sptr/wptr.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Type.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include <string>

using namespace clang;
using namespace ento;

namespace {
class OHPtrImplicitConversionChecker
    : public Checker<check::PreStmt<ImplicitCastExpr>> {
  mutable std::unique_ptr<BugType> BT;

private:
  void reportBug(const ImplicitCastExpr *Cast, CheckerContext &C) const {
    if (!BT) {
      BT.reset(
          new BugType(this, "Implicit convert pointer to sptr/wptr", "OH"));
    }
    ExplodedNode *N = C.generateNonFatalErrorNode();
    if (!N) {
      return;
    }
    auto report =
        std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N);
    report->addRange(Cast->getSourceRange());
    C.emitReport(std::move(report));
  }

public:
  /**
   * Listen all implicit conversion, if the function is the constructor of
   * sptr/wptr, report bugs.
   */
  void checkPreStmt(const ImplicitCastExpr *Cast, CheckerContext &C) const;

  llvm::raw_ostream &OUTS = llvm::outs();
};
} // end anonymous namespace

void OHPtrImplicitConversionChecker::checkPreStmt(const ImplicitCastExpr *Cast,
                                                  CheckerContext &C) const {
  const ProgramStateRef State = C.getState();
  NamedDecl *ND = Cast->getConversionFunction();
  if (!ND) {
    return;
  }
  if (Cast->getCastKind() != CK_ConstructorConversion) {
    return;
  }
  FunctionDecl *FD = dyn_cast<FunctionDecl>(ND);
  if (FD) {
    ArrayRef<ParmVarDecl *> PAR = FD->parameters();
    QualType PT = PAR[0]->getOriginalType();
    std::string PTS = PT.getAsString();
    if (PTS.find("wptr") != std::string::npos ||
        PTS.find("sptr") != std::string::npos) {
      return;
    }
  }

  std::string cf = ND->getNameAsString();
  if (cf == "sptr" || cf == "wptr") {
    return reportBug(Cast, C);
  }
}

// builtin
void ento::registerOHPtrImplicitConversionChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<OHPtrImplicitConversionChecker>();
}

bool ento::shouldRegisterOHPtrImplicitConversionChecker(
    const CheckerManager &mgr) {
  return true;
}