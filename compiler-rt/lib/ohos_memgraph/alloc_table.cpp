//===-- alloc_table.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Alloc table implementation backed by an exact base hash and a page-based
// range index.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#include "alloc_table.h"

#include "memgraph.h"
#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_mutex.h"

namespace __ohos_memgraph {

using namespace __sanitizer;

namespace {

// Hash-bucket counts are rounded to powers of two so bucket selection can use
// a cheap mask instead of a modulo operation on every hot-path lookup.
static uptr NextPow2(uptr value) {
  uptr out = 1;
  while (out < value)
    out <<= 1;
  return out;
}

// Compute how many slab-pointer slots are needed to cover a bounded maximum
// number of entries. This lets Init() size top-level pointer tables once.
static uptr CeilDiv(uptr value, uptr divisor) {
  return value == 0 ? 0 : ((value - 1) / divisor) + 1;
}

}  // namespace

AllocTable::AllocTable()
    : node_slabs_(nullptr), node_slab_ptr_cap_(0), node_slab_count_(0),
      capacity_(0), max_capacity_(0), next_node_(0), free_head_(-1),
      range_slabs_(nullptr), range_slab_ptr_cap_(0), range_slab_count_(0),
      range_capacity_(0), next_range_link_(0), range_free_head_(-1),
      retired_range_slab_ptrs_(nullptr),
      hash_buckets_(nullptr), hash_mutexes_(nullptr), hash_bucket_count_(0),
      page_buckets_(nullptr), page_mutexes_(nullptr), page_bucket_count_(0) {
  pool_mu_.Init();
  atomic_store(&live_count_, 0, memory_order_relaxed);
}

//===----------------------------------------------------------------------===//
// Shared Helpers
//===----------------------------------------------------------------------===//
//
// Shared hash helper used by both:
// - the exact base hash
// - the page-range-index hash
//
// This does not need cryptographic strength. It just needs a stable enough
// bit-mix so neighboring addresses do not keep collapsing into the same small
// set of buckets.
uptr AllocTable::Hash(uptr value) const { return MixUptr(value); }

// Map any address to its logical page id. With `kPageShift = 9`, one logical
// page is 512 B.
uptr AllocTable::PageIdForAddr(uptr addr) const { return addr >> kPageShift; }

// Compute the last logical page covered by `[base, base + size)`.
//
// This uses "last covered byte" semantics:
// - when size == 0, treat the range as covering at least the page containing
//   `base`
// - when size > 0, compute the last page using `base + size - 1`
uptr AllocTable::LastPageIdForRange(uptr base, u32 size) const {
  if (size == 0)
    return PageIdForAddr(base);
  return PageIdForAddr(base + static_cast<uptr>(size - 1));
}

//===----------------------------------------------------------------------===//
// Exact-Hash Helpers (`base -> alloc`)
//===----------------------------------------------------------------------===//

// Compute which exact-hash bucket a base lands in.
uptr AllocTable::BaseBucket(uptr base) const {
  return hash_bucket_count_ ? (Hash(base) & (hash_bucket_count_ - 1)) : 0;
}
// hash_bucket_count_ is set in Init() to NextPow2(max_capacity_ * 2).

// Look for `base` inside one exact-hash bucket while that bucket lock is
// already held.
//
// This is a pure internal helper and does not take locks on its own. Callers
// must establish the synchronization precondition.
s32 AllocTable::FindIdInHashBucketLocked(uptr bucket, uptr base) const {
  s32 id = hash_buckets_[bucket];
  while (id >= 0) {
    const Node *node = GetNode(id);
    if (!node)
      return -1;
    if (node->state == kNodeLive && node->base == base)
      return id;
    id = node->hash_next_or_free;
  }
  return -1;
}

//===----------------------------------------------------------------------===//
// Page-Range Hash Helpers (`page -> candidate alloc`)
//===----------------------------------------------------------------------===//

// Compute which page-range bucket a page id lands in.
uptr AllocTable::PageBucket(uptr page_id) const {
  return page_bucket_count_ ? (Hash(page_id) & (page_bucket_count_ - 1)) : 0;
}

// Insert one RangeLink for every logical page covered by an allocation.
//
// This is the step that lets `store_record(dst_ptr)` resolve the owner without
// a global ordered tree, using the lightweight "page -> candidate alloc" index.
bool AllocTable::InsertRangeLinksForNode(s32 alloc_id, uptr base, u32 size) {
  const uptr first_page = PageIdForAddr(base);
  const uptr last_page = LastPageIdForRange(base, size);
  for (uptr page = first_page; page <= last_page; ++page) {
    const s32 link_id = AcquireRangeLink();
    if (link_id < 0) {
      RollbackInsertedRangeLinks(alloc_id, base, size, page);
      return false;
    }
    RangeLink *link = GetRangeLink(link_id);
    if (!link) {
      ReleaseRangeLink(link_id);
      RollbackInsertedRangeLinks(alloc_id, base, size, page);
      return false;
    }
    const uptr bucket = PageBucket(page);
    SpinMutexLock page_lock(&page_mutexes_[bucket]);
    link->page_id = page;
    link->alloc_id = alloc_id;
    link->next_or_free = page_buckets_[bucket];
    page_buckets_[bucket] = link_id;
  }
  return true;
}

// If page-link insertion fails halfway through, remove only the links that were
// already published before returning failure to the caller.
void AllocTable::RollbackInsertedRangeLinks(s32 alloc_id, uptr base, u32 size,
                                            uptr stop_before_page) {
  const uptr first_page = PageIdForAddr(base);
  const uptr last_page = LastPageIdForRange(base, size);
  const uptr stop =
      stop_before_page > last_page + 1 ? last_page + 1 : stop_before_page;
  for (uptr page = first_page; page < stop; ++page) {
    const uptr bucket = PageBucket(page);
    SpinMutexLock page_lock(&page_mutexes_[bucket]);
    s32 cur = page_buckets_[bucket];
    s32 prev = -1;
    while (cur >= 0) {
      RangeLink *link = GetRangeLink(cur);
      if (!link)
        break;
      if (link->page_id == page && link->alloc_id == alloc_id) {
        const s32 next = link->next_or_free;
        if (prev >= 0) {
          RangeLink *prev_link = GetRangeLink(prev);
          if (prev_link)
            prev_link->next_or_free = next;
        } else {
          page_buckets_[bucket] = next;
        }
        ReleaseRangeLink(cur);
        break;
      }
      prev = cur;
      cur = link->next_or_free;
    }
  }
}

// Remove all page-index coverage for one allocation. This is called from free
// and from same-address reuse before the alloc slot itself is recycled.
void AllocTable::RemoveRangeLinksForNode(s32 alloc_id, uptr base, u32 size) {
  const uptr first_page = PageIdForAddr(base);
  const uptr last_page = LastPageIdForRange(base, size);
  for (uptr page = first_page; page <= last_page; ++page) {
    const uptr bucket = PageBucket(page);
    SpinMutexLock page_lock(&page_mutexes_[bucket]);
    s32 cur = page_buckets_[bucket];
    s32 prev = -1;
    while (cur >= 0) {
      RangeLink *link = GetRangeLink(cur);
      if (!link)
        break;
      if (link->page_id == page && link->alloc_id == alloc_id) {
        const s32 next = link->next_or_free;
        if (prev >= 0) {
          RangeLink *prev_link = GetRangeLink(prev);
          if (prev_link)
            prev_link->next_or_free = next;
        } else {
          page_buckets_[bucket] = next;
        }
        ReleaseRangeLink(cur);
        break;
      }
      prev = cur;
      cur = link->next_or_free;
    }
  }
}

//===----------------------------------------------------------------------===//
// Alloc-Node Slab Storage
//===----------------------------------------------------------------------===//

// Grow the top-level alloc-node slab pointer array.
//
// This grows only the slab-pointer table, not the node slabs themselves. That
// keeps already allocated slab addresses stable while still letting the top
// array expand on demand.
bool AllocTable::EnsureNodeSlabPtrCapacity(uptr need_count) {
  return need_count <= node_slab_ptr_cap_;
}

// Materialize the slab that contains `idx` and preinitialize every node as an
// empty free-list slot.
bool AllocTable::EnsureNodeSlabForIndex(uptr idx) {
  const uptr slab_id = idx / kNodesPerSlab;
  if (!EnsureNodeSlabPtrCapacity(slab_id + 1))
    return false;
  if (node_slabs_[slab_id])
    return true;

  Node *slab = (Node *)InternalAlloc(kNodesPerSlab * sizeof(Node), nullptr, 0);
  if (!slab)
    return false;
  MemStatsOnAllocTableAlloc(kNodesPerSlab * sizeof(Node));
  internal_memset(slab, 0, kNodesPerSlab * sizeof(Node));
  for (uptr i = 0; i < kNodesPerSlab; ++i) {
    slab[i].store_head = -1;
    slab[i].hash_next_or_free = -1;
    slab[i].state = kNodeFree;
    slab[i].store_mu.Init();
  }

  node_slabs_[slab_id] = slab;
  if (slab_id + 1 > node_slab_count_)
    node_slab_count_ = slab_id + 1;
  return true;
}

// Capacity is grown in slab-sized steps but never past the explicit runtime
// maximum. This keeps memory growth incremental while preserving a hard upper
// bound for tracked allocs.
bool AllocTable::GrowNodeSlabs() {
  if (capacity_ >= max_capacity_)
    return false;
  if (!EnsureNodeSlabForIndex(capacity_))
    return false;
  capacity_ += Min<uptr>(kNodesPerSlab, max_capacity_ - capacity_);
  return true;
}

// Stable ids are turned back into node pointers with a simple slab-id/offset
// calculation. This is one of the main reasons the runtime can use compact
// integer references instead of raw pointers.
AllocTable::Node *AllocTable::GetNode(s32 idx) {
  if (idx < 0)
    return nullptr;
  const uptr uidx = static_cast<uptr>(idx);
  if (uidx >= capacity_)
    return nullptr;
  const uptr slab_id = uidx / kNodesPerSlab;
  const uptr offset = uidx % kNodesPerSlab;
  if (!node_slabs_ || slab_id >= node_slab_ptr_cap_ || !node_slabs_[slab_id])
    return nullptr;
  return &node_slabs_[slab_id][offset];
}

const AllocTable::Node *AllocTable::GetNode(s32 idx) const {
  if (idx < 0)
    return nullptr;
  const uptr uidx = static_cast<uptr>(idx);
  if (uidx >= capacity_)
    return nullptr;
  const uptr slab_id = uidx / kNodesPerSlab;
  const uptr offset = uidx % kNodesPerSlab;
  if (!node_slabs_ || slab_id >= node_slab_ptr_cap_ || !node_slabs_[slab_id])
    return nullptr;
  return &node_slabs_[slab_id][offset];
}

// Slot acquisition is split from higher-level insertion so callers can build
// more complex rollback sequences around it. The pool always prefers to reuse a
// released slot before consuming a never-used slot from the current capacity.
s32 AllocTable::AcquireNode() {
  SpinMutexLock lock(&pool_mu_);
  if (free_head_ >= 0) {
    const s32 id = free_head_;
    Node *node = GetNode(id);
    if (!node)
      return -1;
    free_head_ = node->hash_next_or_free;
    node->hash_next_or_free = -1;
    return id;
  }

  if (next_node_ >= capacity_ && !GrowNodeSlabs())
    return -1;

  const s32 id = static_cast<s32>(next_node_);
  ++next_node_;
  return id;
}

// Releasing a node only returns the slot to the alloc free list. The backing
// slab memory is kept for reuse so hot churn does not continuously allocate and
// free internal storage.
void AllocTable::ReleaseNode(s32 id) {
  Node *node = GetNode(id);
  if (!node)
    return;
  SpinMutexLock lock(&pool_mu_);
  node->state = kNodeFree;
  node->hash_next_or_free = free_head_;
  free_head_ = id;
}

//===----------------------------------------------------------------------===//
// Range-Link Slab Storage
//===----------------------------------------------------------------------===//

// Grow the top-level range-link slab pointer array.
//
// RangeLink is the physical storage unit for the page-range index. It is
// managed separately because:
// - one alloc node is represented by one alloc row
// - but the same allocation may cover multiple pages and therefore need
//   multiple RangeLink rows
bool AllocTable::EnsureRangeSlabPtrCapacity(uptr need_count) {
  if (need_count <= range_slab_ptr_cap_)
    return true;

  uptr new_cap = range_slab_ptr_cap_ ? range_slab_ptr_cap_ : 8;
  while (new_cap < need_count)
    new_cap <<= 1;

  RangeLink **new_slabs =
      (RangeLink **)InternalAlloc(new_cap * sizeof(RangeLink *), nullptr, 0);
  if (!new_slabs)
    return false;
  MemStatsOnAllocTableAlloc(new_cap * sizeof(RangeLink *));
  internal_memset(new_slabs, 0, new_cap * sizeof(RangeLink *));

  if (range_slabs_) {
    internal_memcpy(new_slabs, range_slabs_,
                    range_slab_ptr_cap_ * sizeof(RangeLink *));
    RetiredRangeSlabPtrBlock *retired =
        (RetiredRangeSlabPtrBlock *)InternalAlloc(
            sizeof(RetiredRangeSlabPtrBlock), nullptr, 0);
    if (retired) {
      MemStatsOnAllocTableAlloc(sizeof(RetiredRangeSlabPtrBlock));
      retired->slabs = range_slabs_;
      retired->cap = range_slab_ptr_cap_;
      retired->next = retired_range_slab_ptrs_;
      retired_range_slab_ptrs_ = retired;
    }
  }

  range_slabs_ = new_slabs;
  range_slab_ptr_cap_ = new_cap;
  return true;
}

// Materialize the slab that contains `idx` and mark all links as unused.
bool AllocTable::EnsureRangeSlabForIndex(uptr idx) {
  const uptr slab_id = idx / kRangeLinksPerSlab;
  if (!EnsureRangeSlabPtrCapacity(slab_id + 1))
    return false;
  if (range_slabs_[slab_id])
    return true;

  RangeLink *slab =
      (RangeLink *)InternalAlloc(kRangeLinksPerSlab * sizeof(RangeLink),
                                 nullptr, 0);
  if (!slab)
    return false;
  MemStatsOnAllocTableAlloc(kRangeLinksPerSlab * sizeof(RangeLink));
  internal_memset(slab, 0, kRangeLinksPerSlab * sizeof(RangeLink));
  for (uptr i = 0; i < kRangeLinksPerSlab; ++i) {
    slab[i].alloc_id = -1;
    slab[i].next_or_free = -1;
  }

  range_slabs_[slab_id] = slab;
  if (slab_id + 1 > range_slab_count_)
    range_slab_count_ = slab_id + 1;
  return true;
}

// Range links are allowed to grow independently from alloc nodes. A large block
// that spans several pages can therefore consume several range links without
// forcing any change to the alloc-node layout.
bool AllocTable::GrowRangeLinkSlabs() {
  if (!EnsureRangeSlabForIndex(range_capacity_))
    return false;
  range_capacity_ += kRangeLinksPerSlab;
  return true;
}

// Range links use the same stable-id pattern as alloc nodes: id -> slab slot.
AllocTable::RangeLink *AllocTable::GetRangeLink(s32 idx) {
  if (idx < 0)
    return nullptr;
  const uptr uidx = static_cast<uptr>(idx);
  if (uidx >= range_capacity_)
    return nullptr;
  const uptr slab_id = uidx / kRangeLinksPerSlab;
  const uptr offset = uidx % kRangeLinksPerSlab;
  if (!range_slabs_ || slab_id >= range_slab_ptr_cap_ || !range_slabs_[slab_id])
    return nullptr;
  return &range_slabs_[slab_id][offset];
}

const AllocTable::RangeLink *AllocTable::GetRangeLink(s32 idx) const {
  if (idx < 0)
    return nullptr;
  const uptr uidx = static_cast<uptr>(idx);
  if (uidx >= range_capacity_)
    return nullptr;
  const uptr slab_id = uidx / kRangeLinksPerSlab;
  const uptr offset = uidx % kRangeLinksPerSlab;
  if (!range_slabs_ || slab_id >= range_slab_ptr_cap_ || !range_slabs_[slab_id])
    return nullptr;
  return &range_slabs_[slab_id][offset];
}

// Acquire and release of range-link slots mirror alloc-node slot management so
// rollback code can cheaply recycle partially inserted page coverage.
s32 AllocTable::AcquireRangeLink() {
  SpinMutexLock lock(&pool_mu_);
  if (range_free_head_ >= 0) {
    const s32 id = range_free_head_;
    RangeLink *link = GetRangeLink(id);
    if (!link)
      return -1;
    range_free_head_ = link->next_or_free;
    link->next_or_free = -1;
    return id;
  }

  if (next_range_link_ >= range_capacity_ && !GrowRangeLinkSlabs())
    return -1;

  const s32 id = static_cast<s32>(next_range_link_);
  ++next_range_link_;
  return id;
}

void AllocTable::ReleaseRangeLink(s32 id) {
  RangeLink *link = GetRangeLink(id);
  if (!link)
    return;
  SpinMutexLock lock(&pool_mu_);
  link->page_id = 0;
  link->alloc_id = -1;
  link->next_or_free = range_free_head_;
  range_free_head_ = id;
}

//===----------------------------------------------------------------------===//
// Node View / Lock Helpers
//===----------------------------------------------------------------------===//

// Fill an AllocEntry view from one internal live Node.
//
// This intentionally hides internal implementation fields such as:
// - the reused hash/free-list link
// - state
// - internal locks
bool AllocTable::FillEntryLocked(s32 id, const Node &node,
                                 AllocEntry *out) const {
  if (!out || node.state != kNodeLive)
    return false;
  out->base = node.base;
  out->malloc_pc = node.malloc_pc;
  out->size = node.size;
  out->type_id = node.type_id;
  out->var_id = node.var_id;
  out->store_head = node.store_head;
  out->id = id;
  return true;
}

// Pin a live alloc row by taking its allocation-local store lock. Callers use
// this hand-off object when they need a consistent view of:
// - block metadata
// - store_head
// - the allocation's local store chain
bool AllocTable::LockNodeIfLive(s32 id, LockedAlloc *out) const
    SANITIZER_NO_THREAD_SAFETY_ANALYSIS {
  if (!out)
    return false;
  Node *node = const_cast<Node *>(GetNode(id));
  if (!node)
    return false;
  node->store_mu.Lock();
  if (node->state != kNodeLive) {
    node->store_mu.Unlock();
    return false;
  }
  out->base = node->base;
  out->malloc_pc = node->malloc_pc;
  out->size = node->size;
  out->type_id = node->type_id;
  out->var_id = node->var_id;
  out->store_head = node->store_head;
  out->id = id;
  out->locked = true;
  return true;
}

//===----------------------------------------------------------------------===//
// Lifecycle
//===----------------------------------------------------------------------===//

// Initialize the full AllocTable.
//
// This creates two bucket families:
// - exact-hash buckets
// - page-range-index buckets
//
// They are maintained separately because their collision patterns and access
// modes are different.
bool AllocTable::Init(uptr initial_capacity) {
  Destroy();
  max_capacity_ = initial_capacity == 0 ? 1 : initial_capacity;
  hash_bucket_count_ = NextPow2(max_capacity_ * 2);
  page_bucket_count_ = NextPow2(max_capacity_ * 2);
  node_slab_ptr_cap_ = CeilDiv(max_capacity_, kNodesPerSlab);
  if (node_slab_ptr_cap_ == 0)
    node_slab_ptr_cap_ = 1;

  node_slabs_ =
      (Node **)InternalAlloc(node_slab_ptr_cap_ * sizeof(Node *), nullptr, 0);

  hash_buckets_ =
      (s32 *)InternalAlloc(hash_bucket_count_ * sizeof(s32), nullptr, 0);
  hash_mutexes_ = (__sanitizer::StaticSpinMutex *)InternalAlloc(
      hash_bucket_count_ * sizeof(__sanitizer::StaticSpinMutex), nullptr, 0);
  page_buckets_ =
      (s32 *)InternalAlloc(page_bucket_count_ * sizeof(s32), nullptr, 0);
  page_mutexes_ = (__sanitizer::StaticSpinMutex *)InternalAlloc(
      page_bucket_count_ * sizeof(__sanitizer::StaticSpinMutex), nullptr, 0);
  if (!node_slabs_ || !hash_buckets_ || !hash_mutexes_ || !page_buckets_ ||
      !page_mutexes_) {
    Destroy();
    return false;
  }

  MemStatsOnAllocTableAlloc(node_slab_ptr_cap_ * sizeof(Node *));
  MemStatsOnAllocTableAlloc(hash_bucket_count_ * sizeof(s32));
  MemStatsOnAllocTableAlloc(hash_bucket_count_ *
                           sizeof(__sanitizer::StaticSpinMutex));
  MemStatsOnAllocTableAlloc(page_bucket_count_ * sizeof(s32));
  MemStatsOnAllocTableAlloc(page_bucket_count_ *
                           sizeof(__sanitizer::StaticSpinMutex));

  internal_memset(node_slabs_, 0, node_slab_ptr_cap_ * sizeof(Node *));
  for (uptr i = 0; i < hash_bucket_count_; ++i) {
    hash_buckets_[i] = -1;
    hash_mutexes_[i].Init();
  }
  for (uptr i = 0; i < page_bucket_count_; ++i) {
    page_buckets_[i] = -1;
    page_mutexes_[i].Init();
  }
  return true;
}

// Destroy every internal storage layer and clear all state so the table can be
// reinitialized from scratch.
void AllocTable::Destroy() {
  if (node_slabs_) {
    for (uptr i = 0; i < node_slab_ptr_cap_; ++i) {
      if (node_slabs_[i]) {
        MemStatsOnAllocTableFree(kNodesPerSlab * sizeof(Node));
        InternalFree(node_slabs_[i]);
      }
    }
    MemStatsOnAllocTableFree(node_slab_ptr_cap_ * sizeof(Node *));
    InternalFree(node_slabs_);
  }
  if (range_slabs_) {
    for (uptr i = 0; i < range_slab_ptr_cap_; ++i) {
      if (range_slabs_[i]) {
        MemStatsOnAllocTableFree(kRangeLinksPerSlab * sizeof(RangeLink));
        InternalFree(range_slabs_[i]);
      }
    }
    MemStatsOnAllocTableFree(range_slab_ptr_cap_ * sizeof(RangeLink *));
    InternalFree(range_slabs_);
  }
  while (retired_range_slab_ptrs_) {
    RetiredRangeSlabPtrBlock *retired = retired_range_slab_ptrs_;
    retired_range_slab_ptrs_ = retired->next;
    if (retired->slabs) {
      MemStatsOnAllocTableFree(retired->cap * sizeof(RangeLink *));
      InternalFree(retired->slabs);
    }
    MemStatsOnAllocTableFree(sizeof(RetiredRangeSlabPtrBlock));
    InternalFree(retired);
  }
  if (hash_buckets_) {
    MemStatsOnAllocTableFree(hash_bucket_count_ * sizeof(s32));
    InternalFree(hash_buckets_);
  }
  if (hash_mutexes_) {
    MemStatsOnAllocTableFree(hash_bucket_count_ *
                             sizeof(__sanitizer::StaticSpinMutex));
    InternalFree(hash_mutexes_);
  }
  if (page_buckets_) {
    MemStatsOnAllocTableFree(page_bucket_count_ * sizeof(s32));
    InternalFree(page_buckets_);
  }
  if (page_mutexes_) {
    MemStatsOnAllocTableFree(page_bucket_count_ *
                             sizeof(__sanitizer::StaticSpinMutex));
    InternalFree(page_mutexes_);
  }

  node_slabs_ = nullptr;
  node_slab_ptr_cap_ = 0;
  node_slab_count_ = 0;
  capacity_ = 0;
  max_capacity_ = 0;
  next_node_ = 0;
  free_head_ = -1;

  range_slabs_ = nullptr;
  range_slab_ptr_cap_ = 0;
  range_slab_count_ = 0;
  range_capacity_ = 0;
  next_range_link_ = 0;
  range_free_head_ = -1;
  retired_range_slab_ptrs_ = nullptr;

  hash_buckets_ = nullptr;
  hash_mutexes_ = nullptr;
  hash_bucket_count_ = 0;
  page_buckets_ = nullptr;
  page_mutexes_ = nullptr;
  page_bucket_count_ = 0;
  atomic_store(&live_count_, 0, memory_order_relaxed);
}

//===----------------------------------------------------------------------===//
// Composite Publish / Insert
//===----------------------------------------------------------------------===//

// Insert a new allocation.
//
// The sequence is:
// 1. acquire one alloc-node slot
// 2. initialize the Node payload
// 3. publish into the exact hash: `base -> alloc`
// 4. publish into the page-range index: `page -> candidate alloc`
//
// If step 4 fails, step 3 is explicitly rolled back and the slot is returned
// to the free list so no half-published state remains.
bool AllocTable::Insert(uptr base, u32 size, uptr malloc_pc, s32 *out_id) {
  if (!base || max_capacity_ == 0)
    return false;
  const s32 id = AcquireNode();
  if (id < 0)
    return false;

  Node *node = GetNode(id);
  if (!node) {
    ReleaseNode(id);
    return false;
  }

  node->store_mu.Init();
  node->store_mu.Lock();
  node->base = base;
  node->malloc_pc = malloc_pc;
  node->size = size;
  node->type_id = 0;
  node->var_id = 0;
  node->store_head = -1;
  node->hash_next_or_free = -1;
  node->state = kNodeLive;

  const uptr bucket = BaseBucket(base);
  {
    SpinMutexLock bucket_lock(&hash_mutexes_[bucket]);
    if (FindIdInHashBucketLocked(bucket, base) >= 0) {
      node->state = kNodeFree;
      node->base = 0;
      node->malloc_pc = 0;
      node->store_mu.Unlock();
      ReleaseNode(id);
      return false;
    }
    node->hash_next_or_free = hash_buckets_[bucket];
    hash_buckets_[bucket] = id;
  }

  if (!InsertRangeLinksForNode(id, base, size)) {
    SpinMutexLock bucket_lock(&hash_mutexes_[bucket]);
    s32 cur = hash_buckets_[bucket];
    s32 prev = -1;
    while (cur >= 0) {
      Node *cur_node = GetNode(cur);
      if (!cur_node)
        break;
      if (cur == id) {
        if (prev >= 0) {
          Node *prev_node = GetNode(prev);
          if (prev_node)
            prev_node->hash_next_or_free = cur_node->hash_next_or_free;
        } else {
          hash_buckets_[bucket] = cur_node->hash_next_or_free;
        }
        break;
      }
      prev = cur;
      cur = cur_node->hash_next_or_free;
    }
    node->state = kNodeFree;
    node->base = 0;
    node->malloc_pc = 0;
    node->hash_next_or_free = -1;
    node->store_mu.Unlock();
    ReleaseNode(id);
    return false;
  }

  atomic_fetch_add(&live_count_, 1, memory_order_relaxed);
  node->store_mu.Unlock();
  if (out_id)
    *out_id = id;
  return true;
}

//===----------------------------------------------------------------------===//
// Public Exact-Hash Queries
//===----------------------------------------------------------------------===//

// Public exact-lookup entry point.
//
// This intentionally reuses the locked lookup path instead of reading the hash
// bucket directly so that:
// - callers get a consistent view
// - even if another thread is about to modify this allocation's local store
//   chain, readers do not observe a torn state
bool AllocTable::Find(uptr base, AllocEntry *out) const {
  LockedAlloc locked = {};
  if (!LockByBase(base, &locked))
    return false;
  if (out) {
    out->base = locked.base;
    out->malloc_pc = locked.malloc_pc;
    out->size = locked.size;
    out->type_id = locked.type_id;
    out->var_id = locked.var_id;
    out->store_head = locked.store_head;
    out->id = locked.id;
  }
  Unlock(&locked);
  return true;
}

// FindId is the cheapest exact lookup path when a caller only needs the stable
// alloc id and not a locked view of the row.
bool AllocTable::FindId(uptr base, s32 *out_id) const {
  if (!out_id || !base || !hash_buckets_)
    return false;
  const uptr bucket = BaseBucket(base);
  SpinMutexLock bucket_lock(&hash_mutexes_[bucket]);
  const s32 id = FindIdInHashBucketLocked(bucket, base);
  if (id < 0)
    return false;
  *out_id = id;
  return true;
}

// GetById is mostly used by diagnostics and tests. It locks only the alloc's
// local store mutex because ids are already stable and do not require a hash
// lookup.
bool AllocTable::GetById(s32 id, AllocEntry *out) const {
  if (!out)
    return false;
  Node *node = const_cast<Node *>(GetNode(id));
  if (!node)
    return false;
  node->store_mu.Lock();
  const bool ok = FillEntryLocked(id, *node, out);
  node->store_mu.Unlock();
  return ok;
}

// LockByBase combines exact hash lookup with acquisition of the allocation's
// local store lock. The returned LockedAlloc is the runtime's main "pinned
// alloc" hand-off object.
bool AllocTable::LockByBase(uptr base, LockedAlloc *out) const {
  if (!out || !base || !hash_buckets_)
    return false;
  const uptr bucket = BaseBucket(base);
  SpinMutexLock bucket_lock(&hash_mutexes_[bucket]);
  const s32 id = FindIdInHashBucketLocked(bucket, base);
  return id >= 0 ? LockNodeIfLive(id, out) : false;
}

//===----------------------------------------------------------------------===//
// Public Page-Range Queries
//===----------------------------------------------------------------------===//

// Public containing-range lookup entry point.
//
// Semantically this means:
// - find the allocation that contains `addr`
// - return it as AllocEntry
bool AllocTable::FindContaining(uptr addr, AllocEntry *out) const {
  LockedAlloc locked = {};
  if (!LockContaining(addr, &locked))
    return false;
  if (out) {
    out->base = locked.base;
    out->malloc_pc = locked.malloc_pc;
    out->size = locked.size;
    out->type_id = locked.type_id;
    out->var_id = locked.var_id;
    out->store_head = locked.store_head;
    out->id = locked.id;
  }
  Unlock(&locked);
  return true;
}

// Find the allocation containing `addr` and lock its local store mutex before
// returning.
//
// The flow is:
// 1. compute which logical page contains `addr`
// 2. walk the candidate-allocation chain for that page
// 3. find the first live allocation whose `[base, base + size)` covers `addr`
// 4. lock it and return it
//
// Because `store_record()` receives only the field address `dst_ptr` rather
// than the owner base, this is one of the core paths in the formal design.
bool AllocTable::LockContaining(uptr addr, LockedAlloc *out) const
    SANITIZER_NO_THREAD_SAFETY_ANALYSIS {
  if (!out || !page_buckets_)
    return false;
  const uptr page_id = PageIdForAddr(addr);
  const uptr bucket = PageBucket(page_id);
  SpinMutexLock page_lock(&page_mutexes_[bucket]);
  s32 link_id = page_buckets_[bucket];
  while (link_id >= 0) {
    const RangeLink *link = GetRangeLink(link_id);
    if (!link)
      return false;
    if (link->page_id == page_id) {
      Node *node = const_cast<Node *>(GetNode(link->alloc_id));
      if (node) {
        node->store_mu.Lock();
        const uptr end = node->base + static_cast<uptr>(node->size);
        if (node->state == kNodeLive && node->base <= addr && addr < end) {
          out->base = node->base;
          out->size = node->size;
          out->type_id = node->type_id;
          out->var_id = node->var_id;
          out->store_head = node->store_head;
          out->id = link->alloc_id;
          out->locked = true;
          return true;
        }
        node->store_mu.Unlock();
      }
    }
    link_id = link->next_or_free;
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Locked Node Mutations
//===----------------------------------------------------------------------===//

// Release the allocation-local lock acquired by LockByBase()/LockContaining().
void AllocTable::Unlock(LockedAlloc *locked) const
    SANITIZER_NO_THREAD_SAFETY_ANALYSIS {
  if (!locked || !locked->locked)
    return;
  Node *node = const_cast<Node *>(GetNode(locked->id));
  if (node)
    node->store_mu.Unlock();
  locked->locked = false;
}

// Update type/variable metadata on an already locked allocation.
//
// UpdateLocked* helpers change only node payload. They never modify index
// structures; callers are responsible for establishing any index-level
// preconditions before entering.
bool AllocTable::UpdateLockedMeta(LockedAlloc *locked, u32 type_id, u32 var_id,
                                  uptr malloc_pc) {
  if (!locked || !locked->locked)
    return false;
  Node *node = GetNode(locked->id);
  if (!node || node->state != kNodeLive)
    return false;
  node->type_id = type_id;
  node->var_id = var_id;
  node->malloc_pc = malloc_pc;
  locked->type_id = type_id;
  locked->var_id = var_id;
  locked->malloc_pc = malloc_pc;
  return true;
}

bool AllocTable::UpdateLockedSize(LockedAlloc *locked, u32 size) {
  if (!locked || !locked->locked)
    return false;
  Node *node = GetNode(locked->id);
  if (!node || node->state != kNodeLive)
    return false;
  node->size = size;
  locked->size = size;
  return true;
}

bool AllocTable::SetLockedStoreHead(LockedAlloc *locked, s32 store_head) {
  if (!locked || !locked->locked)
    return false;
  Node *node = GetNode(locked->id);
  if (!node || node->state != kNodeLive)
    return false;
  node->store_head = store_head;
  locked->store_head = store_head;
  return true;
}

//===----------------------------------------------------------------------===//
// Removal
//===----------------------------------------------------------------------===//

// Removal step 1.
//
// This does the following:
// 1. find `base` in the exact hash
// 2. detach that allocation from the exact-hash bucket chain
// 3. mark the node as deleting
// 4. copy the allocation contents out to the caller so later steps can remove
//    page-range links and clear the store chain
//
// The flow is split into Begin/Range/Finalize so upper layers can safely clean
// the remaining attached structures after the allocation is already invisible
// to external lookups.
bool AllocTable::BeginRemove(uptr base, AllocEntry *removed) {
  if (!base || !removed || !hash_buckets_)
    return false;
  const uptr bucket = BaseBucket(base);
  SpinMutexLock bucket_lock(&hash_mutexes_[bucket]);

  s32 cur = hash_buckets_[bucket];
  s32 prev = -1;
  while (cur >= 0) {
    Node *node = GetNode(cur);
    if (!node)
      return false;
    if (node->state == kNodeLive && node->base == base) {
      node->store_mu.Lock();
      if (node->state != kNodeLive || node->base != base) {
        node->store_mu.Unlock();
        return false;
      }
      node->state = kNodeDeleting;
      if (prev >= 0) {
        Node *prev_node = GetNode(prev);
        if (prev_node)
          prev_node->hash_next_or_free = node->hash_next_or_free;
      } else {
        hash_buckets_[bucket] = node->hash_next_or_free;
      }
      removed->base = node->base;
      removed->malloc_pc = node->malloc_pc;
      removed->size = node->size;
      removed->type_id = node->type_id;
      removed->var_id = node->var_id;
      removed->store_head = node->store_head;
      removed->id = cur;
      node->store_mu.Unlock();
      return true;
    }
    prev = cur;
    cur = node->hash_next_or_free;
  }
  return false;
}

// Callers must not hold any allocation-local `store_mu` when entering this
// step. The containing page buckets are locked inside RemoveRangeLinksForNode(),
// while owner lookup paths acquire locks in the opposite order
// (`page_mutexes_[bucket]` first, then `node->store_mu`).
void AllocTable::RemoveRangeForEntry(const AllocEntry &entry) {
  RemoveRangeLinksForNode(entry.id, entry.base, entry.size);
}

// FinalizeRemove clears the payload, transitions the row back to free, and
// returns the node slot to the alloc free list.
bool AllocTable::FinalizeRemove(s32 id) {
  Node *node = GetNode(id);
  if (!node)
    return false;
  node->store_mu.Lock();
  if (node->state != kNodeDeleting) {
    node->store_mu.Unlock();
    return false;
  }
  node->base = 0;
  node->malloc_pc = 0;
  node->size = 0;
  node->type_id = 0;
  node->var_id = 0;
  node->store_head = -1;
  node->hash_next_or_free = -1;
  node->store_mu.Unlock();

  ReleaseNode(id);
  atomic_fetch_sub(&live_count_, 1, memory_order_relaxed);
  return true;
}

// Convenience wrapper for callers that do not need the staged remove sequence.
bool AllocTable::Remove(uptr base, AllocEntry *removed) {
  AllocEntry entry = {};
  if (!BeginRemove(base, &entry))
    return false;
  RemoveRangeForEntry(entry);
  const bool ok = FinalizeRemove(entry.id);
  if (ok && removed)
    *removed = entry;
  return ok;
}

//===----------------------------------------------------------------------===//
// Introspection
//===----------------------------------------------------------------------===//

// Return the current number of live allocations.
//
// Note that this is not:
// - total slot capacity
// - the historical total number of allocated nodes
// It is the number of allocations that are currently still live.
uptr AllocTable::Size() const {
  return static_cast<uptr>(atomic_load(&live_count_, memory_order_relaxed));
}

// Return the true byte size of one internal alloc node for layout queries and
// memory statistics.
uptr AllocTable::StorageEntrySize() const { return sizeof(Node); }

}  // namespace __ohos_memgraph
#endif /* OHOS_LLVM */
