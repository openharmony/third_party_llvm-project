// HWAddressSanitizer Test Cases - Scope Issues Category
// ============================================================================
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope -O1 %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s
// ============================================================================
// Classification: Use-After-Scope tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/
// REQUIRES: aarch64-target-arch

#include <sanitizer/hwasan_interface.h>
#include <setjmp.h>
#include <functional>

// ============================================================================
// SECTION 1: Basic Use-After-Scope
// ============================================================================

// Test: Basic use-after-scope
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS
// UAS: ERROR: HWAddressSanitizer: tag-mismatch
// UAS: #0 {{.*}} in main
// UAS: Cause: stack tag-mismatch
volatile int *p_uas = 0;

int test_uas_basic() {
  __hwasan_enable_allocator_tagging();
  { int x = 0; p_uas = &x; }
  *p_uas = 5;
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 2: Control Flow Scope Issues
// ============================================================================

// Test: Use-after-scope in if
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_IF
// UAS_IF: ERROR: HWAddressSanitizer: tag-mismatch
// UAS_IF: Cause: stack tag-mismatch
volatile int *if_p = 0;

int test_uas_if() {
  if (1) { int x = 0; if_p = &x; }
  *if_p = 42;
  return 0;
}

// Test: Use-after-scope in loop
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_LOOP
// UAS_LOOP: ERROR: HWAddressSanitizer: tag-mismatch
// UAS_LOOP: Cause: stack tag-mismatch
volatile int *loop_p = 0;

int test_uas_loop() {
  for (int i = 0; i < 1; i++) { int x = i; loop_p = &x; }
  *loop_p = 42;
  return 0;
}

// ============================================================================
// SECTION 3: Lambda Capture
// ============================================================================

// Test: Lambda capture
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_CAPTURE
// UAS_CAPTURE: ERROR: HWAddressSanitizer: tag-mismatch
// UAS_CAPTURE: Cause: stack tag-mismatch
std::function<int()> capture_func;

int test_uas_capture() {
  { int x = 42; capture_func = [&]() { return x; }; }
  return capture_func();
}

// ============================================================================
// SECTION 4: Stack Use-After-Return
// ============================================================================

// Test: Stack UAR
// RUN: %clang_hwasan -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=STACK_UAR
// STACK_UAR: ERROR: HWAddressSanitizer: tag-mismatch
// STACK_UAR: Cause: stack tag-mismatch
char* f_uar() { char buf[16]; return buf; }

int test_stack_uar() {
  char *p = f_uar();
  *p = 'A';
  return 0;
}

// Test: Stack UAR with alloca
// RUN: %clang_hwasan -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=STACK_UAR_DYN
// STACK_UAR_DYN: ERROR: HWAddressSanitizer: tag-mismatch
#include <alloca.h>
char* f_uar_dynamic() { char *buf = (char*)alloca(16); return buf; }

int test_stack_uar_dynamic() {
  char *p = f_uar_dynamic();
  *p = 'A';
  return 0;
}

// ============================================================================
// SECTION 5: Stack Out-of-Bounds
// ============================================================================

// Test: Stack OOB
// RUN: %clang_hwasan -DSIZE=16 -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=STACK_OOB
// STACK_OOB: ERROR: HWAddressSanitizer: tag-mismatch
// STACK_OOB: Cause: stack tag-mismatch
#ifndef SIZE
#define SIZE 16
#endif
__attribute__((noinline))
int f_stack_oob() {
  char z[SIZE];
  char *volatile p = z;
  return p[SIZE];
}

int test_stack_oob() {
  return f_stack_oob();
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_uas_basic();
  test_uas_if();
  test_uas_loop();
  test_uas_capture();
  test_stack_uar();
  test_stack_uar_dynamic();
  test_stack_oob();
  return 0;
}

// ============================================================================
// SECTION 7: More Scope Tests
// ============================================================================

// Test: Nested scope
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_NESTED
// UAS_NESTED: ERROR: HWAddressSanitizer: tag-mismatch
volatile int *nested_p = 0;
int test_uas_nested() {
  { 
    { 
      int x = 42; 
      nested_p = &x; 
    } 
  }
  *nested_p = 0;
  return 0;
}

// Test: Scope with struct
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_STRUCT
// UAS_STRUCT: ERROR: HWAddressSanitizer: tag-mismatch
struct InnerScope { int x; };
volatile InnerScope *struct_p = 0;
int test_uas_struct() {
  { InnerScope s; s.x = 42; struct_p = &s; }
  struct_p->x = 0;
  return 0;
}

// Test: Scope with array
// RUN: %clangxx_hwasan -mllvm -hwasan-use-after-scope %s -o %t && not %run %t 2>&1 | FileCheck %s --check-prefix=UAS_ARRAY
// UAS_ARRAY: ERROR: HWAddressSanitizer: tag-mismatch
volatile char *array_p = 0;
int test_uas_array() {
  { char arr[32]; arr[0] = 'A'; array_p = arr; }
  array_p[0] = 'B';
  return 0;
}
