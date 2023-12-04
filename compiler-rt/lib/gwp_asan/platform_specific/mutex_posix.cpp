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

// OHOS_LOCAL begin
// 1、OHOS uses recursive locks to avoid deadlock, such as this call chain:
//   `gwp_asan malloc -> find double free -> get lock -> trigger segv ->
//    segv handler -> malloc -> gwp_asan malloc`
// 2、It will be failed to unlock recursive lock after fork because tid changes,
//    so we just create a new recursive lock here.
void Mutex::unlockAtFork() {
  Mu = {{{PTHREAD_MUTEX_RECURSIVE}}};
}
// OHOS_LOCAL end

} // namespace gwp_asan
