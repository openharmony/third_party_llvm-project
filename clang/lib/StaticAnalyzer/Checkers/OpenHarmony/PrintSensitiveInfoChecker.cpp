//== PrintSensitiveInfoChecker.cpp ------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines PrintSensitiveInfoChecker, checks for sensitive information leak
// such as printing password in log.
//
//===----------------------------------------------------------------------===//

#include <string>
#include <vector>

#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "Yaml.h"

using namespace std;
using namespace clang;
using namespace ento;

namespace {
    struct SensitiveState {
    private:
        bool sensitive;

    public:
        bool isSensitive() const { return sensitive; }
        void setSensitive(bool B) { sensitive = B; }

        bool operator==(const SensitiveState &X) const {
            return sensitive == X.sensitive;
        }
        void Profile(llvm::FoldingSetNodeID &ID) const {
            ID.AddBoolean(sensitive);
        }
    };

    class PrintSensitiveInfoChecker : public Checker<check::PreStmt<BinaryOperator>,
                                                     check::PreStmt<DeclStmt>,
                                                     check::PreStmt<CallExpr>> {
    public:
        // lowercase string in following sets
        set<string> m_sensitive_var_set;
        set<string> m_sensitive_func_set;
        set<string> m_output_set;

        struct SensitiveValueConfiguration {
            struct SensitiveList {
                string type;
                vector<string> list;
            };
            vector<SensitiveList> sensitiveList;
        };

        PrintSensitiveInfoChecker();
        ~PrintSensitiveInfoChecker(){};

        void checkPreStmt(const DeclStmt *declStmt, CheckerContext &c) const;
        void checkPreStmt(const BinaryOperator *BO, CheckerContext &c) const;
        void checkPreStmt(const CallExpr *call, CheckerContext &c) const;

        void saveVardeclStateForBo(const Expr *lhs, const Expr *rhs, CheckerContext &c) const;
        void saveVardeclStateForDeclStmt(const DeclStmt *ds, CheckerContext &c) const;
        const VarDecl *GetVarDeclFromExpr(const Expr *E) const;
        string GetCurrentCalleeName(const CallExpr *CE) const;

        void parseConfiguration(CheckerManager &mgr, const std::string &Option, const SensitiveValueConfiguration *config);
        string convertStrToLowerCase(string str) const;

        void report(const Stmt *Opera, const string &msg, CheckerContext &c) const;
    };
}

using sensitiveConfig = PrintSensitiveInfoChecker::SensitiveValueConfiguration;
LLVM_YAML_IS_SEQUENCE_VECTOR(sensitiveConfig::SensitiveList)

namespace llvm {
    namespace yaml {
        template <>
        struct MappingTraits<sensitiveConfig> {
            static void mapping(IO &io, sensitiveConfig &info) {
                io.mapRequired("List", info.sensitiveList);
            }
        };

        template <>
        struct MappingTraits<sensitiveConfig::SensitiveList> {
            static void mapping(IO &io, sensitiveConfig::SensitiveList &info) {
                io.mapRequired("type", info.type);
                io.mapOptional("value", info.list);
            }
        };
    } // end namespace yaml
} // end namespace llvm

REGISTER_MAP_WITH_PROGRAMSTATE(SensitiveInfoMap, const VarDecl *, SensitiveState)

PrintSensitiveInfoChecker::PrintSensitiveInfoChecker() {
    // some base patterns in set
    m_sensitive_var_set.insert("password");
    m_sensitive_var_set.insert("passwd");
    m_sensitive_func_set.insert("getpassword");
    m_output_set.insert("hilog");
}

const VarDecl *PrintSensitiveInfoChecker::GetVarDeclFromExpr(const Expr *E) const {
    if (const auto *DRE = dyn_cast<DeclRefExpr>(E)) {
        if (const VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            return VD;
        }
    }
    return nullptr;
}

string PrintSensitiveInfoChecker::GetCurrentCalleeName(const CallExpr *CE) const {
    assert(CE);
    assert(CE->getDirectCallee());
    return CE->getDirectCallee()->getNameInfo().getName().getAsString();
}

string PrintSensitiveInfoChecker::convertStrToLowerCase(string str) const {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return tolower(c); });
    return str;
}

void PrintSensitiveInfoChecker::parseConfiguration(CheckerManager &mgr, const std::string &Option, const SensitiveValueConfiguration *config) {
    if (config) {
        for (auto &sl : config->sensitiveList) {
            if (sl.type == "fnCall") {
                for (auto value : sl.list) {
                    m_sensitive_func_set.insert(convertStrToLowerCase(value));
                }
            }
            else if (sl.type == "varName") {
                for (auto value : sl.list) {
                    m_sensitive_var_set.insert(convertStrToLowerCase(value));
                }
            }
            else if (sl.type == "outputFn") {
                for (auto value : sl.list) {
                    m_output_set.insert(convertStrToLowerCase(value));
                }
            } else {
                mgr.reportInvalidCheckerOptionValue(
                    this, Option,
                    "a valid key: fnCall, varName, outputFn");
            }
        }
    }
}

void PrintSensitiveInfoChecker::report(const Stmt *operaExpr, const string &msg, CheckerContext &context) const {
    if (operaExpr == nullptr) {
        return;
    }
    AnalysisDeclContext *declContext =
        context.getAnalysisManager().getAnalysisDeclContext(context.getStackFrame()->getDecl());
    PathDiagnosticLocation location =
        PathDiagnosticLocation::createBegin(operaExpr, context.getSourceManager(), declContext);

    SourceRange range = operaExpr->getSourceRange();
    if (!(declContext->getDecl())) {
        return;
    }
    context.getBugReporter().EmitBasicReport(declContext->getDecl(), this,
                                             "PrintSensitiveInfo", "Indicator of poor code quality", msg,
                                             location, range);
    context.addTransition();
}

