// RUN: %clangxx_tsan %s -o %t && %run %t 2>&1 | FileCheck %s

// REQUIRES: ohos_family

#include <mutex>
#include <thread>

static int global = 0;
static std::once_flag flag;

static void thread_func() {
  std::call_once(flag, [] { global = 1; });
  fprintf(stderr, "global = %d.\n", global);
}

int main() {
  fprintf(stderr, "Begin.\n");
  std::thread t1(thread_func);
  std::thread t2(thread_func);
  t1.join();
  t2.join();
  fprintf(stderr, "End.\n");
}

// CHECK: Begin.
// CHECK-NOT: WARNING: ThreadSanitizer
// CHECK: End.
