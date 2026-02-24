//===-- recoverable.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <atomic>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "gwp_asan/common.h"
#include "gwp_asan/crash_handler.h"
#include "gwp_asan/tests/harness.h"

void CheckOnlyOneGwpAsanCrash(const std::string &OutputBuffer) {
  const char *kGwpAsanErrorString = "GWP-ASan detected a memory error";
  size_t FirstIndex = OutputBuffer.find(kGwpAsanErrorString);
  ASSERT_NE(FirstIndex, std::string::npos) << "Didn't detect a GWP-ASan crash";
  ASSERT_EQ(OutputBuffer.find(kGwpAsanErrorString, FirstIndex + 1),
            std::string::npos)
      << "Detected more than one GWP-ASan crash:\n"
      << OutputBuffer;
}

void CheckGwpAsanErrorString(const std::string &OutputBuffer,
                             const char *ErrorSubstring) {
  ASSERT_NE(OutputBuffer.find(ErrorSubstring), std::string::npos)
      << "Expected to find '" << ErrorSubstring << "' in output:\n"
      << OutputBuffer;
}

void ClearAndVerifyNoFurtherReports(gwp_asan::GuardedPoolAllocator &GPA,
                                    void *Ptr, std::string &OutputBuffer,
                                    void (*ErrorAction)(
                                        gwp_asan::GuardedPoolAllocator &,
                                        void *),
                                    int NumIterations = 100) {
  OutputBuffer.clear();
  for (int i = 0; i < NumIterations; ++i) {
    ErrorAction(GPA, Ptr);
    ASSERT_TRUE(OutputBuffer.empty()) << OutputBuffer;
  }
}

void DeallocateMemoryWrapper(gwp_asan::GuardedPoolAllocator &GPA, void *Ptr) {
  DeallocateMemory(GPA, Ptr);
}

void TouchMemoryWrapper(gwp_asan::GuardedPoolAllocator &GPA, void *Ptr) {
  (void)GPA;
  TouchMemory(Ptr);
}

TEST_P(BacktraceGuardedPoolAllocator, MultipleDoubleFreeOnlyOneOutput) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  // First time should generate a crash report.
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  // Ensure the crash is only reported once.
  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

TEST_P(BacktraceGuardedPoolAllocator, MultipleInvalidFreeOnlyOneOutput) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  // First time should generate a crash report.
  DeallocateMemory(GPA, Ptr + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));

  // Ensure the crash is only reported once.
  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    DeallocateMemory(GPA, Ptr + 1);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

TEST_P(BacktraceGuardedPoolAllocator, MultipleUseAfterFreeOnlyOneOutput) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  // First time should generate a crash report.
  TouchMemory(Ptr);
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));

  // Ensure the crash is only reported once.
  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

TEST_P(BacktraceGuardedPoolAllocator, MultipleBufferOverflowOnlyOneOutput) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  // First time should generate a crash report.
  TouchMemory(Ptr - 16);
  TouchMemory(Ptr + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  if (GetOutputBuffer().find("Buffer Overflow") == std::string::npos &&
      GetOutputBuffer().find("Buffer Underflow") == std::string::npos)
    FAIL() << "Failed to detect buffer underflow/overflow:\n"
           << GetOutputBuffer();

  // Ensure the crash is only reported once.
  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    TouchMemory(Ptr - 16);
    TouchMemory(Ptr + 16);
    ASSERT_TRUE(GetOutputBuffer().empty()) << GetOutputBuffer();
  }
}

TEST_P(BacktraceGuardedPoolAllocator, OneDoubleFreeOneUseAfterFree) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  // First time should generate a crash report.
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  // Ensure the crash is only reported once.
  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// We use double-free to detect that each slot can generate as single error.
