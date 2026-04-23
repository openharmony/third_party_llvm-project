// HWAddressSanitizer Test Cases - Memory Allocation Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: malloc, free, realloc, new/delete tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sanitizer/hwasan_interface.h>
#include <sanitizer/allocator_interface.h>
#include <stdio.h>

// ============================================================================
// SECTION 1: Basic malloc Tests
// ============================================================================

// Test: malloc basic functionality
// RUN: %clang_hwasan %s -o %t && %run %t
int test_malloc_basic() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(100);
  p[0] = 'A';
  p[99] = 'Z';
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: malloc with get_allocated_size
// RUN: %clang_hwasan %s -o %t && %run %t
int test_malloc_allocated_size() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(100);
  size_t size = __sanitizer_get_allocated_size(p);
  if (size < 100) return 1;
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: malloc zero size
// RUN: %clang_hwasan %s -o %t && %run %t
int test_malloc_zero() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(0);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 2: Memory Alignment Tests
// ============================================================================

// Test: memalign
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memalign() {
  __hwasan_enable_allocator_tagging();
  for (int i = 0; i < 8; i++) {
    size_t alignment = 16 << i;
    void *p = memalign(alignment, 256);
    if (p) {
      assert(((size_t)p % alignment) == 0);
      free(p);
    }
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: aligned_alloc
// RUN: %clang_hwasan %s -o %t && %run %t
int test_aligned_alloc() {
  __hwasan_enable_allocator_tagging();
  for (int a = 16; a <= 256; a *= 2) {
    void *p = aligned_alloc(a, a * 2);
    if (p) free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: posix_memalign
// RUN: %clang_hwasan %s -o %t && %run %t
int test_posix_memalign() {
  __hwasan_enable_allocator_tagging();
  for (int a = 16; a <= 256; a *= 2) {
    void *p = NULL;
    int ret = posix_memalign(&p, a, a * 2);
    if (ret == 0 && p) free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 3: Calloc Tests
// ============================================================================

// Test: calloc
// RUN: %clang_hwasan %s -o %t && %run %t
int test_calloc() {
  __hwasan_enable_allocator_tagging();
  int *p = (int*)calloc(10, sizeof(int));
  for (int i = 0; i < 10; i++) if (p[i] != 0) return 1;
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: Realloc Tests
// ============================================================================

// Test: realloc basic
// RUN: %clang_hwasan %s -o %t && %run %t
int test_realloc_basic() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(10);
  for (int i = 0; i < 10; i++) p[i] = i;
  char *q = (char*)realloc(p, 20);
  if (!q) { free(p); return 1; }
  for (int i = 0; i < 10; i++) if (q[i] != i) return 1;
  free(q);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: realloc NULL
// RUN: %clang_hwasan %s -o %t && %run %t
int test_realloc_null() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)realloc(NULL, 10);
  if (!p) return 1;
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 5: New/Delete Tests
// ============================================================================

#include <new>

// Test: new/delete
// RUN: %clangxx_hwasan %s -o %t && %run %t
int test_new_delete() {
  __hwasan_enable_allocator_tagging();
  int *p = new int(42);
  if (*p != 42) return 1;
  delete p;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: new[]/delete[]
// RUN: %clangxx_hwasan %s -o %t && %run %t
int test_new_array() {
  __hwasan_enable_allocator_tagging();
  int *arr = new int[10];
  for (int i = 0; i < 10; i++) arr[i] = i;
  for (int i = 0; i < 10; i++) if (arr[i] != i) return 1;
  delete[] arr;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 6: Memory Intrinsics
// ============================================================================

// Test: memcpy
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memcpy() {
  __hwasan_enable_allocator_tagging();
  char src[64], dst[64];
  memset(src, 'A', 64);
  memcpy(dst, src, 64);
  if (memcmp(src, dst, 64) != 0) return 1;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: memmove
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memmove() {
  __hwasan_enable_allocator_tagging();
  char buf[64] = {0};
  memmove(buf + 16, buf, 32);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: memset
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memset() {
  __hwasan_enable_allocator_tagging();
  char buf[64];
  memset(buf, 0xAB, 64);
  for (int i = 0; i < 64; i++) if (buf[i] != (char)0xAB) return 1;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: zero-size operations
// RUN: %clang_hwasan %s -o %t && %run %t
int test_zero_size() {
  __hwasan_enable_allocator_tagging();
  char buf[64] = {0};
  memcpy(buf, buf, 0);
  memmove(buf, buf, 0);
  memset(buf, 0, 0);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 7: Various Sizes
// ============================================================================

// Test: various allocation sizes
// RUN: %clang_hwasan %s -o %t && %run %t
int test_sizes() {
  __hwasan_enable_allocator_tagging();
  for (size_t size = 1; size <= 1024; size *= 2) {
    char *p = (char*)malloc(size);
    if (!p) return 1;
    p[0] = 'S';
    p[size - 1] = 'E';
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_malloc_basic();
  test_malloc_allocated_size();
  test_malloc_zero();
  test_memalign();
  test_aligned_alloc();
  test_posix_memalign();
  test_calloc();
  test_realloc_basic();
  test_realloc_null();
  test_new_delete();
  test_new_array();
  test_memcpy();
  test_memmove();
  test_memset();
  test_zero_size();
  test_sizes();
  printf("All allocation tests passed\n");
  return 0;
}

// ============================================================================
// SECTION 8: Edge Cases
// ============================================================================

// Test: Large allocation
// RUN: %clang_hwasan %s -o %t && %run %t
int test_large_alloc() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(10 * 1024 * 1024);
  if (p) {
    p[0] = 'L';
    p[10 * 1024 * 1024 - 1] = 'Z';
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Many small allocations
// RUN: %clang_hwasan %s -o %t && %run %t
int test_many_small() {
  __hwasan_enable_allocator_tagging();
  char *arr[1000];
  for (int i = 0; i < 1000; i++) {
    arr[i] = (char*)malloc(16);
    arr[i][0] = i;
  }
  for (int i = 0; i < 1000; i++) free(arr[i]);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Realloc to zero
// RUN: %clang_hwasan %s -o %t && %run %t
int test_realloc_zero() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(10);
  char *q = (char*)realloc(p, 0);
  if (q) free(q);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Memalign with various alignments
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memalign_various() {
  __hwasan_enable_allocator_tagging();
  for (int i = 4; i <= 256; i *= 2) {
    void *p = memalign(i, 128);
    if (p) {
      assert(((size_t)p % i) == 0);
      free(p);
    }
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 9: Memory Patterns
// ============================================================================

// Test: Allocation patterns
// RUN: %clang_hwasan %s -o %t && %run %t
int test_alloc_patterns() {
  __hwasan_enable_allocator_tagging();
  for (int pattern = 0; pattern < 3; pattern++) {
    for (int size = 1; size <= 64; size *= 2) {
      char *p = (char*)malloc(size);
      if (!p) return 1;
      for (int i = 0; i < size; i++) p[i] = pattern;
      for (int i = 0; i < size; i++) if (p[i] != pattern) return 1;
      free(p);
    }
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Repeated alloc/free
// RUN: %clang_hwasan %s -o %t && %run %t
int test_repeated_alloc() {
  __hwasan_enable_allocator_tagging();
  for (int i = 0; i < 1000; i++) {
    char *p = (char*)malloc(64);
    p[0] = i;
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}
