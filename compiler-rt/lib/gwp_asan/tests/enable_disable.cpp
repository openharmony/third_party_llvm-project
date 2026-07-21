//===-- enable_disable.cpp --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gwp_asan/tests/harness.h"

#include <sys/wait.h>

constexpr size_t Size = 100;

TEST_F(DefaultGuardedPoolAllocator, Fork) {
  void *P;
  pid_t Pid = fork();
  EXPECT_GE(Pid, 0);
  if (Pid == 0) {
    P = GPA.allocate(Size);
    EXPECT_NE(P, nullptr);
    memset(P, 0x42, Size);
    GPA.deallocate(P);
    _exit(0);
  }
  waitpid(Pid, nullptr, 0);
  P = GPA.allocate(Size);
  EXPECT_NE(P, nullptr);
  memset(P, 0x42, Size);
  GPA.deallocate(P);

  // OHOS recursive mutexes let the atfork prepare handler acquire the already
  // held locks. The post-fork handlers reset them, so both processes can keep
  // using the allocator.
  GPA.disable();
  Pid = fork();
  ASSERT_GE(Pid, 0);
  if (Pid == 0) {
    P = GPA.allocate(Size);
    if (!P)
      _exit(1);
    memset(P, 0x42, Size);
    GPA.deallocate(P);
    _exit(0);
  }
  int Status = 0;
  ASSERT_EQ(waitpid(Pid, &Status, 0), Pid);
  ASSERT_TRUE(WIFEXITED(Status));
  EXPECT_EQ(WEXITSTATUS(Status), 0);
  P = GPA.allocate(Size);
  ASSERT_NE(P, nullptr);
  memset(P, 0x42, Size);
  GPA.deallocate(P);
}

namespace {
pthread_mutex_t Mutex;
pthread_cond_t Conditional = PTHREAD_COND_INITIALIZER;
bool ThreadReady = false;

void *enableMalloc(void *arg) {
  auto &GPA = *reinterpret_cast<gwp_asan::GuardedPoolAllocator *>(arg);

  // Signal the main thread we are ready.
  pthread_mutex_lock(&Mutex);
  ThreadReady = true;
  pthread_cond_signal(&Conditional);
  pthread_mutex_unlock(&Mutex);

  // Wait for the malloc_disable & fork, then enable the allocator again.
  sleep(1);
  GPA.enable();

  return nullptr;
}

TEST_F(DefaultGuardedPoolAllocator, DisableForkEnable) {
  pthread_t ThreadId;
  EXPECT_EQ(pthread_create(&ThreadId, nullptr, &enableMalloc, &GPA), 0);

  // Do not lock the allocator right away, the other thread may need it to start
  // up.
  pthread_mutex_lock(&Mutex);
  while (!ThreadReady)
    pthread_cond_wait(&Conditional, &Mutex);
  pthread_mutex_unlock(&Mutex);

  // Disable the allocator and fork. fork should succeed after malloc_enable.
  GPA.disable();
  pid_t Pid = fork();
  EXPECT_GE(Pid, 0);
  if (Pid == 0) {
    void *P = GPA.allocate(Size);
    EXPECT_NE(P, nullptr);
    GPA.deallocate(P);
    _exit(0);
  }
  waitpid(Pid, nullptr, 0);
  EXPECT_EQ(pthread_join(ThreadId, 0), 0);
}
} // namespace