// Use-after-free would also be acceptable, but buffer-overflow wouldn't be, as
// the random left/right alignment means that one right-overflow can disable
// page protections, and a subsequent left-overflow of a slot that's on the
// right hand side may not trap.
TEST_P(BacktraceGuardedPoolAllocator, OneErrorReportPerSlot) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < GPA.getAllocatorState()->MaxSimultaneousAllocations;
       ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
    DeallocateMemory(GPA, Ptr);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
    // Ensure the crash from this slot is only reported once.
    GetOutputBuffer().clear();
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
    // Reset the buffer, as we're gonna move to the next allocation.
    GetOutputBuffer().clear();
  }

  // All slots should have been used. No further errors should occur.
  for (size_t i = 0; i < 100; ++i)
    ASSERT_EQ(AllocateMemory(GPA), nullptr);
  for (void *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
    TouchMemory(Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

void singleAllocThrashTask(gwp_asan::GuardedPoolAllocator *GPA,
                           std::atomic<bool> *StartingGun,
                           unsigned NumIterations, unsigned Job, char *Ptr) {
  while (!*StartingGun) {
    // Wait for starting gun.
  }

  for (unsigned i = 0; i < NumIterations; ++i) {
    switch (Job) {
    case 0:
      DeallocateMemory(*GPA, Ptr);
      break;
    case 1:
      DeallocateMemory(*GPA, Ptr + 1);
      break;
    case 2:
      TouchMemory(Ptr);
      break;
    case 3:
      TouchMemory(Ptr - 16);
      TouchMemory(Ptr + 16);
      break;
    default:
      __builtin_trap();
    }
  }
}

void runInterThreadThrashingSingleAlloc(unsigned NumIterations,
                                        gwp_asan::GuardedPoolAllocator *GPA) {
  std::atomic<bool> StartingGun{false};
  std::vector<std::thread> Threads;
  constexpr unsigned kNumThreads = 4;
  if (std::thread::hardware_concurrency() < kNumThreads) {
    GTEST_SKIP() << "Not enough threads to run this test";
  }

  char *Ptr = static_cast<char *>(AllocateMemory(*GPA));

  for (unsigned i = 0; i < kNumThreads; ++i) {
    Threads.emplace_back(singleAllocThrashTask, GPA, &StartingGun,
                         NumIterations, i, Ptr);
  }

  StartingGun = true;

  for (auto &T : Threads)
    T.join();
}

TEST_P(BacktraceGuardedPoolAllocator, InterThreadThrashingSingleAlloc) {
  SCOPED_TRACE("");
  constexpr unsigned kNumIterations = 100000;
  runInterThreadThrashingSingleAlloc(kNumIterations, &GPA);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Extended Double Free tests - verify various iteration counts
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeExtendedIterations) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Double Free");

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 500; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Extended Invalid Free tests - different pointer offsets
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeVariousOffsets) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 8);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Invalid (Wild) Free");

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr + 8);
  DeallocateMemory(GPA, Ptr + 16);
  DeallocateMemory(GPA, Ptr - 1);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Extended Use After Free - verify repeated access
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeExtendedAccess) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Use After Free");

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 200; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Buffer underflow only
TEST_P(BacktraceGuardedPoolAllocator, BufferUnderflowOnly) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr - 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Buffer Underflow"));

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    TouchMemory(Ptr - 16);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Buffer overflow only
TEST_P(BacktraceGuardedPoolAllocator, BufferOverflowOnly) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Buffer Overflow"));

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    TouchMemory(Ptr + 16);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Verify GWP-ASan error prefix in output
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeErrorPrefix) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  const std::string &Out = GetOutputBuffer();
  ASSERT_NE(Out.find("GWP-ASan detected a memory error"), std::string::npos);
  ASSERT_NE(Out.find("Double Free"), std::string::npos);
}

// Sequence: Double free then use after free on same slot
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeThenUseAfterFreeSameSlot) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  GetOutputBuffer().clear();
  TouchMemory(Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Multiple slots with double free - each reports once
TEST_P(BacktraceGuardedPoolAllocator, MultipleSlotsDoubleFree) {
  SCOPED_TRACE("");
  constexpr size_t kNumSlotsToTest = 4;
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < kNumSlotsToTest; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
  }

  for (size_t i = 0; i < kNumSlotsToTest; ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
    GetOutputBuffer().clear();
  }
}

// Invalid free with null-ish pointers (middle of allocation)
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeMiddleOfAllocation) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 4);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Invalid (Wild) Free");

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 50; ++i) {
    DeallocateMemory(GPA, Ptr + 4);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Use after free - alternate with double free attempts
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeWithDoubleFreeAttempts) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 50; ++i) {
    DeallocateMemory(GPA, Ptr);
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Buffer overflow with different offsets
TEST_P(BacktraceGuardedPoolAllocator, BufferOverflowVariousOffsets) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 8);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  bool HasOverflow = GetOutputBuffer().find("Buffer Overflow") != std::string::npos;
  bool HasUnderflow = GetOutputBuffer().find("Buffer Underflow") != std::string::npos;
  ASSERT_TRUE(HasOverflow || HasUnderflow);

  GetOutputBuffer().clear();
  TouchMemory(Ptr + 8);
  TouchMemory(Ptr + 32);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Interleaved allocations - double free on first
TEST_P(BacktraceGuardedPoolAllocator, InterleavedAllocationsDoubleFreeFirst) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  void *P3 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  DeallocateMemory(GPA, P3);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Interleaved allocations - invalid free on middle
TEST_P(BacktraceGuardedPoolAllocator, InterleavedAllocationsInvalidFreeMiddle) {
  SCOPED_TRACE("");
  char *P1 = static_cast<char *>(AllocateMemory(GPA));
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  char *P3 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P2 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Invalid (Wild) Free");

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Rapid double free then verify
TEST_P(BacktraceGuardedPoolAllocator, RapidDoubleFreeSequence) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  for (int i = 0; i < 10; ++i)
    DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  GetOutputBuffer().clear();
  for (int i = 0; i < 100; ++i)
    DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Use after free then buffer overflow attempts
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeThenBufferOverflow) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));

  GetOutputBuffer().clear();
  TouchMemory(Ptr + 16);
  TouchMemory(Ptr - 16);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// One error per slot - use after free variant
