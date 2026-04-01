//===-- memgraph_hybrid_frontend_block_smoke.cpp -------------------------===//
//
// Minimal joint smoke test for frontend-inserted alloc_store calls.
// The test only checks block-level IDE data through get_block_info():
// 1. malloc hook should make base/size visible.
// 2. frontend alloc_store should make type/name visible after typed assignment.
//===----------------------------------------------------------------------===//

#include "../ohos_memgraph_hybrid_interface.h"

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
  Node *next;
  int value;
};

typedef void (*ohos_memgraph_hybrid_init_fn)();
typedef int (*ohos_memgraph_hybrid_get_block_info_fn)(
    unsigned long base, ohos_memgraph_hybrid_block_info_t *out);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static int QueryBlock(ohos_memgraph_hybrid_get_block_info_fn get_block_info,
                      unsigned long base, const char *label,
                      ohos_memgraph_hybrid_block_info_t *out) {
  memset(out, 0, sizeof(*out));
  const int ok = get_block_info(base, out);
  printf("%s: ok=%d found=%d base=0x%lx size=%lu type=%s name=%s\n", label, ok,
         out->found, out->base, out->size,
         out->type_name ? out->type_name : "<null>",
         out->name ? out->name : "<null>");
  return ok && out->found;
}

int main() {
  ohos_memgraph_hybrid_init_fn init =
      (ohos_memgraph_hybrid_init_fn)LoadSym("ohos_memgraph_hybrid_init");
  ohos_memgraph_hybrid_get_block_info_fn get_block_info =
      (ohos_memgraph_hybrid_get_block_info_fn)LoadSym(
          "ohos_memgraph_hybrid_get_block_info");
  if (!get_block_info) {
    fprintf(stderr, "FAIL: required get_block_info symbol missing\n");
    return 2;
  }
  if (init)
    init();

  void *raw = malloc(sizeof(Node));
  if (!raw) {
    fprintf(stderr, "FAIL: malloc returned null\n");
    return 3;
  }
  memset(raw, 0, sizeof(Node));

  ohos_memgraph_hybrid_block_info_t hook_info;
  if (!QueryBlock(get_block_info, (unsigned long)raw, "after malloc(raw)",
                  &hook_info)) {
    fprintf(stderr, "FAIL: malloc hook did not expose block info\n");
    free(raw);
    return 4;
  }
  if (hook_info.base != (unsigned long)raw || hook_info.size != sizeof(Node)) {
    fprintf(stderr,
            "FAIL: malloc hook block info mismatch: expect base=0x%lx size=%zu\n",
            (unsigned long)raw, sizeof(Node));
    free(raw);
    return 5;
  }

  Node *typed = (Node *)raw;
  typed->next = nullptr;
  typed->value = 123;

  // Keep the typed assignment and field writes observable under -O0/-O1.
  volatile unsigned long sink =
      (unsigned long)typed + (unsigned long)typed->next +
      (unsigned long)typed->value;
  (void)sink;

  ohos_memgraph_hybrid_block_info_t meta_info;
  if (!QueryBlock(get_block_info, (unsigned long)typed,
                  "after typed pointer assignment", &meta_info)) {
    fprintf(stderr, "FAIL: block info missing after typed pointer assignment\n");
    free(raw);
    return 6;
  }
  if (!meta_info.type_name && !meta_info.name) {
    fprintf(stderr,
            "FAIL: type/name still missing; frontend alloc_store likely not inserted\n");
    free(raw);
    return 7;
  }

  puts("PASS: OHOS memgraph hybrid frontend block smoke works");
  free(raw);
  return 0;
}
