#if defined(OHOS_LLVM) && defined(__OHOS__)
//===-- gwp_asan_c_interface.cpp ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// OHOS_LOCAL: C API exported for OHOS musl / allocator integration.

#include "gwp_asan/guarded_pool_allocator.h"
#include "gwp_asan/optional/printf.h"
#include "gwp_asan/optional/segv_handler.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

static gwp_asan::GuardedPoolAllocator guarded_pool_allocator;

typedef struct gwp_asan_option {
  bool help;
  bool enable;
  bool install_signal_handlers;
  bool install_fork_handlers;
  int sample_rate;
  int max_simultaneous_allocations;
  gwp_asan::options::Backtrace_t backtrace;
  gwp_asan::Printf_t gwp_asan_printf;
  gwp_asan::backtrace::PrintBacktrace_t printf_backtrace;
  gwp_asan::backtrace::SegvBacktrace_t segv_backtrace;
  int min_sample_size;
  const char *white_list_path;
} gwp_asan_option;

void init_gwp_asan(void *init_options) {
  gwp_asan::options::Options opts;
  gwp_asan_option *input_opts =
      reinterpret_cast<gwp_asan_option *>(init_options);
  // Wire allocator Printf before GPA init so check()/die() can report.
  gwp_asan::Printf = input_opts->gwp_asan_printf;
  opts.help = input_opts->help;
  opts.Enabled = input_opts->enable;
  opts.MaxSimultaneousAllocations =
      input_opts->max_simultaneous_allocations;
  opts.SampleRate = input_opts->sample_rate;
  opts.InstallSignalHandlers = input_opts->install_signal_handlers;
  opts.InstallForkHandlers = input_opts->install_fork_handlers;
  opts.Backtrace = input_opts->backtrace;
  opts.MinSampleSize = input_opts->min_sample_size;
  opts.WhiteListPath = input_opts->white_list_path;
  guarded_pool_allocator.init(opts);
  if (input_opts->install_signal_handlers) {
    gwp_asan::segv_handler::installSignalHandlersOhos(
        &guarded_pool_allocator, input_opts->gwp_asan_printf,
        input_opts->printf_backtrace, input_opts->segv_backtrace);
  }
}

void *gwp_asan_malloc(size_t bytes) {
  return guarded_pool_allocator.allocate(bytes);
}

void gwp_asan_free(void *mem) { guarded_pool_allocator.deallocate(mem); }

size_t gwp_asan_get_size(void *mem) {
  return guarded_pool_allocator.getSize(mem);
}

bool gwp_asan_should_sample() { return guarded_pool_allocator.shouldSample(); }

SANITIZER_INTERFACE_ATTRIBUTE void gwp_asan_force_sample_by_funcattr() {
  return guarded_pool_allocator.forceSampleByFuncAttr();
}

SANITIZER_INTERFACE_ATTRIBUTE void gwp_asan_unforce_sample_by_funcattr() {
  return guarded_pool_allocator.unforceSampleByFuncAttr();
}

bool gwp_asan_pointer_is_mine(void *mem) {
  return guarded_pool_allocator.pointerIsMine(mem);
}

bool gwp_asan_has_free_mem() { return guarded_pool_allocator.hasFreeMem(); }

void gwp_asan_disable() { guarded_pool_allocator.disable(); }

void gwp_asan_enable() { guarded_pool_allocator.enable(); }

void gwp_asan_iterate(void *base, size_t size,
                       gwp_asan::GuardedPoolAllocator::iterate_callback cb,
                       void *arg) {
  guarded_pool_allocator.iterate(base, size, cb, arg);
}

#ifdef __cplusplus
}
#endif
#endif // defined(OHOS_LLVM) && defined(__OHOS__)
