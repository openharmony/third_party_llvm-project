//===-- backtrace_ohos.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gwp_asan/optional/backtrace.h"

namespace gwp_asan {
namespace backtrace {

// These interfaces are implemented by OHOS musl which is registered with gwp_asan.
options::Backtrace_t getBacktraceFunction() { return nullptr; }
PrintBacktrace_t getPrintBacktraceFunction() { return nullptr; }
SegvBacktrace_t getSegvBacktraceFunction() { return nullptr; }

} // namespace backtrace
} // namespace gwp_asan
