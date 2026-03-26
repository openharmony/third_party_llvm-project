// HWAddressSanitizer Test Cases - Configuration Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: HWASan configuration, interface tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <sanitizer/hwasan_interface.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// SECTION 1: Interface Tests
// ============================================================================

// Test: Interface basic
// RUN: %clang_hwasan %s -o %t && %run %t
int test_interface() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(100);
  p[0] = 'A';
  p[99] = 'Z';
  free(p);
  __hwasan_disable_allocator_tagging();
  printf("Interface test passed\n");
  return 0;
}

// ============================================================================
// SECTION 2: Tagged Global
// ============================================================================

// Test: No-sanitize global
// RUN: %clang_hwasan %s -o %t && %run %t
__attribute__((no_sanitize("hwaddress"))) char global_buffer[64];
int test_tagged_global() {
  __hwasan_enable_allocator_tagging();
  global_buffer[0] = 'G';
  global_buffer[63] = 'Z';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 3: Memory Usage
// ============================================================================

// Test: Memory usage
// RUN: %clang_hwasan %s -o %t && %run %t
int test_memory_usage() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(1024);
  printf("Allocated: %p\n", (void*)p);
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: Size Tests
// ============================================================================

// Test: Various sizes
// RUN: %clang_hwasan %s -o %t && %run %t
int test_sizes() {
  __hwasan_enable_allocator_tagging();
  for (size_t s = 1; s <= 1024; s *= 2) {
    char *p = (char*)malloc(s);
    if (!p) return 1;
    p[0] = 'S';
    p[s-1] = 'E';
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 5: Border Address (Expected to fail)
// ============================================================================

// Test: Border address overflow
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=BORDER
// BORDER: ERROR: HWAddressSanitizer: tag-mismatch
// BORDER: Cause: heap-buffer-overflow
int test_border() {
  __hwasan_enable_allocator_tagging();
  char * volatile x = (char*)malloc(16);
  x[16] = 'A';
  free(x);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_interface();
  test_tagged_global();
  test_memory_usage();
  test_sizes();
  test_border();
  return 0;
}

// ============================================================================
// SECTION 6: More Configuration Tests
// ============================================================================

// Test: Multiple allocations with same size
// RUN: %clang_hwasan %s -o %t && %run %t
int test_multi_same_size() {
  __hwasan_enable_allocator_tagging();
  char *arr[100];
  for (int i = 0; i < 100; i++) {
    arr[i] = (char*)malloc(16);
    arr[i][0] = i;
  }
  for (int i = 0; i < 100; i++) {
    if (arr[i][0] != i) return 1;
    free(arr[i]);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Interleaved allocations
// RUN: %clang_hwasan %s -o %t && %run %t
int test_interleaved() {
  __hwasan_enable_allocator_tagging();
  char *a = (char*)malloc(16);
  char *b = (char*)malloc(32);
  char *c = (char*)malloc(64);
  a[0] = 'A';
  b[0] = 'B';
  c[0] = 'C';
  free(b);
  free(a);
  free(c);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Pattern fill
// RUN: %clang_hwasan %s -o %t && %run %t
int test_pattern_fill() {
  __hwasan_enable_allocator_tagging();
  for (int pattern = 0x00; pattern <= 0xFF; pattern += 0x55) {
    char *p = (char*)malloc(64);
    memset(p, pattern, 64);
    for (int i = 0; i < 64; i++) if (p[i] != (char)pattern) return 1;
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 7: Boundary Tests
// ============================================================================

// Test: Allocations at boundaries
// RUN: %clang_hwasan %s -o %t && %run %t
int test_boundary_alloc() {
  __hwasan_enable_allocator_tagging();
  for (size_t size = 1; size <= 256; size++) {
    char *p = (char*)malloc(size);
    if (!p) return 1;
    p[0] = 0;
    p[size-1] = 0;
    free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 8: Error Cases in Configuration
// ============================================================================

// Test: Double free config
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=CFG_DOUBLE_FREE
// CFG_DOUBLE_FREE: ERROR: HWAddressSanitizer: invalid-free
int test_config_double_free() {
  __hwasan_enable_allocator_tagging();
  char *p = (char*)malloc(16);
  free(p);
  free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Wild free config
// RUN: %clang_hwasan %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=CFG_WILD_FREE
// CFG_WILD_FREE: ERROR: HWAddressSanitizer: invalid-free
int test_config_wild_free() {
  char *p = (char*)0xDEADBEEF;
  free(p);
  return 0;
}
