# OHOS Memgraph Runtime

This document is the single source of truth for the formal `ohos_memgraph`
runtime. It consolidates the previous design notes, table/flow walkthroughs,
slab notes, frontend integration notes, device workflow, benchmark baseline,
and validation results into one English document.

## 1. Overview

`ohos_memgraph` is a low-level `compiler-rt` runtime for tracking:

- allocation-level metadata
- member-level metadata inside tracked allocations
- lightweight observability data for diagnostics and benchmarking

The runtime is intended to support IDE-style object inspection while keeping:

- bounded runtime-owned memory
- predictable internal storage layout
- minimal reliance on STL containers in low-level runtime code

## 2. Deliverables

Formal runtime directory:

- `compiler-rt/lib/ohos_memgraph`

Formal library names:

- `libclang_rt.memgraph.so`
- `libclang_rt.memgraph.a`

Public C API header:

- `compiler-rt/lib/ohos_memgraph/memgraph_interface.h`

Public exported entry points remain:

- `memgraph_init`
- `alloc_record`
- `store_record`
- `get_block_info`
- `get_member_info`
- `get_info`
- `get_info_records`
- `get_runtime_stats`
- `get_layout`

## 3. Design Goals

- Keep the functional model correct for tracked allocations and member records.
- Keep runtime-owned memory explicitly bounded.
- Avoid heavyweight runtime-internal allocations on hot paths.
- Support ABI-compatible `store_record(dst_ptr, ...)` without changing frontend
  instrumentation.
- Improve concurrency relative to the legacy global-structure design by
  separating:
  - exact base lookup
  - containing lookup
  - per-allocation member storage

## 4. Non-Goals and Constraints

- The runtime does not try to retain every possible piece of metadata forever.
- The current formal runtime prefers bounded memory and simpler cleanup over
  globally optimal retention.
- This code runs in `compiler-rt`, so it avoids depending on
  `std::map/std::unordered_map` on hot paths.
- The current ABI cannot be changed, so `store_record()` still receives a field
  address rather than an explicit owner base.

## 5. Runtime Structure

The implementation is organized around three core tables and one main runtime
module.

### 5.1 AllocTable

Files:

- `alloc_table.h`
- `alloc_table.cpp`

Responsibilities:

- track live allocations
- store block-level metadata
- answer exact `base -> alloc` queries
- answer containing queries for `dst_ptr`
- provide stable integer alloc ids

### 5.2 StoreTable

Files:

- `store_table.h`
- `store_table.cpp`

Responsibilities:

- provide a slab-backed node pool for per-allocation store chains
- store member-level metadata
- clear all member nodes belonging to an allocation at `free(base)`
- support owner-local lookup by `offset`

### 5.3 NameTable

Files:

- `name_table.h`
- `name_table.cpp`

Responsibilities:

- intern frontend string pointers into compact `u32` ids
- maintain separate tables for type names and variable names
- resolve ids back to strings during queries

### 5.4 Main Runtime Logic

Files:

- `memgraph.h`
- `memgraph.cpp`
- `memgraph_interceptors.cpp`
- `memgraph_allocation_functions.cpp`

Responsibilities:

- process-wide initialization
- malloc/free/realloc hook tracking
- metadata write entry points
- query entry points
- observability flag plumbing

### 5.5 Observability Layer

Files:

- `memgraph_monitoring.cpp`
- `memgraph_stats_internal.h`
- `memgraph_flags.h`
- `memgraph_flags.inc`

Responsibilities:

- maintain runtime counters and memory usage snapshots
- power diagnostics and benchmarks
- allow observability to be disabled without disabling the functional path

## 6. Core Data Structures

### 6.1 AllocTable

Each alloc entry logically contains:

- `base`
- `size`
- `type_id`
- `var_id`
- `store_head`
- `id`

Current indexing model:

- exact base hash table
- page-based range index
- slab-backed stable node ids
- free list for node reuse
- allocation-local `store_mu`

The current implementation is not a tree. Exact lookup and containing lookup
are split into two different indexes:

- `base -> alloc` via hash buckets
- `page -> candidate alloc ids` via page buckets

This lets the runtime keep:

- exact `base` operations fast and shardable
- `FindContaining(dst_ptr)` available without relying on a single global
  ordered structure

### 6.2 StoreTable

Each store node contains:

- `dst_offset`
- `type_id`
- `var_id`
- `owner_alloc_id`
- `next_or_free`

The current formal runtime does not maintain:

