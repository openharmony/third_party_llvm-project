// UNSUPPORTED: *
// REQUIRES: shadow-scale-3
// RUN: %clangxx_hwasan -O0 -g %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// This test builds a larger, more realistic call graph while still keeping
// exactly one observable failure: a heap-use-after-free read.
//
// CHECK: ERROR: HWAddressSanitizer: heap-use-after-free
// CHECK: READ of size 1
// CHECK: in final_observation
// CHECK: freed by thread
// CHECK: previously allocated by thread
// CHECK-NOT: stack-buffer-overflow
// CHECK-NOT: global-buffer-overflow

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
  ledger.x = mix64(0x1111u + n);
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

static void prefetch_1(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 1u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 3u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 1u, view.size);
}

static void prefetch_2(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 2u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 6u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 2u, view.size);
}

static void prefetch_3(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 3u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 9u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 3u, view.size);
}

static void prefetch_4(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 4u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 12u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 4u, view.size);
}

static void prefetch_5(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 5u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 15u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 5u, view.size);
}

static void prefetch_6(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 6u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 18u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 6u, view.size);
}

static void prefetch_7(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 7u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 21u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 7u, view.size);
}

static void prefetch_8(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 8u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 24u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 8u, view.size);
}

static void prefetch_9(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 9u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 27u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 9u, view.size);
}

static void prefetch_10(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 10u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 30u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 10u, view.size);
}

static void normalize_11(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 11u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 33u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 11u, view.size);
}

static void normalize_12(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 12u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 36u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 12u, view.size);
}

static void normalize_13(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 13u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 39u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 13u, view.size);
}

static void normalize_14(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 14u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 42u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 14u, view.size);
}

static void normalize_15(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 15u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 45u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 15u, view.size);
}

static void normalize_16(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 16u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 48u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 16u, view.size);
}

static void normalize_17(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 17u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 51u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 17u, view.size);
}

static void normalize_18(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 18u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 54u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 18u, view.size);
}

static void normalize_19(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 19u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 57u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 19u, view.size);
}

static void normalize_20(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 20u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 60u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 20u, view.size);
}

static void reshape_21(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 21u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 63u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 21u, view.size);
}

static void reshape_22(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 22u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 66u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 22u, view.size);
}

static void reshape_23(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 23u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 69u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 23u, view.size);
}

static void reshape_24(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 24u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 72u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 24u, view.size);
}

static void reshape_25(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 25u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 75u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 25u, view.size);
}

static void reshape_26(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 26u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 78u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 26u, view.size);
}

static void reshape_27(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 27u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 81u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 27u, view.size);
}

static void reshape_28(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 28u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 84u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 28u, view.size);
}

static void reshape_29(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 29u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 87u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 29u, view.size);
}

static void reshape_30(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 30u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 90u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 30u, view.size);
}

static void stabilize_31(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 31u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 93u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 31u, view.size);
}

static void stabilize_32(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 32u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 96u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 32u, view.size);
}

static void stabilize_33(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 33u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 99u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 33u, view.size);
}

static void stabilize_34(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 34u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 102u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 34u, view.size);
}

static void stabilize_35(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 35u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 105u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 35u, view.size);
}

static void stabilize_36(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 36u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 108u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 36u, view.size);
}

static void stabilize_37(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 37u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 111u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 37u, view.size);
}

static void stabilize_38(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 38u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 114u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 38u, view.size);
}

static void stabilize_39(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 39u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 117u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 39u, view.size);
}

static void stabilize_40(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 40u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 120u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 40u, view.size);
}

static void correlate_41(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 41u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 123u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 41u, view.size);
}

static void correlate_42(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 42u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 126u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 42u, view.size);
}

static void correlate_43(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 43u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 129u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 43u, view.size);
}

static void correlate_44(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 44u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 132u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 44u, view.size);
}

static void correlate_45(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 45u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 135u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 45u, view.size);
}

static void correlate_46(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 46u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 138u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 46u, view.size);
}

