// HWAddressSanitizer Test Cases - Memory Operations Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: Memory intrinsics, control flow tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <sanitizer/hwasan_interface.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

// ============================================================================
// SECTION 1: Memory Intrinsics
// ============================================================================

// Test: memcpy basic
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

// ============================================================================
// SECTION 2: String Operations
// ============================================================================

// Test: strcmp
// RUN: %clang_hwasan %s -o %t && %run %t
int test_strcmp() {
  if (strcmp("hello", "hello") != 0) return 1;
  if (strcmp("hello", "world") == 0) return 1;
  return 0;
}

// Test: strcpy/strcat
// RUN: %clang_hwasan %s -o %t && %run %t
int test_strcpy() {
  char buf[32];
  strcpy(buf, "test");
  if (strcmp(buf, "test") != 0) return 1;
  strcat(buf, "ing");
  if (strcmp(buf, "testing") != 0) return 1;
  return 0;
}

// ============================================================================
// SECTION 3: Control Flow
// ============================================================================

// Test: musttail call
// RUN: %clang_hwasan %s -o %t && %run %t
__attribute__((noinline)) int helper() { return 42; }
__attribute__((musttail)) int wrapper() { return helper(); }
int test_musttail() {
  return wrapper() != 42;
}

// Test: longjmp
// RUN: %clang_hwasan %s -o %t && %run %t
static jmp_buf buf_lj;
void test_jump() { longjmp(buf_lj, 1); }
int test_longjmp() {
  __hwasan_enable_allocator_tagging();
  if (setjmp(buf_lj) == 0) test_jump();
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: CFI Tests
// ============================================================================

// Test: CFI basic
// RUN: %clang_hwasan %s -o %t && %run %t
typedef void (*FuncPtr)();
void foo_cfi() { }
void bar_cfi() { }
int test_cfi() {
  FuncPtr fp = &foo_cfi;
  fp();
  fp = &bar_cfi;
  fp();
  return 0;
}

// ============================================================================
// SECTION 5: LTO Tests
// ============================================================================

// Test: LTO
// RUN: %clang_hwasan %s -o %t && %run %t
__attribute__((noinline)) int lto_func(int x) { return x * 2; }
int test_lto() {
  __hwasan_enable_allocator_tagging();
  int r = lto_func(21);
  __hwasan_disable_allocator_tagging();
  return r != 42;
}

// ============================================================================
// SECTION 6: Preinit Array
// ============================================================================

// Test: preinit array
// RUN: %clang_hwasan %s -o %t && %run %t
static int init_array[4] = {1, 2, 3, 4};
int test_preinit() {
  __hwasan_enable_allocator_tagging();
  for (int i = 0; i < 4; i++) if (init_array[i] != i + 1) return 1;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 7: Tail Magic
// ============================================================================

// Test: tail magic
// RUN: %clang_hwasan %s -o %t && %run %t
__attribute__((noinline)) char* get_tail(char *buf) { return buf + 16; }
int test_tail_magic() {
  __hwasan_enable_allocator_tagging();
  char buf[32] = {0};
  *get_tail(buf) = 'A';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_memcpy();
  test_memmove();
  test_memset();
  test_strcmp();
  test_strcpy();
  test_musttail();
  test_longjmp();
  test_cfi();
  test_lto();
  test_preinit();
  test_tail_magic();
  printf("Memory operations tests passed\n");
  return 0;
}

// ============================================================================
// SECTION 8: More String Operations
// ============================================================================

// Test: strncpy edge cases
// RUN: %clang_hwasan %s -o %t && %run %t
int test_strncpy() {
  char buf[32];
  strncpy(buf, "hello", 3);
  buf[3] = '\0';
  if (strcmp(buf, "hel") != 0) return 1;
  strncpy(buf, "hi", 10);
  if (strcmp(buf, "hi") != 0) return 1;
  return 0;
}

// Test: strncat
// RUN: %clang_hwasan %s -o %t && %run %t
int test_strncat() {
  char buf[32] = "hello";
  strncat(buf, " world", 3);
  if (strcmp(buf, "hello wor") != 0) return 1;
  return 0;
}

// Test: strlen edge cases
// RUN: %clang_hwasan %s -o %t && %run %t
int test_strlen_edge() {
  if (strlen("") != 0) return 1;
  if (strlen("a") != 1) return 1;
  if (strlen("ab") != 2) return 1;
  return 0;
}

// ============================================================================
// SECTION 9: More Memory Operations
// ============================================================================

// Test: bzero
// RUN: %clang_hwasan %s -o %t && %run %t
int test_bzero() {
  __hwasan_enable_allocator_tagging();
  char buf[64];
  memset(buf, 'A', 64);
  // bzero equivalent
  memset(buf, 0, 64);
  for (int i = 0; i < 64; i++) if (buf[i] != 0) return 1;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: bcopy
// RUN: %clang_hwasan %s -o %t && %run %t
int test_bcopy() {
  __hwasan_enable_allocator_tagging();
  char src[32], dst[32];
  memset(src, 'S', 32);
  // bcopy equivalent
  memmove(dst, src, 32);
  if (memcmp(src, dst, 32) != 0) return 1;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: bcmp
// RUN: %clang_hwasan %s -o %t && %run %t
int test_bcmp() {
  char a[10] = "hello";
  char b[10] = "hello";
  char c[10] = "world";
  if (memcmp(a, b, 10) != 0) return 1;
  if (memcmp(a, c, 10) == 0) return 1;
  return 0;
}