- a global store hash table
- a global store FIFO

Instead, each allocation owns a local singly-linked store chain:

```text
Alloc(id=5, store_head=42)
               |
               v
            slot42 -> slot17 -> slot8
```

Behavior:

- new store nodes are inserted at the head
- repeated writes to the same `owner + offset` append new history nodes
- member queries return the latest matching node
- `free(base)` clears the entire chain

### 6.3 NameTable

The runtime interns frontend string pointers into compact ids. This is pointer
identity interning, not string-content deduplication.

Implication:

- stable frontend-provided static strings work well
- identical string contents from different addresses are treated as different
  entries

## 7. Slab Allocation and Stable IDs

Both alloc nodes and store nodes are slab-backed.

Why:

- incremental runtime-owned memory growth
- fewer internal allocator calls
- predictable layout
- slot reuse via free lists
- stable integer ids instead of heavier pointer-based cross references

Important distinction:

- slab allocation is a storage strategy
- it is not the indexing strategy

The indexing strategy in the current formal runtime is:

- hash + page range index for allocs
- owner-local chains for store nodes

## 8. Locking Model

The formal runtime no longer depends on one global structure for all hot-path
operations. The important synchronization points are:

- alloc pool / slab reuse
- store pool / slab reuse
- alloc hash buckets
- page range index buckets
- per-allocation `store_mu`
- name interning tables

Concurrency characteristics:

- different allocations can often progress independently
- different owners can perform store operations in parallel
- hotspot owners still serialize on their local `store_mu`
- `store_record()` still pays the owner-lookup cost because ABI is unchanged

## 9. Main Runtime Flows

### 9.1 `malloc` Hook

1. runtime sees a new `base`
2. if the same address is already tracked, the old tracked state is cleared
3. a new alloc node is inserted into:
   - exact base hash
   - page range index

### 9.2 `free(base)`

1. exact alloc lookup by `base`
2. remove the alloc from the exact hash
3. remove the alloc from the page range index
4. clear the entire owner store chain
5. return alloc/store slots to their free lists

### 9.3 `realloc(old_base, new_base, new_size)`

- same-address realloc:
  - clear old member state
  - rebuild alloc tracking for the new size
  - preserve block metadata when possible
- moved realloc:
  - behave as `free(old) + alloc(new)`

### 9.4 `alloc_record(base, type, var, alloc_pc)`

1. exact alloc lookup by `base`
2. intern type/name strings
3. if `alloc_pc != 0`, overwrite the fallback `malloc_pc` captured earlier by the hook
4. update block-level metadata on that alloc row

### 9.5 `store_record(dst_ptr, ...)`

1. use the page range index to find the owner alloc containing `dst_ptr`
2. compute `offset = dst_ptr - owner.base`
3. intern type/name strings
4. allocate a store node
5. insert it at the head of that owner's store chain

### 9.6 `get_block_info(base)`

1. exact alloc lookup by `base`
2. resolve block metadata strings
3. return block-level information

### 9.7 `get_member_info(addr)`

1. containing alloc lookup by `addr`
2. compute `offset = addr - owner.base`
3. scan only that allocation's store chain
4. return the first matching offset, which is the latest record

## 10. Capacity Model

The current formal runtime uses bounded-memory `drop-new` semantics.

### 10.1 Alloc Capacity

When alloc capacity is reached:

- new tracked allocs are ignored
- old tracked allocs are not evicted to make room

### 10.2 Store Capacity

When store capacity is reached:

- new store nodes are ignored
- old store nodes are not evicted to make room

This is intentional. It simplifies retention semantics and avoids the legacy
FIFO eviction model.

## 11. Observability Flag

The formal runtime keeps the observability feature flag, and its default is:

- `OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0`

When observability is disabled:

- functional APIs still work:
  - `alloc_record`
  - `store_record`
  - `get_block_info`
  - `get_owner`
  - `get_member_info`
- observability APIs are disabled:
  - `get_info`
  - `get_info_records`
  - `get_runtime_stats`
  - `get_layout`
- `get_live_allocs` remains available
- the `atexit` summary is suppressed

Tests or tools that call observability APIs such as `get_info()`,
`get_info_records()`, or `get_runtime_stats()` must enable
the flag explicitly with:

- `OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=1`

This flag remains part of the formal runtime and has been validated on device.

## 12. Frontend Integration Notes

### 12.1 Key Semantic Point

`get_member_info(addr)` expects:

