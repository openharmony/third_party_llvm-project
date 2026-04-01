//===-- memgraph_ohos_e2e_test.cpp -------------------------------===//
//
// End-to-end verification for the OHOS memgraph runtime.
// Verifies block/member query split, target-block ownership and stale cleanup.
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
typedef int (*memgraph_get_info_fn)(
    unsigned long base, alloc_info_t *out);
typedef unsigned long (*memgraph_get_info_records_fn)(
    unsigned long base, info_record_t *out, unsigned long capacity);
typedef int (*memgraph_get_block_info_fn)(
    unsigned long base, block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);
typedef int (*memgraph_get_owner_fn)(
    unsigned long addr, owner_info_t *out);
typedef unsigned long (*memgraph_get_live_allocs_fn)(
    unsigned long cursor, live_alloc_info_t *out, unsigned long capacity);
typedef int (*memgraph_get_runtime_stats_fn)(
    runtime_stats_t *out);

struct RecordBuffer {
  info_record_t *records;
  unsigned long count;
};

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

static int ExpectTracked(memgraph_get_info_fn get_info, void *ptr,
                         unsigned long expect_size,
                         unsigned long expect_records, const char *label) {
  alloc_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_info((unsigned long)ptr, &info);
  printf("%s tracked? ok=%d base=0x%lx size=%lu records=%lu\n", label, ok,
         info.base, info.size, info.record_count);
  return ok && info.base == (unsigned long)ptr && info.size == expect_size &&
         info.record_count == expect_records;
}

