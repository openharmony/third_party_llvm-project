//===-- memgraph_frontend_auto_block_member_pc_smoke_min_shared.cpp --===//
//
// Minimal shared-library smoke for automatic instrumentation (PC variant).
// Relies on frontend-inserted alloc/store metadata and additionally verifies:
// - get_block_info() returns a non-zero malloc_pc
// - get_member_info() returns a non-zero store_pc
// - both PCs resolve via dladdr() into the shared library under test

#include "memgraph_interface.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
  Node *next;
  int value;
};

// Under the current frontend rules, malloc inside this helper usually does not
// get an automatic alloc_record().
static void *AllocateWrappedNode() { return malloc(sizeof(Node)); }

extern "C" __attribute__((visibility("default"))) int
memgraph_frontend_auto_block_member_pc_smoke_min_run() {
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  auto get_member_info =
      (int (*)(unsigned long, member_info_t *))dlsym(
          RTLD_DEFAULT, "get_member_info");
  if (!get_block_info || !get_member_info)
    return fprintf(stderr, "missing query symbols\n"), 2;

  Dl_info self_info = {};
  if (!dladdr((void *)&memgraph_frontend_auto_block_member_pc_smoke_min_run,
              &self_info) ||
      !self_info.dli_fbase)
    return fprintf(stderr, "dladdr(self) failed\n"), 3;
  const unsigned long self_base = (unsigned long)self_info.dli_fbase;

  // Direct case: the frontend should automatically emit alloc/store metadata.
  void *raw = malloc(sizeof(Node));
  void *other_raw = malloc(sizeof(Node));
  if (!raw || !other_raw)
    return fprintf(stderr, "direct malloc failed\n"), 4;

  Node *node = (Node *)raw;
  Node *other = (Node *)other_raw;
  node->next = other;
  node->value = 123;
  const unsigned long next_addr = (unsigned long)&node->next;
  volatile unsigned long sink =
      (unsigned long)node + (unsigned long)other + (unsigned long)node->next +
      (unsigned long)node->value;
  (void)sink;

  block_info_t direct_block = {};
  if (!get_block_info((unsigned long)raw, &direct_block) ||
      direct_block.base != (unsigned long)raw ||
      direct_block.size != sizeof(Node) || direct_block.malloc_pc == 0 ||
      (!direct_block.type_name && !direct_block.name))
    return fprintf(stderr,
                   "direct alloc_record check failed: type=%s name=%s "
                   "malloc_pc=0x%lx\n",
                   direct_block.type_name ? direct_block.type_name : "<null>",
                   direct_block.name ? direct_block.name : "<null>",
                   direct_block.malloc_pc),
           6;

  member_info_t direct_member = {};
  if (!get_member_info(next_addr, &direct_member) ||
      direct_member.base != (unsigned long)raw ||
      direct_member.member_addr != next_addr ||
      direct_member.offset != offsetof(Node, next) ||
      (!direct_member.type_name && !direct_member.name) ||
      direct_member.store_pc == 0)
    return fprintf(stderr,
                   "direct store_record check failed: offset=%lu type=%s "
                   "name=%s store_pc=0x%lx\n",
                   direct_member.offset,
                   direct_member.type_name ? direct_member.type_name : "<null>",
                   direct_member.name ? direct_member.name : "<null>",
                   direct_member.store_pc),
           7;

  Dl_info direct_malloc_pc_info = {};
  if (!dladdr((void *)direct_block.malloc_pc, &direct_malloc_pc_info) ||
      direct_malloc_pc_info.dli_fbase != self_info.dli_fbase)
    return fprintf(stderr,
                   "direct malloc_pc dladdr check failed: malloc_pc=0x%lx "
                   "pc_base=%p "
                   "self_base=%p\n",
                   direct_block.malloc_pc, direct_malloc_pc_info.dli_fbase,
                   self_info.dli_fbase),
           8;

  Dl_info direct_store_pc_info = {};
  if (!dladdr((void *)direct_member.store_pc, &direct_store_pc_info) ||
      direct_store_pc_info.dli_fbase != self_info.dli_fbase)
    return fprintf(stderr,
                   "direct store_pc dladdr check failed: store_pc=0x%lx "
                   "pc_base=%p "
                   "self_base=%p\n",
                   direct_member.store_pc, direct_store_pc_info.dli_fbase,
                   self_info.dli_fbase),
           9;

  // Wrapped case: under current frontend rules this usually misses
  // alloc_record(), so only the hook fallback should remain.
  void *wrapped_raw = AllocateWrappedNode();
  if (!wrapped_raw)
    return fprintf(stderr, "wrapped malloc failed\n"), 10;

  block_info_t wrapped_block = {};
  if (!get_block_info((unsigned long)wrapped_raw, &wrapped_block) ||
      wrapped_block.base != (unsigned long)wrapped_raw ||
      wrapped_block.size != sizeof(Node) || wrapped_block.malloc_pc == 0)
    return fprintf(stderr,
                   "wrapped hook check failed: base=0x%lx size=%lu "
                   "malloc_pc=0x%lx\n",
                   wrapped_block.base, wrapped_block.size,
                   wrapped_block.malloc_pc),
           11;
  if (wrapped_block.type_name || wrapped_block.name)
    return fprintf(stderr,
                   "wrapped fallback check failed: type=%s name=%s should be "
                   "null without alloc_record\n",
                   wrapped_block.type_name ? wrapped_block.type_name : "<null>",
                   wrapped_block.name ? wrapped_block.name : "<null>"),
           12;

  Dl_info wrapped_malloc_pc_info = {};
  if (!dladdr((void *)wrapped_block.malloc_pc, &wrapped_malloc_pc_info) ||
      wrapped_malloc_pc_info.dli_fbase != self_info.dli_fbase)
    return fprintf(stderr,
                   "wrapped malloc_pc dladdr check failed: malloc_pc=0x%lx "
                   "pc_base=%p self_base=%p\n",
                   wrapped_block.malloc_pc, wrapped_malloc_pc_info.dli_fbase,
                   self_info.dli_fbase),
           13;

  printf(
      "PASS: direct_base=0x%lx direct_size=%lu direct_malloc_pc=0x%lx "
      "direct_malloc_pc_rel=0x%lx direct_store_pc=0x%lx "
      "direct_store_pc_rel=0x%lx wrapped_base=0x%lx wrapped_size=%lu "
      "wrapped_malloc_pc=0x%lx wrapped_malloc_pc_rel=0x%lx so_base=0x%lx "
      "direct_block_type=%s direct_block_name=%s direct_member_type=%s "
      "direct_member_name=%s\n",
      direct_block.base, direct_block.size, direct_block.malloc_pc,
      direct_block.malloc_pc - self_base, direct_member.store_pc,
      direct_member.store_pc - self_base, wrapped_block.base,
      wrapped_block.size, wrapped_block.malloc_pc,
      wrapped_block.malloc_pc - self_base, self_base,
      direct_block.type_name ? direct_block.type_name : "<null>",
      direct_block.name ? direct_block.name : "<null>",
      direct_member.type_name ? direct_member.type_name : "<null>",
      direct_member.name ? direct_member.name : "<null>");

  free(other_raw);
  free(raw);
  free(wrapped_raw);

  block_info_t gone_direct_block = {};
  if (get_block_info((unsigned long)raw, &gone_direct_block))
    return fprintf(stderr, "free check failed: stale block info remains\n"),
           14;
  member_info_t gone_direct_member = {};
  if (get_member_info(next_addr, &gone_direct_member))
    return fprintf(stderr, "free check failed: stale member info remains\n"),
           15;
  block_info_t gone_wrapped_block = {};
  if (get_block_info((unsigned long)wrapped_raw, &gone_wrapped_block))
    return fprintf(stderr,
                   "free check failed: stale wrapped block info remains\n"),
           16;
  return 0;
}
