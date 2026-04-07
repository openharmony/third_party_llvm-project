//===-- memgraph_slab_growth_race_smoke.cpp ----------------------===//
//
// Stress concurrent slab growth against query paths that read alloc/store rows.
// This keeps allocations live while readers race with growth so we can catch
// pointer-table publication issues without mixing in concurrent frees.
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
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_owner_fn)(unsigned long addr, owner_info_t *out);
typedef unsigned long (*memgraph_get_live_allocs_fn)(
    unsigned long cursor, live_alloc_info_t *out, unsigned long capacity);

namespace {

static constexpr unsigned long kAllocCount = 40000;
static constexpr int kReaderThreadCount = 4;
static constexpr unsigned long kOwnerSize = 64;
static constexpr unsigned long kFieldOffsets[4] = {0, 8, 16, 24};
static constexpr const char *kTypeNames[4] = {"Field0*", "Field1*", "Field2*",
                                              "Field3*"};
static constexpr const char *kVarNames[4] = {"field0", "field1", "field2",
                                             "field3"};

struct SharedState {
  alloc_record_fn alloc_record;
  store_record_fn store_record;
  memgraph_get_block_info_fn get_block_info;
  memgraph_get_member_info_fn get_member_info;
  memgraph_get_owner_fn get_owner;
  memgraph_get_live_allocs_fn get_live_allocs;
  unsigned char **owners;
  unsigned char *source_seed;
  std::atomic<unsigned long> published_count;
  std::atomic<int> writer_done;
  std::atomic<unsigned long> failures;
};

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static unsigned long NextRand(unsigned long *state) {
  *state = (*state * 6364136223846793005ULL) + 1ULL;
  return *state;
}

static void RecordFailure(SharedState *shared, const char *what,
                          unsigned long a, unsigned long b, unsigned long c) {
  const unsigned long index = shared->failures.fetch_add(1, std::memory_order_relaxed);
  if (index < 16) {
    fprintf(stderr, "FAIL[%lu]: %s a=%lu b=%lu c=%lu\n", index, what, a, b, c);
  }
}

static void *WriterMain(void *arg) {
  SharedState *shared = reinterpret_cast<SharedState *>(arg);
  for (unsigned long i = 0; i < kAllocCount; ++i) {
    unsigned char *owner =
        reinterpret_cast<unsigned char *>(malloc(kOwnerSize));
    if (!owner) {
      RecordFailure(shared, "malloc-null", i, 0, 0);
      free(owner);
      break;
    }

    const unsigned long base = reinterpret_cast<unsigned long>(owner);
    shared->alloc_record(base, "Node*", "owner", 0);
    for (unsigned long j = 0; j < 4; ++j) {
      shared->store_record(reinterpret_cast<unsigned long>(shared->source_seed),
                           base + kFieldOffsets[j], kTypeNames[j], kVarNames[j],
                           0);
    }

    shared->owners[i] = owner;
    shared->published_count.store(i + 1, std::memory_order_release);

    if ((i & 1023UL) == 0)
      sched_yield();
  }
  shared->writer_done.store(1, std::memory_order_release);
  return nullptr;
}

static unsigned long ValidateLivePage(SharedState *shared, unsigned long cursor) {
  live_alloc_info_t page[64];
  memset(page, 0, sizeof(page));
  const unsigned long written = shared->get_live_allocs(cursor, page, 64);
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
          block.size != page[i].size ||
          ((block.type_name || block.name) && page[i].record_count < 1)) {
        RecordFailure(shared, "live-alloc-block", page[i].id, block.base,
                      page[i].record_count);
      }
    }
  }
  return written;
}

