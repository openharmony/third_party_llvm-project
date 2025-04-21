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
/// ability header.
///
//===----------------------------------------------------------------------===//
#ifndef HWASAN_QUARANTINE_H
#define HWASAN_QUARANTINE_H
#include "hwasan_allocator.h"
namespace __hwasan {
struct HeapQuarantine {
  uptr ptr;
  size_t s;
  u32 alloc_context_id;
  u32 free_context_id;
};
// provide heap quarant for per thread, no data race.
class HeapQuarantineController {
 private:
  u32 heap_quarantine_tail_;
  HeapQuarantine *heap_quarantine_list_;
  size_t count{0};
  size_t persist_interval{0};
  size_t pre_time_point{0};
  void PutInQuarantineWithDealloc(uptr ptr, size_t s, u32 aid, u32 fid,
                                  AllocatorCache *cache);
  void DeallocateWithHeapQuarantcheck(u32 free_count, AllocatorCache *cache);

 public:
  void Init() {
    heap_quarantine_tail_ = 0;
    heap_quarantine_list_ = nullptr;
  }

  void ClearHeapQuarantine(AllocatorCache *cache);

  bool TryPutInQuarantineWithDealloc(uptr ptr, size_t s, u32 aid, u32 fid,
                                     AllocatorCache *cache);

  void consumeQuarantineStayTimeAndCount(size_t &staytime, size_t &staycount);
};

}  // namespace __hwasan

#endif  // HWASAN_QUARANTINE_H