//===-- alloc_table.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exact alloc index backed by a hashed base lookup and a lightweight
// page-based range index. The hash table serves exact base queries while the
// page index answers "which allocation contains this address?" for
// store_record().
//===----------------------------------------------------------------------===//

#ifndef OHOS_MEMGRAPH_ALLOC_TABLE_H
#define OHOS_MEMGRAPH_ALLOC_TABLE_H

#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_mutex.h"

namespace __ohos_memgraph {

using __sanitizer::s32;
using __sanitizer::u32;
using __sanitizer::u8;
using __sanitizer::uptr;

// AllocEntry is the public view of one allocation record.
//
// It omits all internal index-chain, lock, and free-list fields and keeps only
// the semantic allocation data that callers actually need for:
// - returning query results
// - passing removed allocation data back to upper layers
// - diagnostics and tests
struct AllocEntry {
  uptr base;
  uptr malloc_pc;
  u32 size;
  u32 type_id;
  u32 var_id;
  s32 store_head;
  s32 id;
};

// LockedAlloc is the hand-off object for an allocation whose local store chain
// has already been locked.
//
// Some paths need more than just finding the allocation. They also need to
// ensure for a short period of time that:
// - the allocation is still live
// - the allocation's `store_head` cannot be changed concurrently
// - the store chain under that allocation can be traversed or modified safely
//
// Those paths return LockedAlloc. Until Unlock() is called, callers can treat
// it as a pinned view of the allocation.
struct LockedAlloc {
  uptr base;
  uptr malloc_pc;
  u32 size;
  u32 type_id;
  u32 var_id;
  s32 store_head;
  s32 id;
  bool locked;
};

class AllocTable {
public:
  // Construct an empty table. Real memory allocation and bucket/slab
  // initialization are deferred to Init().
  AllocTable();

  // Initialize the table for the given maximum live-allocation capacity.
  //
  // The capacity semantics are:
  // - `max_capacity_` is the maximum number of allocations allowed to be live
  //   at the same time
  // - internal slabs grow incrementally on demand instead of being fully
  //   allocated up front
  bool Init(uptr initial_capacity);

  // Release every internal storage layer used by this table, including:
  // - alloc-node slabs
  // - page-range-index slabs
  // - hash/page bucket arrays
  // - bucket lock arrays
  void Destroy();

  // Insert a new allocation.
  //
  // This publishes to both:
  // - the exact hash: `base -> alloc`
  // - the page-range index: `page -> candidate alloc`
  //
  // On success, return a stable id.
  bool Insert(uptr base, u32 size, uptr malloc_pc, s32 *out_id);

  // Look up an allocation exactly by base and return it as AllocEntry.
  bool Find(uptr base, AllocEntry *out) const;

  // Find which allocation contains the given address.
  //
  // This is the core owner-resolution entry point used by
  // `store_record(dst_ptr, ...)`.
  bool FindContaining(uptr addr, AllocEntry *out) const;

  // Find only the stable allocation id; cheaper than a full Find().
  bool FindId(uptr base, s32 *out_id) const;

  // With a stable id already known, fetch the allocation contents directly.
  bool GetById(s32 id, AllocEntry *out) const;

  // Find an allocation by base and hold its local store lock before returning.
  bool LockByBase(uptr base, LockedAlloc *out) const;

  // Find an allocation by containing-address lookup and hold its local store
  // lock before returning.
  bool LockContaining(uptr addr, LockedAlloc *out) const;

  // Release the allocation-local lock held by LockByBase()/LockContaining().
  void Unlock(LockedAlloc *locked) const;

  // Update block metadata and the final selected malloc_pc while the allocation
  // is already locked.
  bool UpdateLockedMeta(LockedAlloc *locked, u32 type_id, u32 var_id,
                        uptr malloc_pc);

  // Update size while the allocation is already locked.
  // Currently this is mainly used for same-address realloc resize.
  bool UpdateLockedSize(LockedAlloc *locked, u32 size);

  // Update the store-chain head while the allocation is already locked.
  bool SetLockedStoreHead(LockedAlloc *locked, s32 store_head);

  // Removal step 1:
  // - detach the allocation from the exact hash
  // - mark the node as deleting
  // - copy the removed allocation contents out to the caller
  bool BeginRemove(uptr base, AllocEntry *removed);

  // Removal step 2:
  // - remove all page-index links covered by the allocation
  void RemoveRangeForEntry(const AllocEntry &entry);

  // Removal step 3:
  // - clear the node payload
  // - return the node to the alloc free list
  bool FinalizeRemove(s32 id);

  // Convenience removal wrapper that internally performs:
  // BeginRemove -> RemoveRangeForEntry -> FinalizeRemove.
  bool Remove(uptr base, AllocEntry *removed = nullptr);

