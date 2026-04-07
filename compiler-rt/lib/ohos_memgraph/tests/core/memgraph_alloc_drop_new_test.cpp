//===-- memgraph_alloc_drop_new_test.cpp -------------------------===//
//
// Verifies alloc drop-new behaviour when alloc capacity is intentionally reduced.
// Run with: OHOS_MEMGRAPH_ALLOC_TABLE_SIZE=4
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*memgraph_init_fn)();
typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name,
                                const char *var_name, unsigned long alloc_pc);
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_block_info_fn)(unsigned long base,
                                          block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(runtime_stats_t *out);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static int ExpectBlock(memgraph_get_block_info_fn get_block_info,
                       unsigned long base, int expect_found,
                       const char *label) {
  block_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_block_info(base, &info);
  printf("%s block ok=%d found=%d base=0x%lx size=%lu\n", label, ok, info.found,
         info.base, info.size);
  return expect_found ? ok != 0 : ok == 0;
}

static int ExpectMember(memgraph_get_member_info_fn get_member_info,
                        unsigned long base, unsigned long member_addr,
                        int expect_found, const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_member_info(member_addr, &info);
  printf("%s member ok=%d found=%d base=0x%lx member=0x%lx offset=%lu\n",
         label, ok, info.found, info.base, info.member_addr, info.offset);
  return expect_found ? ok != 0 : ok == 0;
}

int main() {
  static constexpr unsigned long kAllocs = 6;
  static constexpr unsigned long kExpectedCap = 4;
  static constexpr unsigned long kObjSize = 32;

  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym("get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym("get_runtime_stats");

  if (!alloc_record || !store_record || !get_block_info || !get_member_info ||
      !get_runtime_stats) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  runtime_stats_t st = {};
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: initial get_runtime_stats failed\n");
    return 3;
  }
  const unsigned long baseline_alloc_live = st.alloc_live_current;
  const unsigned long baseline_store_live = st.store_live_current;
  const unsigned long expected_new_tracked =
      baseline_alloc_live >= kExpectedCap ? 0 : (kExpectedCap - baseline_alloc_live);
  printf("initial alloc_capacity=%lu alloc_live=%lu store_live=%lu\n",
         st.alloc_capacity_current, st.alloc_live_current,
         st.store_live_current);
  if (st.alloc_capacity_current > kExpectedCap) {
    puts("SKIP: set OHOS_MEMGRAPH_ALLOC_TABLE_SIZE=4 to exercise alloc drop-new");
    return 0;
  }

  void *ptrs[kAllocs] = {};
  for (unsigned long i = 0; i < kAllocs; ++i) {
    ptrs[i] = malloc(kObjSize);
    if (!ptrs[i]) {
      fprintf(stderr, "FAIL: malloc %lu failed\n", i);
      return 4;
    }
    alloc_record((unsigned long)ptrs[i], "AllocFifo*", "owner", 0);
    store_record((unsigned long)ptrs[i], (unsigned long)ptrs[i], "Field*",
                 "self", 0);
  }

  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: get_runtime_stats after allocs failed\n");
    return 5;
  }

  printf("after allocs: alloc_live=%lu alloc_peak=%lu store_live=%lu "
         "alloc_cap=%lu\n",
         st.alloc_live_current, st.alloc_live_peak, st.store_live_current,
         st.alloc_capacity_current);

  int ok = 1;
  ok &= st.alloc_capacity_current == kExpectedCap;
  ok &= st.alloc_live_current == kExpectedCap;
  ok &= st.store_live_current == baseline_store_live + expected_new_tracked;

  for (unsigned long i = 0; i < kAllocs; ++i) {
    const int expect_found = i < expected_new_tracked;
    char label[64];
    snprintf(label, sizeof(label), "slot%lu", i);
    ok &= ExpectBlock(get_block_info, (unsigned long)ptrs[i], expect_found,
                      label);
    ok &= ExpectMember(get_member_info, (unsigned long)ptrs[i],
                       (unsigned long)ptrs[i], expect_found, label);
  }

  for (unsigned long i = 0; i < kAllocs; ++i)
    free(ptrs[i]);

  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: final get_runtime_stats failed\n");
    return 6;
  }

  printf("after free: alloc_live=%lu store_live=%lu\n",
         st.alloc_live_current, st.store_live_current);

  ok &= st.alloc_live_current <= baseline_alloc_live;
  ok &= st.store_live_current <= baseline_store_live;

  if (!ok) {
    fprintf(stderr, "FAIL: alloc drop-new validation failed\n");
    return 7;
  }

  puts("PASS: alloc capacity drop-new path works");
  return 0;
}
