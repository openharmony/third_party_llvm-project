//===-- guarded_pool_allocator.h --------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef GWP_ASAN_GUARDED_POOL_ALLOCATOR_H_
#define GWP_ASAN_GUARDED_POOL_ALLOCATOR_H_

#include "gwp_asan/common.h"
#include "gwp_asan/definitions.h"
#include "gwp_asan/mutex.h"
#include "gwp_asan/options.h"
#include "gwp_asan/platform_specific/guarded_pool_allocator_fuchsia.h" // IWYU pragma: keep
#include "gwp_asan/platform_specific/guarded_pool_allocator_posix.h" // IWYU pragma: keep
#include "gwp_asan/platform_specific/guarded_pool_allocator_tls.h"

namespace __sanitizer {
  class LoadedModule;
  class Symbolizer;
} // namespace __sanitizer

#include <stddef.h>
#include <stdint.h>
// IWYU pragma: no_include <__stddef_max_align_t.h>

// OHOS_LOCAL begin
#if defined (__OHOS__)
#define PRINT_COUNTER 100000
#define MUSL_LOG(fmt,...)                \
   if(&musl_log) {                       \
     musl_log(fmt, __VA_ARGS__);         \
   }

extern "C" GWP_ASAN_WEAK int musl_log(const char *fmt, ...);
#endif
// OHOS_LOCAL end

namespace gwp_asan {
// This class is the primary implementation of the allocator portion of GWP-
// ASan. It is the sole owner of the pool of sequentially allocated guarded
// slots. It should always be treated as a singleton.

// Functions in the public interface of this class are thread-compatible until
// init() is called, at which point they become thread-safe (unless specified
// otherwise).
class GuardedPoolAllocator {
public:
  // Name of the GWP-ASan mapping that for `Metadata`.
  static constexpr const char *kGwpAsanMetadataName = "GWP-ASan Metadata";

  // During program startup, we must ensure that memory allocations do not land
  // in this allocation pool if the allocator decides to runtime-disable
  // GWP-ASan. The constructor value-initialises the class such that if no
  // further initialisation takes place, calls to shouldSample() and
  // pointerIsMine() will return false.
  constexpr GuardedPoolAllocator() {}
  GuardedPoolAllocator(const GuardedPoolAllocator &) = delete;
  GuardedPoolAllocator &operator=(const GuardedPoolAllocator &) = delete;

  // Note: This class is expected to be a singleton for the lifetime of the
  // program. If this object is initialised, it will leak the guarded page pool
  // and metadata allocations during destruction. We can't clean up these areas
  // as this may cause a use-after-free on shutdown.
  ~GuardedPoolAllocator() = default;

  // Initialise the rest of the members of this class. Create the allocation
  // pool using the provided options. See options.inc for runtime configuration
  // options.
  void init(const options::Options &Opts);
  void uninitTestOnly();
  // OHOS_LOCAL begin
  bool hasFreeMem();
  // OHOS_LOCAL end

  // Functions exported for libmemunreachable's use on Android. disable()
  // installs a lock in the allocator that prevents any thread from being able
  // to allocate memory, until enable() is called.
  void disable();
  void enable();

  typedef void (*iterate_callback)(uintptr_t base, size_t size, void *arg);
  // Execute the callback Cb for every allocation the lies in [Base, Base +
  // Size). Must be called while the allocator is disabled. The callback can not
  // allocate.
  void iterate(void *Base, size_t Size, iterate_callback Cb, void *Arg);

  // This function is used to signal the allocator to indefinitely stop
  // functioning, as a crash has occurred. This stops the allocator from
  // servicing any further allocations permanently.
  void stop();

  // Return whether the allocation should be randomly chosen for sampling.
  GWP_ASAN_ALWAYS_INLINE bool shouldSample() {
    // OHOS_LOCAL begin
#if defined (__OHOS__)
    Nmalloc++;
    if( Nmalloc % PRINT_COUNTER == 0 ) {
      MUSL_LOG("[gwp_asan]: AvgDuration %{public}u us, FreeSlotsLength %{public}d\n",
               PersistInterval / ReserveCounter, FreeSlotsLength);
      Nmalloc = 0;
    }

    // Make sure the current thread's RandomState is configrued before use
    if (getThreadLocals()->is_configured == false) {
      return false;
    }

    // If the RandomState is calculated from getRandomUnsigned32, the value
    // of RandomState will never be 1, so we use RandomState == 1 to force
    // GWP_ASAN sample.
    if (GWP_ASAN_UNLIKELY(checkLib() || getThreadLocals()->RandomState == 1)) {
      return true;
    }
#endif
    // OHOS_LOCAL end
    // NextSampleCounter == 0 means we "should regenerate the counter".
    //                   == 1 means we "should sample this allocation".
    // AdjustedSampleRatePlusOne is designed to intentionally underflow. This
    // class must be valid when zero-initialised, and we wish to sample as
    // infrequently as possible when this is the case, hence we underflow to
    // UINT32_MAX.

    if (GWP_ASAN_UNLIKELY(getThreadLocals()->NextSampleCounter == 0))
      getThreadLocals()->NextSampleCounter =
          ((getRandomUnsigned32() % (AdjustedSampleRatePlusOne - 1)) + 1) &
          ThreadLocalPackedVariables::NextSampleCounterMask;

    return GWP_ASAN_UNLIKELY(--getThreadLocals()->NextSampleCounter == 0);
  }

