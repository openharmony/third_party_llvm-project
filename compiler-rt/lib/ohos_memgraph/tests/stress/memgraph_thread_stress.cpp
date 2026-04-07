//===-- memgraph_thread_stress.cpp -------------------------------===//
//
// Basic multithreaded stress for hook/alloc_record/store_record/query/free.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
typedef int (*memgraph_get_runtime_stats_fn)(
    runtime_stats_t *out);

struct WorkerCtx {
  int id;
  unsigned long iterations;
  alloc_record_fn alloc_record;
  store_record_fn store_record;
  memgraph_get_member_info_fn get_member_info;
  int query_failures;
  int post_free_failures;
};

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static void *WorkerMain(void *arg) {
  WorkerCtx *ctx = (WorkerCtx *)arg;
  const size_t owner_size = 64u + static_cast<size_t>(ctx->id) * 32u;
  for (unsigned long i = 0; i < ctx->iterations; ++i) {
    unsigned char *source = (unsigned char *)malloc(32);
    unsigned char *owner = (unsigned char *)malloc(owner_size);
    if (!source || !owner) {
      ++ctx->query_failures;
      free(owner);
      free(source);
      continue;
    }

    const unsigned long base = (unsigned long)owner;
    const unsigned long member_addr = base + ((i & 1u) ? 8u : 0u);
    ctx->alloc_record(base, "Thread*", "owner", 0);
    ctx->store_record((unsigned long)source, member_addr, "ThreadField*",
                      "field", 0);

    member_info_t info;
    memset(&info, 0, sizeof(info));
    if (!ctx->get_member_info(member_addr, &info) || !info.found)
      ++ctx->query_failures;

    free(owner);
    memset(&info, 0, sizeof(info));
    if (ctx->get_member_info(member_addr, &info))
      ++ctx->post_free_failures;

    free(source);
  }
  return nullptr;
}

int main() {
  static constexpr int kThreadCount = 4;
  static constexpr unsigned long kIterationsPerThread = 5000;

  memgraph_init_fn init =
      (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym(
          "get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym(
          "get_runtime_stats");

  if (!alloc_record || !store_record || !get_member_info || !get_runtime_stats) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  pthread_t threads[kThreadCount];
  WorkerCtx workers[kThreadCount];
  memset(workers, 0, sizeof(workers));

  for (int i = 0; i < kThreadCount; ++i) {
    workers[i].id = i;
    workers[i].iterations = kIterationsPerThread;
    workers[i].alloc_record = alloc_record;
    workers[i].store_record = store_record;
    workers[i].get_member_info = get_member_info;
    if (pthread_create(&threads[i], nullptr, WorkerMain, &workers[i]) != 0) {
      fprintf(stderr, "FAIL: pthread_create(%d) failed\n", i);
      return 3;
    }
  }

  int failures = 0;
  for (int i = 0; i < kThreadCount; ++i) {
    pthread_join(threads[i], nullptr);
    failures += workers[i].query_failures + workers[i].post_free_failures;
    printf("thread[%d] query_failures=%d post_free_failures=%d\n", i,
           workers[i].query_failures, workers[i].post_free_failures);
  }

  runtime_stats_t st;
  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: get_runtime_stats failed\n");
    return 4;
  }
  printf("thread stats: alloc_live=%lu store_live=%lu alloc_record=%lu "
         "store_record=%lu\n",
         st.alloc_live_current, st.store_live_current, st.malloc_record_calls,
         st.store_record_calls);

  if (failures != 0 || st.store_live_current != 0) {
    fprintf(stderr, "FAIL: thread stress detected %d failures\n", failures);
    return 5;
  }

  puts("PASS: OHOS memgraph thread stress works");
  return 0;
}
