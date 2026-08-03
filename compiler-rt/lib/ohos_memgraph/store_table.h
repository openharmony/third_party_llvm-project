//===-- store_table.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Store node pool for per-allocation member metadata chains. Each allocation
// owns a private singly-linked list of store nodes; the pool only manages
// storage and reuse of those nodes.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#ifndef OHOS_MEMGRAPH_STORE_TABLE_H
#define OHOS_MEMGRAPH_STORE_TABLE_H

#include "name_table.h"
#include "memgraph_interface.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_mutex.h"

namespace __ohos_memgraph {

using __sanitizer::s32;
using __sanitizer::u32;
using __sanitizer::uptr;

struct StoreEntry {
  // Source address recorded for this store entry.
  uptr source_addr;
  // Call-site PC recorded for this store entry.
  uptr store_pc;
  // Field offset relative to the owner allocation base.
  u32 dst_offset;
  // Interned type / variable ids in the name tables.
  u32 type_id;
  u32 var_id;
  // Owning allocation id for this record.
  s32 owner_alloc_id;
  // Stable id of this store record itself.
  s32 id;
};

struct StoreInfoRecordIds {
  // Temporary type / variable ids returned on diagnostic paths.
  u32 type_id;
  u32 var_id;
};

struct StoreRow {
  // Internal backing row; never exposed directly to callers.
  uptr source_addr;
  uptr store_pc;
  u32 dst_offset;
  u32 type_id;
  u32 var_id;
  s32 owner_alloc_id;
  // While live this is the next pointer in the owner-local chain; while free it
  // is reused as the next pointer in the free list.
  s32 next_or_free;
};

class StoreTable {
public:
  // Construct an empty table.
  StoreTable();

  // Initialize the maximum live capacity. Physical slabs grow on demand.
  bool Init(uptr capacity);
  // Release all slabs and reset table state.
  void Destroy();

  // Find the newest record with the given offset in one owner's store chain.
  bool Find(s32 head, u32 dst_offset, StoreEntry *out) const;
  // Allocate a new history node and push it to the head of the owner-local
  // chain.
  s32 UpsertRecord(s32 owner_alloc_id, s32 head, u32 dst_offset, u32 type_id,
                   u32 var_id, uptr source_addr, uptr store_pc);
  // Remove the full store chain for one allocation.
  void RemoveAllForAlloc(s32 head);

  // Diagnostic and observability helpers.
  uptr CountRecords(s32 head) const;
  uptr GetInfoRecordIds(s32 head, StoreInfoRecordIds *out, uptr capacity) const;
  uptr GetInfoRecords(s32 head, const NameTable *type_table,
                      const NameTable *var_table,
                      info_record_t *out, uptr capacity) const;

  // Current physical capacity.
  uptr Capacity() const { return capacity_; }
  // Maximum number of live store records allowed.
  uptr MaxCapacity() const { return max_capacity_; }
  // Current number of live store records.
  uptr LiveCount() const;
  // Number of allocated slabs.
  uptr SlabCount() const { return slab_count_; }
  // Physical size of one store row.
  uptr RowSize() const { return sizeof(StoreRow); }

private:
  // Number of rows stored in each slab.
  static constexpr uptr kRowsPerSlab = 16384;

  // Slab/free-list storage helpers.
  bool EnsureSlabPtrCapacity(uptr need_count);
  bool EnsureSlabForIndex(uptr idx);
  bool GrowOneSlab();
  StoreRow *GetRow(s32 slot);
  const StoreRow *GetRow(s32 slot) const;
  bool FillEntry(s32 slot, const StoreRow &row, StoreEntry *out) const;
  s32 AcquireSlot();
  void ReleaseSlotLocked(s32 slot);

  StoreRow **slabs_;
  uptr slab_ptr_cap_;
  uptr slab_count_;
  uptr capacity_;
  uptr max_capacity_;
  uptr next_slot_;
  s32 free_head_;
  __sanitizer::StaticSpinMutex pool_mu_;
  __sanitizer::atomic_uint64_t live_count_;
};

}  // namespace __ohos_memgraph
#endif  // OHOS_MEMGRAPH_STORE_TABLE_H
#endif /* OHOS_LLVM */
