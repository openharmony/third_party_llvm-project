// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s

#include <sanitizer/hwasan_interface.h>
#include <stdlib.h>

int main() {
  __hwasan_enable_allocator_tagging();
  char *p = (char *)malloc(1);
  free(p + 0x10000000000);
  // OHOS_LOCAL
  // CHECK: ERROR: HWAddressSanitizer: invalid-free on address {{.*}} at pc {{[0x]+}}[[PC:.*]] on thread {{.*}}
  // CHECK: #0 {{[0x]+}}{{.*}}[[PC]] in {{.*}}free
  // CHECK: #1 {{.*}} in main {{.*}}wild-free.c:[[@LINE-4]]
  // CHECK-NOT: Segmentation fault
  // CHECK-NOT: SIGSEGV
  return 0;
}
