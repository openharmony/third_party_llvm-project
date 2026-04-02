# ohos_memgraph Runtime Audit Report

- **Review mode**: Module/runtime audit (Mode 2)
- **Scope**: `compiler-rt/lib/ohos_memgraph/` — all 7 `.cpp`, 7 `.h`/`.inc`,
  `CMakeLists.txt`, `README.md`
- **Review date**: 2026-03-30
- **Reviewed commits up to**: `0f3da9d42321` (`[MEMORY_MAP] publish runtime
  init state with atomics`)
- **Validation level**: Static only — code review without build/test execution

---

## 1. Findings

### F1 — High: `LockContaining` acquires `store_mu` while holding a page bucket lock

**File**: `alloc_table.cpp`, `LockContaining` (line ~851)

```cpp
bool AllocTable::LockContaining(uptr addr, LockedAlloc *out) const {
  // ...
  SpinMutexLock page_lock(&page_mutexes_[bucket]);
  while (link_id >= 0) {
    // ...
        node->store_mu.Lock();   // nested lock under page_mutexes_[bucket]
```

**Why it matters**:

`LockContaining` holds `page_mutexes_[bucket]` and then iterates over
candidate allocations, calling `node->store_mu.Lock()` on each. If the
candidate does not match, it unlocks and tries the next one. This means
the page bucket lock is held for the duration of potentially multiple
`store_mu` acquisitions.

A deadlock could occur if the lock ordering were violated:

- Thread A: `LockContaining` → holds `page_mutexes_[X]` → waits on
  `node->store_mu`
- Thread B: holds `node->store_mu` → calls `RemoveRangeForEntry` → waits
  on `page_mutexes_[X]`

**Mitigating factor**: The current `BeginRemove` design deliberately
unlocks `store_mu` before entering `RemoveRangeForEntry`, so thread B
never holds both locks simultaneously. This makes the deadlock
unreachable under the current code flow. However, the lock ordering
constraint is not documented or enforced by assertions.

**Category**: contract gap

**Recommendation**: Add a comment or assertion at the top of
`RemoveRangeForEntry` stating that callers must not hold any
`store_mu`. This protects against future maintenance breaking the
ordering.

---

### F2 — ~~High~~ Resolved: non-atomic init flags

**File**: `memgraph.cpp`, lines 24–25

**Previously**: `ohos_memgraph_inited` was a plain `int` and
`ohos_memgraph_init_is_running` was a plain `bool`, read without locks
on every hot path.

**Fix** (commit `0f3da9d42321`):

- Both changed to `volatile atomic_uint8_t`.
- All reads now go through `RuntimeInited()` / `RuntimeInitIsRunning()`
  with `memory_order_acquire`.
- All writes go through `PublishRuntimeInited()` /
  `SetRuntimeInitIsRunning()` with `memory_order_release`.
- `DlsymAlloc::UseImpl()` updated to call `RuntimeInitIsRunning()`.
- All interceptors updated to call `RuntimeInited()`.

The acquire/release pairing forms a correct publication pattern: all
table pointer writes happen before the release store in
`PublishRuntimeInited()`, and any thread that sees `inited == true`
via the acquire load is guaranteed to observe fully initialized table
pointers.

**Verdict**: Fully resolved. No residual issues.

---

### F3 — Medium: `BeginRemove` unlocks `store_mu` before store chain cleanup

**File**: `alloc_table.cpp`, `BeginRemove` (line ~960)

```cpp
node->state = kNodeDeleting;
// ... detach from hash ...
removed->store_head = node->store_head;
node->store_mu.Unlock();   // store chain still reachable via page index
return true;
```

**Why it matters**: Between `BeginRemove` unlocking `store_mu` and the
caller completing `RemoveRangeForEntry` + `RemoveAllForAlloc`, a
concurrent `LockContaining` can still find this allocation through the
page range index. It will lock `store_mu`, see
`state == kNodeDeleting`, and return false. This behavior is
functionally correct — a `store_record` arriving during this window
is silently dropped, which is the expected semantic when `free` and
`store` race on the same allocation.

**Category**: contract gap — the window and its intended behavior should
be documented.

---

### F4 — Medium: `NameTable::Intern` may overflow the `u32` id space

**File**: `name_table.cpp`, `Intern` (line ~168)

```cpp
u32 id = static_cast<u32>(++id_size_);
id_to_ptr_[id] = ptr;
```

`id_size_` is `uptr` (64-bit), but the returned `id` is `u32`. If the
number of interned strings ever exceeds `2^32 - 1`, the id wraps to
zero, which is the reserved "no metadata" sentinel. `id_to_ptr_[0]`
would be overwritten with a real pointer, corrupting the id-to-string
mapping.

**Practical risk**: Very low — exceeding 4 billion unique pointer
identities is unrealistic. But the code lacks a capacity guard.