static void *ReaderMain(void *arg) {
  SharedState *shared = reinterpret_cast<SharedState *>(arg);
  unsigned long rng =
      reinterpret_cast<uintptr_t>(pthread_self()) ^ 0x9e3779b97f4a7c15ULL;
  while (true) {
    const unsigned long count =
        shared->published_count.load(std::memory_order_acquire);
    if (count == 0) {
      if (shared->writer_done.load(std::memory_order_acquire))
        break;
      sched_yield();
      continue;
    }

    const unsigned long idx = NextRand(&rng) % count;
    unsigned char *owner = shared->owners[idx];
    if (!owner)
      continue;

    const unsigned long field_index = NextRand(&rng) & 3UL;
    const unsigned long member_addr =
        reinterpret_cast<unsigned long>(owner) + kFieldOffsets[field_index];

    owner_info_t owner_info;
    memset(&owner_info, 0, sizeof(owner_info));
    if (!shared->get_owner(member_addr, &owner_info) || !owner_info.found ||
        owner_info.base != reinterpret_cast<unsigned long>(owner) ||
        owner_info.size != kOwnerSize) {
      RecordFailure(shared, "owner-query", idx, owner_info.base,
                    owner_info.size);
    }

    member_info_t member_info;
    memset(&member_info, 0, sizeof(member_info));
    if (!shared->get_member_info(member_addr, &member_info) ||
        !member_info.found ||
        member_info.base != reinterpret_cast<unsigned long>(owner) ||
        member_info.member_addr != member_addr ||
        member_info.offset != kFieldOffsets[field_index]) {
      RecordFailure(shared, "member-query", idx, member_info.base,
                    member_info.offset);
    }

    if (shared->writer_done.load(std::memory_order_acquire) &&
        count == kAllocCount)
      break;
  }
  return nullptr;
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
  memgraph_get_member_info_fn get_member_info =
      reinterpret_cast<memgraph_get_member_info_fn>(
          LoadSym("get_member_info"));
  memgraph_get_owner_fn get_owner =
      reinterpret_cast<memgraph_get_owner_fn>(LoadSym("get_owner"));
  memgraph_get_live_allocs_fn get_live_allocs =
      reinterpret_cast<memgraph_get_live_allocs_fn>(LoadSym("get_live_allocs"));

  if (!alloc_record || !store_record || !get_block_info || !get_member_info ||
      !get_owner ||
      !get_live_allocs) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  SharedState shared = {};
  shared.alloc_record = alloc_record;
  shared.store_record = store_record;
  shared.get_block_info = get_block_info;
  shared.get_member_info = get_member_info;
  shared.get_owner = get_owner;
  shared.get_live_allocs = get_live_allocs;
  shared.source_seed = reinterpret_cast<unsigned char *>(malloc(32));
  shared.owners = reinterpret_cast<unsigned char **>(
      calloc(kAllocCount, sizeof(unsigned char *)));
  if (!shared.source_seed || !shared.owners) {
    fprintf(stderr, "FAIL: state allocation failed\n");
    free(shared.source_seed);
    free(shared.owners);
    return 3;
  }
  shared.published_count.store(0, std::memory_order_relaxed);
  shared.writer_done.store(0, std::memory_order_relaxed);
  shared.failures.store(0, std::memory_order_relaxed);

  pthread_t writer;
  pthread_t readers[kReaderThreadCount];
  if (pthread_create(&writer, nullptr, WriterMain, &shared) != 0) {
    fprintf(stderr, "FAIL: pthread_create(writer) failed\n");
    free(shared.source_seed);
    free(shared.owners);
    return 4;
  }
  for (int i = 0; i < kReaderThreadCount; ++i) {
    if (pthread_create(&readers[i], nullptr, ReaderMain, &shared) != 0) {
      fprintf(stderr, "FAIL: pthread_create(reader=%d) failed\n", i);
      return 5;
    }
  }

  pthread_join(writer, nullptr);
  for (int i = 0; i < kReaderThreadCount; ++i)
    pthread_join(readers[i], nullptr);

  unsigned long live_page_count = 0;
  {
    unsigned long cursor = 0;
    while (true) {
      live_alloc_info_t page[64];
      memset(page, 0, sizeof(page));
      const unsigned long written = ValidateLivePage(&shared, cursor);
      if (written == 0)
        break;
      const unsigned long confirm_written = get_live_allocs(cursor, page, 64);
      if (confirm_written == 0)
        break;
      live_page_count += written;
      cursor = page[confirm_written - 1].id + 1;
    }
  }

  for (unsigned long i = 0; i < kAllocCount; ++i)
    free(shared.owners[i]);
  free(shared.source_seed);
  free(shared.owners);

  const unsigned long failures =
      shared.failures.load(std::memory_order_relaxed);
  printf("slab-growth-race: published=%lu enumerated=%lu failures=%lu\n",
         kAllocCount, live_page_count, failures);
  if (failures != 0 || live_page_count == 0) {
    fprintf(stderr, "FAIL: slab growth race smoke detected inconsistencies\n");
    return 6;
  }

  puts("PASS: slab growth race smoke completed");
  return 0;
}
