//===-- memgraph_name_table_stress_test.cpp ---------------------===//
//
// Stress high-cardinality type/name interning and verify lookup correctness
// after multiple map/id-array growth steps.
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
typedef void (*store_record_fn)(unsigned long source_addr,
                                unsigned long dst_ptr, const char *type_name,
                                const char *var_name,
                                unsigned long store_pc);
typedef int (*memgraph_get_block_info_fn)(unsigned long base,
                                          block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);

namespace {
constexpr unsigned long kUniqueEntries = 5000;
constexpr unsigned long kOwnerBytes = (kUniqueEntries + 4) * 8;
}

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

static int ExpectMember(memgraph_get_member_info_fn get_member_info,
                        unsigned long member_addr, unsigned long expect_base,
                        unsigned long expect_offset, const char *expect_type,
                        const char *expect_name, const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_member_info(member_addr, &info);
  printf("%s member? ok=%d found=%d base=0x%lx member=0x%lx offset=%lu "
         "type=%s name=%s\n",
         label, ok, info.found, info.base, info.member_addr, info.offset,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  return ok && info.found && info.base == expect_base &&
         info.member_addr == member_addr && info.offset == expect_offset &&
         SameString(info.type_name, expect_type) &&
         SameString(info.name, expect_name);
}

static char *MakeLabel(const char *prefix, unsigned long index) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%s%04lu", prefix, index);
  const size_t len = strlen(buf) + 1;
  char *copy = (char *)malloc(len);
  if (!copy)
    return nullptr;
  memcpy(copy, buf, len);
  return copy;
}

int main() {
  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  alloc_record_fn alloc_record = (alloc_record_fn)LoadSym("alloc_record");
  store_record_fn store_record = (store_record_fn)LoadSym("store_record");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym("get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym("get_member_info");
  if (!alloc_record || !store_record || !get_block_info || !get_member_info) {
    fprintf(stderr, "FAIL: required memgraph symbols missing\n");
    return 2;
  }
  if (init)
    init();

  unsigned char *source = (unsigned char *)malloc(32);
  unsigned char *owner = (unsigned char *)malloc(kOwnerBytes);
  if (!source || !owner) {
    fprintf(stderr, "FAIL: setup allocation failed\n");
    free(owner);
    free(source);
    return 3;
  }

  const unsigned long owner_base = (unsigned long)owner;
  alloc_record(owner_base, "OwnerRoot*", "owner", 0);

  block_info_t block;
  memset(&block, 0, sizeof(block));
  if (!get_block_info(owner_base, &block) || !block.found ||
      !SameString(block.type_name, "OwnerRoot*") ||
      !SameString(block.name, "owner")) {
    fprintf(stderr, "FAIL: owner block metadata missing before stress\n");
    free(owner);
    free(source);
    return 4;
  }

  char **types = (char **)calloc(kUniqueEntries, sizeof(char *));
  char **names = (char **)calloc(kUniqueEntries, sizeof(char *));
  if (!types || !names) {
    fprintf(stderr, "FAIL: label table allocation failed\n");
    free(names);
    free(types);
    free(owner);
    free(source);
    return 5;
  }

  for (unsigned long i = 0; i < kUniqueEntries; ++i) {
    types[i] = MakeLabel("Type_", i);
    names[i] = MakeLabel("field_", i);
    if (!types[i] || !names[i]) {
      fprintf(stderr, "FAIL: failed to allocate labels for entry %lu\n", i);
      free(owner);
      free(source);
      return 6;
    }
    store_record((unsigned long)source, owner_base + i * 8, types[i], names[i],
                 0);
  }

  const char *dup_type = "RepeatedType*";
  const char *dup_name = "repeated_name";
  store_record((unsigned long)source, owner_base + kUniqueEntries * 8, dup_type,
               dup_name, 0);
  store_record((unsigned long)source, owner_base + (kUniqueEntries + 1) * 8,
               dup_type, dup_name, 0);

  if (!ExpectMember(get_member_info, owner_base, owner_base, 0, nullptr,
                    nullptr, "member[0] unexpected") ||
      !ExpectMember(get_member_info, owner_base + 8 * 37, owner_base, 8 * 37,
                    types[37], names[37], "member[37]") ||
      !ExpectMember(get_member_info, owner_base + 8 * 1024, owner_base,
                    8 * 1024, types[1024], names[1024], "member[1024]") ||
      !ExpectMember(get_member_info, owner_base + 8 * 4096, owner_base,
                    8 * 4096, types[4096], names[4096], "member[4096]") ||
      !ExpectMember(get_member_info, owner_base + 8 * (kUniqueEntries - 1),
                    owner_base, 8 * (kUniqueEntries - 1),
                    types[kUniqueEntries - 1], names[kUniqueEntries - 1],
                    "member[last]") ||
      !ExpectMember(get_member_info, owner_base + kUniqueEntries * 8, owner_base,
                    kUniqueEntries * 8, dup_type, dup_name,
                    "member[dup-0]") ||
      !ExpectMember(get_member_info, owner_base + (kUniqueEntries + 1) * 8,
                    owner_base, (kUniqueEntries + 1) * 8, dup_type, dup_name,
                    "member[dup-1]")) {
    free(owner);
    free(source);
    return 7;
  }

  // The runtime interns raw string pointers rather than copying strings, so the
  // stress labels intentionally remain live until process exit.
  puts("PASS: memgraph name table stress test passed");
  free(owner);
  free(source);
  return 0;
}
