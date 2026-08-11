//===-- sanitizer_ffrt_coroutine_stack_ohos.cpp ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// OHOS: resolve FFRT coroutine stack bounds for fast unwind (RTLD_NOLOAD path).
// Logic migrated from HWASan (hwasan_linux.cpp) into sanitizer_common so all
// sanitizers sharing UnwindFast behave consistently.
//
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"

#if SANITIZER_OHOS

#  include "sanitizer_common.h"
#  include "sanitizer_stacktrace.h"
#  include <dlfcn.h>
#  include <stddef.h>

namespace __sanitizer {

namespace {

constexpr const char *kFfrtLibName = "libffrt.so";
constexpr const char *kFfrtStackSymbol = "ffrt_get_current_coroutine_stack";

using FfrtGetCurrentCoroutineStack = bool (*)(void **stack_addr, size_t *size);

// Lazily resolved hook; nullptr means not resolved yet or unavailable.
static FfrtGetCurrentCoroutineStack cached_ffrt_get_current_coroutine_stack;
// Serialize first-time dlopen/dlsym; concurrent waiters re-read the cache.
static int ffrt_stack_resolve_in_progress;
// Negative cache: symbol lookup failed once, skip loader work afterwards.
static int ffrt_stack_symbol_missing;

// Half-open interval [stack_bottom, stack_top).
static bool IsPointerInBounds(uptr addr, uptr stack_bottom, uptr stack_top) {
  return addr != 0 && stack_bottom < stack_top && addr >= stack_bottom &&
         addr < stack_top;
}

// Returns cached `ffrt_get_current_coroutine_stack` when known.
// If allow_resolve is false, never runs dlopen/dlsym (e.g. async signal unwind);
// only the cached pointer is returned.
static FfrtGetCurrentCoroutineStack ResolveFfrtStackFunc(bool allow_resolve) {
  FfrtGetCurrentCoroutineStack fn =
      __atomic_load_n(&cached_ffrt_get_current_coroutine_stack,
                      __ATOMIC_ACQUIRE);
  if (fn || !allow_resolve)
    return fn;

  if (__atomic_load_n(&ffrt_stack_symbol_missing, __ATOMIC_ACQUIRE) != 0)
    return nullptr;

  if (__sync_lock_test_and_set(&ffrt_stack_resolve_in_progress, 1) != 0) {
    return __atomic_load_n(&cached_ffrt_get_current_coroutine_stack,
                           __ATOMIC_ACQUIRE);
  }

  fn = __atomic_load_n(&cached_ffrt_get_current_coroutine_stack,
                       __ATOMIC_ACQUIRE);
  if (!fn && __atomic_load_n(&ffrt_stack_symbol_missing, __ATOMIC_RELAXED) ==
                 0) {
    void *handle = dlopen(kFfrtLibName, RTLD_LAZY | RTLD_NOLOAD);

    if (handle) {
      void *symbol = dlsym(handle, kFfrtStackSymbol);
      if (symbol) {
        fn = reinterpret_cast<FfrtGetCurrentCoroutineStack>(symbol);
        __atomic_store_n(&cached_ffrt_get_current_coroutine_stack, fn,
                         __ATOMIC_RELEASE);
      } else {
        dlclose(handle);
        __atomic_store_n(&ffrt_stack_symbol_missing, 1, __ATOMIC_RELEASE);
      }
    }
  }

  __sync_lock_release(&ffrt_stack_resolve_in_progress);
  return fn;
}

} // namespace

// If current_fp lies on the FFRT coroutine stack, write that stack's
// bounds into *stack_bottom/*stack_top so UnwindFast uses the correct span.
// Otherwise leaves outputs unchanged and returns false.
bool MaybeGetFfrtCoroutineStackBounds(uptr current_fp, uptr *stack_bottom,
                                      uptr *stack_top, bool allow_resolve) {
  if (!stack_bottom || !stack_top || current_fp == 0)
    return false;

  FfrtGetCurrentCoroutineStack fn = ResolveFfrtStackFunc(allow_resolve);
  if (!fn)
    return false;

  void *stack_addr = nullptr;
  size_t stack_size = 0;
  if (!fn(&stack_addr, &stack_size) || !stack_addr || stack_size == 0)
    return false;

  uptr bottom = reinterpret_cast<uptr>(stack_addr);
  if (bottom > ~static_cast<uptr>(0) - stack_size)
    return false;

  uptr top = bottom + stack_size;
  if (!IsPointerInBounds(current_fp, bottom, top))
    return false;

  *stack_bottom = bottom;
  *stack_top = top;
  return true;
}

} // namespace __sanitizer

#endif // SANITIZER_OHOS
