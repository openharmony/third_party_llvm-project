//===-- backtrace_ohos.cpp -----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <stdlib.h>

#include "gwp_asan/definitions.h"
#include "gwp_asan/optional/backtrace.h"
#include "gwp_asan/optional/printf.h"
#include "gwp_asan/options.h"

extern "C" char **backtrace_symbols(void *const *trace, size_t len);

#include "print_backtrace_linux_libc.inc"

extern "C" GWP_ASAN_WEAK size_t
libc_gwp_asan_unwind_fast(size_t *frame_buf, size_t max_record_stack,
                          __attribute__((unused)) void *signal_context);

namespace gwp_asan {
namespace backtrace {

// These interfaces are implemented by OHOS musl which is registered with
// gwp_asan.
options::Backtrace_t getBacktraceFunction() {
  assert(&libc_gwp_asan_unwind_fast &&
         "libc_gwp_asan_unwind_fast wasn't provided from musl.");
  return [](size_t *frame_buf, size_t max_record_stack) {
    return libc_gwp_asan_unwind_fast(frame_buf, max_record_stack, nullptr);
  };
}
PrintBacktrace_t getPrintBacktraceFunction() { return PrintBacktrace; }
SegvBacktrace_t getSegvBacktraceFunction() {
  assert(&libc_gwp_asan_unwind_fast &&
         "libc_gwp_asan_unwind_fast wasn't provided from musl.");
  return libc_gwp_asan_unwind_fast;
}

} // namespace backtrace
} // namespace gwp_asan
