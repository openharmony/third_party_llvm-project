//===-- memgraph_observability_flag_smoke.cpp --------------------===//
//
// Verifies that disabling observability leaves the main functionality intact,
// keeps get_live_allocs available, and disables the remaining monitoring-facing
// APIs.
//===----------------------------------------------------------------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
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
typedef int (*memgraph_get_block_info_fn)(unsigned long base, block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_owner_fn)(unsigned long addr, owner_info_t *out);
typedef int (*memgraph_get_info_fn)(unsigned long base, alloc_info_t *out);
typedef unsigned long (*memgraph_get_info_records_fn)(
    unsigned long base, info_record_t *out, unsigned long capacity);
typedef int (*memgraph_get_runtime_stats_fn)(runtime_stats_t *out);
typedef int (*memgraph_get_layout_fn)(unsigned long *alloc_row_bytes,
                                      unsigned long *store_row_bytes);
typedef unsigned long (*memgraph_get_live_allocs_fn)(
    unsigned long cursor, live_alloc_info_t *out, unsigned long capacity);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static int SameString(const char *lhs, const char *rhs) {
  if (!lhs && !rhs)
    return 1;
  if (!lhs || !rhs)
    return 0;
  return strcmp(lhs, rhs) == 0;
}

static int IsExpectedLiveAlloc(const live_alloc_info_t *row,
                               unsigned long source_base,
                               unsigned long target_base) {
  return row->base == source_base || row->base == target_base;
}

static int FindLiveAlloc(memgraph_get_live_allocs_fn get_live_allocs,
                         unsigned long base, live_alloc_info_t *out) {
  unsigned long cursor = 0;
  live_alloc_info_t page[8];
  while (1) {
    memset(page, 0, sizeof(page));
    const unsigned long written = get_live_allocs(cursor, page, 8);
    if (written == 0)
      return 0;
    for (unsigned long i = 0; i < written; ++i) {
      if (page[i].base == base) {
        if (out)
          *out = page[i];
        return 1;
      }
    }
    cursor = page[written - 1].id + 1;
  }
}

int main() {
  const char *flag = getenv("OHOS_MEMGRAPH_OBSERVABILITY_ENABLED");
  if (!flag || strcmp(flag, "0") != 0) {
    fprintf(stderr, "FAIL: run with OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=0\n");
    return 2;
  }

  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym("get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  memgraph_get_owner_fn get_owner =
      (memgraph_get_owner_fn)LoadSym("get_owner");
  memgraph_get_info_fn get_info = (memgraph_get_info_fn)LoadSym("get_info");
  memgraph_get_info_records_fn get_info_records =
      (memgraph_get_info_records_fn)LoadSym("get_info_records");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym("get_runtime_stats");
  memgraph_get_layout_fn get_layout =
      (memgraph_get_layout_fn)LoadSym("get_layout");
  memgraph_get_live_allocs_fn get_live_allocs =
      (memgraph_get_live_allocs_fn)LoadSym("get_live_allocs");

  if (!alloc_record || !store_record || !get_block_info || !get_member_info ||
      !get_owner || !get_info || !get_info_records ||
      !get_runtime_stats || !get_layout || !get_live_allocs) {
    fprintf(stderr, "FAIL: required symbols missing\n");
    return 3;
  }

  if (init)
    init();

  unsigned char *source = (unsigned char *)malloc(16);
  unsigned char *target = (unsigned char *)malloc(32);
  if (!source || !target) {
    fprintf(stderr, "FAIL: malloc returned null\n");
    return 4;
  }

  const unsigned long base = (unsigned long)target;
  const unsigned long member_addr = base + 8;

  alloc_record(base, "Node*", "root", 0);
  store_record((unsigned long)source, member_addr, "int", "value", 0);

  block_info_t block = {};
  member_info_t member = {};
  owner_info_t owner = {};
  if (!get_block_info(base, &block) || !block.found || block.base != base ||
      block.size != 32 || !SameString(block.type_name, "Node*") ||
      !SameString(block.name, "root")) {
    fprintf(stderr, "FAIL: block query should still work when observability is disabled\n");
    return 5;
  }
  if (!get_member_info(member_addr, &member) || !member.found ||
      member.base != base || member.member_addr != member_addr ||
      member.offset != 8 || !SameString(member.type_name, "int") ||
      !SameString(member.name, "value") ||
      member.source_addr != (unsigned long)source) {
    fprintf(stderr, "FAIL: member query should still work when observability is disabled\n");
    return 6;
  }
  if (!get_owner(member_addr, &owner) || !owner.found || owner.base != base ||
      owner.size != 32) {
    fprintf(stderr, "FAIL: owner query should still work when observability is disabled\n");
    return 7;
  }

  alloc_info_t info = {};
  runtime_stats_t stats = {};
  unsigned long alloc_row_bytes = 0;
  unsigned long store_row_bytes = 0;
  live_alloc_info_t live = {};
  if (get_info(base, &info)) {
    fprintf(stderr, "FAIL: get_info should be disabled\n");
    return 8;
  }
  if (get_info_records(base, nullptr, 0) != 0) {
    fprintf(stderr, "FAIL: get_info_records should be disabled\n");
    return 9;
  }
  if (get_runtime_stats(&stats)) {
    fprintf(stderr, "FAIL: get_runtime_stats should be disabled\n");
    return 10;
  }
  if (get_layout(&alloc_row_bytes, &store_row_bytes)) {
    fprintf(stderr, "FAIL: get_layout should be disabled\n");
    return 11;
  }
  if (!FindLiveAlloc(get_live_allocs, base, &live) &&
      !FindLiveAlloc(get_live_allocs, (unsigned long)source, &live)) {
    fprintf(stderr, "FAIL: get_live_allocs should stay available\n");
    return 12;
  }
  if (!IsExpectedLiveAlloc(&live, (unsigned long)source, base)) {
    fprintf(stderr, "FAIL: get_live_allocs did not report the tracked allocs\n");
    return 13;
  }

  free(target);
  free(source);
  printf("PASS: observability disabled while core functionality and live alloc enumeration still work\n");
  return 0;
}
