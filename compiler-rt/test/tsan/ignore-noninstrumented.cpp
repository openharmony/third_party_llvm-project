// Check that ignore_noninstrumented_modules=1 suppresses reports originating
// from interceptors that are called from an uninstrumented library.

// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir
// RUN: %clangxx_tsan %s -fPIC -shared -DLIBRARY -fno-sanitize=thread -o %t-dir/libignore_noninstrumented.so
// RUN: %clangxx_tsan %s -L%t-dir -lignore_noninstrumented -o %t

// Without the flag there are false positives.
// RUN: env LD_LIBRARY_PATH=%t-dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} %env_tsan_opts=ignore_noninstrumented_modules=0 %deflake %run %t 2>&1 | FileCheck %s --check-prefix=CHECK-RACE
// With the flag no races from the uninstrumented library are reported.
// RUN: env LD_LIBRARY_PATH=%t-dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} %env_tsan_opts=ignore_noninstrumented_modules=1 %run %t 2>&1 | FileCheck %s --implicit-check-not='ThreadSanitizer'
// Races in instrumented user code are still reported.
// RUN: env LD_LIBRARY_PATH=%t-dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} %env_tsan_opts=ignore_noninstrumented_modules=1 %deflake %run %t race 2>&1 | FileCheck %s --check-prefix=CHECK-RACE

// REQUIRES: ohos_family

#include "test.h"

#include <cstring>

#ifdef LIBRARY
namespace library {
#endif
char global_buf[64];

void *Thread1(void *arg) {
  auto barrier_wait = (void (*)())arg;
  barrier_wait();
  strcpy(global_buf, "hello world"); // NOLINT
  return nullptr;
}

void *Thread2(void *arg) {
  auto barrier_wait = (void (*)())arg;
  strcpy(global_buf, "world hello"); // NOLINT
  barrier_wait();
  return nullptr;
}

void Race(void (*barrier_wait)()) {
  pthread_t t[2];
  pthread_create(&t[0], nullptr, Thread1, (void *)barrier_wait);
  pthread_create(&t[1], nullptr, Thread2, (void *)barrier_wait);
  pthread_join(t[0], nullptr);
  pthread_join(t[1], nullptr);
}
#ifdef LIBRARY
} // namespace library
#endif

#ifndef LIBRARY
namespace library {
void Race(void (*barrier_wait)());
}

// Pass this function to the uninstrumented library so it can use TSan-invisible
// barriers.
void my_barrier_wait() { barrier_wait(&barrier); }

int main(int argc, char *argv[]) {
  fprintf(stderr, "Hello world.\n");

  barrier_init(&barrier, 2);
  library::Race(my_barrier_wait);

  if (argc > 1 && strcmp(argv[1], "race") == 0) {
    barrier_init(&barrier, 2);
    Race(my_barrier_wait);
  }

  fprintf(stderr, "Done.\n");
}

#endif // LIBRARY

// CHECK: Hello world.
// CHECK-RACE: SUMMARY: ThreadSanitizer: data race
// CHECK: Done.
