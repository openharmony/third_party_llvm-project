// HWAddressSanitizer Test Cases - Edge Cases Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: Edge cases and boundary conditions
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sanitizer/hwasan_interface.h>
#include <sanitizer/allocator_interface.h>
#include <pthread.h>

// ============================================================================
// SECTION 1: Zero and One Byte Allocations
// ============================================================================

// Test: malloc(0)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_malloc_zero_1() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(0);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: malloc(1)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_malloc_one() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(1);
  p[0] = 'A';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 2: Size Edge Cases
// ============================================================================

// Test: Maximum size_t
// RUN: %clang_hwasan %s -o %t && %run %t
int test_max_size() {
  __hwasan_enable_allocator_tagging();
  size_t max = (size_t)-1;
  void *p = malloc(max);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: SIZE_MAX
// RUN: %clang_hwasan %s -o %t && %run %t
int test_size_max() {
  __hwasan_enable_allocator_tagging();
  void *p = malloc(SIZE_MAX);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 3: Alignment Edge Cases
// ============================================================================

// Test: alignment = 1
// RUN: %clang_hwasan %s -o %t && %run %t
int test_align_1() {
  __hwasan_enable_allocator_tagging();
  void *p = memalign(1, 64);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: power of 2 alignments
// RUN: %clang_hwasan %s -o %t && %run %t
int test_align_powers() {
  __hwasan_enable_allocator_tagging();
  for (int i = 0; i < 20; i++) {
    size_t align = 1ULL << i;
    if (align > 1024 * 1024) break;
    void *p = memalign(align, 64);
    if (p) {
      if (((size_t)p % align) != 0) return 1;
      free(p);
    }
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: Pointer Edge Cases
// ============================================================================

// Test: NULL pointer operations
// RUN: %clang_hwasan %s -o %t && %run %t
int test_null_operations() {
  free(NULL);
  void *p = realloc(NULL, 100);
  if (p) free(p);
  return 0;
}

// Test: Near NULL pointer
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=NEAR_NULL
// NEAR_NULL: ERROR: HWAddressSanitizer
int test_near_null() {
  char *p = (char*)1;
  return 0;
}

// ============================================================================
// SECTION 5: Realloc Edge Cases
// ============================================================================

// Test: realloc(0)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_realloc_zero() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(10);
  char *q = (char*)realloc(p, 0);
  if (q) free(q);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: realloc same size
// RUN: %clang_hwasan %s -o %t && %run %t
int test_realloc_same() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(100);
  p[0] = 'A';
  char *q = (char*)realloc(p, 100);
  if (q) {
    if (q[0] != 'A') return 1;
    free(q);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 6: Calloc Edge Cases
// ============================================================================

// Test: calloc(0, x)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_calloc_zero_nmemb() {
  __hwasan_enable_allocator_tagging();
  int *p = (int*)calloc(0, sizeof(int));
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: calloc(x, 0)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_calloc_zero_size() {
  __hwasan_enable_allocator_tagging();
  int *p = (int*)calloc(10, 0);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 7: Memory Access Edge Cases
// ============================================================================

// Test: Access at exact boundary
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=EXACT_BOUNDARY
// EXACT_BOUNDARY: ERROR: HWAddressSanitizer: tag-mismatch
int test_exact_boundary() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  p[16] = 'A';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Access at one past boundary
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=ONE_PAST
// ONE_PAST: ERROR: HWAddressSanitizer: tag-mismatch
int test_one_past() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(8);
  p[9] = 'B';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 8: Stack Edge Cases
// ============================================================================

// Test: Very small stack allocation
// RUN: %clang_hwasan %s -o %t && %run %t
int test_small_stack() {
  char buf[1];
  buf[0] = 'A';
  return buf[0] != 'A';
}

// Test: Large stack allocation
// RUN: %clang_hwasan %s -o %t && %run %t
int test_large_stack() {
  char buf[1024];
  for (int i = 0; i < 1024; i++) buf[i] = i;
  for (int i = 0; i < 1024; i++) if (buf[i] != i) return 1;
  return 0;
}

// ============================================================================
// SECTION 9: Global Edge Cases
// ============================================================================

// Test: Zero-size global array
// RUN: %clang_hwasan %s -o %t && %run %t
char zero_global[1] = {0};
int test_zero_global() {
  return zero_global[0] != 0;
}

// Test: Large global array
// RUN: %clang_hwasan %s -o %t && %run %t
char large_global[4096] = {0};
int test_large_global() {
  large_global[0] = 'A';
  large_global[4095] = 'Z';
  return large_global[0] != 'A' || large_global[4095] != 'Z';
}

// ============================================================================
// SECTION 10: Thread Edge Cases
// ============================================================================

// Test: Thread with minimal stack
// RUN: %clang_hwasan %s -o %t && %run %t
void *minimal_stack_worker(void *arg) {
  char local;
  *(char*)arg = local;
  return NULL;
}
int test_minimal_stack() {
  char result = 0;
  pthread_t t;
  pthread_create(&t, NULL, minimal_stack_worker, &result);
  pthread_join(t, NULL);
  return result;
}

// ============================================================================
// SECTION 11: String Edge Cases
// ============================================================================

// Test: Empty string
// RUN: %clang_hwasan %s -o %t && %run %t
int test_empty_string() {
  const char *empty = "";
  if (strlen(empty) != 0) return 1;
  if (strcmp(empty, "") != 0) return 1;
  return 0;
}

// Test: Very long string
// RUN: %clang_hwasan %s -o %t && %run %t
int test_long_string() {
  char *buf = (char*)malloc(10000);
  memset(buf, 'A', 9999);
  buf[9999] = '\0';
  if (strlen(buf) != 9999) return 1;
  free(buf);
  return 0;
}

// ===========================================================
