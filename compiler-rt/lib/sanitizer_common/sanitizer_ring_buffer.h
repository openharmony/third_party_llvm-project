//===-- sanitizer_ring_buffer.h ---------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Simple ring buffer.
//
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_RING_BUFFER_H
#define SANITIZER_RING_BUFFER_H

#include "sanitizer_common.h"

namespace __sanitizer {
// RingBuffer<T>: fixed-size ring buffer optimized for speed of push().
// T should be a POD type and sizeof(T) should be divisible by sizeof(void*).
// At creation, all elements are zero.
template<class T>
class RingBuffer {
 public:
  COMPILER_CHECK(sizeof(T) % sizeof(void *) == 0);
  static RingBuffer *New(uptr Size) {
    void *Ptr = MmapOrDie(SizeInBytes(Size), "RingBuffer");
    RingBuffer *RB = reinterpret_cast<RingBuffer*>(Ptr);
    uptr End = reinterpret_cast<uptr>(Ptr) + SizeInBytes(Size);
    RB->last_ = RB->next_ = reinterpret_cast<T*>(End - sizeof(T));
#if SANITIZER_OHOS
    RB->full_ = false;
#endif
    return RB;
  }
  void Delete() {
    UnmapOrDie(this, SizeInBytes(size()));
  }
  uptr size() const {
    return last_ + 1 -
           reinterpret_cast<T *>(reinterpret_cast<uptr>(this) +
#if !SANITIZER_OHOS
                                 2 * sizeof(T *));
#else
                                 sizeof(RingBuffer) - sizeof(T));
#endif
  }

#if SANITIZER_OHOS
  // Number of valid elements. Before the buffer wraps (full_==false), this is
  // less than size(); after wrap, equals size().
  uptr realsize() const {
    if (full_)
      return size();
    return ((uptr)last_ - (uptr)next_) / sizeof(T);
  }
#endif

  static uptr SizeInBytes(uptr Size) {
#if !SANITIZER_OHOS
    return Size * sizeof(T) + 2 * sizeof(T*);
#else
    return Size * sizeof(T) + sizeof(RingBuffer) - sizeof(T);
#endif
  }

  uptr SizeInBytes() { return SizeInBytes(size()); }

  void push(T t) {
    *next_ = t;
    next_--;
    static_assert((sizeof(T) % sizeof(T *)) == 0,
                  "The condition below works only if sizeof(T) is divisible by "
                  "sizeof(T*).");
#if !SANITIZER_OHOS
    if (next_ <= reinterpret_cast<T*>(&next_))
      next_ = last_;
#else
    if (next_ <= reinterpret_cast<T *>(&next_)) {
      next_ = last_;
      full_ = true;
    }
#endif
  }

  T operator[](uptr Idx) const {
    CHECK_LT(Idx, size());
    sptr IdxNext = Idx + 1;
    if (IdxNext > last_ - next_)
      IdxNext -= size();
    return next_[IdxNext];
  }

 private:
  RingBuffer() {}
  ~RingBuffer() {}
  RingBuffer(const RingBuffer&) = delete;

  // Data layout:
#if !SANITIZER_OHOS
  // LNDDDDDDDD
  // D: data elements.
  // L: last_, always points to the last data element.
  // N: next_, initially equals to last_, is decremented on every push,
  //    wraps around if it's less or equal than its own address.
#else
  // FLNDDDDDDDD
  // F: indicates whether the ring buffer is full.
  // D: data elements.
  // L: last_, always points to the last data element.
  // N: next_, initially equals to last_, is decremented on every push,
  //    wraps around if it's less or equal than its own address.
  // full_ starts false; New() also sets full_ = false explicitly.
  bool full_;
#endif
  T *last_;
  T *next_;
  T data_[1];  // flexible array.
};

// A ring buffer with externally provided storage that encodes its state in 8
// bytes. Has significant constraints on size and alignment of storage.
// See a comment in hwasan/hwasan_thread_list.h for the motivation behind this.
#if SANITIZER_WORDSIZE == 64
template <class T>
class CompactRingBuffer {
  // Top byte of long_ stores the buffer size in pages.
  // Lower bytes store the address of the next buffer element.
  static constexpr int kPageSizeBits = 12;
  static constexpr int kSizeShift = 56;
  static constexpr int kSizeBits = 64 - kSizeShift;
  static constexpr uptr kNextMask = (1ULL << kSizeShift) - 1;

