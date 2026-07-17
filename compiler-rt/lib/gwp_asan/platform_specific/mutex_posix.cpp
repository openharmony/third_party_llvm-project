//===-- mutex_posix.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gwp_asan/mutex.h"

#include <assert.h>
#include <pthread.h>

namespace gwp_asan {
void Mutex::lock() {
  int Status = pthread_mutex_lock(&Mu);
  assert(Status == 0);
  // Remove warning for non-debug builds.
  (void)Status;
}

bool Mutex::tryLock() { return pthread_mutex_trylock(&Mu) == 0; }

void Mutex::unlock() {
  int Status = pthread_mutex_unlock(&Mu);
  assert(Status == 0);
  // Remove warning for non-debug builds.
  (void)Status;
}

#if defined(OHOS_LLVM) && defined(__OHOS__)
// OHOS_LOCAL begin
// OHOS uses recursive mutexes to avoid allocator reentrancy deadlocks from
// signal handlers. A recursive mutex cannot be unlocked after fork because the
// owning thread ID changes, so reset it instead.
void Mutex::unlockAtFork() { Mu = {{{PTHREAD_MUTEX_RECURSIVE}}}; }
// OHOS_LOCAL end
#endif // defined(OHOS_LLVM) && defined(__OHOS__)

} // namespace gwp_asan
