//===-- memgraph_interface.h ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Stable public C ABI for OHOS memgraph.
//
// Frontend instrumentation, IDE queries, tests, and benchmarks all interact
// with the runtime through this header.
//===----------------------------------------------------------------------===//

#ifndef OHOS_MEMGRAPH_INTERFACE_H
#define OHOS_MEMGRAPH_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for the primary IDE query result structures.
typedef struct block_info block_info_t;
typedef struct member_info member_info_t;
typedef struct owner_info owner_info_t;

//===----------------------------------------------------------------------===//
// Primary API surface.
//===----------------------------------------------------------------------===//
//
// These interfaces form the main contract:
// - the frontend writes metadata with alloc_record / store_record
// - tools and IDEs read metadata with get_owner / get_block_info /
//   get_member_info

// Attach block-level metadata to the heap object at malloc_addr.
// alloc_pc is provided by the frontend and overrides the fallback malloc_pc
// captured earlier by the runtime hooks when available.
void alloc_record(unsigned long malloc_addr, const char *type_name,
                  const char *var_name, unsigned long alloc_pc);
// Attach metadata to a field write.
// dst_ptr is the field address itself, not the value stored in the field.
void store_record(unsigned long source_addr, unsigned long dst_ptr,
                  const char *type_name, const char *var_name,
                  unsigned long store_pc);
// Query block-level metadata for the object whose base address is base.
int get_block_info(unsigned long base, block_info_t *out);
// Query member-level metadata for any tracked field address and also return the
// recorded source address for the latest write to that field.
int get_member_info(unsigned long addr, member_info_t *out);
// Resolve which tracked heap object currently contains addr.
int get_owner(unsigned long addr, owner_info_t *out);

// Forward declarations for diagnostic / observability result structures.
typedef struct alloc_info alloc_info_t;
typedef struct info_record info_record_t;
typedef struct runtime_stats runtime_stats_t;
typedef struct live_alloc_info live_alloc_info_t;

//===----------------------------------------------------------------------===//
// Diagnostic / observability / benchmark API surface.
//===----------------------------------------------------------------------===//
//
// These interfaces are mainly used by:
// - tests
// - diagnostic tools
// - benchmarks
// - monitoring
// They are not part of the frontend write path or the primary IDE query path.

// Query a tracked alloc summary: whether it exists, its size, and how many
// metadata records are attached to it.
int get_info(unsigned long base, alloc_info_t *out);
// Enumerate all metadata records attached to a tracked alloc.
unsigned long get_info_records(unsigned long base, info_record_t *out,
                               unsigned long capacity);
// Export a snapshot of runtime memory usage, capacities, and event counters.
int get_runtime_stats(runtime_stats_t *out);
// Export alloc/store physical row sizes for monitoring and benchmark sizing.
int get_layout(unsigned long *alloc_row_bytes,
               unsigned long *store_row_bytes);
// Enumerate live tracked allocs whose internal ids are >= cursor.
// Unlike the observability snapshot APIs above, this remains available even
// when OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0.
// Writes up to capacity rows and returns the number written.
unsigned long get_live_allocs(unsigned long cursor, live_alloc_info_t *out,
                              unsigned long capacity);

//===----------------------------------------------------------------------===//
// Runtime management API.
//===----------------------------------------------------------------------===//

// Explicit initialization entry point.
//
// Most callers do not need to invoke this manually because the runtime lazily
// initializes on the first hook or query. This is mainly for:
// - tests that want a fixed initialization point
// - tools that want to bring the runtime up early
void memgraph_init(void);

//===----------------------------------------------------------------------===//
// Output data structures.
//===----------------------------------------------------------------------===//
//
// These structures carry the query results returned by the runtime.

// Result structures for the primary IDE query path.
//
// These structures directly correspond to the most important IDE/object-inspect
// queries:
// - get_owner() -> owner_info
// - get_block_info() -> block_info
// - get_member_info() -> member_info

// Block-level metadata for the heap object at base.
struct block_info {
  unsigned long base;
  unsigned long size;
  const char *type_name;
  const char *name;
  int found;
  unsigned long malloc_pc;
};

// Member-level metadata for a tracked field address within an owner heap
// object.
struct member_info {
  unsigned long base;
  unsigned long member_addr;
  unsigned long offset;
  const char *type_name;
  const char *name;
  int found;
  unsigned long store_pc;
  unsigned long source_addr;
};

// Owner resolution for an arbitrary address inside a tracked heap object.
struct owner_info {
  unsigned long base;
  unsigned long size;
  int found;
};

// Diagnostic / observability result structures.
//
// These are used by tests, diagnostics, benchmarks, and monitoring rather
// than the primary IDE display path.

// Summary returned by get_info(base): whether the alloc is tracked, its size,
// and how many logical metadata records are attached to it.
struct alloc_info {
  unsigned long base;
  unsigned long size;
  unsigned long record_count;
  int found;
};

// One metadata record returned by get_info_records(base).
// The first record may come from alloc_record(), and later records come from
// store_record().
struct info_record {
  const char *type_name;
  const char *name;
};

// Runtime statistics used for monitoring, benchmarking, debugging, and
// capacity planning.
//
// These fields are not required for frontend instrumentation or normal IDE
// queries. They answer questions such as:
// - how much memory the runtime currently uses and what the historical peak was
// - how much memory each internal table currently uses
// - how many live alloc/store records exist
// - current capacities and slab counts
// - how often hooks / alloc_record / store_record have been called
struct runtime_stats {
  // Total runtime memory usage, both current and peak.
  unsigned long runtime_current_bytes;
  unsigned long runtime_peak_bytes;

  // Per-component memory usage, both current and peak.
  unsigned long alloc_table_current_bytes;
  unsigned long alloc_table_peak_bytes;
  unsigned long store_table_current_bytes;
  unsigned long store_table_peak_bytes;
  unsigned long type_table_current_bytes;
  unsigned long type_table_peak_bytes;
  unsigned long var_table_current_bytes;
  unsigned long var_table_peak_bytes;
  unsigned long misc_current_bytes;
  unsigned long misc_peak_bytes;

  // Live record counts, both current and peak.
  unsigned long alloc_live_current;
  unsigned long alloc_live_peak;
  unsigned long store_live_current;
  unsigned long store_live_peak;

  // Current table shape.
  unsigned long alloc_capacity_current;
  unsigned long alloc_slab_count_current;
  unsigned long alloc_bucket_count_current;
  unsigned long alloc_bucket_page_count_current;
  unsigned long store_capacity_current;
  unsigned long store_slab_count_current;

  // Runtime event counters.
  // Monitoring uses these values to validate that hooks and metadata writes
  // behave as expected.
  unsigned long malloc_hook_calls;
  unsigned long free_hook_calls;
  unsigned long realloc_hook_calls;
  unsigned long malloc_record_calls;
  unsigned long store_record_calls;
};

// One live tracked allocation exported by get_live_allocs().
struct live_alloc_info {
  // Stable allocation-row id used as the pagination cursor.
  unsigned long id;
  unsigned long base;
  unsigned long size;
  const char *type_name;
  const char *name;
  unsigned long malloc_pc;
  unsigned long record_count;
};

#ifdef __cplusplus
}
#endif

#endif  // OHOS_MEMGRAPH_INTERFACE_H
