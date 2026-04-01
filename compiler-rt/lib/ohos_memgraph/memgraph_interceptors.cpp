//===-- memgraph_interceptors.cpp -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// malloc/calloc/realloc/free interceptor implementations for the runtime.
//===----------------------------------------------------------------------===//

#include "memgraph.h"

#include "interception/interception.h"
#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_checks.h"
#include "sanitizer_common/sanitizer_allocator_dlsym.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

namespace __ohos_memgraph {

using namespace __sanitizer;

struct DlsymAlloc : public DlSymAllocator<DlsymAlloc> {
  static bool UseImpl() { return RuntimeInitIsRunning(); }
};

class ScopedInterceptorBypass {
public:
  ScopedInterceptorBypass() { ++ohos_memgraph_disable_interceptors; }
  ~ScopedInterceptorBypass() { --ohos_memgraph_disable_interceptors; }
};

// Interceptors must be bypassed while the runtime itself allocates internal
// objects or calls back into allocation hooks. Otherwise the runtime could
// recurse into its own wrappers.
static bool BypassInterceptors() {
  return DlsymAlloc::Use() || ohos_memgraph_disable_interceptors > 0;
}

// Frontend-provided PCs are already source-accurate. Hook fallback PCs come
// from GET_CALLER_PC(), which yields a return address. Normalize them to the
// previous instruction so hook-originated malloc/new metadata lines up with the
// actual call site on the current architecture.
static inline uptr HookCallerPc() {
  return StackTrace::GetPreviousInstructionPc(GET_CALLER_PC());
}

} // namespace __ohos_memgraph

using namespace __ohos_memgraph;

namespace std {
struct nothrow_t;
}

extern "C" {

INTERCEPTOR(void *, malloc, uptr size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(malloc)(size);

  const uptr caller_pc = HookCallerPc();
  void *ptr = REAL(malloc)(size);
  if (ptr) {
    // Metadata insertion must not recurse back into intercepted allocators.
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), size ? size : 1, caller_pc);
  }
  if (ObservabilityEnabled())
    MemStatsOnMallocHookCall();
  return ptr;
}

INTERCEPTOR(void *, calloc, uptr nmemb, uptr size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Callocate(nmemb, size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(calloc)(nmemb, size);

  const uptr caller_pc = HookCallerPc();
  void *ptr = REAL(calloc)(nmemb, size);
  if (ptr && !CheckForCallocOverflow(size, nmemb)) {
    // After overflow checks pass, calloc reports the full object size to the
    // allocation hook.
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), nmemb * size, caller_pc);
  }
  if (ObservabilityEnabled())
    MemStatsOnMallocHookCall();
  return ptr;
}

INTERCEPTOR(void, free, void *ptr) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(free)(ptr);

  {
    // Free must remove metadata before the real allocator reclaims the block.
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  if (ObservabilityEnabled())
    MemStatsOnFreeHookCall();
  REAL(free)(ptr);
}

INTERCEPTOR(void *, realloc, void *ptr, uptr size) {
  if (!ptr) {
    if (DlsymAlloc::Use())
      return DlsymAlloc::Allocate(size);
    if (!RuntimeInited())
      Initialize();
    if (!HooksEnabled() || BypassInterceptors())
      return REAL(malloc)(size);

    const uptr caller_pc = HookCallerPc();
    void *new_ptr = REAL(malloc)(size);
    if (new_ptr) {
      ScopedInterceptorBypass scope;
      TrackHookAlloc(reinterpret_cast<uptr>(new_ptr), size ? size : 1,
                     caller_pc);
    }
    if (ObservabilityEnabled())
      MemStatsOnReallocHookCall();
    return new_ptr;
  }

  if (DlsymAlloc::Use() || DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Realloc(ptr, size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(realloc)(ptr, size);

  const uptr caller_pc = HookCallerPc();
  void *new_ptr = REAL(realloc)(ptr, size);
  {
    // realloc can behave like free, alloc, in-place resize, or move+copy.
    // TrackHookRealloc() normalizes those cases into the runtime metadata
    // model.
    ScopedInterceptorBypass scope;
    if (size == 0)
      TrackHookFree(reinterpret_cast<uptr>(ptr));
    else if (new_ptr)
      TrackHookRealloc(reinterpret_cast<uptr>(ptr),
                       reinterpret_cast<uptr>(new_ptr), size, caller_pc);
  }
  if (ObservabilityEnabled())
    MemStatsOnReallocHookCall();
  return new_ptr;
}

// C++ `new` / `delete` usually bottoms out in malloc/free inside the standard
// library implementation. Intercept them directly and temporarily bypass the
// lower malloc/free hooks while calling the real operators so we do not end up
// with:
// - double accounting for new -> malloc
// - double metadata removal for delete -> free
INTERCEPTOR(void *, _Znwm, SIZE_T size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_Znwm)(size);

  const uptr caller_pc = HookCallerPc();
  void *ptr = nullptr;
  {
    ScopedInterceptorBypass scope;
    ptr = REAL(_Znwm)(size);
  }
  if (ptr) {
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), size ? size : 1, caller_pc);
  }
  return ptr;
}