  uptr GetStorageSize() const { return (long_ >> kSizeShift) << kPageSizeBits; }

  static uptr SignExtend(uptr x) { return ((sptr)x) << kSizeBits >> kSizeBits; }

  void Init(void *storage, uptr size) {
    CHECK_EQ(sizeof(CompactRingBuffer<T>), sizeof(void *));
    CHECK(IsPowerOfTwo(size));
    CHECK_GE(size, 1 << kPageSizeBits);
    CHECK_LE(size, 128 << kPageSizeBits);
    CHECK_EQ(size % 4096, 0);
    CHECK_EQ(size % sizeof(T), 0);
    uptr st = (uptr)storage;
    CHECK_EQ(st % (size * 2), 0);
    CHECK_EQ(st, SignExtend(st & kNextMask));
    long_ = (st & kNextMask) | ((size >> kPageSizeBits) << kSizeShift);
  }

  void SetNext(const T *next) {
    long_ = (long_ & ~kNextMask) | ((uptr)next & kNextMask);
  }

 public:
  CompactRingBuffer(void *storage, uptr size) {
    Init(storage, size);
  }

  // A copy constructor of sorts.
  CompactRingBuffer(const CompactRingBuffer &other, void *storage) {
    uptr size = other.GetStorageSize();
    internal_memcpy(storage, other.StartOfStorage(), size);
    Init(storage, size);
    uptr Idx = other.Next() - (const T *)other.StartOfStorage();
    SetNext((const T *)storage + Idx);
  }

  T *Next() const { return (T *)(SignExtend(long_ & kNextMask)); }

  void *StartOfStorage() const {
    return (void *)((uptr)Next() & ~(GetStorageSize() - 1));
  }

  void *EndOfStorage() const {
    return (void *)((uptr)StartOfStorage() + GetStorageSize());
  }

  uptr size() const { return GetStorageSize() / sizeof(T); }

  void push(T t) {
    T *next = Next();
    *next = t;
    next++;
    next = (T *)((uptr)next & ~GetStorageSize());
    SetNext(next);
  }

  const T &operator[](uptr Idx) const {
    CHECK_LT(Idx, size());
    const T *Begin = (const T *)StartOfStorage();
    sptr StorageIdx = Next() - Begin;
    StorageIdx -= (sptr)(Idx + 1);
    if (StorageIdx < 0)
      StorageIdx += size();
    return Begin[StorageIdx];
  }

 public:
  ~CompactRingBuffer() {}
  CompactRingBuffer(const CompactRingBuffer &) = delete;

  uptr long_;
};
#endif

#if SANITIZER_OHOS
template <class T>
class RingBufferLink {
 public:
  COMPILER_CHECK(sizeof(T) % sizeof(void *) == 0);
#define DEFAULT_N_SIZE 1023
#define DEFAULT_MAX_NUM 1
#define _INT_MAX ((unsigned int)(-1) >> 1)
  static uptr manual_log2(uptr x) {
    int res = 1;
    while (x >>= 1) res++;
    return res;
  }
  struct MapNode {
    void *addr;
    uptr size;
  };
  static RingBufferLink *New(uptr _n_size = DEFAULT_N_SIZE,
                             uptr _max_num = DEFAULT_MAX_NUM) {
    RAW_CHECK(0 < _n_size && _n_size <= _INT_MAX &&
              "Sanitizer RingBufferLink: Invalid size of input cache blocks.");
    RAW_CHECK(0 < _max_num && _max_num <= _INT_MAX &&
              "Sanitizer RingBufferLink: Invalid number of input cache blocks.");

    void *Ptr = MmapOrDie(sizeof(RingBufferLink) + sizeof(T *) * (_max_num - 1),
                          "RingBufferLink");
    RingBufferLink *RBL = reinterpret_cast<RingBufferLink *>(Ptr);
    RBL->full_ = false;
    RBL->n_size = _n_size;
    RBL->max_num = _max_num;
    RBL->top_ = 0;
    void *MapPtr =
        MmapOrDie(sizeof(MapNode) * manual_log2(_max_num), "RingBufferLinkMap");
    MapNode *Map = reinterpret_cast<MapNode *>(MapPtr);
    RBL->mem_map = Map;
    void *NodePtr = MmapOrDie(sizeof(T) * _n_size, "RingBufferLinkList");
    T *RBNode = reinterpret_cast<T *>(NodePtr);
    RBL->list[0] = RBNode;
    RBL->node_num = 1;
    return RBL;
  }
  void Delete() {
    int num = manual_log2(max_num);
    for (int i = 0; i < num; i++) {
      UnmapOrDie(mem_map[i].addr, mem_map[i].size);
    }
    UnmapOrDie(mem_map, sizeof(MapNode) * num);
    UnmapOrDie(this, HeadSizeInBytes());
  }