  // Return the current number of live allocations.
  uptr Size() const;
  // Return the currently materialized alloc-slot capacity.
  uptr Capacity() const { return capacity_; }
  // Return the maximum allowed live-allocation capacity.
  uptr MaxCapacity() const { return max_capacity_; }
  // Return whether the current live-allocation count has reached the limit.
  bool IsAtLiveCapacity() const { return Size() >= max_capacity_; }
  // Return the number of exact-hash buckets.
  uptr BucketCount() const { return hash_bucket_count_; }
  // Return the number of alloc-node slabs.
  uptr SlabCount() const { return node_slab_count_; }
  // Return the number of page-range-index buckets.
  uptr BucketPageCount() const { return page_bucket_count_; }
  // Return the size of one internal alloc-node entry for layout/monitoring
  // queries.
  uptr StorageEntrySize() const;
  // Return the size of one bucket-head slot.
  uptr BucketEntrySize() const { return sizeof(s32); }

private:
  // alloc node states:
  // - free: available for reuse
  // - live: published into indexes and queryable
  // - deleting: removed from the exact index and currently in the staged
  //   removal flow
  enum NodeState : u8 {
    kNodeFree = 0,
    kNodeLive = 1,
    kNodeDeleting = 2,
  };

  // One internal row stored by the alloc table.
  //
  // In addition to semantic allocation fields, it also carries:
  // - a field reused by the hash chain / free list
  // - node state
  // - the per-allocation store lock
  struct Node {
    uptr base;
    uptr malloc_pc;
    u32 size;
    u32 type_id;
    u32 var_id;
    s32 store_head;
    s32 hash_next_or_free;
    u8 state;
    __sanitizer::StaticSpinMutex store_mu;
    u8 pad[2];
  };

  struct RangeLink {
    uptr page_id;
    s32 alloc_id;
    s32 next_or_free;
  };

  struct RetiredRangeSlabPtrBlock {
    RangeLink **slabs;
    uptr cap;
    RetiredRangeSlabPtrBlock *next;
  };

  static constexpr uptr kNodesPerSlab = 4096;
  static constexpr uptr kRangeLinksPerSlab = 4096;
  static constexpr uptr kPageShift = 9;

  uptr Hash(uptr value) const;
  uptr BaseBucket(uptr base) const;
  uptr PageBucket(uptr page_id) const;
  uptr PageIdForAddr(uptr addr) const;
  uptr LastPageIdForRange(uptr base, u32 size) const;

  bool EnsureNodeSlabPtrCapacity(uptr need_count);
  bool EnsureNodeSlabForIndex(uptr idx);
  bool GrowNodeSlabs();
  Node *GetNode(s32 idx);
  const Node *GetNode(s32 idx) const;
  s32 AcquireNode();
  void ReleaseNode(s32 id);

  bool EnsureRangeSlabPtrCapacity(uptr need_count);
  bool EnsureRangeSlabForIndex(uptr idx);
  bool GrowRangeLinkSlabs();
  RangeLink *GetRangeLink(s32 idx);
  const RangeLink *GetRangeLink(s32 idx) const;
  s32 AcquireRangeLink();
  void ReleaseRangeLink(s32 id);

  s32 FindIdInHashBucketLocked(uptr bucket, uptr base) const;
  bool FillEntryLocked(s32 id, const Node &node, AllocEntry *out) const;
  bool LockNodeIfLive(s32 id, LockedAlloc *out) const;
  bool InsertRangeLinksForNode(s32 alloc_id, uptr base, u32 size);
  void RollbackInsertedRangeLinks(s32 alloc_id, uptr base, u32 size,
                                  uptr stop_before_page);
  void RemoveRangeLinksForNode(s32 alloc_id, uptr base, u32 size);

  Node **node_slabs_;
  uptr node_slab_ptr_cap_;
  uptr node_slab_count_;
  uptr capacity_;
  uptr max_capacity_;
  uptr next_node_;
  s32 free_head_;

  RangeLink **range_slabs_;
  uptr range_slab_ptr_cap_;
  uptr range_slab_count_;
  uptr range_capacity_;
  uptr next_range_link_;
  s32 range_free_head_;
  RetiredRangeSlabPtrBlock *retired_range_slab_ptrs_;

  s32 *hash_buckets_;
  __sanitizer::StaticSpinMutex *hash_mutexes_;
  uptr hash_bucket_count_;

  s32 *page_buckets_;
  __sanitizer::StaticSpinMutex *page_mutexes_;
  uptr page_bucket_count_;

  mutable __sanitizer::StaticSpinMutex pool_mu_;
  __sanitizer::atomic_uint64_t live_count_;
};

}  // namespace __ohos_memgraph

#endif  // OHOS_MEMGRAPH_ALLOC_TABLE_H
