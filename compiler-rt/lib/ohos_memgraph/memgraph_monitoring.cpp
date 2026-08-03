//===-- memgraph_monitoring.cpp ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Diagnostic and observability implementation for the OHOS memgraph runtime.
//
// This file intentionally owns the entire observability side of the runtime:
// - internal memory accounting
// - hook / metadata-write event counters
// - get_info / get_info_records / get_runtime_stats / get_layout
// - the atexit summary
//
// Keeping those pieces here lets memgraph.cpp stay focused on the functional
// model: block lifetime, metadata writes, and the primary IDE block/member
// queries.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#include "memgraph.h"

#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_mutex.h"

extern "C" SANITIZER_WEAK_ATTRIBUTE int musl_log(const char *fmt, ...);
#define MEMGRAPH_LOG(fmt, ...)                                          \
  do {                                                                         \
    if (&musl_log)                                                             \
      musl_log(fmt, ##__VA_ARGS__);                                            \
  } while (0)

namespace __ohos_memgraph {

using namespace __sanitizer;

namespace {

// Atomics backing the external observability snapshot. They are never consulted
// for core functional decisions; they are only a reporting layer fed by the
// functional tables.
struct RuntimeStatsAtomic {
  atomic_uint64_t runtime_current;
  atomic_uint64_t runtime_peak;

  atomic_uint64_t alloc_current;
  atomic_uint64_t alloc_peak;
  atomic_uint64_t store_current;
  atomic_uint64_t store_peak;
  atomic_uint64_t type_current;
  atomic_uint64_t type_peak;
  atomic_uint64_t var_current;
  atomic_uint64_t var_peak;
  atomic_uint64_t misc_current;
  atomic_uint64_t misc_peak;

  atomic_uint64_t alloc_live_current;
  atomic_uint64_t alloc_live_peak;
  atomic_uint64_t store_live_current;
  atomic_uint64_t store_live_peak;

  atomic_uint64_t malloc_hook_calls;
  atomic_uint64_t free_hook_calls;
  atomic_uint64_t realloc_hook_calls;
  atomic_uint64_t malloc_record_calls;
  atomic_uint64_t store_record_calls;
};

RuntimeStatsAtomic runtime_stats = {};

inline void AddWithPeak(atomic_uint64_t *current, atomic_uint64_t *peak,
                        u64 delta) {
  const u64 now =
      atomic_fetch_add(current, delta, memory_order_relaxed) + delta;
  u64 old_peak = atomic_load(peak, memory_order_relaxed);
  while (now > old_peak && !atomic_compare_exchange_weak(
                               peak, &old_peak, now, memory_order_relaxed)) {
  }
}

inline void SubNoUnderflow(atomic_uint64_t *current, u64 delta) {
  u64 old = atomic_load(current, memory_order_relaxed);
  while (true) {
    const u64 sub = old < delta ? old : delta;
    const u64 next = old - sub;
    if (atomic_compare_exchange_weak(current, &old, next, memory_order_relaxed))
      return;
  }
}

inline void UpdateMax(atomic_uint64_t *maxv, u64 value) {
  u64 old = atomic_load(maxv, memory_order_relaxed);
  while (value > old && !atomic_compare_exchange_weak(maxv, &old, value,
                                                      memory_order_relaxed)) {
  }
}

} // namespace

//===----------------------------------------------------------------------===//
// Internal observability hooks.
//===----------------------------------------------------------------------===//

// The MemStatsOn* entry points below are intentionally tiny. Hot paths call
// them opportunistically, but the functional runtime never depends on their
// values to decide correctness.

void MemStatsOnAllocTableAlloc(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  AddWithPeak(&runtime_stats.alloc_current, &runtime_stats.alloc_peak, bytes);
  AddWithPeak(&runtime_stats.runtime_current, &runtime_stats.runtime_peak,
              bytes);
}

void MemStatsOnAllocTableFree(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  SubNoUnderflow(&runtime_stats.alloc_current, bytes);
  SubNoUnderflow(&runtime_stats.runtime_current, bytes);
}

void MemStatsOnStoreTableAlloc(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  AddWithPeak(&runtime_stats.store_current, &runtime_stats.store_peak, bytes);
  AddWithPeak(&runtime_stats.runtime_current, &runtime_stats.runtime_peak,
              bytes);
}

void MemStatsOnStoreTableFree(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  SubNoUnderflow(&runtime_stats.store_current, bytes);
  SubNoUnderflow(&runtime_stats.runtime_current, bytes);
}

void MemStatsOnTypeTableAlloc(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  AddWithPeak(&runtime_stats.type_current, &runtime_stats.type_peak, bytes);
  AddWithPeak(&runtime_stats.runtime_current, &runtime_stats.runtime_peak,
              bytes);
}

void MemStatsOnTypeTableFree(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  SubNoUnderflow(&runtime_stats.type_current, bytes);
  SubNoUnderflow(&runtime_stats.runtime_current, bytes);
}

void MemStatsOnVarTableAlloc(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  AddWithPeak(&runtime_stats.var_current, &runtime_stats.var_peak, bytes);
  AddWithPeak(&runtime_stats.runtime_current, &runtime_stats.runtime_peak,
              bytes);
}

void MemStatsOnVarTableFree(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  SubNoUnderflow(&runtime_stats.var_current, bytes);
  SubNoUnderflow(&runtime_stats.runtime_current, bytes);
}

void MemStatsOnMiscAlloc(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  AddWithPeak(&runtime_stats.misc_current, &runtime_stats.misc_peak, bytes);
  AddWithPeak(&runtime_stats.runtime_current, &runtime_stats.runtime_peak,
              bytes);
}

void MemStatsOnMiscFree(uptr bytes) {
  if (!ObservabilityEnabled() || !bytes)
    return;
  SubNoUnderflow(&runtime_stats.misc_current, bytes);
  SubNoUnderflow(&runtime_stats.runtime_current, bytes);
}

void MemStatsOnMallocHookCall() {
  if (!ObservabilityEnabled())
    return;
  atomic_fetch_add(&runtime_stats.malloc_hook_calls, 1, memory_order_relaxed);
}

void MemStatsOnFreeHookCall() {
  if (!ObservabilityEnabled())
    return;
  atomic_fetch_add(&runtime_stats.free_hook_calls, 1, memory_order_relaxed);
}

void MemStatsOnReallocHookCall() {
  if (!ObservabilityEnabled())
    return;
  atomic_fetch_add(&runtime_stats.realloc_hook_calls, 1, memory_order_relaxed);
}

void MemStatsOnMallocRecordCall() {
  if (!ObservabilityEnabled())
    return;
  atomic_fetch_add(&runtime_stats.malloc_record_calls, 1,
                   memory_order_relaxed);
}

void MemStatsOnStoreRecordCall() {
  if (!ObservabilityEnabled())
    return;
  atomic_fetch_add(&runtime_stats.store_record_calls, 1,
                   memory_order_relaxed);
}

void MemStatsUpdateLiveCounters(uptr alloc_live, uptr store_live) {
  if (!ObservabilityEnabled())
    return;
  atomic_store(&runtime_stats.alloc_live_current, alloc_live,
               memory_order_relaxed);
  atomic_store(&runtime_stats.store_live_current, store_live,
               memory_order_relaxed);
  UpdateMax(&runtime_stats.alloc_live_peak, alloc_live);
  UpdateMax(&runtime_stats.store_live_peak, store_live);
}

//===----------------------------------------------------------------------===//
// External observability queries.
//===----------------------------------------------------------------------===//
//
// These queries never mutate runtime state. They only reshape internal state
// into views that are easier to consume in diagnostics, benchmarks and capacity
// analysis.

bool GetInfo(uptr base, alloc_info_t *out) {
  if (!out)
    return false;
  internal_memset(out, 0, sizeof(*out));
  if (!RuntimeInited() || !ObservabilityEnabled())
    return false;

  LockedAlloc entry = {};
  if (!alloc_table->LockByBase(base, &entry))
    return false;

  out->base = (unsigned long)entry.base;
  out->size = (unsigned long)entry.size;
  // The block row itself contributes one logical record when block metadata is
  // present. Member records are counted from the owner-local store chain.
  out->record_count =
      (unsigned long)(((entry.type_id || entry.var_id) ? 1 : 0) +
                      store_table->CountRecords(entry.store_head));
  out->found = 1;
  alloc_table->Unlock(&entry);
  return true;
}

uptr GetInfoRecords(uptr base, info_record_t *out, uptr capacity) {
  if (!RuntimeInited() || !ObservabilityEnabled())
    return 0;

  LockedAlloc entry = {};
  uptr store_count = 0;
  StoreInfoRecordIds *raw_records = nullptr;

  if (!alloc_table->LockByBase(base, &entry))
    return 0;

  store_count = store_table->CountRecords(entry.store_head);
  const uptr store_copy_cap = out && capacity > 0
                                  ? ((entry.type_id || entry.var_id)
                                         ? (capacity > 1 ? capacity - 1 : 0)
                                         : capacity)
                                  : 0;
  if (store_copy_cap > 0) {
    raw_records = (StoreInfoRecordIds *)InternalAlloc(
        store_copy_cap * sizeof(StoreInfoRecordIds), nullptr, 0);
    if (!raw_records) {
      alloc_table->Unlock(&entry);
      return 0;
    }
    internal_memset(raw_records, 0,
                    store_copy_cap * sizeof(StoreInfoRecordIds));
    store_table->GetInfoRecordIds(entry.store_head, raw_records,
                                  store_copy_cap);
  }
  alloc_table->Unlock(&entry);

  uptr count = 0;
  if (entry.type_id || entry.var_id) {
    // Block-level metadata is reported first, followed by store-chain records.
    if (out && count < capacity) {
      out[count].type_name = type_table->Resolve(entry.type_id);
      out[count].name = var_table->Resolve(entry.var_id);
    }
    ++count;
  }

  const uptr max_store_copy =
      out && count < capacity ? capacity - count : 0;
  const uptr copied = Min<uptr>(store_count, max_store_copy);
  for (uptr i = 0; i < copied; ++i) {
    out[count + i].type_name = type_table->Resolve(raw_records[i].type_id);
    out[count + i].name = var_table->Resolve(raw_records[i].var_id);
  }

  if (raw_records)
    InternalFree(raw_records);
  return count + store_count;
}

//===----------------------------------------------------------------------===//
// Runtime observability snapshots and layout information.
//===----------------------------------------------------------------------===//
//
// This section packages atomics and table-shape information into stable
// snapshots for:
// - monitoring records
// - benchmark comparisons
// - capacity and memory-model evaluation

bool MemStatsGetSnapshot(runtime_stats_t *out) {
  if (!out)
    return false;
  internal_memset(out, 0, sizeof(*out));
  if (!ObservabilityEnabled())
    return false;
  out->runtime_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.runtime_current, memory_order_relaxed);
  out->runtime_peak_bytes = (unsigned long)atomic_load(
      &runtime_stats.runtime_peak, memory_order_relaxed);
  out->alloc_table_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.alloc_current, memory_order_relaxed);
  out->alloc_table_peak_bytes = (unsigned long)atomic_load(
      &runtime_stats.alloc_peak, memory_order_relaxed);
  out->store_table_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.store_current, memory_order_relaxed);
  out->store_table_peak_bytes = (unsigned long)atomic_load(
      &runtime_stats.store_peak, memory_order_relaxed);
  out->type_table_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.type_current, memory_order_relaxed);
  out->type_table_peak_bytes = (unsigned long)atomic_load(
      &runtime_stats.type_peak, memory_order_relaxed);
  out->var_table_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.var_current, memory_order_relaxed);
  out->var_table_peak_bytes = (unsigned long)atomic_load(
      &runtime_stats.var_peak, memory_order_relaxed);
  out->misc_current_bytes = (unsigned long)atomic_load(
      &runtime_stats.misc_current, memory_order_relaxed);
  out->misc_peak_bytes = (unsigned long)atomic_load(&runtime_stats.misc_peak,
                                                    memory_order_relaxed);

  out->alloc_live_current = (unsigned long)atomic_load(
      &runtime_stats.alloc_live_current, memory_order_relaxed);
  out->alloc_live_peak = (unsigned long)atomic_load(
      &runtime_stats.alloc_live_peak, memory_order_relaxed);
  out->store_live_current = (unsigned long)atomic_load(
      &runtime_stats.store_live_current, memory_order_relaxed);
  out->store_live_peak = (unsigned long)atomic_load(
      &runtime_stats.store_live_peak, memory_order_relaxed);

  // Table-shape fields are read from the live runtime objects because they are
  // structural properties, not hot-path counters.
  out->alloc_capacity_current =
      alloc_table ? (unsigned long)alloc_table->Capacity() : 0;
  out->alloc_slab_count_current =
      alloc_table ? (unsigned long)alloc_table->SlabCount() : 0;
  out->alloc_bucket_count_current =
      alloc_table ? (unsigned long)alloc_table->BucketCount() : 0;
  out->alloc_bucket_page_count_current =
      alloc_table ? (unsigned long)alloc_table->BucketPageCount() : 0;
  out->store_capacity_current =
      store_table ? (unsigned long)store_table->Capacity() : 0;
  out->store_slab_count_current =
      store_table ? (unsigned long)store_table->SlabCount() : 0;

  out->malloc_hook_calls = (unsigned long)atomic_load(
      &runtime_stats.malloc_hook_calls, memory_order_relaxed);
  out->free_hook_calls = (unsigned long)atomic_load(
      &runtime_stats.free_hook_calls, memory_order_relaxed);
  out->realloc_hook_calls = (unsigned long)atomic_load(
      &runtime_stats.realloc_hook_calls, memory_order_relaxed);
  out->malloc_record_calls = (unsigned long)atomic_load(
      &runtime_stats.malloc_record_calls, memory_order_relaxed);
  out->store_record_calls = (unsigned long)atomic_load(
      &runtime_stats.store_record_calls, memory_order_relaxed);
  return true;
}