**Category**: code bug (unchecked truncation)

**Recommendation**: Add a check after `++id_size_`: if
`id_size_ > UINT32_MAX`, return 0 without inserting.

---

### F5 — Medium: returned string pointers assume process-lifetime stability

**Files**: `memgraph.cpp` (`GetBlockInfo`, `GetMemberInfo`),
`memgraph_monitoring.cpp` (`GetInfoRecords`, `GetLiveAllocs`)

All query paths resolve interned `type_id` / `var_id` back to
`const char *` via `NameTable::Resolve()`. The returned pointer points
into the frontend-provided static string that was originally passed to
`alloc_record` / `store_record`.

**Why it matters**: If the frontend provides strings from a shared
library that is later `dlclose`-d, the pointer becomes dangling. The
runtime has no way to detect this.

**Category**: contract gap

**Recommendation**: Document in the public header
(`memgraph_interface.h`) that string pointers passed to
`alloc_record` / `store_record` must remain valid for the lifetime of
the process.

---

### F6 — Medium: `calloc(0, 0)` edge case

**File**: `memgraph_interceptors.cpp`, `calloc` interceptor (line ~89)

```cpp
void *ptr = REAL(calloc)(nmemb, size);
if (ptr && !CheckForCallocOverflow(size, nmemb)) {
    ScopedInterceptorBypass scope;
    TrackHookAlloc(reinterpret_cast<uptr>(ptr), nmemb * size, caller_pc);
}
```

When both `nmemb` and `size` are zero, some allocators return a non-NULL
pointer. The runtime would track this as a zero-size allocation.
`AllocTable::Insert` stores `ClampSize(0) = 0`, and
`LastPageIdForRange(base, 0)` falls back to covering only the page
containing `base`. This is structurally safe but semantically ambiguous.

**Category**: contract gap (edge case)

---

### F7 — Medium: same-address `realloc` clears all member store records

**File**: `memgraph.cpp`, `TrackHookRealloc` (line ~290)

```cpp
if (old_base && new_base && old_base == new_base) {
    // ... save old block metadata ...
    TrackHookFree(old_base);           // clears entire store chain
    TrackHookAlloc(new_base, new_size, malloc_pc);
    // ... restore block metadata only, store chain is lost ...
```

Same-address realloc preserves block-level metadata (`type_id`,
`var_id`) but destroys all member-level store records. This is
documented as intentional in `README.md` ("clear old member state"),
but could surprise debugger users who realloc an object and find its
member annotations gone.

**Category**: contract gap (by design, but worth documenting in the
public interface)

---

### F8 — Low: `GetLiveAllocs` returns a best-effort traversal, not a snapshot

**File**: `memgraph_monitoring.cpp`, `GetLiveAllocs` (line ~406)

```cpp
for (uptr id = cursor; id < limit && written < capacity; ++id) {
    AllocEntry entry = {};
    if (!alloc_table->GetById(static_cast<s32>(id), &entry))
      continue;
```

Each `GetById` call locks only that allocation's `store_mu`. No global
lock is held across the traversal. Concurrent alloc/free operations can
cause the returned list to be neither a snapshot at traversal start nor
at traversal end.

**Category**: contract gap — acceptable for diagnostics and benchmarks,
but should be documented as "best-effort, not point-in-time consistent".

---

### F9 — ~~Low~~ Mostly resolved: slab pointer table growth thread safety

**Previously**: `node_slabs_` (AllocTable) and `slabs_` (StoreTable)
could be replaced at runtime by `EnsureNodeSlabPtrCapacity` /
`EnsureSlabPtrCapacity`, creating a theoretical UAF if a concurrent
reader held the old pointer.

**Fix** (commit `d7b496c8a503`):

- **`node_slabs_` (AllocTable)**: Pre-allocated in `Init()` with
  `CeilDiv(max_capacity_, kNodesPerSlab)` slots.
  `EnsureNodeSlabPtrCapacity` simplified to a pure bounds check. The
  pointer table is never replaced at runtime. **Fully resolved.**

- **`slabs_` (StoreTable)**: Same pattern — pre-allocated in `Init()`.
  `EnsureSlabPtrCapacity` simplified to a pure bounds check. **Fully
  resolved.**

- **`range_slabs_` (AllocTable RangeLink)**: Still grows dynamically
  (one allocation can span many pages, requiring unbounded RangeLink
  slots). Old slab pointer arrays are now retired into a linked list
  (`RetiredRangeSlabPtrBlock`) instead of being freed, preventing UAF.
  The `range_slabs_` pointer itself is still non-atomic, so a
  theoretical data race remains: a reader could see a stale
  `range_slabs_` pointer paired with a newer `range_slab_ptr_cap_`.
  Practical risk is extremely low because range slab growth is far less
  frequent than hot-path operations.

