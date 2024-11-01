// RUN: %clangxx_tsan -O0 %s -o %t
// RUN: not %run %t          2>&1 | FileCheck %s --check-prefix=CHECK-RACE
// Test size larger than clear_shadow_mmap_threshold, which is handled differently.
// RUN: not %run %t - 262144 2>&1 | FileCheck %s --check-prefix=CHECK-RACE
// RUN:     %run %t ignore   2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE

#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <atomic>

#include "test.h"

// Use atomic to ensure we do not have a race for the pointer value itself.  We
// only want to check races in the mmap'd memory to isolate the test that mmap
// respects ignore annotations.
std::atomic<int*> global_p;

void mmap_ignored(bool ignore, size_t size) {
  if (ignore) AnnotateIgnoreWritesBegin(__FILE__, __LINE__);
  void *p =
      mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (ignore) AnnotateIgnoreWritesEnd(__FILE__, __LINE__);

  // Use relaxed to retain the race between the mmap call and the memory write
  global_p.store((int *)p, std::memory_order_relaxed);
  barrier_wait(&barrier);
}

void *WriteToMemory(void *unused) {
  barrier_wait(&barrier);
  global_p[0] = 7;
  return 0;
}

// Create race between allocating (mmap) and writing memory
int main(int argc, const char *argv[]) {
  bool ignore = (argc > 1) && (strcmp(argv[1], "ignore") == 0);
  size_t size = argc > 2 ? atoi(argv[2]) : sysconf(_SC_PAGESIZE);

  barrier_init(&barrier, 2);
  pthread_t t;
  pthread_create(&t, 0, WriteToMemory, 0);
  mmap_ignored(ignore, size);
  pthread_join(t, 0);

  assert(global_p[0] == 7);
  printf("OK\n");
  return 0;
}

// OHOS_LOCAL begin
// CHECK-RACE-DAG: WARNING: ThreadSanitizer: data race
// CHECK-RACE-DAG: OK
// CHECK-IGNORE-NOT: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE: OK
// CHECK-IGNORE-NOT: WARNING: ThreadSanitizer: data race
// OHOS_LOCAL end
