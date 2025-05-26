#ifndef PRINT_BACKTRACE_LINUX_LIBC
#define PRINT_BACKTRACE_LINUX_LIBC
//===-- print_backtrace_linux_libc.inc --------------------------*- C++ -*-===//
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

namespace {
void PrintBacktrace(uintptr_t *Trace, size_t TraceLength,
                    gwp_asan::Printf_t Printf) {
  if (TraceLength == 0) {
    Printf("  <not found (does your allocator support backtracing?)>\n\n");
    return;
  }

  char **BacktraceSymbols =
      backtrace_symbols(reinterpret_cast<void **>(Trace), TraceLength);

  for (size_t i = 0; i < TraceLength; ++i) {
    if (!BacktraceSymbols)
      Printf("  #%zu %p\n", i, Trace[i]);
    else
      Printf("  #%zu %s\n", i, BacktraceSymbols[i]);
  }

  Printf("\n");
  if (BacktraceSymbols)
    free(BacktraceSymbols);
}
} // anonymous namespace
#endif