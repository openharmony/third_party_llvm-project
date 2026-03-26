// HWAddressSanitizer Test Cases - Memory Corruption Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s
// ============================================================================
// Classification: Use-After-Free, Buffer Overflow, Double-Free tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sanitizer/hwasan_interface.h>

// ============================================================================
// SECTION 1: Use-After-Free Tests (UAF)
// ============================================================================

// Test: Basic heap use-after-free (READ)
// RUN: %clang_hwasan -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=READ
// READ: ERROR: HWAddressSanitizer: tag-mismatch
// READ: READ of size 1 at {{.*}} tags: {{.*}}/{{.*}} (ptr/mem)
// READ: Cause: use-after-free
// READ: is a small unallocated heap chunk
// READ: SUMMARY: HWAddressSanitizer: tag-mismatch
int test_uaf_read() {
  __hwasan_enable_allocator_tagging();
  char * volatile x = (char*)malloc(10);
  free(x);
  __hwasan_disable_allocator_tagging();
  return x[5];
}

// Test: Basic heap use-after-free (WRITE)
// RUN: %clang_hwasan -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=WRITE
// WRITE: ERROR: HWAddressSanitizer: tag-mismatch
// WRITE: WRITE of size 1 at {{.*}} tags: {{.*}}/{{.*}} (ptr/mem)
// WRITE: Cause: use-after-free
int test_uaf_write() {
  __hwasan_enable_allocator_tagging();
  char * volatile x = (char*)malloc(10);
  free(x);
  __hwasan_disable_allocator_tagging();
  x[5] = 42;
  return 0;
}

// Test: Use-after-free with different sizes
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=SIZE4
// SIZE4: ERROR: HWAddressSanitizer: tag-mismatch
// SIZE4: READ of size 4
// SIZE4: Cause: use-after-free
int test_uaf_size_4() {
  __hwasan_enable_allocator_tagging();
  int *p = (int*)malloc(16);
  free(p);
  int val = p[0];
  __hwasan_disable_allocator_tagging();
  return val;
}

// Test: Use-after-free at offset 0
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_OFF0
// UAF_OFF0: ERROR: HWAddressSanitizer: tag-mismatch
// UAF_OFF0: Cause: use-after-free
int test_uaf_offset_0() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  free(p);
  p[0] = 'A';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Use-after-free at last byte
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_OFF15
// UAF_OFF15: ERROR: HWAddressSanitizer: tag-mismatch
// UAF_OFF15: Cause: use-after-free
int test_uaf_offset_15() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  free(p);
  p[15] = 'Z';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 2: Double-Free Tests
// ============================================================================

