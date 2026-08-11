//===-- memgraph.h ----------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Main internal declarations for the OHOS memgraph runtime.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#ifndef OHOS_MEMGRAPH_H
#define OHOS_MEMGRAPH_H

#include "alloc_table.h"
#include "name_table.h"
#include "memgraph_flags.h"
#include "memgraph_interface.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "store_table.h"

namespace __ohos_memgraph {

using __sanitizer::uptr;

// Shared integer hash mixer for memgraph runtime tables.
//
// This is MurmurHash3's fmix64 finalizer: a stable 64-bit integer mixer with
// strong avalanche properties for structured keys such as addresses and page
// ids. It is not intended for cryptographic use.
static inline uptr MixUptr(uptr value) {
  value ^= value >> 33;
  value *= 0xff51afd7ed558ccdULL;
  value ^= value >> 33;
  value *= 0xc4ceb9fe1a85ec53ULL;
  value ^= value >> 33;
  return value;
}

extern __sanitizer::atomic_uint8_t ohos_memgraph_inited;
extern __sanitizer::atomic_uint8_t ohos_memgraph_init_is_running;
extern THREADLOCAL int ohos_memgraph_disable_interceptors;

// Core global runtime state shared by the functional path and the
// observability/diagnostics path.
extern AllocTable *alloc_table;
extern StoreTable *store_table;
extern NameTable *type_table;
extern NameTable *var_table;
extern __sanitizer::StaticSpinMutex alloc_mu;
extern __sanitizer::StaticSpinMutex store_mu;
extern __sanitizer::StaticSpinMutex graph_mu;

//===----------------------------------------------------------------------===//
// Core lifecycle and metadata-write entry points.
//===----------------------------------------------------------------------===//

// Process-wide initialization that creates all core tables and installs
// interceptors.
void Initialize();
bool RuntimeInited();
bool RuntimeInitIsRunning();
// Install the malloc/calloc/free/realloc interceptors.
void InitializeInterceptors();

// Allocation lifecycle hooks reached from malloc/free/realloc interceptors.
void TrackHookAlloc(uptr base, uptr size, uptr malloc_pc);
void TrackHookFree(uptr base);
void TrackHookRealloc(uptr old_base, uptr new_base, uptr new_size,
                      uptr malloc_pc);

// Frontend metadata-write entry points.
// alloc_record attaches block-level metadata to the allocation itself.
void RecordMallocMetadata(uptr base, const char *type_name,
                          const char *var_name, uptr alloc_pc);
// store_record attaches member-level metadata to a field address inside a
// tracked allocation.
void RecordStoreMetadata(uptr source_addr, uptr dst_ptr,
                         const char *type_name, const char *var_name,
                         uptr store_pc);

//===----------------------------------------------------------------------===//
// Core query path used by IDE object inspection.
//===----------------------------------------------------------------------===//

bool GetBlockInfo(uptr base, block_info_t *out);
bool GetMemberInfo(uptr addr, member_info_t *out);
bool GetOwner(uptr addr, owner_info_t *out);

//===----------------------------------------------------------------------===//
// Observability and diagnostic query entry points.
//===----------------------------------------------------------------------===//

// These queries primarily exist for tests, benchmarks and diagnostics. They
// are not part of the primary IDE object-inspection path.
bool GetInfo(uptr base, alloc_info_t *out);
uptr GetInfoRecords(uptr base, info_record_t *out, uptr capacity);
bool GetRuntimeStats(runtime_stats_t *out);
bool GetLayout(unsigned long *alloc_row_bytes, unsigned long *store_row_bytes);
uptr GetLiveAllocs(uptr cursor, live_alloc_info_t *out, uptr capacity);

Flags *flags();
bool HooksEnabled();
bool ObservabilityEnabled();

}  // namespace __ohos_memgraph
#endif  // OHOS_MEMGRAPH_H
#endif /* OHOS_LLVM */