TEST_P(BacktraceGuardedPoolAllocator, OneErrorReportPerSlotUseAfterFree) {
  SCOPED_TRACE("");
  const size_t MaxSlots = GPA.getAllocatorState()->MaxSimultaneousAllocations;
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < MaxSlots; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
    TouchMemory(Ptr);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
    GetOutputBuffer().clear();
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
    GetOutputBuffer().clear();
  }

  for (void *Ptr : Ptrs) {
    TouchMemory(Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// One error per slot - invalid free variant
TEST_P(BacktraceGuardedPoolAllocator, OneErrorReportPerSlotInvalidFree) {
  SCOPED_TRACE("");
  const size_t MaxSlots = GPA.getAllocatorState()->MaxSimultaneousAllocations;
  std::vector<char *> Ptrs;
  for (size_t i = 0; i < MaxSlots; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr + 1);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
    GetOutputBuffer().clear();
    DeallocateMemory(GPA, Ptr + 1);
    ASSERT_TRUE(GetOutputBuffer().empty());
    GetOutputBuffer().clear();
  }

  for (char *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Mixed error types across slots
TEST_P(BacktraceGuardedPoolAllocator, MixedErrorTypesAcrossSlots) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  char *P3 = static_cast<char *>(AllocateMemory(GPA));
  void *P4 = AllocateMemory(GPA);

  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P2 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P3);
  TouchMemory(P3);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
  GetOutputBuffer().clear();

  TouchMemory(P4 + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_TRUE(GetOutputBuffer().find("Buffer Overflow") != std::string::npos ||
              GetOutputBuffer().find("Buffer Underflow") != std::string::npos);

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P4);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Stress: many double frees across many iterations after first report
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeStressNoDuplicateReports) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  constexpr int kStressIterations = 1000;
  for (int i = 0; i < kStressIterations; ++i) {
    DeallocateMemory(GPA, Ptr);
    if (!GetOutputBuffer().empty()) {
      FAIL() << "Unexpected report at iteration " << i;
    }
  }
}

// Stress: many use-after-frees after first report
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeStressNoDuplicateReports) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  constexpr int kStressIterations = 1000;
  for (int i = 0; i < kStressIterations; ++i) {
    TouchMemory(Ptr);
    if (!GetOutputBuffer().empty()) {
      FAIL() << "Unexpected report at iteration " << i;
    }
  }
}

// Inter-thread: double free from multiple threads
void doubleFreeFromThread(gwp_asan::GuardedPoolAllocator *GPA,
                         std::atomic<bool> *Start, void *Ptr, int NumIters) {
  while (!*Start) {}
  for (int i = 0; i < NumIters; ++i)
    DeallocateMemory(*GPA, Ptr);
}

TEST_P(BacktraceGuardedPoolAllocator, InterThreadDoubleFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 2) {
    GTEST_SKIP() << "Need at least 2 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::thread T1(doubleFreeFromThread, &GPA, &Start, Ptr, 5000);
  std::thread T2(doubleFreeFromThread, &GPA, &Start, Ptr, 5000);
  Start = true;
  T1.join();
  T2.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
}

// Inter-thread: use after free from multiple threads
void useAfterFreeFromThread(gwp_asan::GuardedPoolAllocator *GPA,
                            std::atomic<bool> *Start, void *Ptr, int NumIters) {
  while (!*Start) {}
  for (int i = 0; i < NumIters; ++i)
    TouchMemory(Ptr);
}

TEST_P(BacktraceGuardedPoolAllocator, InterThreadUseAfterFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 2) {
    GTEST_SKIP() << "Need at least 2 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::thread T1(useAfterFreeFromThread, &GPA, &Start, Ptr, 5000);
  std::thread T2(useAfterFreeFromThread, &GPA, &Start, Ptr, 5000);
  Start = true;
  T1.join();
  T2.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
}

// Verify output contains allocation info
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeOutputContainsAllocationInfo) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  const std::string &Out = GetOutputBuffer();
  ASSERT_NE(Out.find("GWP-ASan"), std::string::npos);
  ASSERT_NE(Out.find("Double Free"), std::string::npos);
  ASSERT_FALSE(Out.empty());
}

// Allocate, deallocate normally, then invalid free on same slot (different ptr)
TEST_P(BacktraceGuardedPoolAllocator, ValidFreeThenInvalidFreeOnFreedSlot) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  CheckGwpAsanErrorString(GetOutputBuffer(), "Invalid (Wild) Free");

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr + 1);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Buffer underflow with large offset
TEST_P(BacktraceGuardedPoolAllocator, BufferUnderflowLargeOffset) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr - 64);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_TRUE(GetOutputBuffer().find("Buffer Underflow") != std::string::npos ||
              GetOutputBuffer().find("GWP-ASan") != std::string::npos);

  GetOutputBuffer().clear();
  TouchMemory(Ptr - 64);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Inter-thread thrashing with higher iteration count
TEST_P(BacktraceGuardedPoolAllocator, InterThreadThrashingSingleAllocHighIters) {
  SCOPED_TRACE("");
  constexpr unsigned kNumIterations = 200000;
  if (std::thread::hardware_concurrency() < 4) {
    GTEST_SKIP() << "Not enough threads";
  }
  runInterThreadThrashingSingleAlloc(kNumIterations, &GPA);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Sequential slots - double free each in order
TEST_P(BacktraceGuardedPoolAllocator, SequentialSlotsDoubleFree) {
  SCOPED_TRACE("");
  constexpr size_t kNumSlots = 8;
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < kNumSlots; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
  }
  for (size_t i = 0; i < kNumSlots; ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
  }
  for (size_t i = 0; i < kNumSlots; ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
    GetOutputBuffer().clear();
  }
}

