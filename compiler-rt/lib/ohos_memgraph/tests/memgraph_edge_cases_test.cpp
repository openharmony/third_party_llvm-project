//===-- memgraph_edge_cases_test.cpp ------------------------------===//
//
// Additional coverage for partial metadata, invalid member queries and realloc.
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
typedef int (*memgraph_get_block_info_fn)(
    unsigned long base, block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);

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
                        unsigned long base, unsigned long member_addr,
                        int expect_found, unsigned long expect_offset,
                        const char *expect_type, const char *expect_name,
                        const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  int ok = get_member_info(member_addr, &info);
  printf("%s member? ok=%d found=%d base=0x%lx member=0x%lx offset=%lu "
         "type=%s name=%s\n",
         label, ok, info.found, info.base, info.member_addr, info.offset,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  if (!expect_found)
    return !ok;
  return ok && info.found == expect_found && info.base == base &&
         info.member_addr == member_addr && info.offset == expect_offset &&
         SameString(info.type_name, expect_type) &&
         SameString(info.name, expect_name);
}

static int VerifyPartialAndInvalidQueries(
    alloc_record_fn alloc_record, store_record_fn store_record,
    memgraph_get_block_info_fn get_block_info,
    memgraph_get_member_info_fn get_member_info) {
  unsigned char *source = (unsigned char *)malloc(32);
  unsigned char *owner = (unsigned char *)malloc(64);
  unsigned char *other = (unsigned char *)malloc(64);
  if (!source || !owner || !other) {
    fprintf(stderr, "FAIL: edge-case allocation failed\n");
    return 0;
  }

  const unsigned long owner_base = (unsigned long)owner;
  const unsigned long other_base = (unsigned long)other;
  alloc_record(owner_base, "EdgeCase*", "owner", 0);
  store_record((unsigned long)source, owner_base + 8, "OnlyType", nullptr, 0);
  store_record((unsigned long)source, owner_base + 16, nullptr, "only_name",
               0);

  const int ok =
      ExpectBlock(get_block_info, owner_base, 1, 64, "EdgeCase*", "owner",
                  "edge block(owner)") &&
      ExpectMember(get_member_info, owner_base, owner_base + 8, 1, 8,
                   "OnlyType", nullptr, "edge member(type-only)") &&
      ExpectMember(get_member_info, owner_base, owner_base + 16, 1, 16, nullptr,
                   "only_name", "edge member(name-only)") &&
      ExpectMember(get_member_info, owner_base, owner_base - 8, 0, 0, nullptr,
                   nullptr, "edge member(before-base)") &&
      ExpectMember(get_member_info, owner_base, owner_base + 64, 0, 0, nullptr,
                   nullptr, "edge member(out-of-range)") &&
      ExpectMember(get_member_info, other_base, owner_base + 8, 0, 0, nullptr,
                   nullptr, "edge member(wrong-base)");

  free(other);
  free(owner);
  free(source);
  return ok;
}

static int VerifySameAddressRealloc(
    alloc_record_fn alloc_record, store_record_fn store_record,
    memgraph_get_block_info_fn get_block_info,
    memgraph_get_member_info_fn get_member_info) {
  for (unsigned long attempt = 0; attempt < 256; ++attempt) {
    unsigned char *source = (unsigned char *)malloc(32);
    unsigned char *ptr = (unsigned char *)malloc(128);
    if (!source || !ptr) {
      free(ptr);
      free(source);
      return 0;
    }

    const unsigned long base = (unsigned long)ptr;
    alloc_record(base, "Shrink*", "same", 0);
    store_record((unsigned long)source, base + 8, "int", "keep", 0);

    unsigned char *shrunk = (unsigned char *)realloc(ptr, 64);
    if (!shrunk) {
      free(source);
      return 0;
    }
    if (shrunk != ptr) {
      free(shrunk);
      free(source);
      continue;
    }

    const int ok = ExpectBlock(get_block_info, base, 1, 64, "Shrink*", "same",
                               "realloc same-address block") &&
                   ExpectMember(get_member_info, base, base + 8, 0, 0, nullptr,
                                nullptr, "realloc same-address member cleared");
    free(shrunk);
    free(source);
    return ok;
  }

  printf("NOTE: allocator did not produce a same-address realloc case; "
         "skipping that sub-check\n");
  return 1;
}

static int
VerifyMovedRealloc(alloc_record_fn alloc_record, store_record_fn store_record,
                   memgraph_get_block_info_fn get_block_info,
                   memgraph_get_member_info_fn get_member_info) {
  for (unsigned long attempt = 0; attempt < 64; ++attempt) {
    unsigned char *source = (unsigned char *)malloc(32);
    unsigned char *ptr = (unsigned char *)malloc(64);
    unsigned char *blocker = (unsigned char *)malloc(64);
    if (!source || !ptr || !blocker) {
      free(blocker);
      free(ptr);
      free(source);
      return 0;
    }

    const unsigned long old_base = (unsigned long)ptr;
    alloc_record(old_base, "Grow*", "move", 0);
    store_record((unsigned long)source, old_base + 8, "long", "old", 0);

    unsigned char *grown = (unsigned char *)realloc(ptr, 1UL << 20);
    if (!grown) {
      free(blocker);
      free(source);
      return 0;
    }
    if (grown == ptr) {
      free(grown);
      free(blocker);
      free(source);
      continue;
    }

    const unsigned long new_base = (unsigned long)grown;
    const int ok =
        ExpectBlock(get_block_info, old_base, 0, 0, nullptr, nullptr,
                    "realloc moved old block gone") &&
        ExpectMember(get_member_info, old_base, old_base + 8, 0, 0, nullptr,
                     nullptr, "realloc moved old member gone") &&
        ExpectBlock(get_block_info, new_base, 1, 1UL << 20, nullptr, nullptr,
                    "realloc moved new block tracked");
    free(grown);
    free(blocker);
    free(source);
    return ok;
  }

  printf("NOTE: allocator did not produce a moved realloc case; skipping that "
         "sub-check\n");
  return 1;
}

int main() {
  memgraph_init_fn init =
      (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym(
          "get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym(
          "get_member_info");

  if (!alloc_record || !store_record || !get_block_info || !get_member_info) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  if (!VerifyPartialAndInvalidQueries(alloc_record, store_record, get_block_info,
                                      get_member_info)) {
    fprintf(stderr,
            "FAIL: partial metadata or invalid query coverage failed\n");
    return 3;
  }
  if (!VerifySameAddressRealloc(alloc_record, store_record, get_block_info,
                                get_member_info)) {
    fprintf(stderr, "FAIL: same-address realloc coverage failed\n");
    return 4;
  }
  if (!VerifyMovedRealloc(alloc_record, store_record, get_block_info,
                          get_member_info)) {
    fprintf(stderr, "FAIL: moved realloc coverage failed\n");
    return 5;
  }

  puts("PASS: OHOS memgraph edge cases work");
  return 0;
}
