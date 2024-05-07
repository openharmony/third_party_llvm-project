//===-libllvm.cpp - LLVM Shared Library -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
// OHOS_LOCAL begin
#ifdef LLVM_BUILD_BACKEND_MODULE

#include <cstdio>
#include <dlfcn.h>

static void * LoadTargetModule(const char *Target) {
  char SharedObjectName[100];
  void *Handle;

  snprintf(SharedObjectName, 100, "LLVM%sTarget.so", Target);
  Handle = dlopen(SharedObjectName, RTLD_LAZY | RTLD_NOLOAD);
  if (!Handle) {
    Handle = dlopen(SharedObjectName, RTLD_LAZY);
  }

  return Handle;
}

extern "C" {

#define LLVM_INIT_FUNC(TargetName, Component) \
  void LLVMInitialize##TargetName##Component(void) { \
    void *Handle = LoadTargetModule(#TargetName); \
    if (!Handle) \
      return; \
    void (*InitFunc)(void) = (void (*)())(dlsym(Handle, "LLVMInitialize"#TargetName #Component)); \
    if (!InitFunc) { \
      return; \
    } \
    InitFunc(); \
  }


#define LLVM_TARGET(TargetName) \
  LLVM_INIT_FUNC(TargetName, Target) \
  LLVM_INIT_FUNC(TargetName, TargetInfo) \
  LLVM_INIT_FUNC(TargetName, TargetMC) \
  LLVM_INIT_FUNC(TargetName, TargetMCA) \
  LLVM_INIT_FUNC(TargetName, AsmParser) \
  LLVM_INIT_FUNC(TargetName, AsmPrinter) \
  LLVM_INIT_FUNC(TargetName, Disassembler)
#include "llvm/Config/Targets.def"
#undef LLVM_TARGET

}

#endif
// OHOS_LOCAL end