- any tracked field address inside a live heap object
- that address's owner-local offset to have store metadata

The runtime resolves the owner block first and then interprets the field slot:

- whole block: `get_block_info(base)`
- offset-0 member: `get_member_info(base)`
- later field: `get_member_info(base + 8)`

### 12.2 Why `store_record()` Still Needs Owner Lookup

The current ABI cannot be changed, so `store_record()` does not receive the
owner base directly. It receives a field address.

That is why the runtime still needs a containing lookup path:

- `dst_ptr -> owner alloc`

### 12.3 Frontend Smoke Coverage

The runtime tree includes frontend-oriented smoke tests for:

- manual block metadata
- manual block + member metadata
- auto-instrumented frontend cases
- shared-library frontend smoke

Business smoke cases were revalidated on the formal branch. Auto-instrumented
cases still depend on the frontend build flags used to build the test binary,
just like before.

## 13. Tests and Benchmarks

### 13.1 Main Functional Tests

- `memgraph_ohos_e2e_test.cpp`
- `memgraph_edge_cases_test.cpp`
- `memgraph_thread_stress.cpp`
- `memgraph_observability_flag_smoke.cpp`
- `memgraph_alloc_drop_new_test.cpp`
- `memgraph_store_drop_new_test.cpp`
- `memgraph_frontend_business_block_smoke_min.cpp`
- `memgraph_frontend_business_block_member_smoke_min.cpp`

### 13.2 Main Benchmarks

- `memgraph_high_concurrency_bench.cpp`
- `memgraph_read_heavy_bench.cpp`
- `memgraph_write_hotspot_bench.cpp`
- `memgraph_write_steady_state_bench.cpp`
- `memgraph_write_spread_bench.cpp`
- `memgraph_perf_bench.cpp`
- `memgraph_mem_curve.cpp`

### 13.3 Meaning of the Newer Write Benchmarks

`write_hotspot`

- extreme hotspot writes
- intentionally concentrates many writes on a small owner set

`write_steady_state`

- owners are initialized once
- hot loop is mainly `store_record`
- periodic sampling queries
- periodic free/replenish churn

`write_spread`

- low-to-medium contention model
- most writes go to private owners
- a smaller subset goes to shared owners
- intended to model modest collisions rather than extreme hotspots

## 14. Historical Lock-Refactor Baseline

Before the formal runtime design, a legacy V2 branch tried three lock-only
refactors:

1. global spin lock -> blocking lock
2. split name-path locking
3. split into `alloc_mu + store_mu`

Those experiments did not improve the overall system enough because the old data
structure coupling remained the dominant constraint.

That baseline is still useful context:

- it explains why the formal runtime moved toward structural separation rather
  than more lock surgery on the old design

## 15. Current Validation Status

### 15.1 Representative Device Validation Passed

On the clean formal branch, the following representative binaries were rebuilt
and revalidated on device:

- `memgraph_ohos_e2e_test`
- `memgraph_edge_cases_test`
- `memgraph_thread_stress`
- `memgraph_observability_flag_smoke`
- `memgraph_frontend_business_block_smoke_min`
- `memgraph_frontend_business_block_member_smoke_min`
- `memgraph_alloc_drop_new_test`
- `memgraph_store_drop_new_test`

### 15.2 Compatibility Summary

Within the set of functional scenarios already revalidated:

- no new runtime regression was observed versus the legacy V2 baseline
- frontend auto-instrumentation caveats remain the same as before
- the observability toggle remains available and validated

### 15.3 Benchmark Summary

Representative 32-thread comparisons versus the original V2 baseline:

- `high_concurrency`
  - improved from about `3004.8 ns/iter` to `2473.0 ns/iter`
- `read_heavy`
  - improved from about `444.1 ns/query` to `270.3 ns/query`
- `write_hotspot`
  - regressed in the extreme hotspot model
- `write_steady_state`
  - improved by roughly `31.7%`
- `write_spread`
  - improved by roughly `37.7%`

Interpretation:

- the formal runtime is not a universal win for every write pattern
- it performs better on more realistic low-to-medium contention write models
- it still has a known weakness for highly concentrated hotspot owners

### 15.4 Formal-Branch Benchmark Refresh

The following seven benchmarks were rebuilt and rerun on the clean
`memgraph_runtime` formal branch with the current `libclang_rt.memgraph.so`
preloaded on device.

`memgraph_high_concurrency_bench` (default args)

- 32 threads
- `ns_per_iter = 2405.1`
- `max_thread_ms = 768.51`
- no functional failures were reported

