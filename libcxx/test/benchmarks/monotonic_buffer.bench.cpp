//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14

#include <list>
#include <memory_resource>

#include "benchmark/benchmark.h"

static void bm_list(benchmark::State& state) {
  char buffer[16384];
  std::pmr::monotonic_buffer_resource resource(buffer, sizeof(buffer));
  for (auto _ : state) {
#ifndef __OHOS__
    std::pmr::list<int> l(&resource);
    for (int64_t i = 0; i != state.range(); ++i) {
      l.push_back(1);
      benchmark::DoNotOptimize(l);
    }
#else
    // Destroy the list before releasing its node storage from the resource.
    // On AArch64, the use-after-free becomes visible at 2048 elements.
    // On x86, the use-after-free becomes visible at 16384 elements.
    {
      std::pmr::list<int> l(&resource);
      for (int64_t i = 0; i != state.range(); ++i) {
        l.push_back(1);
        benchmark::DoNotOptimize(l);
      }
    }
#endif
    resource.release();
  }
}
BENCHMARK(bm_list)->Range(1, 2048);

BENCHMARK_MAIN();
