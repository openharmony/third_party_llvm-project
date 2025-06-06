
#include "hwasan_thread.h"

#include "hwasan.h"
#include "hwasan_interface_internal.h"
#include "hwasan_mapping.h"
#include "hwasan_poisoning.h"
#include "sanitizer_common/sanitizer_atomic.h"
#include "sanitizer_common/sanitizer_file.h"
#include "sanitizer_common/sanitizer_placement_new.h"
#include "sanitizer_common/sanitizer_tls_get_addr.h"

namespace __hwasan {

static u32 RandomSeed() {
  u32 seed;
  do {
    if (UNLIKELY(!GetRandom(reinterpret_cast<void *>(&seed), sizeof(seed),
                            /*blocking=*/false))) {
      seed = static_cast<u32>(
          (NanoTime() >> 12) ^
          (reinterpret_cast<uptr>(__builtin_frame_address(0)) >> 4));
    }
  } while (!seed);
  return seed;
}

void Thread::InitRandomState() {
  random_state_ = flags()->random_tags ? RandomSeed() : unique_id_;
  random_state_inited_ = true;

  // Push a random number of zeros onto the ring buffer so that the first stack
  // tag base will be random.
  for (tag_t i = 0, e = GenerateRandomTag(); i != e; ++i)
    stack_allocations_->push(0);
}

void Thread::Init(uptr stack_buffer_start, uptr stack_buffer_size,
                  const InitState *state) {
  CHECK_EQ(0, unique_id_);  // try to catch bad stack reuse
  CHECK_EQ(0, stack_top_);
  CHECK_EQ(0, stack_bottom_);

  static atomic_uint64_t unique_id;
  unique_id_ = atomic_fetch_add(&unique_id, 1, memory_order_relaxed);

// OHOS_LOCAL begin
  if (auto sz = IsMainThread() ? flags()->heap_history_size_main_thread
                               : flags()->heap_history_size)
// OHOS_LOCAL end
    heap_allocations_ = HeapAllocationsRingBuffer::New(sz);
  trace_heap_allocation_ = true;  // OHOS_LOCAL
#if !SANITIZER_FUCHSIA
  // Do not initialize the stack ring buffer just yet on Fuchsia. Threads will
  // be initialized before we enter the thread itself, so we will instead call
  // this later.
  InitStackRingBuffer(stack_buffer_start, stack_buffer_size);
#endif
  InitStackAndTls(state);
  tid_ = GetTid();  // OHOS_LOCAL
  heap_quarantine_controller()->Init(); // OHOS_LOCAL
}

void Thread::InitStackRingBuffer(uptr stack_buffer_start,
                                 uptr stack_buffer_size) {
  HwasanTSDThreadInit();  // Only needed with interceptors.
  uptr *ThreadLong = GetCurrentThreadLongPtr();
  // The following implicitly sets (this) as the current thread.
  stack_allocations_ = new (ThreadLong)
      StackAllocationsRingBuffer((void *)stack_buffer_start, stack_buffer_size);
  // OHOS_LOCAL begin
  // For compatibility reason, we keep the tls variable hwasan_tls.
  // but the problem is both of the thread variable store the stack
  // records to the same address but this two action will not synchronize
  // with each other. So if there are A.so use hwasan_tls and B.so use
  // tp - 144, the stack records will overlap. Currently the stack
  // records record by tp - 144 will not be print in the hwasan log.
  uptr *ThreadLongWithoutTls = GetCurrentThreadLongPtrWithoutTls();
  *ThreadLongWithoutTls = *ThreadLong;
  // OHOS_LOCAL end
  // Check that it worked.
  CHECK_EQ(GetCurrentThread(), this);

  // ScopedTaggingDisable needs GetCurrentThread to be set up.
  ScopedTaggingDisabler disabler;

  if (stack_bottom_) {
    int local;
    CHECK(AddrIsInStack((uptr)&local));
    CHECK(MemIsApp(stack_bottom_));
    CHECK(MemIsApp(stack_top_ - 1));
  }

  if (flags()->verbose_threads) {
    if (IsMainThread()) {
      Printf("sizeof(Thread): %zd sizeof(HeapRB): %zd sizeof(StackRB): %zd\n",
             sizeof(Thread), heap_allocations_->SizeInBytes(),
             stack_allocations_->size() * sizeof(uptr));
    }
    Print("Creating  : ");
  }
}

void Thread::ClearShadowForThreadStackAndTLS() {
  if (stack_top_ != stack_bottom_)
    TagMemory(stack_bottom_, stack_top_ - stack_bottom_, 0);
  if (tls_begin_ != tls_end_)
    TagMemory(tls_begin_, tls_end_ - tls_begin_, 0);
}

void Thread::Destroy() {
  if (flags()->verbose_threads)
    Print("Destroying: ");
  // OHOS_LOCAL
  heap_quarantine_controller()->ClearHeapQuarantine(allocator_cache());
  AllocatorSwallowThreadLocalCache(allocator_cache());
  ClearShadowForThreadStackAndTLS();
  if (heap_allocations_)
    heap_allocations_->Delete();
  DTLS_Destroy();
  // Unregister this as the current thread.
  // Instrumented code can not run on this thread from this point onwards, but
  // malloc/free can still be served. Glibc may call free() very late, after all
  // TSD destructors are done.
  CHECK_EQ(GetCurrentThread(), this);
  *GetCurrentThreadLongPtr() = 0;
  // OHOS_LOCAL begin
  *GetCurrentThreadLongPtrWithoutTls() = 0;
  // OHOS_LOCAL end
}

void Thread::Print(const char *Prefix) {
// OHOS_LOCAL begin
  Printf(
      "%sT%zd %p stack: [%p,%p) sz: %zd tls: [%p,%p) rb:(%zd/%u) "
      "records(%llu/o:%llu) tid: %d\n",
      Prefix, unique_id_, (void *)this, stack_bottom(), stack_top(),
      stack_top() - stack_bottom(), tls_begin(), tls_end(),
      heap_allocations() ? heap_allocations()->realsize() : 0,
      IsMainThread() ? flags()->heap_history_size_main_thread
                     : flags()->heap_history_size,
      all_record_count_, all_record_count_overflow_, tid_);
// OHOS_LOCAL end
}

static u32 xorshift(u32 state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}

// Generate a (pseudo-)random non-zero tag.
tag_t Thread::GenerateRandomTag(uptr num_bits) {
  DCHECK_GT(num_bits, 0);
  if (tagging_disabled_)
    return 0;
  tag_t tag;
  const uptr tag_mask = (1ULL << num_bits) - 1;
  do {
    if (flags()->random_tags) {
      if (!random_buffer_) {
        EnsureRandomStateInited();
        random_buffer_ = random_state_ = xorshift(random_state_);
      }
      CHECK(random_buffer_);
      tag = random_buffer_ & tag_mask;
      random_buffer_ >>= num_bits;
    } else {
      EnsureRandomStateInited();
      random_state_ += 1;
      tag = random_state_ & tag_mask;
    }
  } while (!tag);
  return tag;
}

// OHOS_LOCAL begin
bool Thread::TryPutInQuarantineWithDealloc(uptr ptr, size_t s, u32 aid,
                                           u32 fid) {
  return heap_quarantine_controller()->TryPutInQuarantineWithDealloc(
      ptr, s, aid, fid, allocator_cache());
}

void Thread::GetQuarantineStayTimeAndCount(size_t &staytime,
                                           size_t &staycount) {
  heap_quarantine_controller()->consumeQuarantineStayTimeAndCount(staytime,
                                                                  staycount);
}
// OHOS_LOCAL end

} // namespace __hwasan
