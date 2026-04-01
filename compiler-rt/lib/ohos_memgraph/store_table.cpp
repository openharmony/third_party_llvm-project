//===-- store_table.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Store-node pool implementation for per-allocation member metadata chains.
//===----------------------------------------------------------------------===//

#include "store_table.h"

#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_libc.h"

namespace __ohos_memgraph {

using namespace __sanitizer;

namespace {

// Compute how many slab-pointer slots are needed to cover a bounded maximum
// number of rows. StoreTable uses this to preallocate its top-level slab table.
static uptr CeilDiv(uptr value, uptr divisor) {
  return value == 0 ? 0 : ((value - 1) / divisor) + 1;
}

}  // namespace

StoreTable::StoreTable()
    : slabs_(nullptr), slab_ptr_cap_(0), slab_count_(0), capacity_(0),
      max_capacity_(0), next_slot_(0), free_head_(-1) {
  pool_mu_.Init();
  atomic_store(&live_count_, 0, memory_order_relaxed);
}

// Grow the top-level slab pointer array without moving already allocated row
// slabs. This preserves stable slot ids while letting the table expand
// incrementally.
bool StoreTable::EnsureSlabPtrCapacity(uptr need_count) {
  return need_count <= slab_ptr_cap_;
}

// Materialize the slab that contains `idx` and preinitialize every row as a
// free-list element.
bool StoreTable::EnsureSlabForIndex(uptr idx) {
  const uptr slab_id = idx / kRowsPerSlab;
  if (!EnsureSlabPtrCapacity(slab_id + 1))
    return false;
  if (slabs_[slab_id])
    return true;

  StoreRow *slab =
      (StoreRow *)InternalAlloc(kRowsPerSlab * sizeof(StoreRow), nullptr, 0);
  if (!slab)
    return false;
  MemStatsOnStoreTableAlloc(kRowsPerSlab * sizeof(StoreRow));
  internal_memset(slab, 0, kRowsPerSlab * sizeof(StoreRow));
  for (uptr i = 0; i < kRowsPerSlab; ++i) {
    slab[i].owner_alloc_id = -1;
    slab[i].next_or_free = -1;
  }

  slabs_[slab_id] = slab;
  if (slab_id + 1 > slab_count_)
    slab_count_ = slab_id + 1;
  return true;
}

// Capacity grows one row slab at a time until the explicit maximum is reached.
bool StoreTable::GrowOneSlab() {
  if (capacity_ >= max_capacity_)
    return false;
  if (!EnsureSlabForIndex(capacity_))
    return false;
  capacity_ += Min<uptr>(kRowsPerSlab, max_capacity_ - capacity_);
  return true;
}

// Convert a stable store slot id back into the backing row pointer.
StoreRow *StoreTable::GetRow(s32 slot) {
  if (slot < 0)
    return nullptr;
  const uptr uslot = static_cast<uptr>(slot);
  if (uslot >= capacity_)
    return nullptr;
  const uptr slab_id = uslot / kRowsPerSlab;
  const uptr offset = uslot % kRowsPerSlab;
  if (!slabs_ || slab_id >= slab_ptr_cap_ || !slabs_[slab_id])
    return nullptr;
  return &slabs_[slab_id][offset];
}

const StoreRow *StoreTable::GetRow(s32 slot) const {
  if (slot < 0)
    return nullptr;
  const uptr uslot = static_cast<uptr>(slot);
  if (uslot >= capacity_)
    return nullptr;
  const uptr slab_id = uslot / kRowsPerSlab;
  const uptr offset = uslot % kRowsPerSlab;
  if (!slabs_ || slab_id >= slab_ptr_cap_ || !slabs_[slab_id])
    return nullptr;
  return &slabs_[slab_id][offset];
}

// Export one live row into the public StoreEntry shape used by callers.
bool StoreTable::FillEntry(s32 slot, const StoreRow &row, StoreEntry *out) const {
  if (!out || row.owner_alloc_id < 0)
    return false;
  out->source_addr = row.source_addr;
  out->store_pc = row.store_pc;
  out->dst_offset = row.dst_offset;
  out->type_id = row.type_id;
  out->var_id = row.var_id;
  out->owner_alloc_id = row.owner_alloc_id;
  out->id = slot;
  return true;
}

// Init only records the maximum live capacity. Physical slabs are allocated on
// demand as rows are consumed.
bool StoreTable::Init(uptr capacity) {
  Destroy();
  max_capacity_ = capacity == 0 ? 1 : capacity;
  slab_ptr_cap_ = CeilDiv(max_capacity_, kRowsPerSlab);
  if (slab_ptr_cap_ == 0)
    slab_ptr_cap_ = 1;

  slabs_ =
      (StoreRow **)InternalAlloc(slab_ptr_cap_ * sizeof(StoreRow *), nullptr, 0);
  if (!slabs_) {
    Destroy();
    return false;
  }
  MemStatsOnStoreTableAlloc(slab_ptr_cap_ * sizeof(StoreRow *));
  internal_memset(slabs_, 0, slab_ptr_cap_ * sizeof(StoreRow *));
  return true;
}

// Destroy releases all slabs and resets reuse state so the table can be
// reinitialized.
void StoreTable::Destroy() {
  if (slabs_) {
    for (uptr i = 0; i < slab_ptr_cap_; ++i) {
      if (slabs_[i]) {
        MemStatsOnStoreTableFree(kRowsPerSlab * sizeof(StoreRow));
        InternalFree(slabs_[i]);
      }
    }
    MemStatsOnStoreTableFree(slab_ptr_cap_ * sizeof(StoreRow *));
    InternalFree(slabs_);
  }

  slabs_ = nullptr;
  slab_ptr_cap_ = 0;
  slab_count_ = 0;
  capacity_ = 0;
  max_capacity_ = 0;
  next_slot_ = 0;
  free_head_ = -1;
  atomic_store(&live_count_, 0, memory_order_relaxed);
}

// AcquireSlot returns either:
// - a recycled free-list slot
// - or a brand new slot from the current slab-backed capacity
//
// If the explicit max capacity has been exhausted, the formal runtime returns
// failure and the caller applies drop-new semantics.
s32 StoreTable::AcquireSlot() {
  SpinMutexLock lock(&pool_mu_);
  if (free_head_ >= 0) {
    const s32 slot = free_head_;
    StoreRow *row = GetRow(slot);
    if (!row)
      return -1;
    free_head_ = row->next_or_free;
    row->next_or_free = -1;
    return slot;
  }

  if (next_slot_ >= capacity_ && !GrowOneSlab())
    return -1;

  const s32 slot = static_cast<s32>(next_slot_);
  ++next_slot_;
  return slot;
}

// ReleaseSlotLocked resets one row and pushes it back to the store free list.
void StoreTable::ReleaseSlotLocked(s32 slot) {
  StoreRow *row = GetRow(slot);
  if (!row)
    return;
  row->source_addr = 0;
  row->store_pc = 0;
  row->dst_offset = 0;
  row->type_id = 0;
  row->var_id = 0;
  row->owner_alloc_id = -1;
  row->next_or_free = free_head_;
  free_head_ = slot;
}

// Find scans only the owner-local chain that begins at `head`. This is the
// intentional tradeoff in the formal runtime: no global store hash table, but
// cheaper per-owner cleanup and simpler bounded-memory behavior.
bool StoreTable::Find(s32 head, u32 dst_offset, StoreEntry *out) const {
  s32 slot = head;
  while (slot >= 0) {
    const StoreRow *row = GetRow(slot);
    if (!row)
      return false;
    if (row->owner_alloc_id >= 0 && row->dst_offset == dst_offset)
      return FillEntry(slot, *row, out);
    slot = row->next_or_free;
  }
  return false;
}

// Always allocate a new history node and push it to the head of the owner
// chain.
//
// "Upsert" here does not overwrite an old row. Instead it:
// - preserves history for the same owner + offset
// - inserts newest-first at the chain head
// The caller then decides whether to publish that new head back into the alloc
// row.
s32 StoreTable::UpsertRecord(s32 owner_alloc_id, s32 head, u32 dst_offset,
                             u32 type_id, u32 var_id, uptr source_addr,
                             uptr store_pc) {
  const s32 slot = AcquireSlot();
  if (slot < 0)
    return -1;

  StoreRow *row = GetRow(slot);
  if (!row) {
    SpinMutexLock lock(&pool_mu_);
    ReleaseSlotLocked(slot);
    return -1;
  }

  row->source_addr = source_addr;
  row->store_pc = store_pc;
  row->dst_offset = dst_offset;
  row->type_id = type_id;
  row->var_id = var_id;
  row->owner_alloc_id = owner_alloc_id;
  row->next_or_free = head;
  atomic_fetch_add(&live_count_, 1, memory_order_relaxed);
  return slot;
}

// free(base) already knows which owner chain to clear, so RemoveAllForAlloc()
// only needs the chain head. Rows are released back to the shared pool in one
// pass.
void StoreTable::RemoveAllForAlloc(s32 head) {
  if (head < 0)
    return;

  SpinMutexLock lock(&pool_mu_);
  s32 slot = head;
  while (slot >= 0) {
    StoreRow *row = GetRow(slot);
    if (!row)
      break;
    const s32 next = row->next_or_free;
    ReleaseSlotLocked(slot);
    slot = next;
    atomic_fetch_sub(&live_count_, 1, memory_order_relaxed);
  }
}

// CountRecords and the GetInfo* helpers are diagnostics-oriented scans over one
// owner chain. They are intentionally local and do not require any global store
// structure.
uptr StoreTable::CountRecords(s32 head) const {
  uptr count = 0;
  s32 slot = head;
  while (slot >= 0) {
    const StoreRow *row = GetRow(slot);
    if (!row)
      break;
    ++count;
    slot = row->next_or_free;
  }
  return count;
}

uptr StoreTable::GetInfoRecordIds(s32 head, StoreInfoRecordIds *out,
                                  uptr capacity) const {
  uptr count = 0;
  s32 slot = head;
  while (slot >= 0) {
    const StoreRow *row = GetRow(slot);
    if (!row)
      break;
    if (out && count < capacity) {
      out[count].type_id = row->type_id;
      out[count].var_id = row->var_id;
    }
    ++count;
    slot = row->next_or_free;
  }
  return count;
}

uptr StoreTable::GetInfoRecords(s32 head, const NameTable *type_table,
                                const NameTable *var_table, info_record_t *out,
                                uptr capacity) const {
  uptr count = 0;
  s32 slot = head;
  while (slot >= 0) {
    const StoreRow *row = GetRow(slot);
    if (!row)
      break;
    if (out && count < capacity) {
      out[count].type_name =
          type_table ? type_table->Resolve(row->type_id) : nullptr;
      out[count].name = var_table ? var_table->Resolve(row->var_id) : nullptr;
    }
    ++count;
    slot = row->next_or_free;
  }
  return count;
}

uptr StoreTable::LiveCount() const {
  return static_cast<uptr>(atomic_load(&live_count_, memory_order_relaxed));
}

}  // namespace __ohos_memgraph
