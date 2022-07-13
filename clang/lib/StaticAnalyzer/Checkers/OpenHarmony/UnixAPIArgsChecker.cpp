//== UnixAPIArgsChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines UnixAPIArgsChecker, which is a path-sensitive checker
// looking for open a file with open() with GROUP and OTHER having 
// write or execute permission
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

enum class OpenVariant {
  /// The standard open() call:
  ///    int open(const char *pathname, int flags, mode_t mode);
  Open,

  /// The variant taking a directory file descriptor and a relative path:
  ///    int openat(int fd, const char *pathname, int flags, mode_t mode);
  OpenAt
};

namespace {
class UnixAPIArgsChecker : public Checker< check::PreStmt<CallExpr> > {
  mutable std::unique_ptr<BugType> BT_open;
  // value of O_CREAT flag
  const uint64_t Val_O_CREAT = 0100;
  // value of mode being checked
  const uint64_t Val_MODE = 0133;

public:
  void checkPreStmt(const CallExpr *CE, CheckerContext &C) const;

  void CheckOpen(CheckerContext &C, const CallExpr *CE) const;
  void CheckOpenAt(CheckerContext &C, const CallExpr *CE) const;

  void CheckOpenVariant(CheckerContext &C,
                        const CallExpr *CE, OpenVariant Variant) const;

  void ReportOpenBug(CheckerContext &C,
                     ProgramStateRef State,
                     const char *Msg,
                     SourceRange SR) const;
};
} //end anonymous namespace

static void LazyInitialize(const CheckerBase *Checker,
                           std::unique_ptr<BugType> &BT,
                           const char *name) {
  if (BT)
    return;
  BT.reset(new BugType(Checker, name, categories::UnixAPI));
}

//===----------------------------------------------------------------------===//
// "open" (man 2 open)
//===----------------------------------------------------------------------===/

void UnixAPIArgsChecker::checkPreStmt(const CallExpr *CE,
                                        CheckerContext &C) const {
  const FunctionDecl *FD = C.getCalleeDecl(CE);
  if (!FD || FD->getKind() != Decl::Function)
    return;

  // Don't treat functions in namespaces with the same name a Unix function
  // as a call to the Unix function.
  const DeclContext *NamespaceCtx = FD->getEnclosingNamespaceContext();
  if (NamespaceCtx && isa<NamespaceDecl>(NamespaceCtx))
    return;

  StringRef FName = C.getCalleeName(FD);
  if (FName.empty())
    return;

  if (FName == "open")
    CheckOpen(C, CE);

  else if (FName == "openat")
    CheckOpenAt(C, CE);
}
void UnixAPIArgsChecker::ReportOpenBug(CheckerContext &C,
                                         ProgramStateRef State,
                                         const char *Msg,
                                         SourceRange SR) const {
  ExplodedNode *N = C.generateErrorNode(State);
  if (!N)
    return;

  LazyInitialize(this, BT_open, "Improper use of 'open'");

  auto Report = std::make_unique<PathSensitiveBugReport>(*BT_open, Msg, N);
  Report->addRange(SR);
  C.emitReport(std::move(Report));
}

void UnixAPIArgsChecker::CheckOpen(CheckerContext &C,
                                     const CallExpr *CE) const {
  CheckOpenVariant(C, CE, OpenVariant::Open);
}

void UnixAPIArgsChecker::CheckOpenAt(CheckerContext &C,
                                       const CallExpr *CE) const {
  CheckOpenVariant(C, CE, OpenVariant::OpenAt);
}

