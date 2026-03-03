// Check the heap_quarantine size correctly.
// RUN: %clangxx_hwasan %s -o %t
// RUN: %env_hwasan_opts=max_free_fill_size=256:heap_quarantine_max=1025:heap_quarantine_thread_max_count=16\
// RUN:                     not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK

#include <sanitizer/hwasan_interface.h>
#include <stdio.h>
#include <stdlib.h>

static const int kSize = 1 << 4;
static const int kQuarantineCount = 16;
static volatile unsigned long long *sink;
static unsigned long long *x;
static char *y;

void malloc_and_free() {
  for (int i = 0; i < kQuarantineCount; i++) {
    y = (char *)malloc(kSize);
    free(y);
  }
}

// Write the freed ptr in a non-hwasan function so that we don't detect the
// stores as OOB.
__attribute__((no_sanitize("hwaddress"))) int main(int argc, char **argv) {
  __hwasan_enable_allocator_tagging();
  x = (unsigned long long *)malloc(sizeof(unsigned long long));
  sink = x;
  free(x);
  *sink = 0x42;
  malloc_and_free();
  // CHECK: ==write_after_free_quarantine.cpp.tmp=={{.*}}==ERROR: memory_debug: use-after-free on address {{.*}} on thread {{.*}}
  // CHECK: memory was re-written after free at {{.*}}[0]: {{.*}} which filled: {{.*}}, expect: {{.*}}, freed by:
  // CHECK: {{.*}}in main {{.*}}write_after_free_quarantine.cpp:[[@LINE-5]]
  // CHECK: allocated by:
  // CHECK: {{.*}}in main {{.*}}write_after_free_quarantine.cpp:[[@LINE-9]]
  // CHECK: tags: {{.*}}/{{.*}} (ptr/mem)
  // CHECK: {{.*}}==Process memory map follows:
  // CHECK: {{.*}}==End of process memory map.
  return 0;
}