//===-- memgraph_store_drop_new_test.cpp -------------------------===//
//
// Verifies store drop-new behaviour once capacity exceeds the default 2M rows.
//===----------------------------------------------------------------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*memgraph_init_fn)();
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(
    runtime_stats_t *out);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static unsigned long FieldOffset(unsigned long field_index) {
  return field_index * sizeof(unsigned long);
}

static int ExpectMember(memgraph_get_member_info_fn get_member_info,
                        unsigned long base, unsigned long member_addr,
                        int expect_found, const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_member_info(member_addr, &info);
  printf("%s member? ok=%d found=%d base=0x%lx member=0x%lx offset=%lu\n",
         label, ok, info.found, info.base, info.member_addr, info.offset);
  return expect_found ? ok != 0 : ok == 0;
}

int main() {
  static constexpr unsigned long kAllocOps = 520000;
  static constexpr unsigned long kObjSize = 40;
  static constexpr unsigned long kFieldCount = 4;
  static constexpr unsigned long kStoreOps = kAllocOps * kFieldCount;
  static constexpr unsigned long kToleratedBoundarySkew = 1;

  memgraph_init_fn init =
      (memgraph_init_fn)LoadSym("memgraph_init");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym(
          "get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym(
          "get_runtime_stats");

  if (!store_record || !get_member_info || !get_runtime_stats) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  void **ptrs = (void **)malloc(sizeof(void *) * kAllocOps);
  if (!ptrs) {
    fprintf(stderr, "FAIL: pointer table allocation failed\n");
    return 3;
  }

  for (unsigned long i = 0; i < kAllocOps; ++i) {
    ptrs[i] = malloc(kObjSize);
    if (!ptrs[i]) {
      fprintf(stderr, "FAIL: owner allocation %lu failed\n", i);
      return 4;
    }
  }

  for (unsigned long i = 0; i < kStoreOps; ++i) {
    const unsigned long owner_idx = i % kAllocOps;
    const unsigned long field_idx = (i / kAllocOps) % kFieldCount;
    void *owner = ptrs[owner_idx];
    void *source = ptrs[(owner_idx + 17) % kAllocOps];
    const unsigned long dst = (unsigned long)owner + FieldOffset(field_idx);
    store_record((unsigned long)source, dst, "Evict*", "member", 0);
  }

  runtime_stats_t st;
  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: get_runtime_stats failed\n");
    return 5;
  }

  printf("capacity stats: store_live=%lu store_peak=%lu store_capacity=%lu\n",
         st.store_live_current, st.store_live_peak,
         st.store_capacity_current);

  const unsigned long first_base = (unsigned long)ptrs[0];
  const unsigned long survived_base = (unsigned long)ptrs[100000];
  const unsigned long tail_base = (unsigned long)ptrs[kAllocOps - 1];
  const unsigned long expected_capacity = 2000000;
  const int first_survives =
      ExpectMember(get_member_info, first_base, first_base + FieldOffset(0), 1,
                   "first key survives");
  const int later_survives =
      ExpectMember(get_member_info, survived_base,
                   survived_base + FieldOffset(0), 1, "later key survives");
  const int tail_dropped =
      ExpectMember(get_member_info, tail_base, tail_base + FieldOffset(3), 0,
                   "tail key dropped");
  const int stats_ok =
      st.store_capacity_current == expected_capacity &&
      st.store_live_current <= st.store_capacity_current &&
      st.store_live_current + kToleratedBoundarySkew >=
          st.store_capacity_current;
  const int ok = stats_ok && first_survives && later_survives && tail_dropped;

  for (unsigned long i = 0; i < kAllocOps; ++i)
    free(ptrs[i]);
  free(ptrs);

  memset(&st, 0, sizeof(st));
  if (!get_runtime_stats(&st)) {
    fprintf(stderr, "FAIL: get_runtime_stats after free failed\n");
    return 6;
  }
  printf("after free: store_live=%lu\n", st.store_live_current);

  if (!ok || st.store_live_current != 0) {
    fprintf(stderr, "FAIL: store capacity drop-new coverage failed\n");
    return 7;
  }

  puts("PASS: OHOS memgraph store capacity drop-new path works");
  return 0;
}