// Test: Basic double-free
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=DOUBLE_FREE
// DOUBLE_FREE: ERROR: HWAddressSanitizer: invalid-free on address {{.*}}
// DOUBLE_FREE: tags: {{.*}}/{{.*}} (ptr/mem)
// DOUBLE_FREE: freed by thread {{.*}} here
// DOUBLE_FREE: previously allocated by thread {{.*}} here
int test_double_free() {
  __hwasan_enable_allocator_tagging();
  char * volatile x = (char*)malloc(40);
  free(x);
  free(x);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Double-free small allocation
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=DOUBLE_FREE_SMALL
// DOUBLE_FREE_SMALL: ERROR: HWAddressSanitizer: invalid-free
int test_double_free_small() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(8);
  free(p);
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 3: Heap Buffer Overflow
// ============================================================================

// Test: Heap buffer overflow - right side
// RUN: %clang_hwasan %s -o %t
// RUN: not %run %t 40 2>&1 | FileCheck %s --check-prefix=HEAP_BOF_RIGHT
// RUN: not %run %t 80 2>&1 | FileCheck %s --check-prefix=HEAP_BOF_RIGHT2
// HEAP_BOF_RIGHT: ERROR: HWAddressSanitizer: tag-mismatch
// HEAP_BOF_RIGHT: Cause: heap-buffer-overflow
// HEAP_BOF_RIGHT: is located {{[0-9]+}} bytes to the right of 30-byte region
// HEAP_BOF_RIGHT: allocated heap chunk
int test_heap_bof_right(int offset) {
  __hwasan_enable_allocator_tagging();
  int size = 30;
  char * volatile x = (char*)malloc(size);
  volatile char sink = x[offset];
  free(x);
  __hwasan_disable_allocator_tagging();
  return sink;
}

// Test: Heap buffer overflow - left side
// RUN: %clang_hwasan %s -o %t && not %run %t -30 2>&1 | FileCheck %s --check-prefix=HEAP_BOF_LEFT
// HEAP_BOF_LEFT: ERROR: HWAddressSanitizer: tag-mismatch
// HEAP_BOF_LEFT: Cause: heap-buffer-overflow
// HEAP_BOF_LEFT: is located 30 bytes to the left of 30-byte region
int test_heap_bof_left(int offset) {
  __hwasan_enable_allocator_tagging();
  int size = 30;
  char * volatile x = (char*)malloc(size);
  volatile char sink = x[offset];
  free(x);
  __hwasan_disable_allocator_tagging();
  return sink;
}

// Test: Heap buffer overflow at exact boundary
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=HEAP_BOF_BOUNDARY
// HEAP_BOF_BOUNDARY: ERROR: HWAddressSanitizer: tag-mismatch
// HEAP_BOF_BOUNDARY: Cause: heap-buffer-overflow
int test_heap_bof_boundary() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  p[16] = 'A';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Heap buffer overflow into next allocation
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=HEAP_BOF_INTO
// HEAP_BOF_INTO: ERROR: HWAddressSanitizer: tag-mismatch
// HEAP_BOF_INTO: Cause: heap-buffer-overflow
int test_heap_bof_into() {
  __hwasan_enable_allocator_tagging();
  char *x = (char*)malloc(32);
  char *y = (char*)malloc(32);
  x[33] = 'A';
  free(x);
  free(y);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: Stack Buffer Overflow
// ============================================================================

// Test: Stack buffer overflow
// RUN: %clang_hwasan -DSIZE=16 -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=STACK_OOB
// STACK_OOB: ERROR: HWAddressSanitizer: tag-mismatch
// STACK_OOB: Cause: stack tag-mismatch
// STACK_OOB: is located in stack of threa
// STACK_OOB: SUMMARY: HWAddressSanitizer: tag-mismatch
#ifndef SIZE
#define SIZE 16
#endif

__attribute__((noinline))
int test_stack_oob() {
  char z[SIZE];
  char *volatile p = z;
  return p[SIZE];
}

// Test: Stack underflow
// RUN: %clang_hwasan -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=STACK_UNDERFLOW
// STACK_UNDERFLOW: ERROR: HWAddressSanitizer: tag-mismatch
// STACK_UNDERFLOW: Cause: stack tag-mismatch
__attribute__((noinline))
int test_stack_underflow() {
  char z[16];
  char *volatile p = z;
  return p[-1];
}

// ============================================================================
// SECTION 5: Global Buffer Overflow
// ============================================================================

// Test: Global buffer overflow
// RUN: %clang_hwasan %s -o %t && not %run %t 1 2>&1 | FileCheck %s --check-prefix=GLOBAL_OOB
// GLOBAL_OOB: ERROR: HWAddressSanitizer: tag-mismatch
// GLOBAL_OOB: Cause: global-overflow
// GLOBAL_OOB: is located 0 bytes to the right of 4-byte global variable
int a_g = 1;
int x_g = 1;
int b_g = 1;

int test_global_bof(int index) {
  (&x_g)[index] = 1;
  return 0;
}

// Test: Global array overflow
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=

// ============================================================================
// SECTION 9: Large Allocation Tests
// ============================================================================

// Test: Large heap use-after-free
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_LARGE
// UAF_LARGE: ERROR: HWAddressSanitizer: tag-mismatch
// UAF_LARGE: Cause: use-after-free
int test_uaf_large() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(1000000);
  free(p);
  p[0] = 'A';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Large heap buffer overflow
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=HEAP_LARGE_OOF
// HEAP_LARGE_OOF: ERROR: HWAddressSanitizer: tag-mismatch
// HEAP_LARGE_OOF: Cause: heap-buffer-overflow
int test_heap_bof_large() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(1000000);
  p[1000000] = 'B';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 10: Multiple UAF Scenarios
// ============================================================================

// Test: Multiple UAF in sequence
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_SEQ
// UAF_SEQ: ERROR: HWAddressSanitizer: tag-mismatch
int test_uaf_sequence() {
  __hwasan_enable_allocator_tagging();
  for (int i = 0; i < 3; i++) {
    char *p = (char*)malloc(16);
    free(p);
    p[0] = 'A';
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: UAF with different types
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_TYPES
// UAF_TYPES: ERROR: HWAddressSanitizer: tag-mismatch
struct TestStructUAF { int a; double b; char c; };
int test_uaf_struct() {
  __hwasan_enable_allocator_tagging();
  TestStructUAF *p = (TestStructUAF*)malloc(sizeof(TestStructUAF));
  free(p);
  p->a = 42;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 11: Overflow Edge Cases
// ============================================================================

// Test: Overflow at offset 1
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=OOB_OFF1
// OOB_OFF1: ERROR: HWAddressSanitizer: tag-mismatch
int test_oob_offset_1() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  p[17] = 'A';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Overflow at offset 31
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=OOB_OFF31
// OOB_OFF31: ERROR: HWAddressSanitizer: tag-mismatch
int test_oob_offset_31() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(32);
  p[31] = 'B';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 12: Cross-Thread More Scenarios
// ============================================================================

// Test: Multiple threads accessing freed memory
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=MULTI_THREAD_UAF
// MULTI_THREAD_UAF: ERROR: HWAddressSanitizer: tag-mismatch
// MULTI_THREAD_UAF: in thread
char *volatile x_multi;
void *worker_multi(void *arg) {
  x_multi[0] = 'W';
  return NULL;
}
int test_multi_thread_uaf() {
  __hwasan_enable_allocator_tagging();
  x_multi = (char*)malloc(16);
  free(x_multi);
  pthread_t t;
  pthread_create(&t, NULL, worker_multi, NULL);
  pthread_join(t, NULL);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 13: Combined Vulnerabilities
// ============================================================================

// Test: UAF then overflow
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAF_THEN_OOB
// UAF_THEN_OOB: ERROR: HWAddressSanitizer: tag-mismatch
int test_uaf_then_oob() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  free(p);
  p[0] = 'A';
  p[17] = 'B';
  __hwasan_disable_allocator_tagging();
  return 0;
}