**Verdict**: Primary risk eliminated. Residual theoretical race on
`range_slabs_` is acceptable.

---

### F10 — Low (new): retired range slab pointer block allocation failure causes silent memory leak

**File**: `alloc_table.cpp`, `EnsureRangeSlabPtrCapacity` (line ~366)

```cpp
RetiredRangeSlabPtrBlock *retired =
    (RetiredRangeSlabPtrBlock *)InternalAlloc(
        sizeof(RetiredRangeSlabPtrBlock), nullptr, 0);
if (retired) {
    // ... enqueue into retired list ...
}
// if retired == nullptr: old range_slabs_ is silently abandoned
range_slabs_ = new_slabs;
```

If `InternalAlloc` fails for the `RetiredRangeSlabPtrBlock`, the old
`range_slabs_` array is neither freed nor tracked. It becomes a
permanent memory leak that `Destroy()` cannot reclaim.

**Practical risk**: Very low — triggered only under extreme memory
pressure, and the leaked amount is a small pointer array. From a safety
perspective, not freeing the old array is actually the "safe side"
behavior (prevents potential UAF by concurrent readers).

**Category**: code bug (minor leak on allocation failure)

**Recommendation**: Consider returning `false` from
`EnsureRangeSlabPtrCapacity` when the retired block allocation fails,
rather than proceeding with the pointer replacement.

---

## 2. Review Context

| Item                 | Detail                                              |
|----------------------|-----------------------------------------------------|
| Review mode          | Module/runtime audit (Mode 2)                       |
| Scope reviewed       | All `.cpp`, `.h`, `.inc` in `ohos_memgraph/`, plus `CMakeLists.txt` and `README.md` |
| Commits reviewed     | Up to `0f3da9d42321` (3 new commits on top of prior baseline) |
| Validation           | Static only — no build or test execution            |
| Not validated        | Public header `include/sanitizer/memgraph_interface.h`; `sanitizer_common` internals; device-side behavior |

---

## 3. Coverage Gaps and Open Questions

### Missing or unverified tests

| Scenario                                                     | Status            |
|--------------------------------------------------------------|-------------------|
| `LockContaining` under high page bucket collision            | No dedicated test |
| `NameTable` concurrent `Intern` + `Resolve` under rehash    | Lock coverage looks complete; no stress test |
| `realloc(ptr, 0)` precise tracking semantics                 | Not verified      |
| `calloc(0, 0)` returning non-NULL                            | No dedicated test |
| `GetLiveAllocs` under concurrent free                        | No dedicated test |
| `store_record` racing with `free` on the same base           | Likely covered indirectly by `memgraph_thread_stress`; no targeted test |
| Oversized allocation (`size > 2^32`) with `ClampSize`        | No dedicated test |
| `Initialize()` failure and retry via the new rollback path   | No dedicated test |
| `EnsureRangeSlabPtrCapacity` retired block allocation failure | No dedicated test |

### Assumptions made

1. `StaticSpinMutex::Lock()`/`Unlock()` on AArch64 provides
   acquire/release semantics.
2. Frontend-provided `type_name`/`var_name` string pointers remain valid
   for the process lifetime.
3. `DlsymAlloc` behavior matches standard `sanitizer_common`
   implementation.

### Environment limitations

- No access to the full public header
  `include/sanitizer/memgraph_interface.h`.
- No build or test execution.
- No device-side validation.

---

## 4. Recommendation

**Static Review Only — Overall Risk: Low**

### Summary

Code quality is high. The architecture — three-table separation
(`AllocTable` / `StoreTable` / `NameTable`), slab-backed storage,
drop-new capacity model, per-allocation `store_mu` — is sound and
well-implemented.

The three new commits directly address the two most important findings
from the prior audit round:

- **Init flag atomicity** (F2): fully resolved with proper
  acquire/release publication pattern.
- **Slab pointer table growth safety** (F9): `node_slabs_` and store
  `slabs_` are now pre-allocated and never replaced; `range_slabs_`
  uses a retired list to prevent UAF.
- **Init failure rollback** (new): `Initialize()` now properly destroys
  partially constructed state and allows retry.

### Prioritized next actions

1. **Medium**: Document the lock ordering constraint for
   `RemoveRangeForEntry` — callers must not hold any `store_mu` (F1).
2. **Medium**: Add overflow guard in `NameTable::Intern` for
   `id_size_ > UINT32_MAX` (F4).
3. **Low**: Document string pointer lifetime requirements in the public
   header (F5).
4. **Low**: Document `GetLiveAllocs` as best-effort traversal (F8).
5. **Low**: Handle `EnsureRangeSlabPtrCapacity` retired block allocation
   failure more explicitly (F10).