`memgraph_read_heavy_bench` (default args)

- 32 threads
- `ns_per_query = 325.6`
- `max_thread_ms = 520.07`
- no functional failures were reported

`memgraph_write_hotspot_bench` (default args)

- 32 threads
- `ns_per_op = 686.8`
- `max_thread_ms = 2195.85`
- `store_live` reached the bounded store capacity (`2000000`)

`memgraph_write_steady_state_bench` (default args)

- 32 threads
- `ns_per_store = 377.4`
- `max_thread_ms = 238.50`
- no functional failures were reported

`memgraph_write_spread_bench` (default args)

- 32 threads
- `ns_per_store = 465.1`
- `max_thread_ms = 148.27`
- no functional failures were reported

`memgraph_perf_bench` (default args)

- `alloc_ops = 500000`
- `store_ops = 1000000`
- malloc hook throughput: `2.05 Mops/s`
- `alloc_record()` throughput: `5.86 Mops/s`
- `store_record()` throughput: `0.64 Mops/s`
- runtime peak memory: `64034 KB`
- alloc/store peak memory: `44098 KB / 19840 KB`

`memgraph_mem_curve` (default args)

- `alloc_ops = 300000`
- `store_ops = 600000`
- runtime memory grew from `20960 KB` at init to `46626 KB` after the full
  alloc/store build-up
- after freeing the tracked objects, `alloc_live` and `store_live` returned to
  near-zero / zero, while slab-backed runtime memory stayed allocated for reuse

This refresh confirms that the formal branch has now been benchmarked directly,
instead of only inheriting performance conclusions from the earlier indexed
experimental branch.

## 16. Build and Device Workflow

### 16.1 Build the Runtime

```bash
BUILD_DIR=/srv/workspace/memgraph/memgraph_formal_out/clangrt-aarch64-linux-ohos-clean
ninja -C "$BUILD_DIR" clang_rt.memgraph-dynamic-aarch64 clang_rt.memgraph-aarch64
```

### 16.2 Cross-Compile a Test Binary

```bash
LLVM_INSTALL=/srv/workspace/memgraph/master/out/llvm-install
SYSROOT=/srv/workspace/memgraph/master/out/sysroot
TEST_DIR=/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests

"$LLVM_INSTALL/bin/clang++" \
  --target=aarch64-linux-ohos \
  --sysroot="$SYSROOT" \
  -O2 -g -std=c++17 -pthread \
  "$TEST_DIR/memgraph_thread_stress.cpp" \
  -ldl -o /tmp/memgraph_thread_stress
```

### 16.3 HDC from WSL

Recommended setup:

```bash
HDC="/mnt/c/Program Files/Huawei/DevEco Studio/sdk/default/openharmony/toolchains/hdc.exe"
SERIAL=YOUR_DEVICE_SERIAL
REMOTE=/data/local/tmp/memgraph_formal
```

Push runtime and a binary:

```bash
"$HDC" -t "$SERIAL" file send "$(wslpath -w /path/to/libclang_rt.memgraph.so)" "$REMOTE/"
"$HDC" -t "$SERIAL" file send "$(wslpath -w /path/to/test_binary)" "$REMOTE/"
```

Run with preload for tests that only use the functional APIs:

```bash
"$HDC" -t "$SERIAL" shell "cd $REMOTE && \
  export LD_LIBRARY_PATH=/system/lib64/ndk:/system/lib64:\$LD_LIBRARY_PATH && \
  export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && \
  ./test_binary"
```

Run with observability enabled for tests that call `get_info()`,
`get_info_records()`, `get_runtime_stats()`, `get_layout()`, or
`get_live_allocs()`:

```bash
"$HDC" -t "$SERIAL" shell "cd $REMOTE && \
  export LD_LIBRARY_PATH=/system/lib64/ndk:/system/lib64:\$LD_LIBRARY_PATH && \
  export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && \
  OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=1 ./test_binary"
```

Run with observability disabled:

```bash
"$HDC" -t "$SERIAL" shell "cd $REMOTE && \
  export LD_LIBRARY_PATH=/system/lib64/ndk:/system/lib64:\$LD_LIBRARY_PATH && \
  export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && \
  OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0 ./test_binary"
```

Run alloc drop-new with a tiny alloc capacity:

```bash
"$HDC" -t "$SERIAL" shell "cd $REMOTE && export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && OHOS_MEMGRAPH_ALLOC_TABLE_SIZE=4 ./memgraph_alloc_drop_new_test"
```

