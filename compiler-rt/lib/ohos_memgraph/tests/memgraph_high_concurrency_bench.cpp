//===-- memgraph_high_concurrency_bench.cpp ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Contention-oriented benchmark for the OHOS memgraph runtime.
//
// This benchmark intentionally drives the hottest serialized path:
// - malloc hook
// - alloc_record
// - store_record
// - get_member_info
// - free hook
//
// The goal is not to enforce a performance threshold, but to show how
// throughput and per-iteration cost change as thread count grows.
//===----------------------------------------------------------------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef void (*memgraph_init_fn)();
typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name, const char *var_name,
                                unsigned long alloc_pc);
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(runtime_stats_t *out);

struct WorkerCtx {
  unsigned long iterations;
  int worker_id;
  volatile int *start_flag;
  alloc_record_fn alloc_record;
  store_record_fn store_record;
  memgraph_get_member_info_fn get_member_info;
  unsigned long query_failures;
  uint64_t elapsed_ns;
};

static uint64_t NowNs() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static unsigned long ParseArg(const char *s, unsigned long def) {
  if (!s || !*s)
    return def;
  char *end = nullptr;
  unsigned long v = strtoul(s, &end, 10);
  return (end && *end == '\0') ? v : def;
}

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static void *WorkerMain(void *arg) {
  WorkerCtx *ctx = (WorkerCtx *)arg;
  while (!*ctx->start_flag) {
  }

  const uint64_t t0 = NowNs();
  for (unsigned long i = 0; i < ctx->iterations; ++i) {
    unsigned char *source = (unsigned char *)malloc(32);
    unsigned char *owner = (unsigned char *)malloc(64);
    if (!source || !owner) {
      ++ctx->query_failures;
      free(owner);
      free(source);
      continue;
    }

    const unsigned long base = (unsigned long)owner;
    const unsigned long member_addr =
        base + ((unsigned long)((ctx->worker_id + i) % 3u) * 8u);
    ctx->alloc_record(base, "HotNode*", "owner", 0);
    ctx->store_record((unsigned long)source, member_addr, "HotField*", "field",
                      0);

    member_info_t info;
    memset(&info, 0, sizeof(info));
    if (!ctx->get_member_info(member_addr, &info) || !info.found)
      ++ctx->query_failures;

    free(owner);
    free(source);
  }
  ctx->elapsed_ns = NowNs() - t0;
  return nullptr;
}

static int RunScenario(unsigned long thread_count,
                       unsigned long iterations_per_thread,
                       alloc_record_fn alloc_record,
                       store_record_fn store_record,
                       memgraph_get_member_info_fn get_member_info,
                       memgraph_get_runtime_stats_fn get_runtime_stats,
                       unsigned long baseline_alloc_live,
                       unsigned long baseline_store_live,
                       double baseline_ns_per_iter,
                       double *measured_ns_per_iter) {
  pthread_t *threads =
      (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
  WorkerCtx *workers =
      (WorkerCtx *)malloc(sizeof(WorkerCtx) * thread_count);
  if (!threads || !workers) {
    fprintf(stderr, "FAIL: worker allocation failed\n");
    free(workers);
    free(threads);
    return 2;
  }

  memset(workers, 0, sizeof(WorkerCtx) * thread_count);
  volatile int start_flag = 0;
  for (unsigned long i = 0; i < thread_count; ++i) {
    workers[i].iterations = iterations_per_thread;
    workers[i].worker_id = (int)i;
    workers[i].start_flag = &start_flag;
    workers[i].alloc_record = alloc_record;
    workers[i].store_record = store_record;
    workers[i].get_member_info = get_member_info;
    if (pthread_create(&threads[i], nullptr, WorkerMain, &workers[i]) != 0) {
      fprintf(stderr, "FAIL: pthread_create(%lu) failed\n", i);
      free(workers);
      free(threads);
      return 3;
    }
  }

  const uint64_t t0 = NowNs();
  start_flag = 1;
  for (unsigned long i = 0; i < thread_count; ++i)
    pthread_join(threads[i], nullptr);
  const uint64_t elapsed_ns = NowNs() - t0;

  unsigned long failures = 0;
  uint64_t max_thread_ns = 0;
  for (unsigned long i = 0; i < thread_count; ++i) {
    failures += workers[i].query_failures;
    if (workers[i].elapsed_ns > max_thread_ns)
      max_thread_ns = workers[i].elapsed_ns;
  }

  runtime_stats_t st;
  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats || !get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: get_runtime_stats failed\n");
    free(workers);
    free(threads);
    return 4;
  }

  const unsigned long total_iters = thread_count * iterations_per_thread;
  const double seconds = (double)elapsed_ns / 1e9;
  const double iters_per_sec = total_iters / seconds;
  const double ns_per_iter = (double)elapsed_ns / (double)total_iters;
  const double slowdown =
      baseline_ns_per_iter > 0.0 ? ns_per_iter / baseline_ns_per_iter : 1.0;
  if (measured_ns_per_iter)
    *measured_ns_per_iter = ns_per_iter;

  printf("[cont] threads=%lu total_iters=%lu time=%.3fs iter/s=%.2f Mops/s "
         "ns_per_iter=%.1f slowdown_vs_1t=%.2fx max_thread_ms=%.2f "
         "failures=%lu alloc_live=%lu store_live=%lu baseline_alloc=%lu "
         "baseline_store=%lu\n",
         thread_count, total_iters, seconds, iters_per_sec / 1e6, ns_per_iter,
         slowdown, (double)max_thread_ns / 1e6, failures, st.alloc_live_current,
         st.store_live_current, baseline_alloc_live, baseline_store_live);

  free(workers);
  free(threads);

  if (failures != 0 || st.store_live_current != baseline_store_live)
    return 5;
  return 0;
}

int main(int argc, char **argv) {
  const unsigned long iterations_per_thread =
      ParseArg(argc > 1 ? argv[1] : nullptr, 10000);
  const unsigned long max_threads =
      ParseArg(argc > 2 ? argv[2] : nullptr, 32);

  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym("get_runtime_stats");
  if (!alloc_record || !store_record || !get_member_info || !get_runtime_stats) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 1;
  }
  if (init)
    init();

  runtime_stats_t baseline_stats;
  memset(&baseline_stats, 0, sizeof(baseline_stats));
  if (!get_runtime_stats(&baseline_stats)) {
    fprintf(stderr, "FAIL: get_runtime_stats baseline failed\n");
    return 2;
  }

  static const unsigned long kThreadMatrix[] = {1, 4, 8, 16, 32};
  double baseline_ns_per_iter = 0.0;
  for (unsigned long i = 0; i < sizeof(kThreadMatrix) / sizeof(kThreadMatrix[0]);
       ++i) {
    const unsigned long threads = kThreadMatrix[i];
    if (threads > max_threads)
      continue;

    double scenario_ns_per_iter = 0.0;
    const int rc = RunScenario(threads, iterations_per_thread, alloc_record,
                               store_record, get_member_info,
                               get_runtime_stats,
                               baseline_stats.alloc_live_current,
                               baseline_stats.store_live_current,
                               baseline_ns_per_iter,
                               &scenario_ns_per_iter);
    if (rc != 0)
      return rc;

    if (threads == 1)
      baseline_ns_per_iter = scenario_ns_per_iter;
  }

  puts("PASS: memgraph high concurrency benchmark finished");
  return 0;
}