bool GetRuntimeStats(runtime_stats_t *out) {
  if (!ObservabilityEnabled())
    return false;
  return MemStatsGetSnapshot(out);
}

bool GetLayout(unsigned long *alloc_row_bytes, unsigned long *store_row_bytes) {
  if (!alloc_row_bytes || !store_row_bytes || !RuntimeInited() ||
      !ObservabilityEnabled())
    return false;
  *alloc_row_bytes = (unsigned long)alloc_table->StorageEntrySize();
  *store_row_bytes = (unsigned long)store_table->RowSize();
  return true;
}

uptr GetLiveAllocs(uptr cursor, live_alloc_info_t *out, uptr capacity) {
  if (!RuntimeInited() || !out || capacity == 0)
    return 0;

  const uptr limit = alloc_table ? alloc_table->Capacity() : 0;
  uptr written = 0;
  for (uptr id = cursor; id < limit && written < capacity; ++id) {
    AllocEntry entry = {};
    if (!alloc_table->GetById(static_cast<s32>(id), &entry))
      continue;

    out[written].id = (unsigned long)entry.id;
    out[written].base = (unsigned long)entry.base;
    out[written].size = (unsigned long)entry.size;
    out[written].type_name = type_table->Resolve(entry.type_id);
    out[written].name = var_table->Resolve(entry.var_id);
    out[written].malloc_pc = (unsigned long)entry.malloc_pc;
    out[written].record_count =
        (unsigned long)(((entry.type_id || entry.var_id) ? 1 : 0) +
                        store_table->CountRecords(entry.store_head));
    ++written;
  }
  return written;
}

