//===-- gwp_asan_c_interface.cpp ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gwp_asan/guarded_pool_allocator.h"
// OHOS_LOCAL begin
#include "gwp_asan/optional/printf.h"
// OHOS_LOCAL end
#include "gwp_asan/optional/segv_handler.h"
#include "sanitizer_common/sanitizer_internal_defs.h" // OHOS_LOCAL

#include <sigchain.h>
#include <signal.h>

#ifdef __cplusplus
extern "C"{
#endif

static gwp_asan::GuardedPoolAllocator guarded_poll_alloctor;

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
    // OHOS_LOCAL begin
    int min_sample_size;
    const char *white_list_path;
    // OHOS_LOCAL end
} gwp_asan_option;

void init_gwp_asan(void *init_options)
{
    gwp_asan::options::Options opts;
    gwp_asan_option *input_opts = reinterpret_cast<gwp_asan_option*>(init_options);
// OHOS_LOCAL begin
    #if defined(__OHOS__)
    gwp_asan::Printf = input_opts->gwp_asan_printf;
    #endif
// OHOS_LOCAL end
    opts.help = input_opts->help;
    opts.Enabled = input_opts->enable;
    opts.MaxSimultaneousAllocations = input_opts->max_simultaneous_allocations;
    opts.SampleRate = input_opts->sample_rate;
    opts.InstallSignalHandlers =  input_opts->install_signal_handlers;
    opts.InstallForkHandlers = input_opts->install_fork_handlers;
    opts.Backtrace = input_opts->backtrace;
    // OHOS_LOCAL begin
    opts.MinSampleSize = input_opts->min_sample_size;
    opts.WhiteListPath = input_opts->white_list_path;
    // OHOS_LOCAL end
    guarded_poll_alloctor.init(opts);
    if (input_opts->install_signal_handlers) {
        gwp_asan::segv_handler::installSignalHandlersOhos(&guarded_poll_alloctor, input_opts->gwp_asan_printf,
            input_opts->printf_backtrace, input_opts->segv_backtrace);
    }
}

void* gwp_asan_malloc(size_t bytes)
{
    return guarded_poll_alloctor.allocate(bytes);
}

void gwp_asan_free(void *mem)
{
    return guarded_poll_alloctor.deallocate(mem);
}

size_t gwp_asan_get_size(void *mem)
{
    return guarded_poll_alloctor.getSize(mem);
}

bool gwp_asan_should_sample()
{
    return guarded_poll_alloctor.shouldSample();
}

// OHOS_LOCAL begin
SANITIZER_INTERFACE_ATTRIBUTE void gwp_asan_force_sample_by_funcattr()
{
    return guarded_poll_alloctor.forceSampleByFuncAttr();
}

SANITIZER_INTERFACE_ATTRIBUTE void gwp_asan_unforce_sample_by_funcattr()
{
    return guarded_poll_alloctor.unforceSampleByFuncAttr();
}
// OHOS_LOCAL end

bool gwp_asan_pointer_is_mine(void *mem)
{
    return guarded_poll_alloctor.pointerIsMine(mem);
}

bool gwp_asan_has_free_mem()
{
    return guarded_poll_alloctor.hasFreeMem();
}

void gwp_asan_disable()
{
    return guarded_poll_alloctor.disable();
}

void gwp_asan_enable()
{
    return guarded_poll_alloctor.enable();
}

void gwp_asan_iterate(void *base, size_t size, gwp_asan::GuardedPoolAllocator::iterate_callback cb, void *arg)
{
    return guarded_poll_alloctor.iterate(base, size, cb, arg);
}

#ifdef __cplusplus
}
#endif