### 16.4 Shared-Frontend Smoke

The tree also contains shared-library frontend smoke support in:

- `tests/frontend_shared/`

That path is used when the frontend smoke is split into:

- shared frontend library under test
- standalone runner binary

### 16.4.1 Shared-Frontend PC Smoke: `malloc + store`

This section adds a shared-library PC smoke that validates two scenarios:

- direct `malloc(...)`
  - under the current frontend rules, `alloc_record/store_record` should be
    added automatically
  - used to validate frontend-provided `malloc_pc/store_pc`
- wrapped `malloc(...)`
  - wrapped by a helper, which under the current frontend rules usually does
    not receive an automatic `alloc_record`
  - used to validate the hook-fallback `malloc_pc`

File:

- `tests/frontend_shared/memgraph_frontend_auto_block_member_pc_smoke_min_shared.cpp`

This case is meant to run with the actual instrumented shared library produced
by the frontend.
The test itself does not call `alloc_record()` or `store_record()` manually. It
relies on:

- block metadata being written automatically by the runtime/frontend path
- member metadata being written automatically by real frontend instrumentation
- the frontend passing the more accurate alloc PC as an argument to
  `alloc_record(...)`
- the frontend passing the store PC as an argument to `store_record(...)`

Exported entry symbol:

- `memgraph_frontend_auto_block_member_pc_smoke_min_run`

First build the runner:

```bash
LLVM_INSTALL=/srv/workspace/memgraph/master/out/llvm-install
SYSROOT=/srv/workspace/memgraph/master/out/sysroot
TEST_DIR=/srv/workspace/memgraph/memgraph_formal_master/compiler-rt/lib/ohos_memgraph/tests

"$LLVM_INSTALL/bin/clang++" \
  --target=aarch64-linux-ohos \
000

- `/tmp/libmemgraph_frontend_auto_block_member_pc_smoke_min_shared.instrumented.so`

memgraph runtime so path:

- `/srv/workspace/memgraph/memgraph_formal_out/clangrt-aarch64-linux-ohos-clean/lib/aarch64-linux-ohos/libclang_rt.memgraph.so`

Push the runner, instrumented shared so, and runtime so to the device:

```bash
HDC="/mnt/c/Program Files/Huawei/DevEco Studio/sdk/default/openharmony/toolchains/hdc.exe"
SERIAL=YOUR_DEVICE_SERIAL
REMOTE=/data/local/tmp/memgraph_runtime_pc

RUNNER=/tmp/memgraph_frontend_shared_runner_pc
TEST_SO=/tmp/libmemgraph_frontend_auto_block_member_pc_smoke_min_shared.instrumented.so
RUNTIME_SO=/srv/workspace/memgraph/memgraph_formal_out/clangrt-aarch64-linux-ohos-clean/lib/aarch64-linux-ohos/libclang_rt.memgraph.so

"$HDC" -t "$SERIAL" shell "mkdir -p $REMOTE"
"$HDC" -t "$SERIAL" file send "$(wslpath -w "$RUNNER")" "$REMOTE/"
"$HDC" -t "$SERIAL" file send "$(wslpath -w "$TEST_SO")" "$REMOTE/"
"$HDC" -t "$SERIAL" file send "$(wslpath -w "$RUNTIME_SO")" "$REMOTE/"
"$HDC" -t "$SERIAL" shell "cd $REMOTE && chmod 755 memgraph_frontend_shared_runner_pc *.so && ls -l"
```

Run on the device:

```bash
"$HDC" -t "$SERIAL" shell "cd $REMOTE && \
  export LD_LIBRARY_PATH=/system/lib64/ndk:/system/lib64:\$LD_LIBRARY_PATH && \
  export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && \
  ./memgraph_frontend_shared_runner_pc \
    $REMOTE/libmemgraph_frontend_auto_block_member_pc_smoke_min_shared.instrumented.so \
    memgraph_frontend_auto_block_member_pc_smoke_min_run; \
  echo EXIT:\$?"
```

On success it prints:

- direct `malloc_pc`
- direct `malloc_pc_rel`
- direct `store_pc`
- direct `store_pc_rel`
- wrapped `malloc_pc`
- wrapped `wrapped_malloc_pc_rel`
- current shared-library base `so_base`

These outputs are intended for later symbolization on the host:

```bash
LLVM_SYMBOLIZER=/srv/workspace/memgraph/master/out/llvm-install/bin/llvm-symbolizer
TEST_SO=/tmp/libmemgraph_frontend_auto_block_member_pc_smoke_min_shared.instrumented.so

