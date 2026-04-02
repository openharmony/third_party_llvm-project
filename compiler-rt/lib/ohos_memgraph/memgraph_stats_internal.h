//===-- memgraph_stats_internal.h -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Internal observability hook declarations.
//
// These declarations are only used inside the runtime to support:
// - monitoring interfaces such as get_runtime_stats() / get_layout()
// - the atexit summary
// - benchmarks, capacity analysis and on-device diagnostics
//
// The core functional path does not depend on any of these return values. They
// only record runtime-local memory usage, live counts and event counters so
// that observability logic stays separate from the functional model.
//===----------------------------------------------------------------------===//

#ifndef OHOS_MEMGRAPH_STATS_INTERNAL_H
#define OHOS_MEMGRAPH_STATS_INTERNAL_H

#include "memgraph_interface.h"
#include "sanitizer_common/sanitizer_common.h"

namespace __ohos_memgraph {

using __sanitizer::uptr;

// Track memory deltas for runtime-owned components.
void MemStatsOnAllocTableAlloc(uptr bytes);
void MemStatsOnAllocTableFree(uptr bytes);
void MemStatsOnStoreTableAlloc(uptr bytes);
void MemStatsOnStoreTableFree(uptr bytes);
void MemStatsOnTypeTableAlloc(uptr bytes);
void MemStatsOnTypeTableFree(uptr bytes);
void MemStatsOnVarTableAlloc(uptr bytes);
void MemStatsOnVarTableFree(uptr bytes);
void MemStatsOnMiscAlloc(uptr bytes);
void MemStatsOnMiscFree(uptr bytes);

// Track key event counters from the functional path so monitoring can
// understand runtime load and store-table behavior.
void MemStatsOnMallocHookCall();
void MemStatsOnFreeHookCall();
void MemStatsOnReallocHookCall();
void MemStatsOnMallocRecordCall();
void MemStatsOnStoreRecordCall();

// Refresh the current live allocation / live store counts.
//
// Callers compute the live counts while holding their own structural locks and
// then publish the values into monitoring-layer atomics.
void MemStatsUpdateLiveCounters(uptr alloc_live, uptr store_live);

// Build an external observability snapshot and emit the atexit summary.
bool MemStatsGetSnapshot(runtime_stats_t *out);
void MemStatsLogSummary();

} // namespace __ohos_memgraph

#endif // OHOS_MEMGRAPH_STATS_INTERNAL_H
