//===-- memgraph_frontend_block_nodlsym_smoke_min.cpp -------------===//
//
// Minimal block smoke:
// - does not use dlsym
// - only verifies that the malloc hook exposes base/size/malloc_pc
// - calls get_block_info() directly

#include <sanitizer/memgraph_interface.h>

#include <stdio.h>
#include <stdlib.h>

int main() {
  void *p = malloc(16);
  if (!p)
    return fprintf(stderr, "malloc failed\n"), 1;

  block_info_t info = {};
  if (!get_block_info((unsigned long)p, &info) || !info.found ||
      info.base != (unsigned long)p || info.size != 16 || info.malloc_pc == 0)
    return fprintf(stderr,
                   "get_block_info failed: found=%d base=0x%lx size=%lu "
                   "malloc_pc=0x%lx\n",
                   info.found, info.base, info.size, info.malloc_pc),
           2;

  printf("PASS: found=%d base=0x%lx size=%lu malloc_pc=0x%lx\n", info.found,
         info.base, info.size, info.malloc_pc);
  free(p);
  return 0;
}
