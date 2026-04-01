//===-- memgraph.cpp --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Main implementation file for the OHOS memgraph runtime.
//===----------------------------------------------------------------------===//

#include "memgraph.h"

#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_mutex.h"
#include "sanitizer_common/sanitizer_placement_new.h"

namespace __ohos_memgraph {

using namespace __sanitizer;

atomic_uint8_t ohos_memgraph_inited = {};
atomic_uint8_t ohos_memgraph_init_is_running = {};
THREADLOCAL int ohos_memgraph_disable_interceptors = 0;

AllocTable *alloc_table;
StoreTable *store_table;
NameTable *type_table;
NameTable *var_table;
StaticSpinMutex alloc_mu;
StaticSpinMutex store_mu;
StaticSpinMutex graph_mu;

static StaticSpinMutex init_mu;

namespace {

// Runtime-owned objects are allocated from the internal allocator so the
// memgraph runtime never depends on external malloc during its own bootstrap.
template <class T> T *AllocateInternalObject() {
  void *mem = InternalAlloc(sizeof(T), nullptr, 0);
  if (mem)
    MemStatsOnMiscAlloc(sizeof(T));
  return mem ? ::new (mem) T() : nullptr;
}

template <class T> void DestroyInternalObject(T *&obj) {
  if (!obj)
    return;
  obj->Destroy();
  obj->~T();
  MemStatsOnMiscFree(sizeof(T));
  InternalFree(obj);
  obj = nullptr;
}

class ScopedDisableInterceptors {
public:
  ScopedDisableInterceptors() { ++ohos_memgraph_disable_interceptors; }
  ~ScopedDisableInterceptors() { --ohos_memgraph_disable_interceptors; }
};

// Sizes are stored in u32 in the alloc row layout. Oversized external sizes are
// clamped so the runtime stays structurally consistent even if callers request
// something larger than the internal representation can encode.
u32 ClampSize(uptr size) {
  const uptr max_u32 = static_cast<uptr>(~static_cast<u32>(0));
  return static_cast<u32>(size > max_u32 ? max_u32 : size);
}

int ClampPositiveEnvToInt(const char *name, int fallback) {
  const char *value = GetEnv(name);
  if (!value || value[0] == '\0')
    return fallback;
  const s64 parsed = internal_atoll(value);
  return parsed > 0 && parsed <= 0x7fffffffLL ? static_cast<int>(parsed)
                                              : fallback;
}

bool ClampBoolEnv(const char *name, bool fallback) {
  const char *value = GetEnv(name);
  if (!value || value[0] == '\0')
    return fallback;
  if (!internal_strcmp(value, "1") || !internal_strcmp(value, "true") ||
      !internal_strcmp(value, "on") || !internal_strcmp(value, "yes"))
    return true;
  if (!internal_strcmp(value, "0") || !internal_strcmp(value, "false") ||
      !internal_strcmp(value, "off") || !internal_strcmp(value, "no"))
    return false;
  return fallback;
}

bool HasFrontendMeta(const char *value) { return value && value[0] != '\0'; }

// Memgraph only needs common sanitizer flags to be initialized so platform
// defaults take effect. We explicitly keep proc-maps decoration enabled to
// make runtime-owned anonymous mappings show readable names on OHOS.
void InitializeCommonRuntimeFlags() {
  SetCommonFlagsDefaults();

  CommonFlags cf;
  cf.CopyFrom(*common_flags());
  cf.decorate_proc_maps = true;
  OverrideCommonFlags(cf);

  InitializeCommonFlags();
}

// Live counters are recomputed from the authoritative tables after structural
// mutations complete. Observability is intentionally fed from the functional
// structures rather than maintaining an independent source of truth.
void RefreshLiveCountersLocked() {
  if (!ObservabilityEnabled())
    return;
  const u64 alloc_live =
      alloc_table ? static_cast<u64>(alloc_table->Size()) : 0;
  const u64 store_live =
      store_table ? static_cast<u64>(store_table->LiveCount()) : 0;
  MemStatsUpdateLiveCounters(alloc_live, store_live);
}

bool FindMemberStoreLocked(const LockedAlloc &alloc, uptr member_addr,
                           uptr *offset_out, StoreEntry *store_out) {
  if (member_addr < alloc.base || !store_out)
    return false;
  const uptr offset = member_addr - alloc.base;
  if (offset >= static_cast<uptr>(alloc.size) ||
      offset > static_cast<uptr>(~static_cast<u32>(0))) {
    return false;
  }
  if (!store_table->Find(alloc.store_head, static_cast<u32>(offset), store_out))
    return false;
  if (offset_out)
    *offset_out = offset;
  return true;
}

}  // namespace

bool RuntimeInited() {
  return atomic_load(&ohos_memgraph_inited, memory_order_acquire) != 0;
}

bool RuntimeInitIsRunning() {
  return atomic_load(&ohos_memgraph_init_is_running, memory_order_acquire) != 0;
}

namespace {

void SetRuntimeInitIsRunning(bool value) {
  atomic_store(&ohos_memgraph_init_is_running, value ? 1 : 0,
               memory_order_release);
}

void PublishRuntimeInited() {
  atomic_store(&ohos_memgraph_inited, 1, memory_order_release);
}

}  // namespace

void Flags::SetDefaults() {
#define OHOS_MEMGRAPH_FLAG(Type, Name, Default, Description) Name = Default;
#include "memgraph_flags.inc"
#undef OHOS_MEMGRAPH_FLAG

  alloc_table_size = ClampPositiveEnvToInt("OHOS_MEMGRAPH_ALLOC_TABLE_SIZE",
                                           alloc_table_size);
  store_table_size = ClampPositiveEnvToInt("OHOS_MEMGRAPH_STORE_TABLE_SIZE",
                                           store_table_size);
  observability_enabled = ClampBoolEnv("OHOS_MEMGRAPH_OBSERVABILITY_ENABLED",
                                       observability_enabled);
}

static Flags memgraph_flags;

Flags *flags() { return &memgraph_flags; }
bool HooksEnabled() { return flags()->enabled; }
bool ObservabilityEnabled() { return flags()->observability_enabled; }

// Initialize builds the runtime in a fixed order:
// 1. read flags and environment overrides
// 2. allocate runtime-owned tables
// 3. initialize capacity-bounded alloc/store storage
// 4. initialize string tables
// 5. install interceptors
void Initialize() {
  SpinMutexLock lock(&init_mu);
  if (RuntimeInited() || RuntimeInitIsRunning())
    return;
  SetRuntimeInitIsRunning(true);

  InitializeCommonRuntimeFlags();
  flags()->SetDefaults();

  alloc_table = AllocateInternalObject<AllocTable>();
  store_table = AllocateInternalObject<StoreTable>();
  type_table = AllocateInternalObject<NameTable>();
  var_table = AllocateInternalObject<NameTable>();
  if (!alloc_table || !store_table || !type_table || !var_table) {
    DestroyInternalObject(alloc_table);
    DestroyInternalObject(store_table);
    DestroyInternalObject(type_table);
    DestroyInternalObject(var_table);
    SetRuntimeInitIsRunning(false);
    return;
  }

  const uptr alloc_cap =
      flags()->alloc_table_size > 0 ? (uptr)flags()->alloc_table_size : 65536;
  const uptr store_cap =
      flags()->store_table_size > 0 ? (uptr)flags()->store_table_size : 2000000;

  if (!alloc_table->Init(alloc_cap) || !store_table->Init(store_cap) ||
      !type_table->Init(kTypeNameTable, 1024) ||
      !var_table->Init(kVarNameTable, 4096)) {
    DestroyInternalObject(alloc_table);
    DestroyInternalObject(store_table);
    DestroyInternalObject(type_table);
    DestroyInternalObject(var_table);
    SetRuntimeInitIsRunning(false);
    return;
  }
  RefreshLiveCountersLocked();
  if (ObservabilityEnabled())
    Atexit(MemStatsLogSummary);

  InitializeInterceptors();
  SetRuntimeInitIsRunning(false);
  PublishRuntimeInited();
}

// Handle one new allocation event.
//
// If the underlying allocator reuses an old address, treat the old allocation
// as freed before inserting the new row so the runtime never keeps more than
// one live allocation for the same base.
void TrackHookAlloc(uptr base, uptr size, uptr malloc_pc) {
  if (!base)
    return;
  if (!RuntimeInited())
    Initialize();
  if (!RuntimeInited())
    return;
  if (!HooksEnabled())
    return;

  ScopedDisableInterceptors scope;

  s32 old_id = -1;
  // Reused addresses are treated as a free followed by a new allocation so the
  // runtime never keeps two live rows for the same base.
  if (alloc_table->FindId(base, &old_id))
    TrackHookFree(base);

  s32 new_id = -1;
  if (!alloc_table->Insert(base, ClampSize(size), malloc_pc, &new_id))
    return;

  RefreshLiveCountersLocked();
}

// TrackHookFree removes the alloc from both alloc indexes, clears the full
// owner-local store chain, and finally returns the alloc slot to the free list.
void TrackHookFree(uptr base) {
  if (!base || !RuntimeInited() || !HooksEnabled())
    return;

  ScopedDisableInterceptors scope;

  AllocEntry removed = {};
  if (!alloc_table->BeginRemove(base, &removed))
    return;
  alloc_table->RemoveRangeForEntry(removed);
  if (removed.store_head >= 0)
    store_table->RemoveAllForAlloc(removed.store_head);
  alloc_table->FinalizeRemove(removed.id);
  RefreshLiveCountersLocked();
}

// Normalize realloc into two runtime-level cases:
// - same-address resize, preserving existing block metadata when possible
// - or free(old) + alloc(new)
void TrackHookRealloc(uptr old_base, uptr new_base, uptr new_size,
                      uptr malloc_pc) {
  if (!RuntimeInited())
    Initialize();
  if (!RuntimeInited())
    return;
  if (!HooksEnabled())
    return;

  if (old_base && new_base && old_base == new_base) {
    AllocEntry old = {};
    const bool had_old = alloc_table->Find(old_base, &old);
    TrackHookFree(old_base);
    TrackHookAlloc(new_base, new_size, malloc_pc);
    if (had_old && (old.type_id || old.var_id)) {
      LockedAlloc alloc = {};
      if (alloc_table->LockByBase(new_base, &alloc)) {
        alloc_table->UpdateLockedMeta(&alloc, old.type_id, old.var_id,
                                      alloc.malloc_pc);
        alloc_table->Unlock(&alloc);
      }
    }
    return;
  }

  if (old_base)
    TrackHookFree(old_base);
  if (new_base)
    TrackHookAlloc(new_base, new_size, malloc_pc);
}

// Write block-level metadata.
//
// This path updates only the alloc row itself. It does not touch:
// - the page range index
// - the store-node pool
void RecordMallocMetadata(uptr base, const char *type_name,
                          const char *var_name, uptr alloc_pc) {
  if (!base)
    return;
  if (!RuntimeInited())
    Initialize();
  if (!RuntimeInited())
    return;
  if (!HooksEnabled())
    return;

  const u32 type_id =
      HasFrontendMeta(type_name) ? type_table->Intern(type_name) : 0;
  const u32 var_id =
      HasFrontendMeta(var_name) ? var_table->Intern(var_name) : 0;
  if (type_id == 0 && var_id == 0)
    return;

  if (ObservabilityEnabled())
    MemStatsOnMallocRecordCall();
  ScopedDisableInterceptors scope;

  LockedAlloc alloc = {};
  if (!alloc_table->LockByBase(base, &alloc))
    return;
  const uptr final_malloc_pc = alloc_pc ? alloc_pc : alloc.malloc_pc;
  if (alloc.type_id == type_id && alloc.var_id == var_id &&
      alloc.malloc_pc == final_malloc_pc) {
    alloc_table->Unlock(&alloc);
    return;
  }
  alloc_table->UpdateLockedMeta(&alloc, type_id, var_id, final_malloc_pc);
  alloc_table->Unlock(&alloc);
}

// Write member-level metadata.
//
// This path:
// 1. resolves the owner allocation from the field address `dst_ptr`
// 2. computes the field offset
// 3. inserts a newest-first history node at the head of the owner-local store
//    chain
void RecordStoreMetadata(uptr source_addr, uptr dst_ptr,
                         const char *type_name, const char *var_name,
                         uptr store_pc) {
  if (!dst_ptr)
    return;
  if (!RuntimeInited())
    Initialize();
  if (!RuntimeInited())
    return;
  if (!HooksEnabled())
    return;

  const bool has_type = HasFrontendMeta(type_name);
  const bool has_var = HasFrontendMeta(var_name);
  if (!has_type && !has_var)
    return;

  const u32 type_id = has_type ? type_table->Intern(type_name) : 0;
  const u32 var_id = has_var ? var_table->Intern(var_name) : 0;
  if (type_id == 0 && var_id == 0)
    return;

  if (ObservabilityEnabled())
    MemStatsOnStoreRecordCall();
  ScopedDisableInterceptors scope;

  LockedAlloc owner = {};
  // store_record() receives a field address rather than the owner base. The
  // page range index maps the field address back to its containing allocation.
  if (!alloc_table->LockContaining(dst_ptr, &owner))
    return;

  const uptr offset = dst_ptr - owner.base;
  if (offset >= static_cast<uptr>(owner.size) ||
      offset > static_cast<uptr>(~static_cast<u32>(0))) {
    alloc_table->Unlock(&owner);
    return;
  }

  const s32 new_head =
      store_table->UpsertRecord(owner.id, owner.store_head,
                                static_cast<u32>(offset), type_id, var_id,
                                source_addr, store_pc);
  if (new_head >= 0) {
    alloc_table->SetLockedStoreHead(&owner, new_head);
    RefreshLiveCountersLocked();
  }
  alloc_table->Unlock(&owner);
}

// Block queries only need an exact alloc lookup and string resolution.
bool GetBlockInfo(uptr base, block_info_t *out) {
  if (!out)
    return false;
  internal_memset(out, 0, sizeof(*out));
  if (!RuntimeInited() || base == 0)
    return false;

  AllocEntry alloc = {};
  if (!alloc_table->Find(base, &alloc))
    return false;

  out->base = (unsigned long)alloc.base;
  out->size = (unsigned long)alloc.size;
  out->type_name = type_table->Resolve(alloc.type_id);
  out->name = var_table->Resolve(alloc.var_id);
  out->found = 1;
  out->malloc_pc = (unsigned long)alloc.malloc_pc;
  return true;
}

// Member queries intentionally scan only one owner's store chain. The first
// match is the newest value because store nodes are inserted at the head.
bool GetMemberInfo(uptr addr, member_info_t *out) {
  if (!out)
    return false;
  internal_memset(out, 0, sizeof(*out));
  if (!RuntimeInited() || addr == 0)
    return false;

  LockedAlloc alloc = {};
  if (!alloc_table->LockContaining(addr, &alloc))
    return false;

  uptr offset = 0;
  StoreEntry store = {};
  const bool found = FindMemberStoreLocked(alloc, addr, &offset, &store);
  alloc_table->Unlock(&alloc);
  if (!found)
    return false;

  out->base = (unsigned long)alloc.base;
  out->member_addr = (unsigned long)addr;
  out->offset = (unsigned long)offset;
  out->type_name = type_table->Resolve(store.type_id);
  out->name = var_table->Resolve(store.var_id);
  out->found = 1;
  out->store_pc = (unsigned long)store.store_pc;
  out->source_addr = (unsigned long)store.source_addr;
  return true;
}

bool GetOwner(uptr addr, owner_info_t *out) {
  if (!out)
    return false;
  internal_memset(out, 0, sizeof(*out));
  if (!RuntimeInited() || addr == 0)
    return false;

  AllocEntry alloc = {};
  if (!alloc_table->FindContaining(addr, &alloc))
    return false;

  out->base = (unsigned long)alloc.base;
  out->size = (unsigned long)alloc.size;
  out->found = 1;
  return true;
}

}  // namespace __ohos_memgraph