INTERCEPTOR(void *, _Znam, SIZE_T size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_Znam)(size);

  const uptr caller_pc = HookCallerPc();
  void *ptr = nullptr;
  {
    ScopedInterceptorBypass scope;
    ptr = REAL(_Znam)(size);
  }
  if (ptr) {
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), size ? size : 1, caller_pc);
  }
  return ptr;
}

INTERCEPTOR(void *, _ZnwmRKSt9nothrow_t, SIZE_T size,
            std::nothrow_t const &nt) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZnwmRKSt9nothrow_t)(size, nt);

  const uptr caller_pc = HookCallerPc();
  void *ptr = nullptr;
  {
    ScopedInterceptorBypass scope;
    ptr = REAL(_ZnwmRKSt9nothrow_t)(size, nt);
  }
  if (ptr) {
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), size ? size : 1, caller_pc);
  }
  return ptr;
}

INTERCEPTOR(void *, _ZnamRKSt9nothrow_t, SIZE_T size,
            std::nothrow_t const &nt) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZnamRKSt9nothrow_t)(size, nt);

  const uptr caller_pc = HookCallerPc();
  void *ptr = nullptr;
  {
    ScopedInterceptorBypass scope;
    ptr = REAL(_ZnamRKSt9nothrow_t)(size, nt);
  }
  if (ptr) {
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), size ? size : 1, caller_pc);
  }
  return ptr;
}

INTERCEPTOR(void, _ZdlPv, void *ptr) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdlPv)(ptr);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdlPv)(ptr);
  }
}

INTERCEPTOR(void, _ZdaPv, void *ptr) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdaPv)(ptr);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdaPv)(ptr);
  }
}

INTERCEPTOR(void, _ZdlPvRKSt9nothrow_t, void *ptr,
            std::nothrow_t const &nt) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdlPvRKSt9nothrow_t)(ptr, nt);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdlPvRKSt9nothrow_t)(ptr, nt);
  }
}

INTERCEPTOR(void, _ZdaPvRKSt9nothrow_t, void *ptr,
            std::nothrow_t const &nt) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdaPvRKSt9nothrow_t)(ptr, nt);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdaPvRKSt9nothrow_t)(ptr, nt);
  }
}

INTERCEPTOR(void, _ZdlPvm, void *ptr, SIZE_T size) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdlPvm)(ptr, size);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdlPvm)(ptr, size);
  }
}

INTERCEPTOR(void, _ZdaPvm, void *ptr, SIZE_T size) {
  if (!ptr)
    return;
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  if (!RuntimeInited())
    Initialize();
  if (!HooksEnabled() || BypassInterceptors())
    return REAL(_ZdaPvm)(ptr, size);

  {
    ScopedInterceptorBypass scope;
    TrackHookFree(reinterpret_cast<uptr>(ptr));
  }
  {
    ScopedInterceptorBypass scope;
    REAL(_ZdaPvm)(ptr, size);
  }
}

} // extern "C"

namespace __ohos_memgraph {

void InitializeInterceptors() {
  static int inited;
  if (inited)
    return;

#if !SANITIZER_FUCHSIA
  // The formal runtime only depends on the standard allocation family. More
  // wrappers can be added later if the product surface grows.
  INTERCEPT_FUNCTION(malloc);
  INTERCEPT_FUNCTION(calloc);
  INTERCEPT_FUNCTION(free);
  INTERCEPT_FUNCTION(realloc);
  INTERCEPT_FUNCTION(_Znwm);
  INTERCEPT_FUNCTION(_Znam);
  INTERCEPT_FUNCTION(_ZnwmRKSt9nothrow_t);
  INTERCEPT_FUNCTION(_ZnamRKSt9nothrow_t);
  INTERCEPT_FUNCTION(_ZdlPv);
  INTERCEPT_FUNCTION(_ZdaPv);
  INTERCEPT_FUNCTION(_ZdlPvRKSt9nothrow_t);
  INTERCEPT_FUNCTION(_ZdaPvRKSt9nothrow_t);
  INTERCEPT_FUNCTION(_ZdlPvm);
  INTERCEPT_FUNCTION(_ZdaPvm);
#endif

  inited = 1;
}

} // namespace __ohos_memgraph
