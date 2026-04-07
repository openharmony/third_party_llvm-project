//===-- memgraph_perf_bench.cpp ----------------------------------===//
//
// Performance benchmark for the OHOS memgraph runtime.
// Measures hook allocation throughput, member metadata calls and split queries.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef void (*alloc_record_fn)(unsigned long malloc_addr,
                                const char *type_name, const char *var_name,
                                unsigned long alloc_pc);
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_block_info_fn)(
    unsigned long base, block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_runtime_stats_fn)(
    runtime_stats_t *out);
typedef int (*memgraph_get_layout_fn)(
    unsigned long *alloc_row_bytes, unsigned long *store_row_bytes);

static uint64_t NowNs() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

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

static unsigned long CoveredFields(unsigned long alloc_ops,
                                   unsigned long store_ops,
                                   unsigned long field_count) {
  if (alloc_ops == 0 || field_count == 0)
    return 0;
  unsigned long rounds = store_ops / alloc_ops;
  if ((store_ops % alloc_ops) != 0)
    ++rounds;
  return rounds < field_count ? rounds : field_count;
}

int main(int argc, char **argv) {
  const unsigned long alloc_ops =
      ParseArg(argc > 1 ? argv[1] : nullptr, 500000);
  unsigned long store_ops = ParseArg(argc > 2 ? argv[2] : nullptr, 1000000);
  const unsigned long obj_size = ParseArg(argc > 3 ? argv[3] : nullptr, 32);
  const unsigned long query_samples =
      ParseArg(argc > 4 ? argv[4] : nullptr, 20000);

  const char *type_names[] = {"Node*", "Edge*", "Payload*"};
  const char *var_names[] = {"head",  "next",    "prev",
                             "owner", "payload", "link"};
  const unsigned long type_count = sizeof(type_names) / sizeof(type_names[0]);
  const unsigned long var_count = sizeof(var_names) / sizeof(var_names[0]);
  const unsigned long field_count = FieldCountForSize(obj_size);
  const unsigned long max_unique_store = alloc_ops * field_count;
  if (store_ops > max_unique_store) {
    printf("[bench] clamp store_ops from %lu to %lu to avoid overwriting the "
           "same owner+offset keys\n",
           store_ops, max_unique_store);
    store_ops = max_unique_store;
  }
  const unsigned long covered_fields =
      CoveredFields(alloc_ops, store_ops, field_count);

  alloc_record_fn alloc_record =
      (alloc_record_fn)dlsym(RTLD_DEFAULT, "alloc_record");
  store_record_fn store_record =
      (store_record_fn)dlsym(RTLD_DEFAULT, "store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)dlsym(
          RTLD_DEFAULT, "get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)dlsym(
          RTLD_DEFAULT, "get_member_info");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)dlsym(
          RTLD_DEFAULT, "get_runtime_stats");
  memgraph_get_layout_fn get_layout =
      (memgraph_get_layout_fn)dlsym(
          RTLD_DEFAULT, "get_layout");
  if (!alloc_record || !store_record || !get_block_info || !get_member_info) {
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

  uint64_t t0 = NowNs();
  unsigned long alloc_ok = 0;
  for (unsigned long i = 0; i < alloc_ops; ++i) {
    ptrs[i] = malloc(obj_size);
    if (ptrs[i])
      ++alloc_ok;
  }
  uint64_t t1 = NowNs();

  unsigned long malloc_record_ok = 0;
  for (unsigned long i = 0; i < alloc_ops; ++i) {
    if (!ptrs[i])
      continue;
    alloc_record((unsigned long)ptrs[i], type_names[i % type_count],
                 var_names[i % var_count], 0);
    ++malloc_record_ok;
  }
  uint64_t t2 = NowNs();

  unsigned long store_done = 0;
  for (unsigned long i = 0; i < store_ops; ++i) {
    void *owner = ptrs[i % alloc_ops];
    void *source = ptrs[(i * 2654435761u + 17u) % alloc_ops];
    const unsigned long field_idx = (i / alloc_ops) % field_count;
    if (!owner || !source)
      continue;
    unsigned long dst = (unsigned long)owner + FieldOffset(field_idx);
    store_record((unsigned long)source, dst, type_names[(i + 1) % type_count],
                 var_names[(i + 2) % var_count], 0);
    ++store_done;
  }
  uint64_t t3 = NowNs();

  unsigned long block_found = 0;
  unsigned long member_found = 0;
  for (unsigned long i = 0; i < query_samples; ++i) {
    const unsigned long idx = (i * 1315423911u + 7u) % alloc_ops;
    void *ptr = ptrs[idx];
    if (!ptr)
      continue;

    if ((i & 1u) == 0) {
      block_info_t info;
      memset(&info, 0, sizeof(info));
      if (get_block_info((unsigned long)ptr, &info))
        ++block_found;
    } else {
      const unsigned long field_idx = (i / 2) % field_count;
      member_info_t info;
      memset(&info, 0, sizeof(info));
      if (get_member_info((unsigned long)ptr + FieldOffset(field_idx), &info)) {
        ++member_found;
      }
    }
  }
  uint64_t t4 = NowNs();

  for (unsigned long i = 0; i < alloc_ops; ++i)
    free(ptrs[i]);
  free(ptrs);
  uint64_t t5 = NowNs();

  const double alloc_s = (double)(t1 - t0) / 1e9;
  const double malloc_record_s = (double)(t2 - t1) / 1e9;
  const double store_s = (double)(t3 - t2) / 1e9;
  const double query_s = (double)(t4 - t3) / 1e9;
  const double free_s = (double)(t5 - t4) / 1e9;

  printf("[bench] alloc_ops=%lu store_ops=%lu obj_size=%lu field_count=%lu "
         "covered_fields=%lu query_samples=%lu\n",
         alloc_ops, store_ops, obj_size, field_count, covered_fields,
         query_samples);
  printf("[bench] malloc hook alloc_ok=%lu time=%.3fs qps=%.2f Mops/s\n",
         alloc_ok, alloc_s, alloc_ops / alloc_s / 1e6);
  printf("[bench] alloc_record ok=%lu time=%.3fs qps=%.2f Mops/s\n",
         malloc_record_ok, malloc_record_s,
         malloc_record_ok / malloc_record_s / 1e6);
  printf("[bench] store_record done=%lu time=%.3fs qps=%.2f Mops/s\n",
         store_done, store_s, (store_done ? store_done : 1) / store_s / 1e6);
  printf("[bench] query block_found=%lu/%lu member_found=%lu/%lu time=%.3fs\n",
         block_found, query_samples / 2, member_found, query_samples / 2,
         query_s);
  printf("[bench] expected member hit ratio ~= %lu/%lu based on store rounds\n",
         covered_fields, field_count);
  printf("[bench] free time=%.3fs VmRSS=%zu KB\n", free_s, ReadVmRssKB());

  if (get_runtime_stats) {
    runtime_stats_t st;
    memset(&st, 0, sizeof(st));
    if (get_runtime_stats(&st)) {
      printf("[bench] runtime_mem current=%lu KB peak=%lu KB\n",
             st.runtime_current_bytes / 1024, st.runtime_peak_bytes / 1024);
      printf("[bench] alloc/store/type/var peak_kb=%lu/%lu/%lu/%lu "
             "alloc_live_peak=%lu store_live_peak=%lu\n",
             st.alloc_table_peak_bytes / 1024, st.store_table_peak_bytes / 1024,
             st.type_table_peak_bytes / 1024, st.var_table_peak_bytes / 1024,
             st.alloc_live_peak, st.store_live_peak);
      printf(
          "[bench] capacity alloc=%lu slabs=%lu buckets=%lu bucket_pages=%lu "
          "store=%lu store_slabs=%lu\n",
          st.alloc_capacity_current, st.alloc_slab_count_current,
          st.alloc_bucket_count_current, st.alloc_bucket_page_count_current,
          st.store_capacity_current, st.store_slab_count_current);
      printf("[bench] hooks malloc=%lu free=%lu realloc=%lu records malloc=%lu "
             "store=%lu\n",
             st.malloc_hook_calls, st.free_hook_calls, st.realloc_hook_calls,
             st.malloc_record_calls, st.store_record_calls);
    }
  }

  if (get_layout) {
    unsigned long alloc_row_bytes = 0;
    unsigned long store_row_bytes = 0;
    if (get_layout(&alloc_row_bytes, &store_row_bytes)) {
      printf("[bench] layout alloc_row_bytes=%lu store_row_bytes=%lu\n",
             alloc_row_bytes, store_row_bytes);
    }
  }

  if (alloc_ok == 0 || block_found == 0 || member_found == 0) {
    fprintf(stderr, "FAIL: benchmark sanity check failed\n");
    return 4;
  }

  puts("PASS: memgraph benchmark finished");
  return 0;
}
