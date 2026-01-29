//===-- memory_collect.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Test for collectAllocationsByTimeRange (gwp_asan_collect_allocations_by_time_range
// C interface).
//===----------------------------------------------------------------------===//

#include "gwp_asan/tests/harness.h"

#include <cstdint>
#include <cstddef>
#include <set>
#include <vector>

namespace {

// Buffer layout matches GuardedPoolAllocator::collectAllocationsByTimeRange:
// - Header: max_slots (size_t), sample_rate (size_t)
// - Entry: addr (uintptr_t), size (size_t), lifetime (uint64_t), trace[Depth] (uintptr_t[])
size_t getHeaderSize() { return 2 * sizeof(size_t); }
size_t getEntrySize(size_t Depth) {
  return sizeof(uintptr_t) + sizeof(size_t) + sizeof(uint64_t) +
         Depth * sizeof(uintptr_t);
}

} // namespace

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_Basic) {
  const size_t kNumSlots = 5;
  const size_t kDepth = 2;
  const size_t kMaxCount = 10;
  const uint64_t kTimespanSec = 3600; // 1 hour, enough to include recent allocs

  InitNumSlots(kNumSlots);

  std::vector<std::pair<void *, size_t>> Allocated;
  Allocated.push_back({GPA.allocate(8), 8});
  Allocated.push_back({GPA.allocate(16), 16});
  Allocated.push_back({GPA.allocate(32), 32});

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);
  uintptr_t *BufPtr = Buffer.data();

  size_t Count = GPA.collectAllocationsByTimeRange(
      kTimespanSec, BufPtr, kMaxCount, kDepth);

  EXPECT_EQ(Count, 3u);

  // Check header: first two size_t are max_slots and sample_rate
  size_t *Header = reinterpret_cast<size_t *>(BufPtr);
  EXPECT_EQ(Header[0], kNumSlots);
  EXPECT_EQ(Header[1], 5000u); // default sample_rate from options.inc

  // Check entries: addr, size, lifetime, trace[]
  std::set<std::pair<uintptr_t, size_t>> Expected;
  for (const auto &P : Allocated)
    Expected.insert({reinterpret_cast<uintptr_t>(P.first), P.second});

  char *DataBase = reinterpret_cast<char *>(BufPtr) + HeaderSize;
  for (size_t i = 0; i < Count; ++i) {
    char *EntryBase = DataBase + (i * EntrySize);
    uintptr_t Addr = *reinterpret_cast<uintptr_t *>(EntryBase);
    size_t Size = *reinterpret_cast<size_t *>(EntryBase + sizeof(uintptr_t));
    uint64_t Lifetime =
        *reinterpret_cast<uint64_t *>(EntryBase + sizeof(uintptr_t) +
                                      sizeof(size_t));
    EXPECT_GT(Expected.count({Addr, Size}), 0u) << "entry " << i;
    EXPECT_GE(Lifetime, 0u);
  }

  for (const auto &P : Allocated)
    GPA.deallocate(P.first);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_EmptyBuffer) {
  InitNumSlots(3);
  void *P = GPA.allocate(4);
  EXPECT_NE(P, nullptr);

  size_t Count = GPA.collectAllocationsByTimeRange(3600, nullptr, 10, 1);
  EXPECT_EQ(Count, 0u);

  uintptr_t DummyBuf;
  Count = GPA.collectAllocationsByTimeRange(3600, &DummyBuf, 0, 1);
  EXPECT_EQ(Count, 0u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_MaxCountLimit) {
  const size_t kNumSlots = 8;
  const size_t kDepth = 1;
  const size_t kMaxCount = 2; // only want 2 entries
  const uint64_t kTimespanSec = 3600;

  InitNumSlots(kNumSlots);

  std::vector<void *> Ptrs;
  for (int i = 0; i < 4; ++i)
    Ptrs.push_back(GPA.allocate(8));

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      kTimespanSec, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 2u);

  for (void *P : Ptrs)
    GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_NoAllocations) {
  InitNumSlots(4);
  const size_t kDepth = 1;
  const size_t kMaxCount = 5;
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 0u);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_SingleAllocation) {
  InitNumSlots(4);
  const size_t kDepth = 3;
  const size_t kMaxCount = 4;
  const uint64_t kTimespanSec = 7200;

  void *P = GPA.allocate(24);
  EXPECT_NE(P, nullptr);

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      kTimespanSec, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 1u);
  size_t *Header = reinterpret_cast<size_t *>(Buffer.data());
  EXPECT_EQ(Header[0], 4u);
  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  uintptr_t Addr = *reinterpret_cast<uintptr_t *>(DataBase);
  size_t Size = *reinterpret_cast<size_t *>(DataBase + sizeof(uintptr_t));
  EXPECT_EQ(Addr, reinterpret_cast<uintptr_t>(P));
  EXPECT_EQ(Size, 24u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_DepthZeroTreatedAsOne) {
  InitNumSlots(3);
  void *P = GPA.allocate(1);
  EXPECT_NE(P, nullptr);

  size_t HeaderSize = getHeaderSize();
  size_t EntrySizeForDepth1 = getEntrySize(1);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + 5 * EntrySizeForDepth1 + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(3600, Buffer.data(), 5, 0);
  EXPECT_EQ(Count, 1u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_VariousSizes) {
  const size_t kNumSlots = 6;
  const size_t kDepth = 1;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  std::vector<std::pair<void *, size_t>> Allocated;
  Allocated.push_back({GPA.allocate(0), 0});
  Allocated.push_back({GPA.allocate(1), 1});
  Allocated.push_back({GPA.allocate(7), 7});
  Allocated.push_back({GPA.allocate(64), 64});
  Allocated.push_back({GPA.allocate(256), 256});

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 5u);
  std::set<std::pair<uintptr_t, size_t>> Expected;
  for (const auto &Pr : Allocated)
    Expected.insert({reinterpret_cast<uintptr_t>(Pr.first), Pr.second});

  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  for (size_t i = 0; i < Count; ++i) {
    char *EntryBase = DataBase + (i * EntrySize);
    uintptr_t Addr = *reinterpret_cast<uintptr_t *>(EntryBase);
    size_t Size = *reinterpret_cast<size_t *>(EntryBase + sizeof(uintptr_t));
    EXPECT_GT(Expected.count({Addr, Size}), 0u);
  }

  for (const auto &Pr : Allocated)
    GPA.deallocate(Pr.first);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_OnlyLiveAllocations) {
  const size_t kNumSlots = 5;
  const size_t kDepth = 2;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  void *P0 = GPA.allocate(8);
  void *P1 = GPA.allocate(16);
  void *P2 = GPA.allocate(32);
  GPA.deallocate(P1);
  std::vector<std::pair<void *, size_t>> Live;
  Live.push_back({P0, 8});
  Live.push_back({P2, 32});

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 2u);
  std::set<std::pair<uintptr_t, size_t>> Expected;
  for (const auto &Pr : Live)
    Expected.insert({reinterpret_cast<uintptr_t>(Pr.first), Pr.second});

  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  for (size_t i = 0; i < Count; ++i) {
    char *EntryBase = DataBase + (i * EntrySize);
    uintptr_t Addr = *reinterpret_cast<uintptr_t *>(EntryBase);
    size_t Size = *reinterpret_cast<size_t *>(EntryBase + sizeof(uintptr_t));
    EXPECT_GT(Expected.count({Addr, Size}), 0u);
  }

  GPA.deallocate(P0);
  GPA.deallocate(P2);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_FullSlots) {
  const size_t kNumSlots = 4;
  const size_t kDepth = 1;
  const size_t kMaxCount = 8;
  InitNumSlots(kNumSlots);

  std::vector<void *> Ptrs;
  for (size_t i = 0; i < kNumSlots; ++i)
    Ptrs.push_back(GPA.allocate(8));

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, kNumSlots);

  for (void *P : Ptrs)
    GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_LargeDepth) {
  const size_t kNumSlots = 3;
  const size_t kDepth = 16;
  const size_t kMaxCount = 5;
  InitNumSlots(kNumSlots);

  void *P = GPA.allocate(8);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);

  EXPECT_EQ(Count, 1u);
  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  size_t TraceOffset = sizeof(uintptr_t) + sizeof(size_t) + sizeof(uint64_t);
  for (size_t j = 0; j < kDepth; ++j) {
    uintptr_t Frame = *reinterpret_cast<uintptr_t *>(
        DataBase + TraceOffset + j * sizeof(uintptr_t));
    (void)Frame;
  }
  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_MultipleCollectCalls) {
  const size_t kNumSlots = 5;
  const size_t kDepth = 1;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  void *P1 = GPA.allocate(8);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count1 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count1, 1u);

  void *P2 = GPA.allocate(16);
  size_t Count2 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count2, 2u);

  GPA.deallocate(P1);
  size_t Count3 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count3, 1u);

  GPA.deallocate(P2);
  size_t Count4 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count4, 0u);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_TimespanZero) {
  InitNumSlots(4);
  void *P = GPA.allocate(8);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(1);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + 5 * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      0, Buffer.data(), 5, 1);
  EXPECT_EQ(Count, 0u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_TimespanOneSecond) {
  InitNumSlots(3);
  void *P = GPA.allocate(4);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(1);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + 5 * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      1, Buffer.data(), 5, 1);
  EXPECT_EQ(Count, 1u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_OneSlotOnly) {
  InitNumSlots(1);
  const size_t kDepth = 1;
  const size_t kMaxCount = 2;
  void *P = GPA.allocate(1);
  EXPECT_NE(P, nullptr);

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 1u);
  size_t *Header = reinterpret_cast<size_t *>(Buffer.data());
  EXPECT_EQ(Header[0], 1u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_NoDuplicateEntries) {
  const size_t kNumSlots = 4;
  const size_t kDepth = 1;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  std::vector<void *> Ptrs;
  for (int i = 0; i < 3; ++i)
    Ptrs.push_back(GPA.allocate(8));

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 3u);

  std::set<uintptr_t> Addrs;
  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  for (size_t i = 0; i < Count; ++i) {
    char *EntryBase = DataBase + (i * EntrySize);
    uintptr_t Addr = *reinterpret_cast<uintptr_t *>(EntryBase);
    EXPECT_EQ(Addrs.count(Addr), 0u) << "duplicate addr in entry " << i;
    Addrs.insert(Addr);
  }

  for (void *P : Ptrs)
    GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_LifetimeNonNegative) {
  const size_t kNumSlots = 4;
  const size_t kDepth = 1;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  void *P = GPA.allocate(8);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 1u);

  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  uint64_t Lifetime = *reinterpret_cast<uint64_t *>(
      DataBase + sizeof(uintptr_t) + sizeof(size_t));
  EXPECT_GE(Lifetime, 0u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_MaxCountLargerThanSlots) {
  const size_t kNumSlots = 3;
  const size_t kDepth = 1;
  const size_t kMaxCount = 20;
  InitNumSlots(kNumSlots);

  void *P1 = GPA.allocate(8);
  void *P2 = GPA.allocate(16);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 2u);

  GPA.deallocate(P1);
  GPA.deallocate(P2);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_AllocateDeallocateCollect) {
  const size_t kNumSlots = 6;
  const size_t kDepth = 2;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  void *P1 = GPA.allocate(8);
  void *P2 = GPA.allocate(16);
  GPA.deallocate(P1);
  void *P3 = GPA.allocate(32);

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 2u);

  std::set<uintptr_t> Expected;
  Expected.insert(reinterpret_cast<uintptr_t>(P2));
  Expected.insert(reinterpret_cast<uintptr_t>(P3));
  char *DataBase = reinterpret_cast<char *>(Buffer.data()) + HeaderSize;
  for (size_t i = 0; i < Count; ++i) {
    uintptr_t Addr = *reinterpret_cast<uintptr_t *>(DataBase + (i * EntrySize));
    EXPECT_GT(Expected.count(Addr), 0u);
  }

  GPA.deallocate(P2);
  GPA.deallocate(P3);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_HeaderUnaffectedByCount) {
  const size_t kNumSlots = 5;
  const size_t kDepth = 1;
  const size_t kMaxCount = 10;
  InitNumSlots(kNumSlots);

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count0 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  size_t *Header = reinterpret_cast<size_t *>(Buffer.data());
  EXPECT_EQ(Header[0], kNumSlots);
  EXPECT_EQ(Header[1], 5000u);
  EXPECT_EQ(Count0, 0u);

  void *P = GPA.allocate(8);
  size_t Count1 = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Header[0], kNumSlots);
  EXPECT_EQ(Header[1], 5000u);
  EXPECT_EQ(Count1, 1u);

  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_ExactlyMaxCountEntries) {
  const size_t kNumSlots = 10;
  const size_t kDepth = 1;
  const size_t kMaxCount = 5;
  InitNumSlots(kNumSlots);

  std::vector<void *> Ptrs;
  for (int i = 0; i < 5; ++i)
    Ptrs.push_back(GPA.allocate(8));

  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(kDepth);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + kMaxCount * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);

  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), kMaxCount, kDepth);
  EXPECT_EQ(Count, 5u);

  for (void *P : Ptrs)
    GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_SlotCountTwo) {
  InitNumSlots(2);
  void *P = GPA.allocate(1);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(1);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + 4 * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);
  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), 4, 1);
  EXPECT_EQ(Count, 1u);
  size_t *Header = reinterpret_cast<size_t *>(Buffer.data());
  EXPECT_EQ(Header[0], 2u);
  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_SlotCountEight) {
  InitNumSlots(8);
  void *P = GPA.allocate(1);
  size_t HeaderSize = getHeaderSize();
  size_t EntrySize = getEntrySize(1);
  std::vector<uintptr_t> Buffer(
      (HeaderSize + 4 * EntrySize + sizeof(uintptr_t) - 1) /
      sizeof(uintptr_t),
      0);
  size_t Count = GPA.collectAllocationsByTimeRange(
      3600, Buffer.data(), 4, 1);
  EXPECT_EQ(Count, 1u);
  size_t *Header = reinterpret_cast<size_t *>(Buffer.data());
  EXPECT_EQ(Header[0], 8u);
  GPA.deallocate(P);
}

TEST_F(CustomGuardedPoolAllocator, CollectAllocationsByTimeRange_AlternatingDepth) {
  InitNumSlots(4);
  void *P = GPA.allocate(8);
  size_t HeaderSize = getHeaderSize();
  std::vector<uintptr_t> Buffer;

  for (size_t Depth : {1u, 2u, 4u, 8u}) {
    size_t EntrySize = getEntrySize(Depth);
    Buffer.resize((HeaderSize + 2 * EntrySize + sizeof(uintptr_t) - 1) /
                  sizeof(uintptr_t),
                  0);
    size_t Count = GPA.collectAllocationsByTimeRange(
        3600, Buffer.data(), 2, Depth);
    EXPECT_EQ(Count, 1u);
  }

  GPA.deallocate(P);
}
