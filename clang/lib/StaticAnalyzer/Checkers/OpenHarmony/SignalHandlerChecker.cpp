//== SignalHandlerChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines SignalHandlerChecker, checks for calling non-async-safe
// functionos within signalhandler function
//
//===----------------------------------------------------------------------===//

#include <iterator>
#include <queue>

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "llvm/ADT/DenseSet.h"
#include "Yaml.h"

using namespace std;
using namespace clang;
using namespace ento;
using namespace ast_matchers;

namespace {
    static bool isSystemCall(const FunctionDecl *FD) {
        return FD->getASTContext().getSourceManager().isInSystemHeader(
                FD->getCanonicalDecl()->getLocation());
    }
    AST_MATCHER(FunctionDecl, isSystemCallCheck) { return isSystemCall(&Node); }

    class SignalHandlerChecker : public Checker<check::EndOfTranslationUnit> {
    public:
        set<string> m_async_safe_func_set;
        struct FunctionConfiguration
        {
            struct FunctionList {
                string type;
                vector<string> list;
            };
            vector<FunctionList> functionList;
        };
        void checkEndOfTranslationUnit(const TranslationUnitDecl *TU,
                                       AnalysisManager &AM,
                                       BugReporter &BR) const;
        void parseConfiguration(CheckerManager &mgr,
                                const std::string &Option,
                                const FunctionConfiguration *config);
    };

    class Callback : public MatchFinder::MatchCallback {
        const SignalHandlerChecker *C;
        BugReporter &BR;
        AnalysisDeclContext *ADC;
        const llvm::StringSet<> StrictConformingFunctions = {
                "signal", "abort", "_Exit", "quick_exit"};

    public :
        void run(const MatchFinder::MatchResult &Result) override;
        void Report(const Stmt *stmt, const string &msg);
        bool isSystemCallAllowed(const FunctionDecl *FD) const;
        Callback(const SignalHandlerChecker *C,
                 BugReporter &BR, AnalysisDeclContext *ADC)
                : C(C), BR(BR), ADC(ADC) {}
    };
}

using functionConfig = SignalHandlerChecker::FunctionConfiguration;
LLVM_YAML_IS_SEQUENCE_VECTOR(functionConfig::FunctionList)

namespace llvm
{
    namespace yaml
    {
        template <>
        struct MappingTraits<functionConfig>
        {
            static void mapping(IO &io, functionConfig &info)
            {
                io.mapRequired("List", info.functionList);
            }
        };

        template <>
        struct MappingTraits<functionConfig::FunctionList>
        {
            static void mapping(IO &io, functionConfig::FunctionList &info)
            {
                io.mapRequired("type", info.type);
                io.mapOptional("value", info.list);
            }
        };
    } // end namespace yaml
} // end namespace llvm

void SignalHandlerChecker::parseConfiguration(CheckerManager &mgr,
                                              const std::string &Option,
                                              const FunctionConfiguration *config) {
    if (config) {
        for (auto &sl : config->functionList) {
            if (sl.type == "asyncSafeFunction") {
                for (auto &value : sl.list) {
                    m_async_safe_func_set.insert(value);
                }
            } else {
                mgr.reportInvalidCheckerOptionValue(
                    this, Option,
                    "a valid key: asyncSafeFunction");
            }
        }
    }
}

void SignalHandlerChecker::checkEndOfTranslationUnit(const TranslationUnitDecl *TU,
                                                     AnalysisManager &AM,
                                                     BugReporter &BR) const {
    MatchFinder F;
    Callback CB(this, BR, AM.getAnalysisDeclContext(TU));

    auto SignalFunction = functionDecl(hasAnyName("signal", "::signal", "::std::signal"),
                                       parameterCountIs(2), isSystemCallCheck());
    auto HandlerExpr =
            declRefExpr(hasDeclaration(functionDecl().bind("handler_decl")))
                    .bind("handler_expr");
    F.addMatcher((callExpr(callee(SignalFunction), hasArgument(1, HandlerExpr))
            .bind("register_call")), &CB);
    F.matchAST(AM.getASTContext());
}

