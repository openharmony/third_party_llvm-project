//== MemcpyChecker.cpp ------------------------------*- C++ -*--==//
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

using namespace clang;
using namespace ento;

namespace {
class MemcpyChecker : public Checker<check::PreCall> {
    CallDescription MemcpyS;

    std::unique_ptr<BugType> OverflowBugType;
public:
    MemcpyChecker();
    void checkPreCall(const CallEvent &Call, CheckerContext &C) const;
};

MemcpyChecker::MemcpyChecker()
    : MemcpyS("memcpy_s") {
    OverflowBugType.reset(
        new BugType(this, "Unsafe buffer operation", categories::UnixAPI));
    }

void MemcpyChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const {
  if (!MemcpyS.matches(Call)) {
    return;
  }

    SValBuilder &SVB = C.getSValBuilder();
    ProgramStateRef state = C.getState();
    SVal dstAddrSVal = Call.getArgSVal(0);
    SVal srcLengthSVal = Call.getArgSVal(3);

    const MemRegion *dstAddrMR = dstAddrSVal.getAsRegion();
    if (!dstAddrMR) {
        return;
    }

    const ElementRegion *dstAddrER = dyn_cast<ElementRegion>(dstAddrMR);
    if (!dstAddrER) {
        return;
    }

    DefinedOrUnknownSVal Idx = dstAddrER->getIndex().castAs<DefinedOrUnknownSVal>();
    Optional<DefinedSVal> IdxSVal = Idx.getAs<DefinedSVal>();
    if (!IdxSVal) {
        return;
    }

    DefinedOrUnknownSVal ElementCount = getDynamicElementCount(
      state, dstAddrER->getSuperRegion(), C.getSValBuilder(), dstAddrER->getValueType());

    Optional<DefinedSVal> dstAddrLenSVal = ElementCount.getAs<DefinedSVal>();
    if (!dstAddrLenSVal) {
        return;
    }

    Optional<DefinedSVal> srcLengthDSVal = srcLengthSVal.getAs<DefinedSVal>();
    if (!srcLengthDSVal) {
        return;
    }

    SVal srcLenDSval = SVB.evalBinOp(state, BO_Add, *srcLengthDSVal, *IdxSVal, SVB.getArrayIndexType());

    SVal dstLessThanSrcLength = SVB.evalBinOp(state, BO_LT, *dstAddrLenSVal, srcLenDSval, SVB.getConditionType());

    Optional<DefinedSVal> dstLessThanSrcLengthDVal = dstLessThanSrcLength.getAs<DefinedSVal>();
    if (!dstLessThanSrcLengthDVal) {
        return;
    }

    if (state->assume(*dstLessThanSrcLengthDVal, true)) {
        // it is possible that dst less than src length
        ExplodedNode *ErrNode = C.generateNonFatalErrorNode();
        // If we've already reached this node on another path, return.
        if (!ErrNode)
            return;

        // Generate the report.
        auto R = std::make_unique<PathSensitiveBugReport>(
            *OverflowBugType, "memcpy_s(): src length may be larger than dst length", ErrNode);
        R->addRange(Call.getSourceRange());
        C.emitReport(std::move(R));
        return;
    }
}
}

void ento::registerMemcpyChecker(CheckerManager &mgr) {
  mgr.registerChecker<MemcpyChecker>();
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterMemcpyChecker(const CheckerManager &mgr) {
  return true;
}
