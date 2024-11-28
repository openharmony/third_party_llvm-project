//===-- backtrace_linux_libc.cpp --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <execinfo.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gwp_asan/definitions.h"
#include "gwp_asan/optional/backtrace.h"
#include "gwp_asan/optional/printf.h"
#include "gwp_asan/options.h"

#include "print_backtrace_linux_libc.inc" // OHOS_LOCAL

namespace {
size_t Backtrace(uintptr_t *TraceBuffer, size_t Size) {
  static_assert(sizeof(uintptr_t) == sizeof(void *), "uintptr_t is not void*");

  return backtrace(reinterpret_cast<void **>(TraceBuffer), Size);
}

// We don't need any custom handling for the Segv backtrace - the libc unwinder
// has no problems with unwinding through a signal handler. Force inlining here
// to avoid the additional frame.
GWP_ASAN_ALWAYS_INLINE size_t SegvBacktrace(uintptr_t *TraceBuffer, size_t Size,
                                            void * /*Context*/) {
  return Backtrace(TraceBuffer, Size);
}
} // anonymous namespace

namespace gwp_asan {
namespace backtrace {

options::Backtrace_t getBacktraceFunction() { return Backtrace; }
PrintBacktrace_t getPrintBacktraceFunction() { return PrintBacktrace; }
SegvBacktrace_t getSegvBacktraceFunction() { return SegvBacktrace; }

} // namespace backtrace
} // namespace gwp_asan