// Sequential slots - invalid free each in order
TEST_P(BacktraceGuardedPoolAllocator, SequentialSlotsInvalidFree) {
  SCOPED_TRACE("");
  constexpr size_t kNumSlots = 8;
  std::vector<char *> Ptrs;
  for (size_t i = 0; i < kNumSlots; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
  }
  for (size_t i = 0; i < kNumSlots; ++i) {
    DeallocateMemory(GPA, Ptrs[i] + (i % 4 + 1));
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
    GetOutputBuffer().clear();
  }
}

// Double free immediately after first deallocation
TEST_P(BacktraceGuardedPoolAllocator, ImmediateDoubleFree) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  GetOutputBuffer().clear();
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Use after free - read and write
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeReadWrite) {
  SCOPED_TRACE("");
  volatile char *Ptr = reinterpret_cast<volatile char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, const_cast<char *>(Ptr));
  *Ptr = 42;
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));

  GetOutputBuffer().clear();
  (void)*Ptr;
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Multiple invalid frees - same invalid pointer
TEST_P(BacktraceGuardedPoolAllocator, MultipleInvalidFreeSamePointer) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  char *InvalidPtr = Ptr + 1;
  DeallocateMemory(GPA, InvalidPtr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 100; ++i) {
    DeallocateMemory(GPA, InvalidPtr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Verify no crash on valid operations after error
TEST_P(BacktraceGuardedPoolAllocator, ValidOpsAfterDoubleFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  void *P3 = AllocateMemory(GPA);
  ASSERT_NE(P3, nullptr);
  DeallocateMemory(GPA, P3);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Slot exhaustion after errors
TEST_P(BacktraceGuardedPoolAllocator, SlotExhaustionAfterErrors) {
  SCOPED_TRACE("");
  const size_t MaxSlots = GPA.getAllocatorState()->MaxSimultaneousAllocations;
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < MaxSlots; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
    DeallocateMemory(GPA, Ptr);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }

  ASSERT_EQ(AllocateMemory(GPA), nullptr);
  for (void *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Alternating error types - double free and invalid free
TEST_P(BacktraceGuardedPoolAllocator, AlternatingDoubleFreeAndInvalidFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P2 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P2 + 1);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Recoverable mode: ensure process continues after error
TEST_P(BacktraceGuardedPoolAllocator, ProcessContinuesAfterError) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  void *NewPtr = AllocateMemory(GPA);
  ASSERT_NE(NewPtr, nullptr);
  DeallocateMemory(GPA, NewPtr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Single slot - full cycle: alloc, dealloc, double free, verify no more
TEST_P(BacktraceGuardedPoolAllocator, SingleSlotFullErrorCycle) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  ASSERT_NE(Ptr, nullptr);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();

  for (int i = 0; i < 10; ++i) {
    DeallocateMemory(GPA, Ptr);
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Two slots - first double free, second use after free
TEST_P(BacktraceGuardedPoolAllocator, TwoSlotsDifferentErrors) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P2);
  TouchMemory(P2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
  GetOutputBuffer().clear();

  DeallocateMemory(GPA, P1);
  TouchMemory(P2);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Many allocations, one double free
TEST_P(BacktraceGuardedPoolAllocator, ManyAllocationsOneDoubleFree) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < 16; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 4u);
  DeallocateMemory(GPA, Ptrs[2]);
  DeallocateMemory(GPA, Ptrs[2]);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));

  GetOutputBuffer().clear();
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    if (i != 2) DeallocateMemory(GPA, Ptrs[i]);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Output buffer growth check - single report
TEST_P(BacktraceGuardedPoolAllocator, SingleReportNonEmptyOutput) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  ASSERT_FALSE(GetOutputBuffer().empty());
  ASSERT_GT(GetOutputBuffer().size(), 10u);
}

// Repeated buffer overflow - same direction
TEST_P(BacktraceGuardedPoolAllocator, RepeatedBufferOverflowSameDirection) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 50; ++i) {
    TouchMemory(Ptr + 16);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Repeated buffer underflow - same direction
TEST_P(BacktraceGuardedPoolAllocator, RepeatedBufferUnderflowSameDirection) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr - 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());

  GetOutputBuffer().clear();
  for (size_t i = 0; i < 50; ++i) {
    TouchMemory(Ptr - 16);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Triple free - still only one report
TEST_P(BacktraceGuardedPoolAllocator, TripleFreeOnlyOneReport) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();
  for (int i = 0; i < 100; ++i)
    DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Quadruple invalid free - only one report
TEST_P(BacktraceGuardedPoolAllocator, QuadrupleInvalidFreeOnlyOneReport) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  for (int i = 0; i < 4; ++i)
    DeallocateMemory(GPA, Ptr + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr + 1);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Five slots with use-after-free each
TEST_P(BacktraceGuardedPoolAllocator, FiveSlotsUseAfterFree) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 5; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
  }
  for (int i = 0; i < 5; ++i) {
    TouchMemory(Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
    GetOutputBuffer().clear();
  }
  for (void *Ptr : Ptrs) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Six slots with buffer overflow each
TEST_P(BacktraceGuardedPoolAllocator, SixSlotsBufferOverflow) {
  SCOPED_TRACE("");
  std::vector<char *> Ptrs;
  for (int i = 0; i < 6; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
  }
  for (int i = 0; i < 6; ++i) {
    TouchMemory(Ptrs[i] + 16);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_TRUE(GetOutputBuffer().find("Buffer Overflow") != std::string::npos ||
                GetOutputBuffer().find("Buffer Underflow") != std::string::npos);
    GetOutputBuffer().clear();
  }
  for (char *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Inter-thread: 3 threads double free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadThreeThreadsDoubleFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 3) {
    GTEST_SKIP() << "Need at least 3 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::thread T1(doubleFreeFromThread, &GPA, &Start, Ptr, 3000);
  std::thread T2(doubleFreeFromThread, &GPA, &Start, Ptr, 3000);
  std::thread T3(doubleFreeFromThread, &GPA, &Start, Ptr, 3000);
  Start = true;
  T1.join();
  T2.join();
  T3.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Inter-thread: 4 threads use after free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadFourThreadsUseAfterFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 4) {
    GTEST_SKIP() << "Need at least 4 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::thread T1(useAfterFreeFromThread, &GPA, &Start, Ptr, 3000);
  std::thread T2(useAfterFreeFromThread, &GPA, &Start, Ptr, 3000);
  std::thread T3(useAfterFreeFromThread, &GPA, &Start, Ptr, 3000);
  std::thread T4(useAfterFreeFromThread, &GPA, &Start, Ptr, 3000);
  Start = true;
  T1.join();
  T2.join();
  T3.join();
  T4.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Chain: alloc P1, alloc P2, free P1, double free P1, free P2
TEST_P(BacktraceGuardedPoolAllocator, ChainAllocFreeDoubleFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Verify output contains "detected"
TEST_P(BacktraceGuardedPoolAllocator, OutputContainsDetected) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  ASSERT_NE(GetOutputBuffer().find("detected"), std::string::npos);
}

// Verify output contains "memory error"
TEST_P(BacktraceGuardedPoolAllocator, OutputContainsMemoryError) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 1);
  ASSERT_NE(GetOutputBuffer().find("memory error"), std::string::npos);
}

// Invalid free with offset 2
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOffset2) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Invalid free with offset 16
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOffset16) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Use-after-free then allocate new - verify new alloc works
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeThenNewAllocation) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  TouchMemory(P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  void *P2 = AllocateMemory(GPA);
  ASSERT_NE(P2, nullptr);
  DeallocateMemory(GPA, P2);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Double free then allocate - verify new alloc works
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeThenNewAllocation) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  void *P2 = AllocateMemory(GPA);
  ASSERT_NE(P2, nullptr);
  DeallocateMemory(GPA, P2);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Ten sequential double frees on different slots
