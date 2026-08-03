//===-- memgraph_frontend_business_new_smoke_min.cpp -------------===//
//
// Minimal C++ new/delete smoke:
// - verifies that the new hook sees base/size
// - verifies that malloc_pc is written
// - verifies that delete clears metadata

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <stdio.h>

struct Node {
  Node *next;
  int value;
};

int main() {
  auto get_block_info =
      (int (*)(unsigned long, block_info_t *))dlsym(RTLD_DEFAULT,
                                                    "get_block_info");
  if (!get_block_info)
    return fprintf(stderr, "missing get_block_info\n"), 2;

  Node *node = new Node();
  node->next = node;
  node->value = 7;

  block_info_t info = {};
  if (!get_block_info((unsigned long)node, &info) || !info.found ||
      info.base != (unsigned long)node || info.size != sizeof(Node) ||
      info.malloc_pc == 0 || info.type_name || info.name)
    return fprintf(stderr,
                   "new hook check failed: found=%d base=0x%lx size=%lu "
                   "malloc_pc=0x%lx type=%s name=%s\n",
                   info.found, info.base, info.size, info.malloc_pc,
                   info.type_name ? info.type_name : "<null>",
                   info.name ? info.name : "<null>"),
           3;

  delete node;

  block_info_t gone = {};
  if (get_block_info((unsigned long)node, &gone))
    return fprintf(stderr, "delete check failed: stale block info remains\n"),
           4;

  Dl_info pc_info = {};
  dladdr((void *)info.malloc_pc, &pc_info);
  printf("PASS: base=0x%lx size=%lu malloc_pc=0x%lx module_base=0x%lx "
         "symbol=%s\n",
         info.base, info.size, info.malloc_pc, (unsigned long)pc_info.dli_fbase,
         pc_info.dli_sname ? pc_info.dli_sname : "<null>");
  return 0;
}