void Callback::run(const MatchFinder::MatchResult &Result) {
    const auto *HandlerDecl = Result.Nodes.getNodeAs<FunctionDecl>("handler_decl");
    const auto *HandlerExpr = Result.Nodes.getNodeAs<DeclRefExpr>("handler_expr");
    llvm::DenseSet<const FunctionDecl *> SeenFunctions;

    // The worklist of the callgraph visitation algorithm.
    queue<const CallExpr *> CalledFunctions;

    auto ProcessFunction = [&](const FunctionDecl *F, const Expr *CallOrRef) {
        if (F == nullptr || CallOrRef == nullptr) {
            return false;
        }
        // Ensure that canonical declaration is used.
        F = F->getCanonicalDecl();

        // Do not visit function if already encountered.
        if (!SeenFunctions.insert(F).second)
            return true;
        // Check if the call is allowed.
        // Non-system calls are not considered.
        if (isSystemCall(F)) {
            if (isSystemCallAllowed(F)) {
                return true;
            }
            // disallowed
            const string funcName = F->getNameInfo().getName().getAsString();
            string msg = "The non-async-safe function '" + funcName + "' cannot be used in the callback function of signal";
            Report(CallOrRef, msg);
            return false;
        }

        // Get the body of the encountered non-system call function.
        const FunctionDecl *FBody;
        if (!F->hasBody(FBody)) {
            const string funcName = F->getNameInfo().getName().getAsString();
            string msg = "The non-async-safe function '" + funcName + "' cannot be used in the callback function of signal";
            Report(CallOrRef, msg);
            return false;
        }

        // Collect all called functions.
        auto Matches = match(decl(forEachDescendant(callExpr().bind("call"))),
                                *FBody, FBody->getASTContext());
        for (const auto &Match: Matches) {
            const auto *CE = Match.getNodeAs<CallExpr>("call");
            if (CE && isa<FunctionDecl>(CE->getCalleeDecl())) {
                CalledFunctions.push(CE);
            }
        }
        return true;
    };

    if (!ProcessFunction(HandlerDecl, HandlerExpr))
        return;

    // Visit the definition of every function referenced by the handler function.
    // Check for allowed function calls.
    while (!CalledFunctions.empty()) {
        const CallExpr *FunctionCall = CalledFunctions.front();
        CalledFunctions.pop();
        // At insertion we have already ensured that only function calls are there.
        const auto *F = cast<FunctionDecl>(FunctionCall->getCalleeDecl());

        if (!ProcessFunction(F, FunctionCall))
            break;
    }
}

bool Callback::isSystemCallAllowed(const FunctionDecl *FD) const {
    const IdentifierInfo *II = FD->getIdentifier();
    // Unnamed functions are not explicitly allowed.
    if (!II)
        return false;

    // user defined in yaml configuration
    if (C->m_async_safe_func_set.count(II->getName().str())) {
        return true;
    }

    if (StrictConformingFunctions.count(II->getName()))
        return true;

    return false;
}

void Callback::Report(const Stmt *stmt, const std::string &msg)
{
    PathDiagnosticLocation ceLoc = PathDiagnosticLocation::createBegin(stmt, BR.getSourceManager(), ADC);
    SourceRange r = stmt->getSourceRange();
    if (ADC->getDecl() == nullptr) {
        return;
    }
    BR.EmitBasicReport(ADC->getDecl(), C,
                       "SignalHandlerChecker", "Indicator of Poor Code Quality", msg, ceLoc, r);
}

//===----------------------------------------------------------------------===//
// Registration.
//===----------------------------------------------------------------------===//

void ento::registerSignalHandlerChecker(CheckerManager &mgr) {
    auto *Checker = mgr.registerChecker<SignalHandlerChecker>();
    std::string Option{"Config"};
    StringRef ConfigFile = mgr.getAnalyzerOptions().getCheckerStringOption(Checker, Option);
    llvm::Optional<functionConfig> obj = getConfiguration<functionConfig>(mgr, Checker, Option, ConfigFile);
    if (obj) {
        Checker->parseConfiguration(mgr, Option, obj.getPointer());
    }
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterSignalHandlerChecker(const CheckerManager &mgr) {
    return true;
}
