//===--------- Definition of the GWPAddressSanitizer class -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the GWPAddressSanitizer.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_GWPADDRESSSANITIZER_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_GWPADDRESSSANITIZER_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class Functions;
class Module;

/// This is a public interface to the gwp address sanitizer pass for
/// instrumenting malloc to gwp_asan_malloc to check for memory errors at runtime.
struct GWPAddressSanitizerPass : public PassInfoMixin<GWPAddressSanitizerPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; }
};

struct ModuleGWPAddressSanitizerPass
  : public PassInfoMixin<ModuleGWPAddressSanitizerPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // namespace llvm

#endif
