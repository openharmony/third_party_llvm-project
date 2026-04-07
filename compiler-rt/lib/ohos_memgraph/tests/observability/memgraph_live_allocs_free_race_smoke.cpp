//===-- memgraph_live_allocs_free_race_smoke.cpp -----------------===//
//
// Stress get_live_allocs() against concurrent alloc_record/store_record/free to
// catch torn live-allocation snapshots or crashes on the monitoring path.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

typedef void (*memgraph_init_fn)();
typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name, const char *var_name,
                                unsigned long alloc_pc);
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_block_info_fn)(unsigned long base, block_info_t *out);
typedef unsigned long (*memgraph_get_live_allocs_fn)(
    unsigned long cursor, live_alloc_info_t *out, unsigned long capacity);

namespace {

static constexpr int kWorkerThreadCount = 4;
static constexpr unsigned long kIterationsPerThread = 20000;
static constexpr unsigned long kOwnerSize = 64;

struct SharedState {
  alloc_record_fn alloc_record;
  store_record_fn store_record;
  memgraph_get_block_info_fn get_block_info;
  memgraph_get_live_allocs_fn get_live_allocs;
  std::atomic<int> workers_done;
  std::atomic<unsigned long> failures;
  std::atomic<unsigned long> scans;
};

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static void RecordFailure(SharedState *shared, const char *what,
                          unsigned long a, unsigned long b, unsigned long c) {
  const unsigned long index = shared->failures.fetch_add(1, std::memory_order_relaxed);
  if (index < 16) {
    fprintf(stderr, "FAIL[%lu]: %s a=%lu b=%lu c=%lu\n", index, what, a, b, c);
  }
}

static void *WorkerMain(void *arg) {
  SharedState *shared = reinterpret_cast<SharedState *>(arg);
  for (unsigned long i = 0; i < kIterationsPerThread; ++i) {
    unsigned char *source = reinterpret_cast<unsigned char *>(malloc(32));
    unsigned char *owner =
        reinterpret_cast<unsigned char *>(malloc(kOwnerSize));
    if (!source || !owner) {
      RecordFailure(shared, "malloc-null", i, 0, 0);
      free(source);
      free(owner);
      continue;
    }

    const unsigned long base = reinterpret_cast<unsigned long>(owner);
    shared->alloc_record(base, "Node*", "owner", 0);
    shared->store_record(reinterpret_cast<unsigned long>(source), base + 8,
                         "Field*", "left", 0);
    shared->store_record(reinterpret_cast<unsigned long>(source), base + 16,
                         "Field*", "right", 0);

    if ((i & 7UL) == 0)
      sched_yield();

    free(owner);
    free(source);
  }

  shared->workers_done.fetch_add(1, std::memory_order_release);
  return nullptr;
}

static void *EnumeratorMain(void *arg) {
  SharedState *shared = reinterpret_cast<SharedState *>(arg);
  live_alloc_info_t page[32];
  unsigned long cursor = 0;

  while (true) {
    memset(page, 0, sizeof(page));
    const unsigned long written = shared->get_live_allocs(cursor, page, 32);
    if (written == 0) {
      cursor = 0;
      if (shared->workers_done.load(std::memory_order_acquire) ==
          kWorkerThreadCount)
        break;
      sched_yield();
      continue;
    }

    shared->scans.fetch_add(written, std::memory_order_relaxed);
    for (unsigned long i = 0; i < written; ++i) {
      if (page[i].base == 0 || page[i].size == 0) {
        RecordFailure(shared, "live-alloc-zero", page[i].id, page[i].base,
                      page[i].size);
        continue;
      }

      block_info_t block;
      memset(&block, 0, sizeof(block));
      if (shared->get_block_info(page[i].base, &block)) {
        if (!block.found || block.base != page[i].base ||
            block.size != page[i].size) {
          RecordFailure(shared, "live-alloc-block", page[i].base, block.base,
                        block.size);
        }
      }
    }

    cursor = page[written - 1].id + 1;
  }
  return nullptr;
}

static unsigned long CountLiveAllocs(memgraph_get_live_allocs_fn get_live_allocs) {
  unsigned long total = 0;
  unsigned long cursor = 0;
  live_alloc_info_t page[32];
  while (true) {
    memset(page, 0, sizeof(page));
    const unsigned long written = get_live_allocs(cursor, page, 32);
    if (written == 0)
      break;
    total += written;
    cursor = page[written - 1].id + 1;
  }
  return total;
}

}  // namespace

int main() {
  memgraph_init_fn init =
      reinterpret_cast<memgraph_init_fn>(LoadSym("memgraph_init"));
  alloc_record_fn alloc_record =
      reinterpret_cast<alloc_record_fn>(LoadSym("alloc_record"));
  store_record_fn store_record =
      reinterpret_cast<store_record_fn>(LoadSym("store_record"));
  memgraph_get_block_info_fn get_block_info =
      reinterpret_cast<memgraph_get_block_info_fn>(LoadSym("get_block_info"));
  memgraph_get_live_allocs_fn get_live_allocs =
      reinterpret_cast<memgraph_get_live_allocs_fn>(LoadSym("get_live_allocs"));

  if (!alloc_record || !store_record || !get_block_info || !get_live_allocs) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  const unsigned long baseline_live = CountLiveAllocs(get_live_allocs);

  SharedState shared = {};
  shared.alloc_record = alloc_record;
  shared.store_record = store_record;
  shared.get_block_info = get_block_info;
  shared.get_live_allocs = get_live_allocs;
  shared.workers_done.store(0, std::memory_order_relaxed);
  shared.failures.store(0, std::memory_order_relaxed);
  shared.scans.store(0, std::memory_order_relaxed);

  pthread_t workers[kWorkerThreadCount];
  pthread_t enumerator;
  if (pthread_create(&enumerator, nullptr, EnumeratorMain, &shared) != 0) {
    fprintf(stderr, "FAIL: pthread_create(enumerator) failed\n");
    return 3;
  }
  for (int i = 0; i < kWorkerThreadCount; ++i) {
    if (pthread_create(&workers[i], nullptr, WorkerMain, &shared) != 0) {
      fprintf(stderr, "FAIL: pthread_create(worker=%d) failed\n", i);
      return 4;
    }
  }

  for (int i = 0; i < kWorkerThreadCount; ++i)
    pthread_join(workers[i], nullptr);
  pthread_join(enumerator, nullptr);

  const unsigned long final_live = CountLiveAllocs(get_live_allocs);

  const unsigned long failures =
      shared.failures.load(std::memory_order_relaxed);
  const unsigned long scans = shared.scans.load(std::memory_order_relaxed);
  printf("live-allocs-free-race: scans=%lu failures=%lu baseline_live=%lu "
         "final_live=%lu\n",
         scans, failures, baseline_live, final_live);
  if (failures != 0 || final_live > baseline_live) {
    fprintf(stderr, "FAIL: live alloc enumeration race detected inconsistencies\n");
    return 5;
  }

  puts("PASS: live allocs/free race smoke completed");
  return 0;
}