TEST_P(BacktraceGuardedPoolAllocator, TenSequentialDoubleFrees) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 10; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 5u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
  }
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
}

// Buffer underflow offset 8
TEST_P(BacktraceGuardedPoolAllocator, BufferUnderflowOffset8) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr - 8);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(Ptr - 8);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Buffer overflow offset 32
TEST_P(BacktraceGuardedPoolAllocator, BufferOverflowOffset32) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 32);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(Ptr + 32);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Recoverable: no crash on double free
TEST_P(BacktraceGuardedPoolAllocator, RecoverableNoCrashDoubleFree) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  ASSERT_FALSE(GetOutputBuffer().empty());
}

// Recoverable: no crash on use after free
TEST_P(BacktraceGuardedPoolAllocator, RecoverableNoCrashUseAfterFree) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  ASSERT_FALSE(GetOutputBuffer().empty());
}

// Recoverable: no crash on invalid free
TEST_P(BacktraceGuardedPoolAllocator, RecoverableNoCrashInvalidFree) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 1);
  ASSERT_FALSE(GetOutputBuffer().empty());
}

// Recoverable: no crash on buffer overflow
TEST_P(BacktraceGuardedPoolAllocator, RecoverableNoCrashBufferOverflow) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 16);
  ASSERT_FALSE(GetOutputBuffer().empty());
}

// Multiple slots - alternating double free and use after free
TEST_P(BacktraceGuardedPoolAllocator, AlternatingDoubleFreeAndUseAfterFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  void *P3 = AllocateMemory(GPA);
  void *P4 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  TouchMemory(P2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3);
  DeallocateMemory(GPA, P3);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P4);
  TouchMemory(P4);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
}

