// HWAddressSanitizer Test Cases - Large/Complex Tests Category
// ============================================================================
// RUN: %clangxx_hwasan -O0 -g %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s
// ============================================================================
// Classification: Large complex test scenarios
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/
// UNSUPPORTED: *
// REQUIRES: shadow-scale-3

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <sanitizer/hwasan_interface.h>

// ============================================================================
// SECTION 1: Heap Use-After-Free (Large)
// ============================================================================

// Test: Heap use-after-free with complex call graph
// RUN: %clangxx_hwasan -O0 -g %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=HEAP_UAF
// HEAP_UAF: ERROR: HWAddressSanitizer: heap-use-after-free
// HEAP_UAF: READ of size 1
// HEAP_UAF: in final_observation
// HEAP_UAF: freed by thread
// HEAP_UAF: previously allocated by thread
// HEAP_UAF: SUMMARY: HWAddressSanitizer: heap-use-after-free

namespace {

struct BufferView {
  char *ptr;
  size_t size;
  size_t cursor;
};

struct Ledger {
  uint64_t x;
  uint64_t y;
  uint64_t z;
  uint64_t w;
};

static volatile uint64_t g_sink = 0;

static inline uint64_t mix64(uint64_t v) {
  v ^= v >> 33;
  v *= 0xff51afd7ed558ccdULL;
  v ^= v >> 33;
  v *= 0xc4ceb9fe1a85ec53ULL;
  v ^= v >> 33;
  return v;
}

static inline size_t ring(size_t i, size_t n) {
  return n == 0 ? 0 : (i % n);
}

static void prime_buffer(BufferView &view) {
  for (size_t i = 0; i < view.size; ++i)
    view.ptr[i] = static_cast<char>((i * 13u + 29u) & 0x7f);
}

static void seed_ledger(Ledger &ledger, size_t n) {
  ledger.x = mix64(0x1111u + n * 3u);
  ledger.y = mix64(0x2222u + n * 3u);
  ledger.z = mix64(0x3333u + n * 5u);
  ledger.w = mix64(0x4444u + n * 7u);
}

static void absorb(Ledger &ledger, unsigned char byte, size_t pos) {
  ledger.x ^= mix64(static_cast<uint64_t>(byte) + pos + ledger.w);
  ledger.y += mix64(static_cast<uint64_t>(byte) * 3u + ledger.x);
  ledger.z ^= ledger.y >> ((pos % 11u) + 1u);
  ledger.w += ledger.z ^ (ledger.x << ((pos % 5u) + 1u));
}

static void fold(const Ledger &ledger) {
  g_sink ^= ledger.x;
  g_sink += ledger.y;
  g_sink ^= ledger.z << 1;
  g_sink += ledger.w >> 1;
}

static void transform_1(BufferView &view, Ledger &ledger) {
  prime_buffer(view);
  seed_ledger(ledger, 1);
  for (size_t i = 0; i < view.size; ++i) {
    unsigned char byte = view.ptr[i] ^ 3u;
    absorb(ledger, byte, i);
  }
}

static void transform_2(BufferView &view, Ledger &ledger) {
  prime_buffer(view);
  seed_ledger(ledger, 2);
  for (size_t i = 0; i < view.size; ++i) {
    unsigned char byte = view.ptr[i] ^ 6u;
    absorb(ledger, byte, i);
  }
}

static void prepare_observation_buffer(BufferView &view) {
  for (size_t i = 0; i < view.size; ++i) view.ptr[i] = 0;
  view.cursor = 0;
}

static void perform_observation(BufferView &view, Ledger &ledger) {
  for (size_t i = 0; i < 8; ++i) {
    size_t p = ring(view.cursor + i, view.size);
    absorb(ledger, view.ptr[p], i);
    fold(ledger);
  }
  view.cursor = ring(view.cursor + ledger.z % 7u + 1u, view.size);
}

// This is where the use-after-free is detected
static void final_observation(BufferView &view) {
  // HEAP_UAF: ERROR: HWAddressSanitizer: heap-use-after-free
  // HEAP_UAF: READ of size 1
  // HEAP_UAF: in final_observation
  g_sink = view.ptr[0];
}

static void cleanup_buffers(BufferView &view) {
  free(view.ptr);
  view.ptr = nullptr;
  view.size = 0;
  view.cursor = 0;
}

}  // namespace

int main_heap_uaf() {
  BufferView view;
  view.size = 128;
  view.ptr = (char*)malloc(view.size);
  view.cursor = 0;
  
  Ledger ledger = {0, 0, 0, 0};
  
  // Perform various transformations
  for (int iteration = 0; iteration < 3; ++iteration) {
    transform_1(view, ledger);
    transform_2(view, ledger);
  }
  
  // Prepare for observation phase
  prepare_observation_buffer(view);
  
  // Perform observations - this keeps the buffer alive
  for (int i = 0; i < 10; ++i) {
    seed_ledger(ledger, i);
    perform_observation(view, ledger);
  }
  
  // Free the buffer
  cleanup_buffers(view);
  
  // This will cause a use-after-free when we try to access the freed buffer
  final_observation(view);
  
  return static_cast<int>(g_sink);
}

// ============================================================================
// SECTION 2: Combined UAF and Global Overflow
// ============================================================================

// Test: Combined test
// RUN: %clangxx_hwasan -O0 -g %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=COMBINED
// COMBINED: ERROR: HWAddressSanitizer

static volatile uint64_t sink_large = 0;

struct TestStruct {
  uint64_t a;
  uint64_t b;
  char data[64];
};

static TestStruct* g_struct = nullptr;
static char g_global_buffer[256] = {0};

void test_uaf() {
  TestStruct* local = (TestStruct*)malloc(sizeof(TestStruct));
  local->a = 1;
  local->b = 2;
  g_struct = local;
  free(local);
  // COMBINED: ERROR: HWAddressSanitizer
  sink_large = g_struct->a;
}

void test_global_overflow() {
  // COMBINED: ERROR: HWAddressSanitizer
  g_global_buffer[256] = 'X';
}

int main_large_combined() {
  __hwasan_enable_allocator_tagging();
  test_uaf();
  test_global_overflow();
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  return main_heap_uaf();
}
