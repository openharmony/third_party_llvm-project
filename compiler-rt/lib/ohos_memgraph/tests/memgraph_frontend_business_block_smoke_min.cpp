//===-- memgraph_frontend_business_block_smoke_min.cpp ------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
  Node *next;
  int value;
};

int main() {
  // This case focuses only on block-level metadata and does not cover
  // member-level writes.
  auto alloc_record =
      (void (*)(unsigned long, const char *, const char *, unsigned long))
          dlsym(RTLD_DEFAULT, "alloc_record");
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  if (!alloc_record || !get_block_info)
    return fprintf(stderr, "missing alloc_record/get_block_info\n"), 2;

  void *raw = malloc(sizeof(Node));
  if (!raw)
    return fprintf(stderr, "malloc failed\n"), 3;

  block_info_t before = {}, after = {};
  if (!get_block_info((unsigned long)raw, &before) || before.base != (unsigned long)raw ||
      before.size != sizeof(Node) || before.malloc_pc == 0)
    return fprintf(stderr,
                   "hook check failed: base=0x%lx size=%lu malloc_pc=0x%lx\n",
                   before.base, before.size, before.malloc_pc),
           4;

  Node *node = (Node *)raw;
  node->next = nullptr;
  node->value = 123;
  volatile unsigned long sink = (unsigned long)node + (unsigned long)node->next + (unsigned long)node->value;
  (void)sink;

  if (!get_block_info((unsigned long)raw, &after))
    return fprintf(stderr, "post-typed query failed\n"), 5;
  if (after.type_name || after.name)
    return fprintf(stderr, "unexpected metadata before manual alloc_record: type=%s name=%s\n",
                   after.type_name ? after.type_name : "<null>",
                   after.name ? after.name : "<null>"),
           6;

  const unsigned long expected_alloc_pc = (unsigned long)(void *)&main;
  alloc_record((unsigned long)raw, "Node*", "node", expected_alloc_pc);

  if (!get_block_info((unsigned long)raw, &after) || (!after.type_name && !after.name) ||
      after.malloc_pc != expected_alloc_pc)
    return fprintf(stderr,
                   "manual alloc_record check failed: type=%s name=%s malloc_pc=0x%lx\n",
                   after.type_name ? after.type_name : "<null>",
                   after.name ? after.name : "<null>", after.malloc_pc),
           7;

  printf("PASS: base=0x%lx size=%lu malloc_pc=0x%lx type=%s name=%s\n",
         after.base, after.size, after.malloc_pc,
         after.type_name ? after.type_name : "<null>",
         after.name ? after.name : "<null>");
  free(raw);
  block_info_t gone = {};
  if (get_block_info((unsigned long)raw, &gone))
    return fprintf(stderr, "free check failed: stale block info remains\n"), 8;
  return 0;
}