// Stress: 2000 iterations no duplicate after double free
TEST_P(BacktraceGuardedPoolAllocator, Stress2000DoubleFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 2000; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Stress: 2000 iterations no duplicate after use after free
TEST_P(BacktraceGuardedPoolAllocator, Stress2000UseAfterFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 2000; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Seven slots - mix of double free and invalid free
TEST_P(BacktraceGuardedPoolAllocator, SevenSlotsMixedDoubleAndInvalidFree) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 7; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 4u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    if (i % 2 == 0) {
      DeallocateMemory(GPA, Ptrs[i]);
      DeallocateMemory(GPA, Ptrs[i]);
    } else {
      DeallocateMemory(GPA, static_cast<char *>(Ptrs[i]) + 1);
    }
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
}

// Inter-thread thrashing with 8 threads
void multiThrashTask(gwp_asan::GuardedPoolAllocator *GPA,
                    std::atomic<bool> *Start, void *Ptr, int NumIters, int Job) {
  while (!*Start) {}
  for (int i = 0; i < NumIters; ++i) {
    if (Job % 4 == 0)
      DeallocateMemory(*GPA, Ptr);
    else if (Job % 4 == 1)
      TouchMemory(Ptr);
    else if (Job % 4 == 2)
      TouchMemory(static_cast<char *>(Ptr) - 16);
    else
      TouchMemory(static_cast<char *>(Ptr) + 16);
  }
}

TEST_P(BacktraceGuardedPoolAllocator, InterThreadEightThreadsThrashing) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 8) {
    GTEST_SKIP() << "Need at least 8 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::vector<std::thread> Threads;
  for (int i = 0; i < 8; ++i) {
    Threads.emplace_back(multiThrashTask, &GPA, &Start, Ptr, 5000, i);
  }
  Start = true;
  for (auto &T : Threads)
    T.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// First slot double free, second slot buffer overflow
TEST_P(BacktraceGuardedPoolAllocator, FirstDoubleFreeSecondBufferOverflow) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();
  TouchMemory(P2 + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_TRUE(GetOutputBuffer().find("Buffer Overflow") != std::string::npos ||
              GetOutputBuffer().find("Buffer Underflow") != std::string::npos);
}

// Verify single GWP-ASan crash string in output
TEST_P(BacktraceGuardedPoolAllocator, SingleGwpAsanCrashString) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  const std::string &Out = GetOutputBuffer();
  size_t First = Out.find("GWP-ASan detected a memory error");
  ASSERT_NE(First, std::string::npos);
  ASSERT_EQ(Out.find("GWP-ASan detected a memory error", First + 1),
            std::string::npos);
}

// All slots used for double free - verify slot exhaustion
TEST_P(BacktraceGuardedPoolAllocator, AllSlotsDoubleFreeExhaustion) {
  SCOPED_TRACE("");
  const size_t MaxSlots = GPA.getAllocatorState()->MaxSimultaneousAllocations;
  std::vector<void *> Ptrs;
  for (size_t i = 0; i < MaxSlots; ++i) {
    void *Ptr = AllocateMemory(GPA);
    ASSERT_NE(Ptr, nullptr);
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
    DeallocateMemory(GPA, Ptr);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
  ASSERT_EQ(AllocateMemory(GPA), nullptr);
}

// Interleaved: alloc A, alloc B, free A, free B, double free A
TEST_P(BacktraceGuardedPoolAllocator, InterleavedDoubleFreeAfterBothFreed) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P2);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
}

// Buffer overflow then underflow on same allocation - second triggers no report
TEST_P(BacktraceGuardedPoolAllocator, BufferOverflowThenUnderflowSameAlloc) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(Ptr - 16);
  ASSERT_TRUE(GetOutputBuffer().empty());
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Three allocations, invalid free on all
TEST_P(BacktraceGuardedPoolAllocator, ThreeAllocsInvalidFreeAll) {
  SCOPED_TRACE("");
  char *P1 = static_cast<char *>(AllocateMemory(GPA));
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  char *P3 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2 + 2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3 + 3);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P2);
  DeallocateMemory(GPA, P3);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Output size reasonable
TEST_P(BacktraceGuardedPoolAllocator, OutputSizeReasonable) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  size_t Len = GetOutputBuffer().size();
  ASSERT_GT(Len, 50u);
  ASSERT_LT(Len, 100000u);
}

// Recursive-like: free, double free, triple free
TEST_P(BacktraceGuardedPoolAllocator, MultipleDoubleFreeRapid) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  for (int i = 0; i < 20; ++i)
    DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 100; ++i)
    DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Final sanity: recoverable mode allows continued execution