  // OHOS_LOCAL begin
  // Force the allocation to do sample according to the function attribute.
  void forceSampleByFuncAttr() {
    getThreadLocals()->RandomState = 1;
  }

  // Unforce the allocation to do sample when out the fucntion scope with
  // attribute.
  void unforceSampleByFuncAttr() {
    // Initialised to a magic constant so that an uninitialised GWP-ASan won't
    // regenerate its sample counter for as long as possible. The xorshift32()
    // algorithm used below results in getRandomUnsigned32(0xacd979ce) ==
    // 0xfffffffe.
    getThreadLocals()->RandomState = 0xacd979ce;
  }
  // OHOS_LOCAL end
 
  // Returns whether the provided pointer is a current sampled allocation that
  // is owned by this pool.
  GWP_ASAN_ALWAYS_INLINE bool pointerIsMine(const void *Ptr) const {
    return State.pointerIsMine(Ptr);
  }

  // Allocate memory in a guarded slot, with the specified `Alignment`. Returns
  // nullptr if the pool is empty, if the alignnment is not a power of two, or
  // if the size/alignment makes the allocation too large for this pool to
  // handle. By default, uses strong alignment (i.e. `max_align_t`), see
  // http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2293.htm for discussion of
  // alignment issues in the standard.
  void *allocate(size_t Size, size_t Alignment = alignof(max_align_t));

  // Deallocate memory in a guarded slot. The provided pointer must have been
  // allocated using this pool. This will set the guarded slot as inaccessible.
  void deallocate(void *Ptr);

  // Returns the size of the allocation at Ptr.
  size_t getSize(const void *Ptr);

  // Returns a pointer to the Metadata region, or nullptr if it doesn't exist.
  const AllocationMetadata *getMetadataRegion() const { return Metadata; }

  // Returns a pointer to the AllocatorState region.
  const AllocatorState *getAllocatorState() const { return &State; }

  // Exposed as protected for testing.
protected:
  // Returns the actual allocation size required to service an allocation with
  // the provided Size and Alignment.
  static size_t getRequiredBackingSize(size_t Size, size_t Alignment,
                                       size_t PageSize);

  // Returns the provided pointer that meets the specified alignment, depending
  // on whether it's left or right aligned.
  static uintptr_t alignUp(uintptr_t Ptr, size_t Alignment);
  static uintptr_t alignDown(uintptr_t Ptr, size_t Alignment);

private:
  // Name of actively-occupied slot mappings.
  static constexpr const char *kGwpAsanAliveSlotName = "GWP-ASan Alive Slot";
  // Name of the guard pages. This includes all slots that are not actively in
  // use (i.e. were never used, or have been free()'d).)
  static constexpr const char *kGwpAsanGuardPageName = "GWP-ASan Guard Page";
  // Name of the mapping for `FreeSlots`.
  static constexpr const char *kGwpAsanFreeSlotsName = "GWP-ASan Metadata";

  static constexpr size_t kInvalidSlotID = SIZE_MAX;

  // These functions anonymously map memory or change the permissions of mapped
  // memory into this process in a platform-specific way. Pointer and size
  // arguments are expected to be page-aligned. These functions will never
  // return on error, instead electing to kill the calling process on failure.
  // The pool memory is initially reserved and inaccessible, and RW mappings are
  // subsequently created and destroyed via allocateInGuardedPool() and
  // deallocateInGuardedPool(). Each mapping is named on platforms that support
  // it, primarily Android. This name must be a statically allocated string, as
  // the Android kernel uses the string pointer directly.
  void *map(size_t Size, const char *Name) const;
  void unmap(void *Ptr, size_t Size) const;

  // The pool is managed separately, as some platforms (particularly Fuchsia)
  // manage virtual memory regions as a chunk where individual pages can still
  // have separate permissions. These platforms maintain metadata about the
  // region in order to perform operations. The pool is unique as it's the only
  // thing in GWP-ASan that treats pages in a single VM region on an individual
  // basis for page protection.
  // The pointer returned by reserveGuardedPool() is the reserved address range
  // of (at least) Size bytes.
  void *reserveGuardedPool(size_t Size);
  // allocateInGuardedPool() Ptr and Size must be a subrange of the previously
  // reserved pool range.
  void allocateInGuardedPool(void *Ptr, size_t Size) const;
  // deallocateInGuardedPool() Ptr and Size must be an exact pair previously
  // passed to allocateInGuardedPool().
  void deallocateInGuardedPool(void *Ptr, size_t Size) const;
  void unreserveGuardedPool();

