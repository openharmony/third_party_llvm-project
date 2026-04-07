//===-- memgraph_read_heavy_bench.cpp ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Read-mostly benchmark for the OHOS memgraph runtime.
//
// The benchmark first builds a stable tracked object set, then runs only
// get_block_info()/get_member_info() concurrently. It is meant to show the
// benefit of future lock changes on read-heavy IDE-style workloads.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

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
typedef int (*memgraph_get_block_info_fn)(unsigned long base,
                                          block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(runtime_stats_t *out);

struct Dataset {
  unsigned char **owners;
  unsigned char **sources;
  unsigned long owner_count;
};

struct WorkerCtx {
  unsigned long iterations;
  int worker_id;
  volatile int *start_flag;
  Dataset *dataset;
  memgraph_get_block_info_fn get_block_info;
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

static bool BuildDataset(Dataset *dataset, unsigned long owner_count,
                         alloc_record_fn alloc_record,
                         store_record_fn store_record) {
  dataset->owners =
      (unsigned char **)calloc(owner_count, sizeof(unsigned char *));
  dataset->sources =
      (unsigned char **)calloc(owner_count, sizeof(unsigned char *));
  dataset->owner_count = owner_count;
  if (!dataset->owners || !dataset->sources)
    return false;

  for (unsigned long i = 0; i < owner_count; ++i) {
    dataset->owners[i] = (unsigned char *)malloc(64);
    dataset->sources[i] = (unsigned char *)malloc(32);
    if (!dataset->owners[i] || !dataset->sources[i])
      return false;

    const unsigned long base = (unsigned long)dataset->owners[i];
    const unsigned long source = (unsigned long)dataset->sources[i];
    alloc_record(base, "ReadNode*", "owner", 0);
    store_record(source, base, "ReadField*", "head", 0);
    store_record(source, base + 8u, "ReadField*", "next", 0);
    store_record(source, base + 16u, "ReadField*", "prev", 0);
  }
  return true;
}

static void DestroyDataset(Dataset *dataset) {
  if (!dataset)
    return;
  for (unsigned long i = 0; i < dataset->owner_count; ++i) {
    free(dataset->owners ? dataset->owners[i] : nullptr);
    free(dataset->sources ? dataset->sources[i] : nullptr);
  }
  free(dataset->owners);
  free(dataset->sources);
  dataset->owners = nullptr;
  dataset->sources = nullptr;
  dataset->owner_count = 0;
}

static void *WorkerMain(void *arg) {
  WorkerCtx *ctx = (WorkerCtx *)arg;
  while (!*ctx->start_flag) {
  }

  const uint64_t t0 = NowNs();
  for (unsigned long i = 0; i < ctx->iterations; ++i) {
    const unsigned long idx =
        (i * 2654435761u + (unsigned long)ctx->worker_id * 17u) %
        ctx->dataset->owner_count;
    const unsigned long base = (unsigned long)ctx->dataset->owners[idx];
    if ((i & 1u) == 0) {
      block_info_t info;
      memset(&info, 0, sizeof(info));
      if (!ctx->get_block_info(base, &info) || !info.found)
        ++ctx->query_failures;
    } else {
      const unsigned long field_idx = (i / 2u) % 3u;
      member_info_t info;
      memset(&info, 0, sizeof(info));
      if (!ctx->get_member_info(base + field_idx * 8u, &info) || !info.found) {
        ++ctx->query_failures;
      }
    }
  }
  ctx->elapsed_ns = NowNs() - t0;
  return nullptr;
}

static int RunScenario(unsigned long thread_count,
                       unsigned long iterations_per_thread, Dataset *dataset,
                       memgraph_get_block_info_fn get_block_info,
                       memgraph_get_member_info_fn get_member_info,
                       double baseline_ns_per_query,
                       double *measured_ns_per_query) {
  pthread_t *threads =
      (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
  WorkerCtx *workers =
      (WorkerCtx *)calloc(thread_count, sizeof(WorkerCtx));
  if (!threads || !workers) {
    fprintf(stderr, "FAIL: worker allocation failed\n");
    free(workers);
    free(threads);
    return 2;
  }

  volatile int start_flag = 0;
  for (unsigned long i = 0; i < thread_count; ++i) {
    workers[i].iterations = iterations_per_thread;
    workers[i].worker_id = (int)i;
    workers[i].start_flag = &start_flag;
    workers[i].dataset = dataset;
    workers[i].get_block_info = get_block_info;
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

  const unsigned long total_queries = thread_count * iterations_per_thread;
  const double seconds = (double)elapsed_ns / 1e9;
  const double qps = total_queries / seconds;
  const double ns_per_query = (double)elapsed_ns / (double)total_queries;
  const double slowdown =
      baseline_ns_per_query > 0.0 ? ns_per_query / baseline_ns_per_query : 1.0;
  if (measured_ns_per_query)
    *measured_ns_per_query = ns_per_query;

  printf("[read] threads=%lu total_queries=%lu time=%.3fs qps=%.2f Mops/s "
         "ns_per_query=%.1f slowdown_vs_1t=%.2fx max_thread_ms=%.2f "
         "failures=%lu\n",
         thread_count, total_queries, seconds, qps / 1e6, ns_per_query,
         slowdown, (double)max_thread_ns / 1e6, failures);

  free(workers);
  free(threads);
  return failures == 0 ? 0 : 4;
}

int main(int argc, char **argv) {
  const unsigned long owner_count =
      ParseArg(argc > 1 ? argv[1] : nullptr, 20000);
  const unsigned long iterations_per_thread =
      ParseArg(argc > 2 ? argv[2] : nullptr, 50000);
  const unsigned long max_threads =
      ParseArg(argc > 3 ? argv[3] : nullptr, 32);

  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym("get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym("get_runtime_stats");
  if (!alloc_record || !store_record || !get_block_info || !get_member_info) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 1;
  }
  if (init)
    init();

  Dataset dataset = {};
  if (!BuildDataset(&dataset, owner_count, alloc_record, store_record)) {
    fprintf(stderr, "FAIL: dataset build failed\n");
    DestroyDataset(&dataset);
    return 2;
  }

  static const unsigned long kThreadMatrix[] = {1, 4, 8, 16, 32};
  double baseline_ns_per_query = 0.0;
  for (unsigned long i = 0; i < sizeof(kThreadMatrix) / sizeof(kThreadMatrix[0]);
       ++i) {
    const unsigned long threads = kThreadMatrix[i];
    if (threads > max_threads)
      continue;

    double scenario_ns_per_query = 0.0;
    const int rc = RunScenario(threads, iterations_per_thread, &dataset,
                               get_block_info, get_member_info,
                               baseline_ns_per_query, &scenario_ns_per_query);
    if (rc != 0) {
      DestroyDataset(&dataset);
      return rc;
    }
    if (threads == 1)
      baseline_ns_per_query = scenario_ns_per_query;
  }

  if (get_runtime_stats) {
    runtime_stats_t st;
    memset(&st, 0, sizeof(st));
    if (get_runtime_stats(&st)) {
      printf("[read] alloc_live=%lu store_live=%lu runtime_peak_kb=%lu\n",
             st.alloc_live_current, st.store_live_current,
             st.runtime_peak_bytes / 1024);
    }
  }

  DestroyDataset(&dataset);
  puts("PASS: memgraph read-heavy benchmark finished");
  return 0;
}