static void correlate_47(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 47u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 141u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 47u, view.size);
}

static void correlate_48(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 48u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 144u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 48u, view.size);
}

static void correlate_49(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 49u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 147u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 49u, view.size);
}

static void correlate_50(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 50u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 150u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 50u, view.size);
}

static void finalize_51(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 51u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 153u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 51u, view.size);
}

static void finalize_52(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 52u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 156u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 52u, view.size);
}

static void finalize_53(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 53u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 159u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 53u, view.size);
}

static void finalize_54(BufferView &view, Ledger &ledger) {
  const size_t step = 6u;
  const size_t start = ring(ledger.x + ledger.y + 54u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 162u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 54u, view.size);
}

static void finalize_55(BufferView &view, Ledger &ledger) {
  const size_t step = 7u;
  const size_t start = ring(ledger.x + ledger.y + 55u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 165u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 55u, view.size);
}

static void finalize_56(BufferView &view, Ledger &ledger) {
  const size_t step = 1u;
  const size_t start = ring(ledger.x + ledger.y + 56u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 168u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (2u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 56u, view.size);
}

static void finalize_57(BufferView &view, Ledger &ledger) {
  const size_t step = 2u;
  const size_t start = ring(ledger.x + ledger.y + 57u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 171u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (3u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 57u, view.size);
}

static void finalize_58(BufferView &view, Ledger &ledger) {
  const size_t step = 3u;
  const size_t start = ring(ledger.x + ledger.y + 58u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 174u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (4u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 58u, view.size);
}

static void finalize_59(BufferView &view, Ledger &ledger) {
  const size_t step = 4u;
  const size_t start = ring(ledger.x + ledger.y + 59u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 177u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (5u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 59u, view.size);
}

static void finalize_60(BufferView &view, Ledger &ledger) {
  const size_t step = 5u;
  const size_t start = ring(ledger.x + ledger.y + 60u, view.size);
  for (size_t k = 0; k < view.size; k += step) {
    const size_t p = ring(start + k + view.cursor, view.size);
    unsigned char byte = static_cast<unsigned char>(view.ptr[p] ^ 180u);
    absorb(ledger, byte, p + k);
    view.ptr[p] ^= static_cast<char>((ledger.z >> (1u)) & 0x3f);
  }
  view.cursor = ring(view.cursor + step + 60u, view.size);
}

static void run_prefetch(BufferView &view, Ledger &ledger) {
  prefetch_1(view, ledger);
  prefetch_2(view, ledger);
  prefetch_3(view, ledger);
  prefetch_4(view, ledger);
  fold(ledger);
  prefetch_5(view, ledger);
  prefetch_6(view, ledger);
  prefetch_7(view, ledger);
  prefetch_8(view, ledger);
  fold(ledger);
  prefetch_9(view, ledger);
  prefetch_10(view, ledger);
}

static void run_normalize(BufferView &view, Ledger &ledger) {
  normalize_11(view, ledger);
  normalize_12(view, ledger);
  fold(ledger);
  normalize_13(view, ledger);
  normalize_14(view, ledger);
  normalize_15(view, ledger);
  normalize_16(view, ledger);
  fold(ledger);
  normalize_17(view, ledger);
  normalize_18(view, ledger);
  normalize_19(view, ledger);
  normalize_20(view, ledger);
  fold(ledger);
}

static void run_reshape(BufferView &view, Ledger &ledger) {
  reshape_21(view, ledger);
  reshape_22(view, ledger);
  reshape_23(view, ledger);
  reshape_24(view, ledger);
  fold(ledger);
  reshape_25(view, ledger);
  reshape_26(view, ledger);
  reshape_27(view, ledger);
  reshape_28(view, ledger);
  fold(ledger);
  reshape_29(view, ledger);
  reshape_30(view, ledger);
}

static void run_stabilize(BufferView &view, Ledger &ledger) {
  stabilize_31(view, ledger);
  stabilize_32(view, ledger);
  fold(ledger);
  stabilize_33(view, ledger);
  stabilize_34(view, ledger);
  stabilize_35(view, ledger);
  stabilize_36(view, ledger);
  fold(ledger);
  stabilize_37(view, ledger);
  stabilize_38(view, ledger);
  stabilize_39(view, ledger);
  stabilize_40(view, ledger);
  fold(ledger);
}

static void run_correlate(BufferView &view, Ledger &ledger) {
  correlate_41(view, ledger);
  correlate_42(view, ledger);
  correlate_43(view, ledger);
  correlate_44(view, ledger);
  fold(ledger);
  correlate_45(view, ledger);
  correlate_46(view, ledger);
  correlate_47(view, ledger);
  correlate_48(view, ledger);
  fold(ledger);
  correlate_49(view, ledger);
  correlate_50(view, ledger);
}

static void run_finalize(BufferView &view, Ledger &ledger) {
  finalize_51(view, ledger);
  finalize_52(view, ledger);
  fold(ledger);
  finalize_53(view, ledger);
  finalize_54(view, ledger);
  finalize_55(view, ledger);
  finalize_56(view, ledger);
  fold(ledger);
  finalize_57(view, ledger);
  finalize_58(view, ledger);
  finalize_59(view, ledger);
  finalize_60(view, ledger);
  fold(ledger);
}

static void prepare_story(BufferView &view, Ledger &ledger) {
  run_prefetch(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_reshape(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void shape_midpoint(BufferView &view, Ledger &ledger) {
  run_normalize(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_stabilize(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void rebalance_edges(BufferView &view, Ledger &ledger) {
  run_reshape(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_correlate(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void add_cross_checks(BufferView &view, Ledger &ledger) {
  run_stabilize(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_finalize(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void stitch_history(BufferView &view, Ledger &ledger) {
  run_correlate(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_prefetch(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void compose_layers(BufferView &view, Ledger &ledger) {
  run_finalize(view, ledger);
  ledger.x ^= mix64(view.cursor + ledger.y);
  run_normalize(view, ledger);
  ledger.w += mix64(ledger.z ^ view.size);
  fold(ledger);
}

static void audit_window(BufferView &view, Ledger &ledger) {
  run_normalize(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void blend_cursor(BufferView &view, Ledger &ledger) {
  run_reshape(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void settle_marks(BufferView &view, Ledger &ledger) {
  run_stabilize(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void rebucket_trace(BufferView &view, Ledger &ledger) {
  run_correlate(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void trace_turn(BufferView &view, Ledger &ledger) {
  run_finalize(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void seal_story(BufferView &view, Ledger &ledger) {
  run_prefetch(view, ledger);
  ledger.y ^= mix64(ledger.x + view.cursor + view.size);
  fold(ledger);
}

static void deep_pipeline(BufferView &view, Ledger &ledger) {
  prepare_story(view, ledger);
  shape_midpoint(view, ledger);
  rebalance_edges(view, ledger);
  add_cross_checks(view, ledger);
  stitch_history(view, ledger);
  compose_layers(view, ledger);
  audit_window(view, ledger);
  blend_cursor(view, ledger);
  settle_marks(view, ledger);
  rebucket_trace(view, ledger);
  trace_turn(view, ledger);
  seal_story(view, ledger);
}

static void final_observation(char *ptr) {
  volatile unsigned char byte = static_cast<unsigned char>(ptr[3]);
  g_sink ^= byte;
}

static void run_single_case() {
  BufferView view;
  view.size = 256;
  view.cursor = 0;
  view.ptr = static_cast<char *>(malloc(view.size));
  if (!view.ptr)
    return;

  Ledger ledger;
  prime_buffer(view);
  seed_ledger(ledger, view.size);
  deep_pipeline(view, ledger);
  fold(ledger);

  char *dangling = view.ptr;
  free(view.ptr);

  final_observation(dangling);
}

}  // namespace

int main() {
  run_single_case();
  return static_cast<int>(g_sink & 1u);
}
