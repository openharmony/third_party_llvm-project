//===-- hwasan_quarantine.cpp -----------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Copyright (c) 2025 Huawei Device Co., Ltd. All rights reserved.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file is a part of HWAddressSanitizer. Provide allocation quarantine
/// ability.
///
//===----------------------------------------------------------------------===//
#include "hwasan_quarantine.h"

#include "hwasan_allocator.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_stackdepot.h"
namespace __hwasan {

void HeapQuarantineController::ClearHeapQuarantine(AllocatorCache *cache) {
  CHECK(cache);
  if (heap_quarantine_list_) {
    DeallocateWithHeapQuarantcheck(heap_quarantine_tail_, cache);
    size_t sz = RoundUpTo(
        flags()->heap_quarantine_thread_max_count * sizeof(HeapQuarantine),
        GetPageSizeCached());
    UnmapOrDie(heap_quarantine_list_, sz);
    heap_quarantine_tail_ = 0;
    heap_quarantine_list_ = nullptr;
  }
  heap_quarantine_list_ = nullptr;
}

bool HeapQuarantineController::TryPutInQuarantineWithDealloc(
    uptr ptr, size_t s, u32 aid, u32 fid, AllocatorCache *cache) {
  if (IsInPrintf())
    return false;
  if ((flags()->heap_quarantine_max > 0) &&
      (flags()->heap_quarantine_max > s && flags()->heap_quarantine_min <= s)) {
    if (UNLIKELY(flags()->heap_quarantine_thread_max_count <= 0))
      return false;
    if (UNLIKELY(heap_quarantine_list_ == nullptr)) {
      size_t sz = RoundUpTo(
          flags()->heap_quarantine_thread_max_count * sizeof(HeapQuarantine),
          GetPageSizeCached());
      heap_quarantine_list_ = reinterpret_cast<HeapQuarantine *>(
          MmapOrDie(sz, "HeapQuarantine", 0));
      if (UNLIKELY(heap_quarantine_list_ == nullptr)) {
        return false;
      }
    }
    PutInQuarantineWithDealloc(ptr, s, aid, fid, cache);
    return true;
  }
  return false;
}

void HeapQuarantineController::PutInQuarantineWithDealloc(
    uptr ptr, size_t s, u32 aid, u32 fid, AllocatorCache *cache) {
  if (UNLIKELY(heap_quarantine_tail_ >=
               flags()->heap_quarantine_thread_max_count)) {
    // free 1/3 heap_quarantine_list
    u32 free_count = heap_quarantine_tail_ / 3;
    free_count = free_count > 0 ? free_count : 1;
    u32 left_count = heap_quarantine_tail_ - free_count;
    DeallocateWithHeapQuarantcheck(free_count, cache);
    internal_memcpy(
        (char *)heap_quarantine_list_,
        (char *)heap_quarantine_list_ + free_count * sizeof(HeapQuarantine),
        left_count * sizeof(HeapQuarantine));
    internal_memset(
        (char *)heap_quarantine_list_ + left_count * sizeof(HeapQuarantine), 0,
        free_count * sizeof(HeapQuarantine));
    heap_quarantine_tail_ -= free_count;
  }

  heap_quarantine_list_[heap_quarantine_tail_].ptr = ptr;
  heap_quarantine_list_[heap_quarantine_tail_].s = s;
  heap_quarantine_list_[heap_quarantine_tail_].alloc_context_id = aid;
  heap_quarantine_list_[heap_quarantine_tail_].free_context_id = fid;
  heap_quarantine_tail_++;
  return;
}

void HeapQuarantineController::DeallocateWithHeapQuarantcheck(
    u32 free_count, AllocatorCache *cache) {
  static u64 magic;
  internal_memset(&magic, flags()->free_fill_byte, sizeof(magic));
  for (u32 i = 0; i < free_count; i++) {
    u64 *ptrBeg = reinterpret_cast<u64 *>(heap_quarantine_list_[i].ptr);
    if (heap_quarantine_list_[i].s > sizeof(u64)) {
      if (flags()->max_free_fill_size > 0) {
        size_t fill_size =
            Min(heap_quarantine_list_[i].s, (size_t)flags()->max_free_fill_size);
        for (size_t j = 0; j < fill_size / sizeof(u64); j++) {
          if (ptrBeg[j] != magic) {
            Printf(
                "ptrBeg was re-written after free %p[%zu], %p "
                "%016llx:%016llx, freed by:\n",
                ptrBeg, j, &ptrBeg[j], ptrBeg[j], magic);
            StackDepotGet(heap_quarantine_list_[i].free_context_id).Print();
            Printf("allocated by:\n");
            StackDepotGet(heap_quarantine_list_[i].alloc_context_id).Print();
            break;
          }
        }
      }
    }
    SimpleThreadDeallocate((void *)ptrBeg, cache);
  }
}

}  // namespace __hwasan