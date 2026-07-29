//===-- guarded_pool_allocator_tls.h ----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef GWP_ASAN_GUARDED_POOL_ALLOCATOR_TLS_H_
#define GWP_ASAN_GUARDED_POOL_ALLOCATOR_TLS_H_

#include "gwp_asan/definitions.h"

#include <stdint.h>

namespace gwp_asan {
// Pack the thread local variables into a struct to ensure that they're in
// the same cache line for performance reasons. These are the most touched
// variables in GWP-ASan.
struct ThreadLocalPackedVariables {
#if defined(OHOS_LLVM) && defined(__OHOS__)
  constexpr ThreadLocalPackedVariables()
      : RandomState(0xacd979ce), NextSampleCounter(0), RecursiveGuard(false),
        is_configured(false) {}
#else
  constexpr ThreadLocalPackedVariables()
      : RandomState(0xacd979ce), NextSampleCounter(0), RecursiveGuard(false) {}
#endif
  // Initialised to a magic constant so that an uninitialised GWP-ASan won't
  // regenerate its sample counter for as long as possible. The xorshift32()
  // algorithm used below results in getRandomUnsigned32(0xacd979ce) ==
  // 0xfffffffe.
  uint32_t RandomState;
  // Thread-local decrementing counter that indicates that a given allocation
  // should be sampled when it reaches zero.
#if defined(OHOS_LLVM) && defined(__OHOS__)
  uint32_t NextSampleCounter : 30;
  // The mask is needed to silence conversion errors.
  static const uint32_t NextSampleCounterMask = (1U << 30) - 1;
#else
  uint32_t NextSampleCounter : 31;
  // The mask is needed to silence conversion errors.
  static const uint32_t NextSampleCounterMask = (1U << 31) - 1;
#endif
  // Guard against recursivity. Unwinders often contain complex behaviour that
  // may not be safe for the allocator (i.e. the unwinder calls dlopen(),
  // which calls malloc()). When recursive behaviour is detected, we will
  // automatically fall back to the supporting allocator to supply the
  // allocation.
  bool RecursiveGuard : 1;
#if defined(OHOS_LLVM) && defined(__OHOS__)
  // OHOS musl reserves this bit for per-thread GWP-ASan configuration state.
  bool is_configured : 1;
#endif
};
static_assert(sizeof(ThreadLocalPackedVariables) == sizeof(uint64_t),
              "thread local data does not fit in a uint64_t");
} // namespace gwp_asan

#ifdef GWP_ASAN_PLATFORM_TLS_HEADER
#include GWP_ASAN_PLATFORM_TLS_HEADER
#else
#if defined(OHOS_LLVM) && defined(__OHOS__)
// OHOS musl stores the GWP-ASan TLS state in its pthread structure.
// Weak so linking succeeds when musl has not yet provided the symbol.
extern "C" GWP_ASAN_WEAK void *get_platform_gwp_asan_tls_slot();
namespace gwp_asan {
inline ThreadLocalPackedVariables *getThreadLocals() {
  return reinterpret_cast<ThreadLocalPackedVariables *>(
      get_platform_gwp_asan_tls_slot());
}
} // namespace gwp_asan
#else
namespace gwp_asan {
inline ThreadLocalPackedVariables *getThreadLocals() {
  alignas(8) static GWP_ASAN_TLS_INITIAL_EXEC ThreadLocalPackedVariables Locals;
  return &Locals;
}
} // namespace gwp_asan
#endif
#endif // GWP_ASAN_PLATFORM_TLS_HEADER

#endif // GWP_ASAN_GUARDED_POOL_ALLOCATOR_TLS_H_
