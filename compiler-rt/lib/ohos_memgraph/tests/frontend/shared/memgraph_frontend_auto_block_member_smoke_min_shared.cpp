//===-- memgraph_frontend_auto_block_member_smoke_min_shared.cpp --===//

#include "../../../memgraph_interface.h"

#include <stddef.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
  Node *next;
  int value;
};

extern "C" __attribute__((visibility("default"))) int
memgraph_frontend_auto_block_member_smoke_min_run() {
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  auto get_member_info =
      (int (*)(unsigned long, member_info_t *))dlsym(
          RTLD_DEFAULT, "get_member_info");
  if (!get_block_info || !get_member_info)
    return fprintf(stderr, "missing query symbols\n"), 2;

  void *raw = malloc(sizeof(Node));
  void *other_raw = malloc(sizeof(Node));
  if (!raw || !other_raw)
    return fprintf(stderr, "malloc failed\n"), 3;

  block_info_t before = {}, after = {};
  if (!get_block_info((unsigned long)raw, &before) ||
      before.base != (unsigned long)raw || before.size != sizeof(Node))
    return fprintf(stderr, "hook check failed: base=0x%lx size=%lu\n",
                   before.base, before.size),
           4;

  Node *node = (Node *)raw;
  Node *other = (Node *)other_raw;
  node->next = other;
  node->value = 123;
  const unsigned long next_addr = (unsigned long)&node->next;
  volatile unsigned long sink =
      (unsigned long)node + (unsigned long)other + (unsigned long)node->next +
      (unsigned long)node->value;
  (void)sink;

  if (!get_block_info((unsigned long)raw, &after) ||
      (!after.type_name && !after.name))
    return fprintf(stderr,
                   "alloc_record check failed: type=%s type_ptr=%p name=%s "
                   "name_ptr=%p\n",
                   after.type_name ? after.type_name : "<null>",
                   (const void *)after.type_name,
                   after.name ? after.name : "<null>",
                   (const void *)after.name),
           5;

  member_info_t member = {};
  if (!get_member_info(next_addr, &member) ||
      member.base != (unsigned long)raw || member.member_addr != next_addr ||
      member.offset != offsetof(Node, next) ||
      (!member.type_name && !member.name))
    return fprintf(stderr, "store_record check failed: offset=%lu type=%s "
                           "type_ptr=%p name=%s name_ptr=%p\n",
                   member.offset,
                   member.type_name ? member.type_name : "<null>",
                   (const void *)member.type_name,
                   member.name ? member.name : "<null>",
                   (const void *)member.name),
           6;

  printf(
      "PASS: base=0x%lx size=%lu block_type=%s block_type_ptr=%p "
      "block_name=%s block_name_ptr=%p member_type=%s member_type_ptr=%p "
      "member_name=%s member_name_ptr=%p\n",
      after.base, after.size, after.type_name ? after.type_name : "<null>",
      (const void *)after.type_name, after.name ? after.name : "<null>",
      (const void *)after.name,
      member.type_name ? member.type_name : "<null>",
      (const void *)member.type_name,
      member.name ? member.name : "<null>", (const void *)member.name);

  free(other_raw);
  free(raw);

  block_info_t gone_block = {};
  if (get_block_info((unsigned long)raw, &gone_block))
    return fprintf(stderr, "free check failed: stale block info remains\n"), 7;
  member_info_t gone_member = {};
  if (get_member_info(next_addr, &gone_member))
    return fprintf(stderr, "free check failed: stale member info remains\n"), 8;
  return 0;
}
