//===-- memgraph_live_allocs_pagination_test.cpp ----------------===//
//
// Verify get_live_allocs() pagination semantics in a single-threaded setting.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*memgraph_init_fn)();
typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name, const char *var_name,
                                unsigned long alloc_pc);
typedef unsigned long (*memgraph_get_live_allocs_fn)(unsigned long cursor,
                                                     live_alloc_info_t *out,
                                                     unsigned long capacity);

namespace {
constexpr unsigned long kAllocCount = 23;
constexpr unsigned long kPageSize = 5;
}

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static long FindBase(void **ptrs, unsigned long count, unsigned long base) {
  for (unsigned long i = 0; i < count; ++i) {
    if ((unsigned long)ptrs[i] == base)
      return (long)i;
  }
  return -1;
}

static int EnumerateAll(memgraph_get_live_allocs_fn get_live_allocs, void **ptrs,
                        unsigned long count, unsigned long *ids_out) {
  unsigned char seen[kAllocCount];
  memset(seen, 0, sizeof(seen));

  live_alloc_info_t page[kPageSize];
  memset(page, 0, sizeof(page));
  if (get_live_allocs(0, page, 0) != 0) {
    fprintf(stderr, "FAIL: get_live_allocs(cursor, out, 0) should return 0\n");
    return 0;
  }

  unsigned long cursor = 0;
  unsigned long total = 0;
  unsigned long last_id = 0;
  int have_last = 0;
  while (true) {
    memset(page, 0, sizeof(page));
    const unsigned long written = get_live_allocs(cursor, page, kPageSize);
    if (written == 0)
      break;
    for (unsigned long i = 0; i < written; ++i) {
      const live_alloc_info_t &row = page[i];
      printf("page row[%lu] id=%lu base=0x%lx size=%lu type=%s name=%s\n", i,
             row.id, row.base, row.size,
             row.type_name ? row.type_name : "<null>",
             row.name ? row.name : "<null>");
      if (have_last && row.id <= last_id) {
        fprintf(stderr, "FAIL: live alloc ids are not strictly increasing\n");
        return 0;
      }
      const long idx = FindBase(ptrs, count, row.base);
      if (idx < 0) {
        fprintf(stderr, "FAIL: unexpected base 0x%lx in pagination\n",
                row.base);
        return 0;
      }
      if (seen[idx]) {
        fprintf(stderr, "FAIL: duplicated base 0x%lx across pages\n", row.base);
        return 0;
      }
      seen[idx] = 1;
      if (ids_out)
        ids_out[total] = row.id;
      ++total;
      last_id = row.id;
      have_last = 1;
    }
    cursor = page[written - 1].id + 1;
  }

  if (total != count) {
    fprintf(stderr, "FAIL: expected %lu live allocs, got %lu\n", count, total);
    return 0;
  }
  for (unsigned long i = 0; i < count; ++i) {
    if (!seen[i]) {
      fprintf(stderr, "FAIL: base 0x%lx missing from pagination\n",
              (unsigned long)ptrs[i]);
      return 0;
    }
  }
  return 1;
}

int main() {
  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  memgraph_get_live_allocs_fn get_live_allocs =
      (memgraph_get_live_allocs_fn)LoadSym("get_live_allocs");
  if (!alloc_record || !get_live_allocs) {
    fprintf(stderr, "FAIL: required symbols missing\n");
    return 2;
  }
  if (init)
    init();

  void *ptrs[kAllocCount];
  unsigned long ids[kAllocCount];
  memset(ptrs, 0, sizeof(ptrs));
  memset(ids, 0, sizeof(ids));

  for (unsigned long i = 0; i < kAllocCount; ++i) {
    ptrs[i] = malloc(32);
    if (!ptrs[i])
      return fprintf(stderr, "FAIL: malloc(%lu) failed\n", i), 3;
    alloc_record((unsigned long)ptrs[i], "Paginated*", "owner", 0);
  }

  if (!EnumerateAll(get_live_allocs, ptrs, kAllocCount, ids))
    return 4;

  live_alloc_info_t page[kPageSize];
  memset(page, 0, sizeof(page));
  const unsigned long suffix = get_live_allocs(ids[7] + 1, page, kPageSize);
  printf("suffix from cursor=%lu written=%lu first_id=%lu\n", ids[7] + 1,
         suffix, suffix ? page[0].id : 0);
  if (suffix == 0 || page[0].id <= ids[7]) {
    fprintf(stderr, "FAIL: pagination cursor did not advance correctly\n");
    return 5;
  }

  for (unsigned long i = 0; i < kAllocCount; i += 2) {
    free(ptrs[i]);
    ptrs[i] = nullptr;
  }

  void *survivors[kAllocCount / 2];
  unsigned long survivor_count = 0;
  for (unsigned long i = 1; i < kAllocCount; i += 2)
    survivors[survivor_count++] = ptrs[i];

  if (!EnumerateAll(get_live_allocs, survivors, survivor_count, nullptr))
    return 6;

  for (unsigned long i = 1; i < kAllocCount; i += 2)
    free(ptrs[i]);

  puts("PASS: memgraph live alloc pagination test passed");
  return 0;
}