void PrintSensitiveInfoChecker::checkPreStmt(const DeclStmt *ds, CheckerContext &c) const {
    if (ds == nullptr) {
        return;
    }
    saveVardeclStateForDeclStmt(ds, c);
}

void PrintSensitiveInfoChecker::checkPreStmt(const BinaryOperator *binary, CheckerContext &c) const {

    if (binary == nullptr) {
        return;
    }
    if (binary->getOpcode() == clang::BO_Assign)
    {
        const Expr *lhs = binary->getLHS()->IgnoreParenImpCasts();
        const Expr *rhs = binary->getRHS()->IgnoreParenImpCasts();
        if (lhs == nullptr || rhs == nullptr)
        {
            return;
        }
        saveVardeclStateForBo(lhs, rhs, c);
    }
}

void PrintSensitiveInfoChecker::checkPreStmt(const CallExpr *call, CheckerContext &c) const {
    if (!call || !call->getDirectCallee()) {
        return;
    }
    string funcName = GetCurrentCalleeName(call);
    if (m_output_set.find(convertStrToLowerCase(funcName)) == m_output_set.end()) {
        return;
    }
    unsigned int nums = call->getNumArgs();

    for (unsigned int i = 0; i < nums; i++) {
        const Expr *arg = call->getArg(i)->IgnoreParenImpCasts();
        if (arg == nullptr) {
            continue;
        }

        const VarDecl *varDecl = GetVarDeclFromExpr(arg);
        if (varDecl == nullptr) {
            continue;
        }
        // check by variable's name only
        if (m_sensitive_var_set.find(convertStrToLowerCase(varDecl->getNameAsString())) != m_sensitive_var_set.end()) {
            string msg = varDecl->getNameAsString() + " is a sensitive information";
            report(call, msg, c);
            continue;
        }
        // check by state map
        ProgramStateRef state = c.getState();
        const SensitiveState *sens = state->get<SensitiveInfoMap>(varDecl);
        if (sens == nullptr) {
            continue;
        }
        if (sens->isSensitive()) {
            // report bug
            string msg = varDecl->getNameAsString() + " is a sensitive information";
            report(call, msg, c);
        }
    }
}

void PrintSensitiveInfoChecker::saveVardeclStateForBo(const Expr *lhs, const Expr *rhs,
                                                      CheckerContext &c) const {
    if (rhs == nullptr || lhs == nullptr) {
        return;
    }
    const VarDecl *varDecl = GetVarDeclFromExpr(lhs);
    if (varDecl == nullptr) {
        return;
    }
    if (isa<CallExpr>(rhs)) {
        const CallExpr *call = llvm::dyn_cast_or_null<CallExpr>(rhs);
        if (!call || !call->getDirectCallee()) {
            return;
        }
        string funcName = GetCurrentCalleeName(call);
        if (m_sensitive_func_set.find(convertStrToLowerCase(funcName)) != m_sensitive_func_set.end()) {
            ProgramStateRef state = c.getState();
            SensitiveState sens;
            sens.setSensitive(true);
            state = state->set<SensitiveInfoMap>(varDecl, sens);
            c.addTransition(state);
        }
    }
}

void PrintSensitiveInfoChecker::saveVardeclStateForDeclStmt(const DeclStmt *ds, CheckerContext &c) const {

    const VarDecl *varDecl = llvm::dyn_cast_or_null<VarDecl>(ds->getSingleDecl());
    if (varDecl == nullptr || varDecl->getInit()==nullptr) {
        return;
    }
    const Expr *expr = varDecl->getInit()->IgnoreParenImpCasts();
    if (expr == nullptr) {
        return;
    }

    if (isa<CallExpr>(expr)) {
        const CallExpr *call = llvm::dyn_cast_or_null<CallExpr>(expr);
        if (!call || !call->getDirectCallee()) {
            return;
        }
        string funcName = GetCurrentCalleeName(call);
        if (m_sensitive_func_set.find(convertStrToLowerCase(funcName)) != m_sensitive_func_set.end()) {
            ProgramStateRef state = c.getState();
            SensitiveState sens;
            sens.setSensitive(true);
            state = state->set<SensitiveInfoMap>(varDecl, sens);
            c.addTransition(state);
        }
    }
}

//===----------------------------------------------------------------------===//
// Registration.
//===----------------------------------------------------------------------===//

void ento::registerPrintSensitiveInfoChecker(CheckerManager &mgr) {
    auto *Checker = mgr.registerChecker<PrintSensitiveInfoChecker>();
    string Option{"Config"};
    StringRef ConfigFile = mgr.getAnalyzerOptions().getCheckerStringOption(Checker, Option);
    llvm::Optional<sensitiveConfig> obj = getConfiguration<sensitiveConfig>(mgr, Checker, Option, ConfigFile);
    // If no Config is provided, obj is null
    if (obj) {
        Checker->parseConfiguration(mgr, Option, obj.getPointer());
    }
}

// This checker should be enabled regardless of how language options are set.
bool ento::shouldRegisterPrintSensitiveInfoChecker(const CheckerManager &mgr) {
    return true;
}