  // Get the page size from the platform-specific implementation. Only needs to
  // be called once, and the result should be cached in PageSize in this class.
  static size_t getPlatformPageSize();

  // Returns a pointer to the metadata for the owned pointer. If the pointer is
  // not owned by this pool, the result is undefined.
  AllocationMetadata *addrToMetadata(uintptr_t Ptr) const;

  // Reserve a slot for a new guarded allocation. Returns kInvalidSlotID if no
  // slot is available to be reserved.
  size_t reserveSlot();

  // Unreserve the guarded slot.
  void freeSlot(size_t SlotIndex);

  // Raise a SEGV and set the corresponding fields in the Allocator's State in
  // order to tell the crash handler what happened. Used when errors are
  // detected internally (Double Free, Invalid Free).
  void trapOnAddress(uintptr_t Address, Error E);

  static GuardedPoolAllocator *getSingleton();

  // Install a pthread_atfork handler.
  void installAtFork();

  // OHOS_LOCAL begin
  void accumulatePersistInterval(size_t reservedSlotsLength);
#if defined (__OHOS__)
  bool checkLib();
  void findmodule();
  void parseWhiteList();
#endif
  // OHOS_LOCAL end

  gwp_asan::AllocatorState State;

  // OHOS_LOCAL begin
  uint32_t MinSampleSize{0};
  const char *WhiteListPath = "";
  // OHOS_LOCAL end

  // A mutex to protect the guarded slot and metadata pool for this class.
  Mutex PoolMutex;
// OHOS_LOCAL begin
#if defined (__OHOS__)
  // A mutex to protect the find library.
  Mutex FindModMutex;
#endif
// OHOS_LOCAL end
  // Some unwinders can grab the libdl lock. In order to provide atfork
  // protection, we need to ensure that we allow an unwinding thread to release
  // the libdl lock before forking.
  Mutex BacktraceMutex;
  // Record the number allocations that we've sampled. We store this amount so
  // that we don't randomly choose to recycle a slot that previously had an
  // allocation before all the slots have been utilised.
  size_t NumSampledAllocations = 0;
  // Pointer to the allocation metadata (allocation/deallocation stack traces),
  // if any.
  AllocationMetadata *Metadata = nullptr;

  // Pointer to an array of free slot indexes.
  size_t *FreeSlots = nullptr;
  // The current length of the list of free slots.
  size_t FreeSlotsLength = 0;

  // See options.{h, inc} for more information.
  bool PerfectlyRightAlign = false;

  // Backtrace function provided by the supporting allocator. See `options.h`
  // for more information.
  options::Backtrace_t Backtrace = nullptr;

  // The adjusted sample rate for allocation sampling. Default *must* be
  // nonzero, as dynamic initialisation may call malloc (e.g. from libstdc++)
  // before GPA::init() is called. This would cause an error in shouldSample(),
  // where we would calculate modulo zero. This value is set UINT32_MAX, as when
  // GWP-ASan is disabled, we wish to never spend wasted cycles recalculating
  // the sample rate.
  uint32_t AdjustedSampleRatePlusOne = 0;
  // OHOS_LOCAL begin
  // The Count that GWP_ASan tracks the malloc and prints upon reaching PRINT_COUNT.
  size_t Nmalloc{0};
  size_t PersistInterval{0};
  size_t PreTime{0};
  size_t ReserveCounter{0};
#if defined (__OHOS__)
  __sanitizer::Symbolizer *Symbolizer = nullptr;
  // Pointer to an array of modules.
  const __sanitizer::LoadedModule **Modules = nullptr; 
  uint8_t ModuleLength{0};
  // Pointer to an array of LibraryPath.
  char **LibraryPath = nullptr;
  uint8_t LibraryPathLength{0};
#endif
  // OHOS_LOCAL end

  // Additional platform specific data structure for the guarded pool mapping.
  PlatformSpecificMapData GuardedPagePoolPlatformData = {};

  class ScopedRecursiveGuard {
  public:
    ScopedRecursiveGuard() { getThreadLocals()->RecursiveGuard = true; }
    ~ScopedRecursiveGuard() { getThreadLocals()->RecursiveGuard = false; }
  };

  // Initialise the PRNG, platform-specific.
  void initPRNG();

  // xorshift (32-bit output), extremely fast PRNG that uses arithmetic
  // operations only. Seeded using platform-specific mechanisms by initPRNG().
  uint32_t getRandomUnsigned32();
};
} // namespace gwp_asan

#endif // GWP_ASAN_GUARDED_POOL_ALLOCATOR_H_
