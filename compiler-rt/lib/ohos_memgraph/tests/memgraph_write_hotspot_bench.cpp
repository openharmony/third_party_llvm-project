//===-- memgraph_write_hotspot_bench.cpp -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Write-hotspot benchmark for the OHOS memgraph runtime.
//
// Multiple threads repeatedly update a small shared owner set so we can see
// how the hottest metadata write paths behave under heavy contention.
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
  alloc_record_fn alloc_record;
  store_record_fn store_record;
  memgraph_get_member_info_fn get_member_info;
  unsigned long failures;
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

static bool BuildDataset(Dataset *dataset, unsigned long owner_count) {
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

  const char *type_names[] = {"HotNode*", "HotEdge*", "HotPayload*"};
  const char *var_names[] = {"head", "next", "tail", "link"};
  const uint64_t t0 = NowNs();

  for (unsigned long i = 0; i < ctx->iterations; ++i) {
    const unsigned long idx =
        (i + (unsigned long)ctx->worker_id * 7u) % ctx->dataset->owner_count;
    const unsigned long field_idx = (i + (unsigned long)ctx->worker_id) % 3u;
    const unsigned long base = (unsigned long)ctx->dataset->owners[idx];
    const unsigned long source =
        (unsigned long)ctx->dataset->sources[(idx + field_idx) %
                                             ctx->dataset->owner_count];
    const unsigned long member_addr = base + field_idx * 8u;

    ctx->alloc_record(base, type_names[(i + field_idx) % 3u],
                      var_names[(i + idx) % 4u], 0);
    ctx->store_record(source, member_addr, type_names[(i + 1u) % 3u],
                      var_names[(i + 2u) % 4u], 0);

    if ((i & 1023u) == 0u) {
      member_info_t info;
      memset(&info, 0, sizeof(info));
      if (!ctx->get_member_info(member_addr, &info) || !info.found)
        ++ctx->failures;
    }
  }

  ctx->elapsed_ns = NowNs() - t0;
  return nullptr;
}

static int RunScenario(unsigned long thread_count,
                       unsigned long iterations_per_thread, Dataset *dataset,
                       alloc_record_fn alloc_record,
                       store_record_fn store_record,
                       memgraph_get_member_info_fn get_member_info,
                       double baseline_ns_per_op, double *measured_ns_per_op) {
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
    failures += workers[i].failures;
    if (workers[i].elapsed_ns > max_thread_ns)
      max_thread_ns = workers[i].elapsed_ns;
  }

  const unsigned long total_iterations = thread_count * iterations_per_thread;
  const unsigned long total_ops = total_iterations * 2u;
  const double seconds = (double)elapsed_ns / 1e9;
  const double ops_per_sec = total_ops / seconds;
  const double ns_per_op = (double)elapsed_ns / (double)total_ops;
  const double slowdown =
      baseline_ns_per_op > 0.0 ? ns_per_op / baseline_ns_per_op : 1.0;
  if (measured_ns_per_op)
    *measured_ns_per_op = ns_per_op;

  printf("[write] threads=%lu owners=%lu total_iters=%lu ops=%lu time=%.3fs "
         "ops/s=%.2f Mops/s ns_per_op=%.1f slowdown_vs_1t=%.2fx "
         "max_thread_ms=%.2f failures=%lu\n",
         thread_count, dataset->owner_count, total_iterations, total_ops,
         seconds, ops_per_sec / 1e6, ns_per_op, slowdown,
         (double)max_thread_ns / 1e6, failures);

  free(workers);
  free(threads);
  return failures == 0 ? 0 : 4;
}

int main(int argc, char **argv) {
  const unsigned long owner_count =
      ParseArg(argc > 1 ? argv[1] : nullptr, 64);
  const unsigned long iterations_per_thread =
      ParseArg(argc > 2 ? argv[2] : nullptr, 50000);
  const unsigned long max_threads =
      ParseArg(argc > 3 ? argv[3] : nullptr, 32);

  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym("get_runtime_stats");
  if (!alloc_record || !store_record || !get_member_info) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 1;
  }
  if (init)
    init();

  Dataset dataset = {};
  if (!BuildDataset(&dataset, owner_count)) {
    fprintf(stderr, "FAIL: dataset build failed\n");
    DestroyDataset(&dataset);
    return 2;
  }

  static const unsigned long kThreadMatrix[] = {1, 4, 8, 16, 32};
  double baseline_ns_per_op = 0.0;
  for (unsigned long i = 0; i < sizeof(kThreadMatrix) / sizeof(kThreadMatrix[0]);
       ++i) {
    const unsigned long threads = kThreadMatrix[i];
    if (threads > max_threads)
      continue;

    double scenario_ns_per_op = 0.0;
    const int rc = RunScenario(threads, iterations_per_thread, &dataset,
                               alloc_record, store_record, get_member_info,
                               baseline_ns_per_op, &scenario_ns_per_op);
    if (rc != 0) {
      DestroyDataset(&dataset);
      return rc;
    }
    if (threads == 1)
      baseline_ns_per_op = scenario_ns_per_op;
  }

  if (get_runtime_stats) {
    runtime_stats_t st;
    memset(&st, 0, sizeof(st));
    if (get_runtime_stats(&st)) {
      printf("[write] alloc_live=%lu store_live=%lu runtime_peak_kb=%lu "
             "alloc_record_calls=%lu store_record_calls=%lu\n",
             st.alloc_live_current, st.store_live_current,
             st.runtime_peak_bytes / 1024, st.malloc_record_calls,
             st.store_record_calls);
    }
  }

  DestroyDataset(&dataset);
  puts("PASS: memgraph write-hotspot benchmark finished");
  return 0;
}
