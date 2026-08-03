//===-- memgraph_mem_curve.cpp ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Prints runtime memory snapshots while alloc/store records grow.
//===----------------------------------------------------------------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name, const char *var_name,
                                unsigned long alloc_pc);
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_runtime_stats_fn)(
    runtime_stats_t *out);

static size_t ReadVmRssKB() {
  FILE *fp = fopen("/proc/self/status", "r");
  if (!fp)
    return 0;
  char line[256];
  size_t rss = 0;
  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "VmRSS:", 6) == 0) {
      unsigned long kb = 0;
      if (sscanf(line + 6, "%lu", &kb) == 1)
        rss = (size_t)kb;
      break;
    }
  }
  fclose(fp);
  return rss;
}

static unsigned long ParseArg(const char *s, unsigned long def) {
  if (!s || !*s)
    return def;
  char *end = nullptr;
  unsigned long v = strtoul(s, &end, 10);
  return (end && *end == '\0') ? v : def;
}

static unsigned long FieldCountForSize(unsigned long obj_size) {
  unsigned long words = obj_size / sizeof(unsigned long);
  if (words <= 1)
    return 1;
  if (words > 5)
    words = 5;
  return words - 1;
}

static unsigned long FieldOffset(unsigned long field_index) {
  return field_index * sizeof(unsigned long);
}

static void PrintSnapshot(const char *phase, unsigned long progress,
                          memgraph_get_runtime_stats_fn stats_fn) {
  if (!stats_fn)
    return;
  runtime_stats_t st;
  memset(&st, 0, sizeof(st));
  if (!stats_fn(&st))
    return;
  printf("[curve] phase=%s progress=%lu alloc_live=%lu store_live=%lu "
         "runtime_kb=%lu alloc_kb=%lu store_kb=%lu vmrss_kb=%zu "
         "alloc_cap=%lu alloc_slabs=%lu buckets=%lu bucket_pages=%lu "
         "store_cap=%lu store_slabs=%lu\n",
         phase, progress, st.alloc_live_current, st.store_live_current,
         st.runtime_current_bytes / 1024, st.alloc_table_current_bytes / 1024,
         st.store_table_current_bytes / 1024, ReadVmRssKB(),
         st.alloc_capacity_current, st.alloc_slab_count_current,
         st.alloc_bucket_count_current, st.alloc_bucket_page_count_current,
         st.store_capacity_current, st.store_slab_count_current);
}

int main(int argc, char **argv) {
  const unsigned long alloc_ops =
      ParseArg(argc > 1 ? argv[1] : nullptr, 300000);
  unsigned long store_ops = ParseArg(argc > 2 ? argv[2] : nullptr, 600000);
  const unsigned long obj_size = ParseArg(argc > 3 ? argv[3] : nullptr, 32);
  const unsigned long alloc_step =
      ParseArg(argc > 4 ? argv[4] : nullptr, 50000);
  const unsigned long store_step =
      ParseArg(argc > 5 ? argv[5] : nullptr, 50000);

  const char *store_types[] = {"Node*", "Edge*", "Payload*"};
  const char *store_vars[] = {"head", "next", "owner"};
  const unsigned long store_meta_count =
      sizeof(store_types) / sizeof(store_types[0]);
  const unsigned long field_count = FieldCountForSize(obj_size);
  const unsigned long max_unique_store = alloc_ops * field_count;
  if (store_ops > max_unique_store) {
    printf(
        "[curve] clamp store_ops from %lu to %lu to match unique owner+offset "
        "space\n",
        store_ops, max_unique_store);
    store_ops = max_unique_store;
  }

  alloc_record_fn alloc_record =
      (alloc_record_fn)dlsym(RTLD_DEFAULT, "alloc_record");
  store_record_fn store_record =
      (store_record_fn)dlsym(RTLD_DEFAULT, "store_record");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)dlsym(
          RTLD_DEFAULT, "get_runtime_stats");
  if (!alloc_record || !store_record || !get_runtime_stats) {
    fprintf(
        stderr,
        "FAIL: runtime symbols missing. Ensure LD_PRELOAD is set.\n");
    return 2;
  }

  void **ptrs = (void **)malloc(sizeof(void *) * alloc_ops);
  if (!ptrs) {
    fprintf(stderr, "FAIL: temporary pointer array allocation failed\n");
    return 3;
  }
  memset(ptrs, 0, sizeof(void *) * alloc_ops);

  printf("[curve] alloc_ops=%lu store_ops=%lu obj_size=%lu field_count=%lu "
         "alloc_step=%lu store_step=%lu\n",
         alloc_ops, store_ops, obj_size, field_count, alloc_step, store_step);
  PrintSnapshot("init", 0, get_runtime_stats);

  unsigned long alloc_done = 0;
  for (unsigned long i = 0; i < alloc_ops; ++i) {
    ptrs[i] = malloc(obj_size);
    if (!ptrs[i])
      continue;
    alloc_record((unsigned long)ptrs[i], "Node*", "head", 0);
    ++alloc_done;
    if (((i + 1) % alloc_step) == 0 || (i + 1) == alloc_ops)
      PrintSnapshot("alloc", i + 1, get_runtime_stats);
  }

  unsigned long store_done = 0;
  for (unsigned long i = 0; i < store_ops; ++i) {
    if (alloc_ops == 0)
      break;
    const unsigned long owner_idx = i % alloc_ops;
    const unsigned long field_idx = (i / alloc_ops) % field_count;
    const unsigned long meta_idx = i % store_meta_count;
    void *owner = ptrs[owner_idx];
    void *source = ptrs[(owner_idx + 17) % alloc_ops];
    if (!owner || !source)
      continue;

    const unsigned long dst = (unsigned long)owner + FieldOffset(field_idx);
    store_record((unsigned long)source, dst, store_types[meta_idx],
                 store_vars[meta_idx], 0);
    ++store_done;
    if (((i + 1) % store_step) == 0 || (i + 1) == store_ops)
      PrintSnapshot("store", i + 1, get_runtime_stats);
  }

  for (unsigned long i = 0; i < alloc_ops; ++i)
    free(ptrs[i]);
  free(ptrs);
  PrintSnapshot("free", alloc_done, get_runtime_stats);

  if (alloc_done == 0) {
    fprintf(stderr, "FAIL: no allocations succeeded\n");
    return 4;
  }
  printf("PASS: memgraph memory curve finished alloc=%lu store=%lu\n",
         alloc_done, store_done);
  return 0;
}