static int ExpectBlock(memgraph_get_block_info_fn get_block_info,
                       unsigned long base, int expect_found,
                       unsigned long expect_size, const char *expect_type,
                       const char *expect_name, const char *label) {
  block_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_block_info(base, &info);
  printf("%s block? ok=%d found=%d base=0x%lx size=%lu type=%s name=%s\n",
         label, ok, info.found, info.base, info.size,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  if (!expect_found)
    return !ok;
  return ok && info.found == expect_found && info.base == base &&
         info.size == expect_size && SameString(info.type_name, expect_type) &&
         SameString(info.name, expect_name);
}

static int ExpectMember(memgraph_get_member_info_fn get_member_info,
                        unsigned long member_addr, int expect_found,
                        unsigned long expect_base,
                        unsigned long expect_offset,
                        const char *expect_type, const char *expect_name,
                        unsigned long expect_source_addr,
                        const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_member_info(member_addr, &info);
  printf("%s member? ok=%d found=%d base=0x%lx member=0x%lx offset=%lu "
         "type=%s name=%s source=0x%lx\n",
         label, ok, info.found, info.base, info.member_addr, info.offset,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>", info.source_addr);
  if (!expect_found)
    return !ok;
  return ok && info.found == expect_found && info.base == expect_base &&
         info.member_addr == member_addr && info.offset == expect_offset &&
         SameString(info.type_name, expect_type) &&
         SameString(info.name, expect_name) &&
         info.source_addr == expect_source_addr;
}

static int ExpectOwner(memgraph_get_owner_fn get_owner, unsigned long addr,
                       int expect_found, unsigned long expect_base,
                       unsigned long expect_size, const char *label) {
  owner_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_owner(addr, &info);
  printf("%s owner? ok=%d found=%d base=0x%lx size=%lu\n", label, ok,
         info.found, info.base, info.size);
  if (!expect_found)
    return !ok;
  return ok && info.found == expect_found && info.base == expect_base &&
         info.size == expect_size;
}

static int FindLiveAlloc(memgraph_get_live_allocs_fn get_live_allocs,
                         unsigned long base, live_alloc_info_t *out) {
  unsigned long cursor = 0;
  live_alloc_info_t page[8];
  while (true) {
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

static RecordBuffer
LoadRecords(memgraph_get_info_records_fn get_info_records,
            void *ptr) {
  RecordBuffer buffer;
  buffer.records = nullptr;
  buffer.count = get_info_records((unsigned long)ptr, nullptr, 0);
  printf("records(base=0x%lx) total=%lu\n", (unsigned long)ptr, buffer.count);
  if (buffer.count == 0)
    return buffer;

  buffer.records = (info_record_t *)malloc(
      buffer.count * sizeof(*buffer.records));
  if (!buffer.records) {
    buffer.count = 0;
    return buffer;
  }

  unsigned long written =
      get_info_records((unsigned long)ptr, buffer.records, buffer.count);
  buffer.count = written;
  for (unsigned long i = 0; i < written; ++i) {
    printf("  record[%lu]: type=%s name=%s\n", i,
           buffer.records[i].type_name ? buffer.records[i].type_name : "<null>",
           buffer.records[i].name ? buffer.records[i].name : "<null>");
  }
  return buffer;
}

static void FreeRecords(RecordBuffer *buffer) {
  if (!buffer)
    return;
  free(buffer->records);
  buffer->records = nullptr;
  buffer->count = 0;
}

static unsigned long CountRecord(const RecordBuffer &buffer,
                                 const char *type_name, const char *var_name) {
  unsigned long count = 0;
  for (unsigned long i = 0; i < buffer.count; ++i) {
    if (SameString(buffer.records[i].type_name, type_name) &&
        SameString(buffer.records[i].name, var_name)) {
      ++count;
    }
  }
  return count;
}

static void *TryReuseAddress(unsigned long size, void *expect_addr,
                             unsigned long attempts) {
  for (unsigned long i = 0; i < attempts; ++i) {
    void *candidate = malloc(size);
    if (!candidate)
      return nullptr;
    if (candidate == expect_addr)
      return candidate;
    free(candidate);
  }
  return malloc(size);
}

int main() {
  memgraph_init_fn init =
      (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_info_fn get_info =
      (memgraph_get_info_fn)LoadSym(
          "get_info");
  memgraph_get_info_records_fn get_info_records =
      (memgraph_get_info_records_fn)LoadSym(
          "get_info_records");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym(
          "get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym(
          "get_member_info");
  memgraph_get_owner_fn get_owner =
      (memgraph_get_owner_fn)LoadSym("get_owner");
  memgraph_get_live_allocs_fn get_live_allocs =
      (memgraph_get_live_allocs_fn)LoadSym("get_live_allocs");
  memgraph_get_runtime_stats_fn get_runtime_stats =
      (memgraph_get_runtime_stats_fn)LoadSym(
          "get_runtime_stats");

  if (!alloc_record || !store_record || !get_info || !get_info_records ||
      !get_block_info || !get_member_info || !get_owner || !get_live_allocs) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }

  if (init)
    init();

  unsigned char *src = (unsigned char *)malloc(16);
  unsigned char *other = (unsigned char *)malloc(24);
  unsigned char *target = (unsigned char *)malloc(64);
  if (!src || !other || !target) {
    fprintf(stderr, "FAIL: malloc returned null\n");
    return 3;
  }
  const unsigned long target_base = (unsigned long)target;

  if (!ExpectTracked(get_info, src, 16, 0, "malloc(src)") ||
      !ExpectTracked(get_info, other, 24, 0, "malloc(other)") ||
      !ExpectTracked(get_info, target, 64, 0, "malloc(target)")) {
    fprintf(stderr,
            "FAIL: hook alloc tracking failed "
            "(get_info/get_live_allocs require "
            "OHOS_MEMGRAPH_OBSERVABILITY_ENABLED=1)\n");
    return 4;
  }
  if (!ExpectOwner(get_owner, target_base + 8, 1, target_base, 64,
                   "owner(target+8)") ||
      !ExpectOwner(get_owner, 1, 0, 0, 0, "owner(invalid)")) {
    fprintf(stderr, "FAIL: owner query semantics mismatch\n");
    return 5;
  }

  alloc_record(target_base, "Node*", "obj", 0);
  if (!ExpectTracked(get_info, target, 64, 1, "alloc_record(target)") ||
      !ExpectBlock(get_block_info, target_base, 1, 64, "Node*", "obj",
                   "block(target)")) {
    fprintf(stderr, "FAIL: alloc metadata write failed\n");
    return 6;
  }

  store_record((unsigned long)src, target_base, "int", "first", 0);
  store_record((unsigned long)src, target_base + 8, "int", "value", 0);
  store_record((unsigned long)other, target_base + 8, "char*", "name", 0);
  store_record((unsigned long)src, target_base + 16, nullptr, nullptr, 0);

  if (!ExpectTracked(get_info, target, 64, 4, "store_record(target-fields)")) {
    fprintf(stderr,
            "FAIL: record_count should reflect alloc + all appended member "
            "records\n");
    return 7;
  }

  live_alloc_info_t src_live = {};
  live_alloc_info_t other_live = {};
  live_alloc_info_t target_live = {};
  if (!FindLiveAlloc(get_live_allocs, (unsigned long)src, &src_live) ||
      !FindLiveAlloc(get_live_allocs, (unsigned long)other, &other_live) ||
      !FindLiveAlloc(get_live_allocs, target_base, &target_live) ||
      src_live.size != 16 || src_live.record_count != 0 ||
      other_live.size != 24 || other_live.record_count != 0 ||
      target_live.size != 64 || target_live.record_count != 4 ||
      !SameString(target_live.type_name, "Node*") ||
      !SameString(target_live.name, "obj")) {
    fprintf(stderr, "FAIL: live alloc export mismatch\n");
    return 8;
  }

  RecordBuffer records = LoadRecords(get_info_records, target);
  if (records.count != 4 || CountRecord(records, "Node*", "obj") != 1 ||
      CountRecord(records, "int", "first") != 1 ||
      CountRecord(records, "int", "value") != 1 ||
      CountRecord(records, "char*", "name") != 1) {
    fprintf(stderr,
            "FAIL: expected alloc metadata plus three member records with "
            "empty metadata ignored\n");
    FreeRecords(&records);
    return 9;
  }
  FreeRecords(&records);

  if (!ExpectMember(get_member_info, target_base, 1, target_base, 0, "int",
                    "first", (unsigned long)src, "member(target+0)") ||
      !ExpectMember(get_member_info, target_base + 8, 1, target_base, 8,
                    "char*", "name", (unsigned long)other,
                    "member(target+8 latest)") ||
      !ExpectMember(get_member_info, target_base + 16, 0, 0, 0, nullptr,
                    nullptr, 0, "member(target+16 ignored-empty)") ||
      !ExpectMember(get_member_info, target_base + 24, 0, 0, 0, nullptr,
                    nullptr, 0, "member(target+24 missing)")) {
    fprintf(stderr, "FAIL: member query semantics mismatch\n");
    return 10;
  }

  free(target);
  alloc_info_t after_free;
  block_info_t after_free_block;
  member_info_t after_free_member;
  memset(&after_free, 0, sizeof(after_free));
  memset(&after_free_block, 0, sizeof(after_free_block));
  memset(&after_free_member, 0, sizeof(after_free_member));
  if (get_info(target_base, &after_free) ||
      get_block_info(target_base, &after_free_block) ||
      get_member_info(target_base + 8, &after_free_member) ||
      FindLiveAlloc(get_live_allocs, target_base, nullptr)) {
    fprintf(stderr, "FAIL: freed block still exposes stale metadata\n");
    return 11;
  }
  if (!ExpectOwner(get_owner, target_base + 8, 0, 0, 0,
                   "owner(freed target+8)")) {
    fprintf(stderr, "FAIL: freed block should not resolve an owner\n");
    return 12;
  }

  unsigned char *reused = (unsigned char *)TryReuseAddress(64, target, 32768);
  if (!reused) {
    fprintf(stderr, "FAIL: could not allocate replacement block\n");
    return 13;
  }
  const int reused_same_addr = ((unsigned long)reused == target_base);
  if (!reused_same_addr) {
    printf("NOTE: allocator did not reuse exact address; continuing with a "
           "fresh block at 0x%lx\n",
           (unsigned long)reused);
  }
  if (!ExpectTracked(get_info, reused, 64, 0, "malloc(reused)") ||
      !ExpectMember(get_member_info, (unsigned long)reused + 8, 0, 0, 0,
                    nullptr, nullptr, 0,
                    "member(reused+8 before new store)")) {
    fprintf(stderr,
            "FAIL: replacement block still carries stale store metadata\n");
    return 14;
  }

  alloc_record((unsigned long)reused, "Buf*", "buf", 0);
  store_record((unsigned long)src, (unsigned long)reused + 8, "long", "next",
               0);
  if (!ExpectBlock(get_block_info, (unsigned long)reused, 1, 64, "Buf*", "buf",
                   "block(reused)") ||
      !ExpectMember(get_member_info, (unsigned long)reused + 8, 1,
                    (unsigned long)reused, 8, "long", "next",
                    (unsigned long)src, "member(reused+8 new store)")) {
    fprintf(stderr, "FAIL: reused address should only expose new metadata\n");
    return 15;
  }

  free(reused);
  free(src);
  free(other);

  if (get_runtime_stats) {
    runtime_stats_t st;
    memset(&st, 0, sizeof(st));
    if (get_runtime_stats(&st)) {
      printf("stats: runtime_peak_kb=%lu alloc_peak_kb=%lu store_peak_kb=%lu "
             "alloc_live_peak=%lu store_live_peak=%lu hooks(m=%lu f=%lu r=%lu) "
             "records(malloc=%lu store=%lu)\n",
             st.runtime_peak_bytes / 1024, st.alloc_table_peak_bytes / 1024,
             st.store_table_peak_bytes / 1024, st.alloc_live_peak,
             st.store_live_peak, st.malloc_hook_calls, st.free_hook_calls,
             st.realloc_hook_calls, st.malloc_record_calls,
             st.store_record_calls);
    }
  }

  puts("PASS: OHOS memgraph E2E path works");
  return 0;
}