  uptr size() const { return n_size * node_num; }
  uptr realsize() const {
    if (full_)
      return size();
    return top_;
  }

  uptr HeadSizeInBytes() {
    return sizeof(RingBufferLink) + sizeof(T *) * (max_num - 1);
  }

  uptr SizeInBytes() {
    return HeadSizeInBytes() + sizeof(T) * n_size * node_num +
           sizeof(MapNode) * manual_log2(max_num);
  }

  static uptr SizeInBytes(uptr _n_size, uptr _max_num) {
    return sizeof(RingBufferLink) + sizeof(T *) * (_max_num - 1) +
           sizeof(T) * _n_size * _max_num +
           sizeof(MapNode) * manual_log2(_max_num);
  }

  uptr getNodeId(uptr num) const { return (num / n_size) % max_num; }
  uptr getNodeOffset(uptr num) const { return num % n_size; }

  void push(T t) {
    uptr n_id = getNodeId(top_);
    uptr n_offset = getNodeOffset(top_);
    if (!full_ && n_id >= node_num && n_id < max_num) {
      uptr expandNum = (node_num * 2 > max_num) ? max_num : node_num * 2;
      uptr x = node_num;
      uptr mem_size = sizeof(T) * n_size * (expandNum - node_num);
      void *NodePtr = MmapOrDie(mem_size, "RingBufferLinkNode");
      mem_map[manual_log2(n_id)] = {NodePtr, mem_size};
      T *RBNode = reinterpret_cast<T *>(NodePtr);
      for (; node_num < expandNum; node_num++) {
        list[node_num] = RBNode;
        RBNode += n_size;
      }
      CHECK_LE(((char *)RBNode - (char *)NodePtr),
               sizeof(T) * n_size * (expandNum - x));
    }
    list[n_id][n_offset] = t;
    top_++;
    if (top_ >= n_size * max_num) {
      full_ = true;
      top_ = top_ % (n_size * max_num);
    }
  }

  T operator[](uptr Idx) const {
    uptr n_id, n_offset, realId;
    if (full_) {
      realId = Idx + top_;
    } else {
      realId = Idx;
    }
    n_id = getNodeId(realId);
    n_offset = getNodeOffset(realId);
    return list[n_id][n_offset];
  }

  T *getIdAddr(uptr Idx) const {
    uptr n_id, n_offset, realId;
    if (full_) {
      realId = Idx + top_;
    } else {
      realId = Idx;
    }
    n_id = getNodeId(realId);
    n_offset = getNodeOffset(realId);
    return &(list[n_id][n_offset]);
  }
  T *top() const { return getIdAddr(top_ - 1); }

 private:
  RingBufferLink() {}
  ~RingBufferLink() {}
  RingBufferLink(const RingBufferLink &) = delete;

  bool full_;
  uptr n_size;
  uptr max_num;
  uptr top_;
  uptr node_num;
  MapNode *mem_map;
  T *list[1];
};
#endif /* SANITIZER_OHOS */
}  // namespace __sanitizer

#endif  // SANITIZER_RING_BUFFER_H