TEST_P(BacktraceGuardedPoolAllocator, RecoverableModeContinuedExecution) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 10; ++i) {
    void *P = AllocateMemory(GPA);
    if (P) {
      DeallocateMemory(GPA, P);
    }
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Verify that after a slot has crashed, no further errors are reported from it.
// This ensures the recoverable mode correctly marks slots as crashed and
// suppresses duplicate reports for the same allocation.
TEST_P(BacktraceGuardedPoolAllocator, CrashedSlotNoFurtherReports) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 500; ++i) {
    DeallocateMemory(GPA, Ptr);
    TouchMemory(Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Comprehensive test: all error types in sequence, each reported once
TEST_P(BacktraceGuardedPoolAllocator, AllErrorTypesSequential) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  char *P3 = static_cast<char *>(AllocateMemory(GPA));
  char *P4 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3);
  TouchMemory(P3);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
  GetOutputBuffer().clear();
  TouchMemory(P4 + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_TRUE(GetOutputBuffer().find("Buffer Overflow") != std::string::npos ||
              GetOutputBuffer().find("Buffer Underflow") != std::string::npos);
}

// Additional double free stress - 3000 iterations
TEST_P(BacktraceGuardedPoolAllocator, Stress3000DoubleFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 3000; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Additional use-after-free stress - 3000 iterations
TEST_P(BacktraceGuardedPoolAllocator, Stress3000UseAfterFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 3000; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Twelve slots double free in sequence
TEST_P(BacktraceGuardedPoolAllocator, TwelveSlotsDoubleFreeSequence) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 12; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 6u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
  }
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    DeallocateMemory(GPA, Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
}

// Fifteen slots invalid free with varying offsets
TEST_P(BacktraceGuardedPoolAllocator, FifteenSlotsInvalidFreeVaryingOffsets) {
  SCOPED_TRACE("");
  std::vector<char *> Ptrs;
  for (int i = 0; i < 15; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 5u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    DeallocateMemory(GPA, Ptrs[i] + (i % 8 + 1));
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Invalid (Wild) Free"));
    GetOutputBuffer().clear();
  }
}

// Inter-thread: 5 threads double free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadFiveThreadsDoubleFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 5) {
    GTEST_SKIP() << "Need at least 5 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::vector<std::thread> Threads;
  for (int i = 0; i < 5; ++i) {
    Threads.emplace_back(doubleFreeFromThread, &GPA, &Start, Ptr, 4000);
  }
  Start = true;
  for (auto &T : Threads)
    T.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Inter-thread: 6 threads use after free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadSixThreadsUseAfterFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 6) {
    GTEST_SKIP() << "Need at least 6 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::vector<std::thread> Threads;
  for (int i = 0; i < 6; ++i) {
    Threads.emplace_back(useAfterFreeFromThread, &GPA, &Start, Ptr, 4000);
  }
  Start = true;
  for (auto &T : Threads)
    T.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Invalid free offset 4
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOffset4) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 4);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Invalid free offset 8
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOffset8) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 8);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Invalid free offset 32
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOffset32) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 32);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Buffer underflow offset 32
TEST_P(BacktraceGuardedPoolAllocator, BufferUnderflowOffset32) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr - 32);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(Ptr - 32);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Buffer overflow offset 64
TEST_P(BacktraceGuardedPoolAllocator, BufferOverflowOffset64) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  TouchMemory(Ptr + 64);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(Ptr + 64);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Twenty rapid double frees - only one report
TEST_P(BacktraceGuardedPoolAllocator, TwentyRapidDoubleFrees) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  for (int i = 0; i < 20; ++i)
    DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Double Free"));
  GetOutputBuffer().clear();
  for (int i = 0; i < 100; ++i)
    DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Fifty rapid invalid frees - only one report
TEST_P(BacktraceGuardedPoolAllocator, FiftyRapidInvalidFrees) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  for (int i = 0; i < 50; ++i)
    DeallocateMemory(GPA, Ptr + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Fifty rapid use-after-frees - only one report
TEST_P(BacktraceGuardedPoolAllocator, FiftyRapidUseAfterFrees) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  for (int i = 0; i < 50; ++i)
    TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
  GetOutputBuffer().clear();
  for (int i = 0; i < 100; ++i)
    TouchMemory(Ptr);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Mixed: double free, then new allocation, then use after free on old
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeThenNewAllocThenUseAfterFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  void *P2 = AllocateMemory(GPA);
  ASSERT_NE(P2, nullptr);
  TouchMemory(P1);
  ASSERT_TRUE(GetOutputBuffer().empty());
  DeallocateMemory(GPA, P2);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Chain: alloc A, B, C; free A; double free A; free B; use after free B
TEST_P(BacktraceGuardedPoolAllocator, ChainDoubleFreeThenUseAfterFree) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  void *P2 = AllocateMemory(GPA);
  void *P3 = AllocateMemory(GPA);
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2);
  TouchMemory(P2);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3);
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Eight slots: alternating double free and invalid free
TEST_P(BacktraceGuardedPoolAllocator, EightSlotsAlternatingErrors) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 8; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 4u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    if (i % 2 == 0) {
      DeallocateMemory(GPA, Ptrs[i]);
      DeallocateMemory(GPA, Ptrs[i]);
    } else {
      DeallocateMemory(GPA, static_cast<char *>(Ptrs[i]) + 1);
    }
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
}

// Nine slots: double free, invalid free, use after free, repeat
TEST_P(BacktraceGuardedPoolAllocator, NineSlotsMixedErrorTypes) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 9; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 3u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    int Type = i % 3;
    if (Type == 0) {
      DeallocateMemory(GPA, Ptrs[i]);
      DeallocateMemory(GPA, Ptrs[i]);
    } else if (Type == 1) {
      DeallocateMemory(GPA, static_cast<char *>(Ptrs[i]) + 1);
    } else {
      DeallocateMemory(GPA, Ptrs[i]);
      TouchMemory(Ptrs[i]);
    }
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    GetOutputBuffer().clear();
  }
}

// Verify output contains "Free" for double free
TEST_P(BacktraceGuardedPoolAllocator, DoubleFreeOutputContainsFree) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  ASSERT_NE(GetOutputBuffer().find("Free"), std::string::npos);
}

// Verify output contains "Wild" for invalid free
TEST_P(BacktraceGuardedPoolAllocator, InvalidFreeOutputContainsWild) {
  SCOPED_TRACE("");
  char *Ptr = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, Ptr + 1);
  ASSERT_NE(GetOutputBuffer().find("Wild"), std::string::npos);
}

// Verify output contains "Use" for use after free
TEST_P(BacktraceGuardedPoolAllocator, UseAfterFreeOutputContainsUse) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  ASSERT_NE(GetOutputBuffer().find("Use"), std::string::npos);
}