//===----------------------------------------------------------------------===//
// Process-exit observability summary.
//===----------------------------------------------------------------------===//
//
// These prints are only for on-device observation and benchmark summaries.
// They never participate in functional decisions and never mutate runtime
// structures.

void MemStatsLogSummary() {
  if (!ObservabilityEnabled())
    return;
  runtime_stats_t st = {};
  MemStatsGetSnapshot(&st);

  ++ohos_memgraph_disable_interceptors;
  MEMGRAPH_LOG(
      "[memgraph] summary runtime_kb(cur=%{public}lu peak=%{public}lu) "
      "alloc_live_peak=%{public}lu store_live_peak=%{public}lu "
      "alloc_cap=%{public}lu store_cap=%{public}lu\n",
      st.runtime_current_bytes / 1024, st.runtime_peak_bytes / 1024,
      st.alloc_live_peak, st.store_live_peak, st.alloc_capacity_current,
      st.store_capacity_current);
  MEMGRAPH_LOG(
      "[memgraph] summary alloc_kb(cur=%{public}lu peak=%{public}lu) "
      "store_kb(cur=%{public}lu peak=%{public}lu) "
      "type_kb(cur=%{public}lu peak=%{public}lu) "
      "var_kb(cur=%{public}lu peak=%{public}lu) "
      "alloc_slabs=%{public}lu bucket_count=%{public}lu "
      "bucket_pages=%{public}lu "
      "store_slabs=%{public}lu\n",
      st.alloc_table_current_bytes / 1024, st.alloc_table_peak_bytes / 1024,
      st.store_table_current_bytes / 1024, st.store_table_peak_bytes / 1024,
      st.type_table_current_bytes / 1024, st.type_table_peak_bytes / 1024,
      st.var_table_current_bytes / 1024, st.var_table_peak_bytes / 1024,
      st.alloc_slab_count_current, st.alloc_bucket_count_current,
      st.alloc_bucket_page_count_current, st.store_slab_count_current);
  MEMGRAPH_LOG(
      "[memgraph] summary hooks malloc=%{public}lu free=%{public}lu "
      "realloc=%{public}lu alloc_record=%{public}lu store_record=%{public}lu\n",
      st.malloc_hook_calls, st.free_hook_calls, st.realloc_hook_calls,
      st.malloc_record_calls, st.store_record_calls);
  --ohos_memgraph_disable_interceptors;
}

} // namespace __ohos_memgraph
#endif /* OHOS_LLVM */
