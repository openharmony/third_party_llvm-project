//===-- memgraph_frontend_business_block_member_smoke_min.cpp -----===//

#include "../../memgraph_interface.h"

#include <stddef.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
  Node *next;
  int value;
};

int main() {
  // Explicitly simulate the smallest "business code + manual metadata write"
  // flow.
  auto alloc_record =
      (void (*)(unsigned long, const char *, const char *, unsigned long))
          dlsym(RTLD_DEFAULT, "alloc_record");
  auto store_record =
      (void (*)(unsigned long, unsigned long, const char *, const char *,
                unsigned long))
          dlsym(RTLD_DEFAULT, "store_record");
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  auto get_member_info =
      (int (*)(unsigned long, member_info_t *))dlsym(
          RTLD_DEFAULT, "get_member_info");
  if (!alloc_record || !store_record || !get_block_info || !get_member_info)
    return fprintf(stderr, "missing alloc_record/store_record/query symbols\n"), 2;

  void *raw = malloc(sizeof(Node));
  if (!raw)
    return fprintf(stderr, "malloc failed\n"), 3;

  block_info_t block = {};
  if (!get_block_info((unsigned long)raw, &block) || block.base != (unsigned long)raw ||
      block.size != sizeof(Node) || block.malloc_pc == 0)
    return fprintf(stderr,
                   "hook check failed: base=0x%lx size=%lu malloc_pc=0x%lx\n",
                   block.base, block.size, block.malloc_pc),
           4;

  Node *node = (Node *)raw;
  node->next = node;
  node->value = 123;
  const unsigned long next_addr = (unsigned long)&node->next;
  volatile unsigned long sink =
      (unsigned long)node + (unsigned long)node->next + (unsigned long)node->value;
  (void)sink;

  const unsigned long expected_alloc_pc = (unsigned long)(void *)&main;
  alloc_record((unsigned long)raw, "Node*", "node", expected_alloc_pc);
  if (!get_block_info((unsigned long)raw, &block) || (!block.type_name && !block.name) ||
      block.malloc_pc != expected_alloc_pc)
    return fprintf(stderr,
                   "manual alloc_record check failed: type=%s name=%s malloc_pc=0x%lx\n",
                   block.type_name ? block.type_name : "<null>",
                   block.name ? block.name : "<null>", block.malloc_pc),
           5;

  const unsigned long expected_store_pc = 0x12345678UL;
  store_record((unsigned long)raw, next_addr, "Node*", "next",
               expected_store_pc);
  member_info_t member = {};
  if (!get_member_info(next_addr, &member) ||
      member.base != (unsigned long)raw || member.member_addr != next_addr ||
      member.offset != offsetof(Node, next) ||
      (!member.type_name && !member.name) ||
      member.store_pc != expected_store_pc)
    return fprintf(stderr,
                   "manual store_record check failed: offset=%lu type=%s "
                   "name=%s store_pc=0x%lx\n",
                   member.offset, member.type_name ? member.type_name : "<null>",
                   member.name ? member.name : "<null>", member.store_pc),
           6;

  Dl_info pc_info = {};
  dladdr((void *)block.malloc_pc, &pc_info);
  printf("PASS: base=0x%lx size=%lu malloc_pc=0x%lx module_base=0x%lx symbol=%s "
         "block_type=%s block_name=%s member_type=%s member_name=%s\n",
         block.base, block.size, block.malloc_pc,
         (unsigned long)pc_info.dli_fbase,
         pc_info.dli_sname ? pc_info.dli_sname : "<null>",
         block.type_name ? block.type_name : "<null>",
         block.name ? block.name : "<null>",
         member.type_name ? member.type_name : "<null>",
         member.name ? member.name : "<null>");
  free(raw);

  block_info_t gone_block = {};
  if (get_block_info((unsigned long)raw, &gone_block))
    return fprintf(stderr, "free check failed: stale block info remains\n"), 7;
  member_info_t gone_member = {};
  if (get_member_info(next_addr, &gone_member))
    return fprintf(stderr, "free check failed: stale member info remains\n"), 8;
  return 0;
}
