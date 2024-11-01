// RUN: %clang_tsan -O1 %s -o %t && %run %t 2>&1 | FileCheck %s
// UNSUPPORTED: tvos, watchos
// XFAIL: ios && !iossim
// OHOS_LOCAL
// ucontext.h not available on musl
// UNSUPPORTED: ohos_family
#include "sanitizer_common/sanitizer_ucontext.h"
#include "test.h"

char stack[64 * 1024] __attribute__((aligned(16)));

ucontext_t uc, orig_uc;
void *fiber, *orig_fiber;

int var;

void func() {
  var = 1;
  __tsan_switch_to_fiber(orig_fiber, 0);
  swapcontext(&uc, &orig_uc);
}

int main() {
  orig_fiber = __tsan_get_current_fiber();
  fiber = __tsan_create_fiber(0);
  getcontext(&uc);
  uc.uc_stack.ss_sp = stack;
  uc.uc_stack.ss_size = sizeof(stack);
  uc.uc_link = 0;
  makecontext(&uc, func, 0);
  var = 2;
  __tsan_switch_to_fiber(fiber, 0);
  swapcontext(&orig_uc, &uc);
  __tsan_destroy_fiber(fiber);
  fprintf(stderr, "PASS\n");
  return 0;
}

// CHECK-NOT: WARNING: ThreadSanitizer:
// CHECK: PASS
