//===-- memgraph_frontend_auto_new_pc_smoke_min_shared.cpp --------===//
//
// Minimal shared-library smoke for automatic instrumentation on new (PC
// variant).
// Relies on frontend-inserted alloc metadata and verifies:
// - get_block_info() returns a non-zero malloc_pc
// - block metadata is queryable
// - malloc_pc resolves via dladdr() into the shared library under test

#include "memgraph_interface.h"

#include <dlfcn.h>
#include <stdio.h>

struct Node {
  Node *next;
  int value;
};

// Under the current frontend rules, new inside this helper usually does not
// get an automatic alloc_record().
static Node *AllocateWrappedNodeWithNew() { return new Node(); }

extern "C" __attribute__((visibility("default"))) int
memgraph_frontend_auto_new_pc_smoke_min_run() {
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  if (!get_block_info)
    return fprintf(stderr, "missing get_block_info\n"), 2;

  Dl_info self_info = {};
  if (!dladdr((void *)&memgraph_frontend_auto_new_pc_smoke_min_run,
              &self_info) ||
      !self_info.dli_fbase)
    return fprintf(stderr, "dladdr(self) failed\n"), 3;
  const unsigned long self_base = (unsigned long)self_info.dli_fbase;

  // Direct case: the frontend should automatically emit alloc_record().
  Node *direct_node = new Node();
  if (!direct_node)
    return fprintf(stderr, "direct new failed\n"), 4;

  direct_node->next = direct_node;
  direct_node->value = 7;
  volatile unsigned long sink =
      (unsigned long)direct_node + (unsigned long)direct_node->next +
      (unsigned long)direct_node->value;
  (void)sink;

  block_info_t direct_block = {};
  if (!get_block_info((unsigned long)direct_node, &direct_block) ||
      !direct_block.found || direct_block.base != (unsigned long)direct_node ||
      direct_block.size != sizeof(Node) || direct_block.malloc_pc == 0 ||
      (!direct_block.type_name && !direct_block.name))
    return fprintf(stderr,
                   "direct new alloc metadata check failed: found=%d "
                   "base=0x%lx size=%lu malloc_pc=0x%lx type=%s name=%s\n",
                   direct_block.found, direct_block.base, direct_block.size,
                   direct_block.malloc_pc,
                   direct_block.type_name ? direct_block.type_name : "<null>",
                   direct_block.name ? direct_block.name : "<null>"),
           5;

  Dl_info direct_malloc_pc_info = {};
  if (!dladdr((void *)direct_block.malloc_pc, &direct_malloc_pc_info) ||
      direct_malloc_pc_info.dli_fbase != self_info.dli_fbase)
    return fprintf(stderr,
                   "direct malloc_pc dladdr check failed: malloc_pc=0x%lx "
                   "pc_base=%p self_base=%p\n",
                   direct_block.malloc_pc, direct_malloc_pc_info.dli_fbase,
                   self_info.dli_fbase),
           6;

  // Wrapped case: under current frontend rules this usually misses
  // alloc_record(), so only the hook fallback should remain.
  Node *wrapped_node = AllocateWrappedNodeWithNew();
  if (!wrapped_node)
    return fprintf(stderr, "wrapped new failed\n"), 7;

  block_info_t wrapped_block = {};
  if (!get_block_info((unsigned long)wrapped_node, &wrapped_block) ||
      !wrapped_block.found ||
      wrapped_block.base != (unsigned long)wrapped_node ||
      wrapped_block.size != sizeof(Node) || wrapped_block.malloc_pc == 0)
    return fprintf(stderr,
                   "wrapped new hook check failed: found=%d base=0x%lx "
                   "size=%lu malloc_pc=0x%lx\n",
                   wrapped_block.found, wrapped_block.base, wrapped_block.size,
                   wrapped_block.malloc_pc),
           8;
  if (wrapped_block.type_name || wrapped_block.name)
    return fprintf(stderr,
                   "wrapped new fallback check failed: type=%s name=%s "
                   "should be null without alloc_record\n",
                   wrapped_block.type_name ? wrapped_block.type_name : "<null>",
                   wrapped_block.name ? wrapped_block.name : "<null>"),
           9;

  Dl_info wrapped_malloc_pc_info = {};
  if (!dladdr((void *)wrapped_block.malloc_pc, &wrapped_malloc_pc_info) ||
      wrapped_malloc_pc_info.dli_fbase != self_info.dli_fbase)
    return fprintf(stderr,
                   "wrapped malloc_pc dladdr check failed: malloc_pc=0x%lx "
                   "pc_base=%p self_base=%p\n",
                   wrapped_block.malloc_pc, wrapped_malloc_pc_info.dli_fbase,
                   self_info.dli_fbase),
           10;

  printf("PASS: direct_base=0x%lx direct_size=%lu direct_malloc_pc=0x%lx "
         "direct_malloc_pc_rel=0x%lx wrapped_base=0x%lx wrapped_size=%lu "
         "wrapped_malloc_pc=0x%lx wrapped_malloc_pc_rel=0x%lx "
         "so_base=0x%lx direct_block_type=%s direct_block_name=%s\n",
         direct_block.base, direct_block.size, direct_block.malloc_pc,
         direct_block.malloc_pc - self_base, wrapped_block.base,
         wrapped_block.size, wrapped_block.malloc_pc,
         wrapped_block.malloc_pc - self_base, self_base,
         direct_block.type_name ? direct_block.type_name : "<null>",
         direct_block.name ? direct_block.name : "<null>");

  delete direct_node;
  delete wrapped_node;

  block_info_t gone_direct = {};
  if (get_block_info((unsigned long)direct_node, &gone_direct))
    return fprintf(stderr, "delete check failed: stale block info remains\n"),
           11;
  block_info_t gone_wrapped = {};
  if (get_block_info((unsigned long)wrapped_node, &gone_wrapped))
    return fprintf(stderr,
                   "delete check failed: stale wrapped block info remains\n"),
           12;
  return 0;
}
