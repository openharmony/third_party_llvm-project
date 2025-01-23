//===-- hwasan_thread.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of HWAddressSanitizer.
//
//===----------------------------------------------------------------------===//

#ifndef HWASAN_THREAD_H
#define HWASAN_THREAD_H

#include "hwasan_allocator.h"
#include "hwasan_quarantine.h"  // OHOS_LOCAL
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_ring_buffer.h"

namespace __hwasan {

typedef __sanitizer::CompactRingBuffer<uptr> StackAllocationsRingBuffer;

class Thread {
 public:
  // These are optional parameters that can be passed to Init.
  struct InitState;

  void Init(uptr stack_buffer_start, uptr stack_buffer_size,
            const InitState *state = nullptr);

  void InitStackAndTls(const InitState *state = nullptr);

  // Must be called from the thread itself.
  void InitStackRingBuffer(uptr stack_buffer_start, uptr stack_buffer_size);

  inline void EnsureRandomStateInited() {
    if (UNLIKELY(!random_state_inited_))
      InitRandomState();
  }

  void Destroy();

  uptr stack_top() { return stack_top_; }
  uptr stack_bottom() { return stack_bottom_; }
  uptr stack_size() { return stack_top() - stack_bottom(); }
  uptr tls_begin() { return tls_begin_; }
  uptr tls_end() { return tls_end_; }
  bool IsMainThread() { return unique_id_ == 0; }

// OHOS_LOCAL begin
  void inc_record(void) {
    all_record_count_++;
    if (all_record_count_ == 0) {
      all_record_count_overflow_++;
    }
  }
// OHOS_LOCAL end

  bool AddrIsInStack(uptr addr) {
    return addr >= stack_bottom_ && addr < stack_top_;
  }

  AllocatorCache *allocator_cache() { return &allocator_cache_; }
  HeapAllocationsRingBuffer *heap_allocations() { return heap_allocations_; }
  StackAllocationsRingBuffer *stack_allocations() { return stack_allocations_; }

  tag_t GenerateRandomTag(uptr num_bits = kTagBits);

  void DisableTagging() { tagging_disabled_++; }
  void EnableTagging() { tagging_disabled_--; }

// OHOS_LOCAL begin
  void EnableTracingHeapAllocation() { trace_heap_allocation_ = true; }
  void DisableTracingHeapAllocation() { trace_heap_allocation_ = false; }
  bool AllowTracingHeapAllocation() { return trace_heap_allocation_; }
// OHOS_LOCAL end

  u64 unique_id() const { return unique_id_; }

// OHOS_LOCAL begin
  int tid() const { return tid_; }
  void Announce() { Print("Thread: "); }
// OHOS_LOCAL end

  uptr &vfork_spill() { return vfork_spill_; }

// OHOS_LOCAL begin
  HeapQuarantineController *heap_quarantine_controller() {
    return &heap_quarantine_controller_;
  }

  bool TryPutInQuarantineWithDealloc(uptr ptr, size_t s, u32 aid, u32 fid);
// OHOS_LOCAL end

 private:
  // NOTE: There is no Thread constructor. It is allocated
  // via mmap() and *must* be valid in zero-initialized state.
  void ClearShadowForThreadStackAndTLS();
  void Print(const char *prefix);
  void InitRandomState();
  uptr vfork_spill_;
  uptr stack_top_;
  uptr stack_bottom_;
  uptr tls_begin_;
  uptr tls_end_;

  u32 random_state_;
  u32 random_buffer_;

  AllocatorCache allocator_cache_;
  HeapAllocationsRingBuffer *heap_allocations_;
  StackAllocationsRingBuffer *stack_allocations_;

// OHOS_LOCAL
  HeapQuarantineController heap_quarantine_controller_;

  u64 unique_id_;  // counting from zero.

  u32 tagging_disabled_;  // if non-zero, malloc uses zero tag in this thread.

  bool announced_;

  bool random_state_inited_;  // Whether InitRandomState() has been called.

// OHOS_LOCAL begin
  bool trace_heap_allocation_;

  int tid_ = -1;  // Thread ID

  u64 all_record_count_ = 0;  // Count record
  u64 all_record_count_overflow_ = 0;  // Whether all_record_count_ overflow.
// OHOS_LOCAL end

  friend struct ThreadListHead;
};

Thread *GetCurrentThread();
uptr *GetCurrentThreadLongPtr();

struct ScopedTaggingDisabler {
  ScopedTaggingDisabler() { GetCurrentThread()->DisableTagging(); }
  ~ScopedTaggingDisabler() { GetCurrentThread()->EnableTagging(); }
};

} // namespace __hwasan

#endif // HWASAN_THREAD_H