// Stress: 5000 iterations no duplicate after double free
TEST_P(BacktraceGuardedPoolAllocator, Stress5000DoubleFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 5000; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Stress: 5000 iterations no duplicate after use after free
TEST_P(BacktraceGuardedPoolAllocator, Stress5000UseAfterFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 5000; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Inter-thread thrashing 250000 iterations
TEST_P(BacktraceGuardedPoolAllocator, InterThreadThrashing250000) {
  SCOPED_TRACE("");
  constexpr unsigned kNumIterations = 250000;
  if (std::thread::hardware_concurrency() < 4) {
    GTEST_SKIP() << "Not enough threads";
  }
  runInterThreadThrashingSingleAlloc(kNumIterations, &GPA);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Ten slots buffer overflow each
TEST_P(BacktraceGuardedPoolAllocator, TenSlotsBufferOverflow) {
  SCOPED_TRACE("");
  std::vector<char *> Ptrs;
  for (int i = 0; i < 10; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 5u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    TouchMemory(Ptrs[i] + 16);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_TRUE(GetOutputBuffer().find("Buffer Overflow") != std::string::npos ||
                GetOutputBuffer().find("Buffer Underflow") != std::string::npos);
    GetOutputBuffer().clear();
  }
  for (char *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Ten slots buffer underflow each
TEST_P(BacktraceGuardedPoolAllocator, TenSlotsBufferUnderflow) {
  SCOPED_TRACE("");
  std::vector<char *> Ptrs;
  for (int i = 0; i < 10; ++i) {
    char *Ptr = static_cast<char *>(AllocateMemory(GPA));
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
  }
  ASSERT_GE(Ptrs.size(), 5u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    TouchMemory(Ptrs[i] - 16);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Buffer Underflow"));
    GetOutputBuffer().clear();
  }
  for (char *Ptr : Ptrs) {
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_TRUE(GetOutputBuffer().empty());
}

// Fourteen slots use after free each
TEST_P(BacktraceGuardedPoolAllocator, FourteenSlotsUseAfterFree) {
  SCOPED_TRACE("");
  std::vector<void *> Ptrs;
  for (int i = 0; i < 14; ++i) {
    void *Ptr = AllocateMemory(GPA);
    if (!Ptr) break;
    Ptrs.push_back(Ptr);
    DeallocateMemory(GPA, Ptr);
  }
  ASSERT_GE(Ptrs.size(), 5u);
  for (size_t i = 0; i < Ptrs.size(); ++i) {
    TouchMemory(Ptrs[i]);
    CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
    ASSERT_NE(std::string::npos, GetOutputBuffer().find("Use After Free"));
    GetOutputBuffer().clear();
  }
  for (void *Ptr : Ptrs) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty());
  }
}

// Inter-thread: 7 threads double free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadSevenThreadsDoubleFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 7) {
    GTEST_SKIP() << "Need at least 7 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::vector<std::thread> Threads;
  for (int i = 0; i < 7; ++i) {
    Threads.emplace_back(doubleFreeFromThread, &GPA, &Start, Ptr, 3000);
  }
  Start = true;
  for (auto &T : Threads)
    T.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Inter-thread: 8 threads use after free
TEST_P(BacktraceGuardedPoolAllocator, InterThreadEightThreadsUseAfterFree) {
  SCOPED_TRACE("");
  if (std::thread::hardware_concurrency() < 8) {
    GTEST_SKIP() << "Need at least 8 threads";
  }
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  std::atomic<bool> Start{false};
  std::vector<std::thread> Threads;
  for (int i = 0; i < 8; ++i) {
    Threads.emplace_back(useAfterFreeFromThread, &GPA, &Start, Ptr, 3000);
  }
  Start = true;
  for (auto &T : Threads)
    T.join();
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

// Stress: 10000 iterations no duplicate after double free
TEST_P(BacktraceGuardedPoolAllocator, Stress10000DoubleFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  DeallocateMemory(GPA, Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 10000; ++i) {
    DeallocateMemory(GPA, Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Stress: 10000 iterations no duplicate after use after free
TEST_P(BacktraceGuardedPoolAllocator, Stress10000UseAfterFreeNoDuplicate) {
  SCOPED_TRACE("");
  void *Ptr = AllocateMemory(GPA);
  DeallocateMemory(GPA, Ptr);
  TouchMemory(Ptr);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  for (int i = 0; i < 10000; ++i) {
    TouchMemory(Ptr);
    ASSERT_TRUE(GetOutputBuffer().empty()) << "Iteration " << i;
  }
}

// Final comprehensive: all four error types, each slot reports once
TEST_P(BacktraceGuardedPoolAllocator, AllFourErrorTypesFourSlots) {
  SCOPED_TRACE("");
  void *P1 = AllocateMemory(GPA);
  char *P2 = static_cast<char *>(AllocateMemory(GPA));
  void *P3 = AllocateMemory(GPA);
  char *P4 = static_cast<char *>(AllocateMemory(GPA));
  DeallocateMemory(GPA, P1);
  DeallocateMemory(GPA, P1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P2 + 1);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  DeallocateMemory(GPA, P3);
  TouchMemory(P3);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
  GetOutputBuffer().clear();
  TouchMemory(P4 + 16);
  CheckOnlyOneGwpAsanCrash(GetOutputBuffer());
}

INSTANTIATE_TEST_SUITE_P(RecoverableTests, BacktraceGuardedPoolAllocator,
                         /* Recoverable */ testing::Values(true));