"$LLVM_SYMBOLIZER" --obj="$TEST_SO" --relative-address 0x<MALLOC_PC_REL>
"$LLVM_SYMBOLIZER" --obj="$TEST_SO" --relative-address 0x<STORE_PC_REL>
"$LLVM_SYMBOLIZER" --obj="$TEST_SO" --relative-address 0x<WRAPPED_MALLOC_PC_REL>
```

On AArch64 these PCs are return addresses.
If you want to land exactly on the call-instruction line, subtract `4` bytes
from the relative address before symbolizing.

### 16.4.2 Shared-Frontend PC Smoke: `new`

This shared-library `new` smoke also covers two scenarios:

- direct `new Node()`
  - under the current frontend rules, `alloc_record` should be added
    automatically
  - used to validate the frontend-provided `malloc_pc`
- wrapped `new Node()`
  - wrapped by a helper, which under the current frontend rules usually does
    not receive an automatic `alloc_record`
  - used to validate the `new` hook-fallback `malloc_pc`

File:

- `tests/frontend_shared/memgraph_frontend_auto_new_pc_smoke_min_shared.cpp`

First build the raw shared so:

```bash
TEST_SO_RAW=/tmp/libmemgraph_frontend_auto_new_pc_smoke_min_shared.so

"$LLVM_INSTALL/bin/clang++" \
  --target=aarch64-linux-ohos \
  --sysroot="$SYSROOT" \
  -g -Xclang -freference-tracking -O2 -std=c++17 \
  -mllvm -global-isel=false \
  -shared -fPIC \
  -Wl,--unresolved-symbols=ignore-all \
  -I "$OHOS_MEMGRAPH_DIR" \
  "$TEST_DIR/frontend_shared/memgraph_frontend_auto_new_pc_smoke_min_shared.cpp" \
  -ldl -o "$TEST_SO_RAW"
```

Then run binxo:

```bash
TEST_SO=/tmp/libmemgraph_frontend_auto_new_pc_smoke_min_shared_binxo.so

"$BINXO" \
  --hookmode 2 \
  --hook-inputfile "$TEST_SO_RAW" \
  --hook-outputfile "$TEST_SO"
```

Push and run:

```bash
"$HDC" -t "$SERIAL" file send "$(wslpath -w "$TEST_SO")" "$REMOTE/"
"$HDC" -t "$SERIAL" shell "cd $REMOTE && chmod 755 *.so"

"$HDC" -t "$SERIAL" shell "cd $REMOTE && \
  export LD_LIBRARY_PATH=/system/lib64/ndk:/system/lib64:\$LD_LIBRARY_PATH && \
  export LD_PRELOAD=$REMOTE/libclang_rt.memgraph.so && \
  ./memgraph_frontend_shared_runner_pc \
    $REMOTE/libmemgraph_frontend_auto_new_pc_smoke_min_shared_binxo.so \
    memgraph_frontend_auto_new_pc_smoke_min_run; \
  echo EXIT:\$?"
```

On success it prints:

- direct `malloc_pc`
- direct `malloc_pc_rel`
- wrapped `malloc_pc`
- wrapped `wrapped_malloc_pc_rel`
- current shared-library base `so_base`

Host-side symbolization:

```bash
"$LLVM_SYMBOLIZER" --obj="$TEST_SO" --relative-address 0x<DIRECT_MALLOC_PC_REL>
"$LLVM_SYMBOLIZER" --obj="$TEST_SO" --relative-address 0x<WRAPPED_MALLOC_PC_REL>
```

## 17. Review Checklist

For code review, the most important things to verify are:

- address reuse always clears old member metadata
- `free(base)` always clears the full owner chain
- `store_record(dst_ptr)` resolves the correct owner under concurrency
- `drop-new` does not corrupt already tracked alloc/store state
- observability remains strictly optional and does not affect the functional
  path

## 18. Bottom Line

The formal `ohos_memgraph` runtime is now:

- a clean master-based branch design
- renamed without `v2/indexed` in the runtime naming
- cleaned of legacy FIFO-era helper interfaces and eviction counters
- documented in English in one place
- rebuilt successfully
- representative-device-validated

The main known tradeoff that remains is hotspot-owner write behavior, which is
already visible in the dedicated benchmark results.
