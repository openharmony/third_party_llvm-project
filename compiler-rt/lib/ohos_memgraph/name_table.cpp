//===-- name_table.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Pointer-identity-based name interning implementation.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#include "name_table.h"

#include "memgraph.h"
#include "memgraph_stats_internal.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_libc.h"

namespace __ohos_memgraph {

using namespace __sanitizer;

namespace {
constexpr uptr kMinMapCap = 64;
} // namespace

// The table starts empty. Init() chooses the table kind and allocates the
// backing hash map and id array.
NameTable::NameTable()
    : kind_(kTypeNameTable), map_(nullptr), map_cap_(0), map_used_(0),
      id_to_ptr_(nullptr), id_cap_(0), id_size_(0) {}

// Type names and variable names share the same implementation. The only
// difference is which memory-stat bucket receives the accounting.
void NameTable::OnAlloc(uptr bytes) {
  if (kind_ == kTypeNameTable)
    MemStatsOnTypeTableAlloc(bytes);
  else
    MemStatsOnVarTableAlloc(bytes);
}

// Release-side accounting that mirrors OnAlloc() so per-table stats stay
// balanced.
void NameTable::OnFree(uptr bytes) {
  if (kind_ == kTypeNameTable)
    MemStatsOnTypeTableFree(bytes);
  else
    MemStatsOnVarTableFree(bytes);
}

// Hash by pointer identity rather than string contents. Repeated use of the
// same static string literal naturally resolves to the same slot.
uptr NameTable::Hash(const char *ptr) const {
  return MixUptr(reinterpret_cast<uptr>(ptr));
}

// The map uses open addressing. Rehashing only relocates live slots while
// preserving ids, so type_id / var_id values already stored in alloc/store
// metadata remain valid.
bool NameTable::Rehash(uptr new_cap) {
  if (new_cap < kMinMapCap)
    new_cap = kMinMapCap;
  uptr cap = 1;
  while (cap < new_cap)
    cap <<= 1;

  Slot *new_map = (Slot *)InternalAlloc(cap * sizeof(Slot), nullptr, 0);
  if (!new_map)
    return false;
  OnAlloc(cap * sizeof(Slot));
  internal_memset(new_map, 0, cap * sizeof(Slot));

  if (map_) {
    for (uptr i = 0; i < map_cap_; ++i) {
      const Slot &slot = map_[i];
      if (slot.state != 1)
        continue;

      uptr dst = Hash(slot.ptr) & (cap - 1);
      while (new_map[dst].state == 1)
        dst = (dst + 1) & (cap - 1);
      new_map[dst] = slot;
    }
    OnFree(map_cap_ * sizeof(Slot));
    InternalFree(map_);
  }

  map_ = new_map;
  map_cap_ = cap;
  return true;
}

// Resolve(id) depends on an id -> ptr side array, so we must ensure that array
// has spare capacity before assigning a new id.
bool NameTable::EnsureIdCapacity() {
  if (id_cap_ > 0 && id_size_ + 1 < id_cap_)
    return true;

  uptr new_cap = id_cap_ ? (id_cap_ << 1) : 1024;
  const char **new_ids =
      (const char **)InternalAlloc(new_cap * sizeof(const char *), nullptr, 0);
  if (!new_ids)
    return false;
  OnAlloc(new_cap * sizeof(const char *));
  internal_memset(new_ids, 0, new_cap * sizeof(const char *));

  if (id_to_ptr_) {
    internal_memcpy(new_ids, id_to_ptr_, id_cap_ * sizeof(const char *));
    OnFree(id_cap_ * sizeof(const char *));
    InternalFree(id_to_ptr_);
  }

  id_to_ptr_ = new_ids;
  id_cap_ = new_cap;
  return true;
}

// Reset the table to the requested kind and prepare both the hash map and the
// id-to-pointer array.
bool NameTable::Init(NameTableKind kind, uptr initial_map_capacity) {
  Destroy();
  kind_ = kind;
  if (!Rehash(initial_map_capacity))
    return false;
  return EnsureIdCapacity();
}

// Release both backing arrays and clear all state so the table can be
// reinitialized.
void NameTable::Destroy() {
  if (map_) {
    OnFree(map_cap_ * sizeof(Slot));
    InternalFree(map_);
  }
  if (id_to_ptr_) {
    OnFree(id_cap_ * sizeof(const char *));
    InternalFree(id_to_ptr_);
  }
  map_ = nullptr;
  map_cap_ = 0;
  map_used_ = 0;
  id_to_ptr_ = nullptr;
  id_cap_ = 0;
  id_size_ = 0;
}

// Intern returns a compact id. Id 0 is reserved to mean "no metadata", so real
// ids start at 1.
u32 NameTable::Intern(const char *ptr) {
  if (!ptr)
    return 0;
  SpinMutexLock lock(&mu_);

  if (!map_ || map_used_ * 10 >= map_cap_ * 7) {
    uptr want = map_cap_ ? (map_cap_ << 1) : 2048;
    if (!Rehash(want))
      return 0;
  }

  uptr slot_idx = Hash(ptr) & (map_cap_ - 1);
  while (true) {
    Slot &slot = map_[slot_idx];
    if (slot.state == 0) {
      if (!EnsureIdCapacity())
        return 0;
      u32 id = static_cast<u32>(++id_size_);
      id_to_ptr_[id] = ptr;
      slot.ptr = ptr;
      slot.id = id;
      slot.state = 1;
      ++map_used_;
      return id;
    }
    if (slot.ptr == ptr)
      return slot.id;
    slot_idx = (slot_idx + 1) & (map_cap_ - 1);
  }
}

// Query paths eventually resolve stored ids back to the original string
// pointer.
const char *NameTable::Resolve(u32 id) const {
  SpinMutexLock lock(&mu_);
  if (id == 0 || !id_to_ptr_ || id > id_size_)
    return nullptr;
  return id_to_ptr_[id];
}

// Expose the physical slot size for monitoring and layout estimation.
uptr NameTable::MapEntrySize() const { return sizeof(Slot); }

} // namespace __ohos_memgraph
#endif /* OHOS_LLVM */
