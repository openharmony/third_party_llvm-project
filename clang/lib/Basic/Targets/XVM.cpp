//===--- XVM.cpp - Implement XVM target feature support -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements XVM TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "XVM.h"
#include "Targets.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/ADT/StringRef.h"

using namespace clang;
using namespace clang::targets;

const Builtin::Info XVMTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "clang/Basic/Builtins.def"
};

void XVMTargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  Builder.defineMacro("__xvm__");
  Builder.defineMacro("__XVM__");
}

static constexpr llvm::StringLiteral ValidCPUNames[] = {"xvm", "XVM"};

bool XVMTargetInfo::isValidCPUName(StringRef Name) const {
  return llvm::is_contained(ValidCPUNames, Name);
}

void XVMTargetInfo::fillValidCPUList(SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

ArrayRef<Builtin::Info> XVMTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, 1);
}