void UnixAPIArgsChecker::CheckOpenVariant(CheckerContext &C,
                                            const CallExpr *CE,
                                            OpenVariant Variant) const {
  // The index of the argument taking the flags open flags (O_RDONLY,
  // O_WRONLY, O_CREAT, etc.),
  unsigned int FlagsArgIndex;
  switch (Variant) {
  case OpenVariant::Open:
    FlagsArgIndex = 1;
    break;
  case OpenVariant::OpenAt:
    FlagsArgIndex = 2;
    break;
  };

  // All calls should at least provide arguments up to the 'flags' parameter.
  unsigned int MinArgCount = FlagsArgIndex + 1;

  // If the flags has O_CREAT set then open/openat() require an additional
  // argument specifying the file mode (permission bits) for the created file.
  unsigned int CreateModeArgIndex = FlagsArgIndex + 1;

  // The create mode argument should be the last argument.
  unsigned int MaxArgCount = CreateModeArgIndex + 1;

  ProgramStateRef state = C.getState();

  // Checked via UnixAPIChecker
  if (CE->getNumArgs() < MinArgCount || CE->getNumArgs() > MaxArgCount) {
    return;
  } else if (CE->getNumArgs() == MaxArgCount) {
    const Expr *Arg = CE->getArg(CreateModeArgIndex);
    QualType QT = Arg->getType();
    if (!QT->isIntegerType()) {
      return;
    }
  }

  // Now check if oflags has O_CREAT set.
  const Expr *oflagsEx = CE->getArg(FlagsArgIndex);
  const SVal V = C.getSVal(oflagsEx);
  if (!V.getAs<NonLoc>()) {
    // The case where 'V' can be a location can only be due to a bad header,
    // so in this case bail out.
    return;
  }
  NonLoc oflags = V.castAs<NonLoc>();
  NonLoc ocreateFlag = C.getSValBuilder()
      .makeIntVal(Val_O_CREAT, oflagsEx->getType()).castAs<NonLoc>();
  SVal maskedFlagsUC = C.getSValBuilder().evalBinOpNN(state, BO_And,
                                                      oflags, ocreateFlag,
                                                      oflagsEx->getType());
  if (maskedFlagsUC.isUnknownOrUndef())
    return;
  DefinedSVal maskedFlags = maskedFlagsUC.castAs<DefinedSVal>();

  // Check if maskedFlags is non-zero.
  ProgramStateRef trueState, falseState;
  std::tie(trueState, falseState) = state->assume(maskedFlags);

  // Only emit an error if the value of 'maskedFlags' is properly
  // constrained;
  if (!(trueState && !falseState))
    return;

  if (CE->getNumArgs() < MaxArgCount) {
    return;
  }

  // Now check mode when O_CREAT flag is set, GROUP and OTHER should not
  // have write or execute permission
  const Expr *createModeEx = CE->getArg(CreateModeArgIndex);
  const SVal CM = C.getSVal(createModeEx);
  if (!CM.getAs<NonLoc>()) {
    return;
  }
  NonLoc createMode = CM.castAs<NonLoc>();
  NonLoc createModeCheck = C.getSValBuilder().
    makeIntVal(Val_MODE, createModeEx->getType()).castAs<NonLoc>();

  SVal maskedCreateMode = C.getSValBuilder().evalBinOpNN(state, BO_And,
                                                      createMode, createModeCheck,
                                                      createModeEx->getType());
  if (maskedCreateMode.isUnknownOrUndef())
    return;
  DefinedSVal maskedCreateModeSVal = maskedCreateMode.castAs<DefinedSVal>();

  // Check if maskedFlags is non-zero.
  ProgramStateRef t, f;
  std::tie(t, f) = state->assume(maskedCreateModeSVal);

  // Only emit an error if the value of 'maskedFlags' is properly
  // constrained;
  if (t && !f) {
    SmallString<256> SBuf;
    llvm::raw_svector_ostream OS(SBuf);
    OS << "Open() system call, GROUP/OTHER should not have write or execute permission";
    ReportOpenBug(C, t,
                  SBuf.c_str(),
                  createModeEx->getSourceRange());
  }
}

//===----------------------------------------------------------------------===//
// Registration.
//===----------------------------------------------------------------------===//

#define REGISTER_CHECKER(CHECKERNAME)                                          \
  void ento::register##CHECKERNAME(CheckerManager &mgr) {                      \
    mgr.registerChecker<CHECKERNAME>();                                        \
  }                                                                            \
                                                                               \
  bool ento::shouldRegister##CHECKERNAME(const CheckerManager &mgr) {              \
    return true;                                                               \
  }

REGISTER_CHECKER(UnixAPIArgsChecker)
