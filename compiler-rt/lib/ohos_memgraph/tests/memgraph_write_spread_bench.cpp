//===-- memgraph_write_spread_bench.cpp --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Low-to-medium contention write benchmark for the OHOS memgraph runtime.
//
// The benchmark builds:
// - a large per-thread private owner pool
// - a much smaller shared owner pool
//
// Most writes go to private owners; a small fraction go to the shared pool.
// This models the more common case where threads mostly write different owners,
// but still occasionally collide on the same owner.
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
typedef int (*memgraph_get_block_info_fn)(unsigned long base,
                                          block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(runtime_stats_t *out);

struct Dataset {
  unsigned char **private_owners;
  unsigned char **shared_owners;
  unsigned long private_owners_per_thread;
  unsigned long shared_owner_count;
  unsigned long max_threads;
};

struct WorkerCtx {
  unsigned long iterations;
  unsigned long private_owners_per_thread;
  unsigned long shared_owner_count;
  unsigned long query_every;
  unsigned long shared_every;
  int worker_id;
  volatile int *start_flag;
  Dataset *dataset;
  store_record_fn store_record;
  memgraph_get_block_info_fn get_block_info;
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

static bool BuildDataset(Dataset *dataset, unsigned long max_threads,
                         unsigned long private_owners_per_thread,
                         unsigned long shared_owner_count,
                         alloc_record_fn alloc_record) {
  const unsigned long total_private = max_threads * private_owners_per_thread;
  dataset->private_owners =
      (unsigned char **)calloc(total_private, sizeof(unsigned char *));
  dataset->shared_owners =
      (unsigned char **)calloc(shared_owner_count, sizeof(unsigned char *));
  dataset->private_owners_per_thread = private_owners_per_thread;
  dataset->shared_owner_count = shared_owner_count;
  dataset->max_threads = max_threads;
  if (!dataset->private_owners || !dataset->shared_owners)
    return false;

  const char *type_names[] = {"SpreadNode*", "SpreadEdge*", "SpreadPayload*"};
  const char *var_names[] = {"owner", "node", "payload", "entry"};

  for (unsigned long t = 0; t < max_threads; ++t) {
    for (unsigned long i = 0; i < private_owners_per_thread; ++i) {
      const unsigned long idx = t * private_owners_per_thread + i;
      dataset->private_owners[idx] = (unsigned char *)malloc(64);
      if (!dataset->private_owners[idx])
        return false;
      alloc_record((unsigned long)dataset->private_owners[idx],
                   type_names[(t + i) % 3u], var_names[(t + i) % 4u], 0);
    }
  }

  for (unsigned long i = 0; i < shared_owner_count; ++i) {
    dataset->shared_owners[i] = (unsigned char *)malloc(64);
    if (!dataset->shared_owners[i])
      return false;
    alloc_record((unsigned long)dataset->shared_owners[i], type_names[i % 3u],
                 var_names[i % 4u], 0);
  }
  return true;
}

static void DestroyDataset(Dataset *dataset) {
  if (!dataset)
    return;
  if (dataset->private_owners) {
    const unsigned long total_private =
        dataset->private_owners_per_thread * dataset->max_threads;
    for (unsigned long i = 0; i < total_private; ++i)
      free(dataset->private_owners[i]);
  }
  if (dataset->shared_owners) {
    for (unsigned long i = 0; i < dataset->shared_owner_count; ++i)
      free(dataset->shared_owners[i]);
  }
  free(dataset->private_owners);
  free(dataset->shared_owners);
  dataset->private_owners = nullptr;
  dataset->shared_owners = nullptr;
  dataset->private_owners_per_thread = 0;
  dataset->shared_owner_count = 0;
  dataset->max_threads = 0;
}

static void *WorkerMain(void *arg) {
  WorkerCtx *ctx = (WorkerCtx *)arg;
  while (!*ctx->start_flag) {
  }

  const unsigned char **private_shard =
      (const unsigned char **)&ctx->dataset
           ->private_owners[(unsigned long)ctx->worker_id *
                            ctx->private_owners_per_thread];
  const char *type_names[] = {"SpreadField*", "SpreadLink*", "SpreadMeta*"};
  const char *var_names[] = {"head", "next", "tail", "link"};
  const uint64_t t0 = NowNs();

  for (unsigned long i = 0; i < ctx->iterations; ++i) {
    const bool use_shared =
        ctx->shared_owner_count > 0 &&
        (((i + (unsigned long)ctx->worker_id) % ctx->shared_every) == 0u);
    unsigned char *owner = nullptr;
    if (use_shared) {
      const unsigned long idx =
          (i * 1103515245u + (unsigned long)ctx->worker_id * 17u) %
          ctx->shared_owner_count;
      owner = ctx->dataset->shared_owners[idx];
    } else {
      const unsigned long idx =
          (i * 2654435761u + (unsigned long)ctx->worker_id * 97u) %
          ctx->private_owners_per_thread;
      owner = const_cast<unsigned char *>(private_shard[idx]);
    }

    if (!owner) {
      ++ctx->failures;
      continue;
    }

    const unsigned long base = (unsigned long)owner;
    const unsigned long field_idx = (i + (unsigned long)ctx->worker_id) % 3u;
    const unsigned long member_addr = base + field_idx * 8u;
    ctx->store_record(0, member_addr, type_names[(i + 1u) % 3u],
                      var_names[(i + 2u) % 4u], 0);

    if (ctx->query_every > 0 && ((i + 1u) % ctx->query_every) == 0u) {
      block_info_t block;
      member_info_t member;
      memset(&block, 0, sizeof(block));
      memset(&member, 0, sizeof(member));
      if (!ctx->get_block_info(base, &block) || !block.found)
        ++ctx->failures;
      if (!ctx->get_member_info(member_addr, &member) || !member.found)
        ++ctx->failures;
    }
  }

  ctx->elapsed_ns = NowNs() - t0;
  return nullptr;
}

static int RunScenario(unsigned long thread_count,
                       unsigned long iterations_per_thread,
                       unsigned long private_owners_per_thread,
                       unsigned long shared_owner_count,
                       unsigned long shared_every, unsigned long query_every,
                       Dataset *dataset, store_record_fn store_record,
                       memgraph_get_block_info_fn get_block_info,
                       memgraph_get_member_info_fn get_member_info,
                       double baseline_ns_per_store,
                       double *measured_ns_per_store) {
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
    workers[i].private_owners_per_thread = private_owners_per_thread;
    workers[i].shared_owner_count = shared_owner_count;
    workers[i].query_every = query_every;
    workers[i].shared_every = shared_every;
    workers[i].worker_id = (int)i;
    workers[i].start_flag = &start_flag;
    workers[i].dataset = dataset;
    workers[i].store_record = store_record;
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
    failures += workers[i].failures;
    if (workers[i].elapsed_ns > max_thread_ns)
      max_thread_ns = workers[i].elapsed_ns;
  }

  const unsigned long total_store_ops = thread_count * iterations_per_thread;
  const double seconds = (double)elapsed_ns / 1e9;
  const double ops_per_sec = total_store_ops / seconds;
  const double ns_per_store = (double)elapsed_ns / (double)total_store_ops;
  const double slowdown =
      baseline_ns_per_store > 0.0 ? ns_per_store / baseline_ns_per_store : 1.0;
  if (measured_ns_per_store)
    *measured_ns_per_store = ns_per_store;

  printf("[spread] threads=%lu private_owners_per_thread=%lu shared_owners=%lu "
         "shared_every=%lu total_store_ops=%lu time=%.3fs ops/s=%.2f Mops/s "
         "ns_per_store=%.1f slowdown_vs_1t=%.2fx max_thread_ms=%.2f "
         "failures=%lu\n",
         thread_count, private_owners_per_thread, shared_owner_count,
         shared_every, total_store_ops, seconds, ops_per_sec / 1e6,
         ns_per_store, slowdown, (double)max_thread_ns / 1e6, failures);

  free(workers);
  free(threads);
  return failures == 0 ? 0 : 4;
}

int main(int argc, char **argv) {
  const unsigned long private_owners_per_thread =
      ParseArg(argc > 1 ? argv[1] : nullptr, 4096);
  const unsigned long shared_owner_count =
      ParseArg(argc > 2 ? argv[2] : nullptr, 256);
  const unsigned long iterations_per_thread =
      ParseArg(argc > 3 ? argv[3] : nullptr, 10000);
  const unsigned long max_threads =
      ParseArg(argc > 4 ? argv[4] : nullptr, 32);
  const unsigned long shared_every =
      ParseArg(argc > 5 ? argv[5] : nullptr, 8);
  const unsigned long query_every =
      ParseArg(argc > 6 ? argv[6] : nullptr, 100);

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
  if (!BuildDataset(&dataset, max_threads, private_owners_per_thread,
                    shared_owner_count, alloc_record)) {
    fprintf(stderr, "FAIL: dataset build failed\n");
    DestroyDataset(&dataset);
    return 2;
  }

  static const unsigned long kThreadMatrix[] = {1, 4, 8, 16, 32};
  double baseline_ns_per_store = 0.0;
  for (unsigned long i = 0; i < sizeof(kThreadMatrix) / sizeof(kThreadMatrix[0]);
       ++i) {
    const unsigned long threads = kThreadMatrix[i];
    if (threads > max_threads)
      continue;

    double scenario_ns_per_store = 0.0;
    const int rc = RunScenario(threads, iterations_per_thread,
                               private_owners_per_thread, shared_owner_count,
                               shared_every, query_every, &dataset, store_record,
                               get_block_info, get_member_info,
                               baseline_ns_per_store, &scenario_ns_per_store);
    if (rc != 0) {
      DestroyDataset(&dataset);
      return rc;
    }
    if (threads == 1)
      baseline_ns_per_store = scenario_ns_per_store;
  }

  if (get_runtime_stats) {
    runtime_stats_t st;
    memset(&st, 0, sizeof(st));
    if (get_runtime_stats(&st)) {
      printf("[spread] alloc_live=%lu store_live=%lu runtime_peak_kb=%lu "
             "alloc_record_calls=%lu store_record_calls=%lu\n",
             st.alloc_live_current, st.store_live_current,
             st.runtime_peak_bytes / 1024, st.malloc_record_calls,
             st.store_record_calls);
    }
  }

  DestroyDataset(&dataset);
  puts("PASS: memgraph write spread benchmark finished");
  return 0;
}
