//===-- memgraph_hybrid_frontend_business_block_smoke.cpp ----------------===//
//
// Business-like smoke test for frontend-inserted alloc_store calls.
// The "business" path only does malloc and typed pointer use. Verification
// happens separately through get_block_info(), which matches IDE-style reads.
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

static __attribute__((noinline)) void *BusinessAllocateNode() {
  void *raw = malloc(sizeof(Node));
  if (raw)
    memset(raw, 0, sizeof(Node));
  return raw;
}

static __attribute__((noinline)) void BusinessBindTypedNode(void *raw) {
  Node *node = (Node *)raw;
  node->next = nullptr;
  node->value = 123;

  // Keep the typed variable and its field writes observable.
  volatile unsigned long sink =
      (unsigned long)node + (unsigned long)node->next +
      (unsigned long)node->value;
  (void)sink;
}

int main() {
  ohos_memgraph_hybrid_get_block_info_fn get_block_info =
      (ohos_memgraph_hybrid_get_block_info_fn)LoadSym(
          "ohos_memgraph_hybrid_get_block_info");
  if (!get_block_info) {
    fprintf(stderr, "FAIL: required get_block_info symbol missing\n");
    return 2;
  }

  void *raw = BusinessAllocateNode();
  if (!raw) {
    fprintf(stderr, "FAIL: malloc returned null\n");
    return 3;
  }

  ohos_memgraph_hybrid_block_info_t hook_info;
  if (!QueryBlock(get_block_info, (unsigned long)raw,
                  "IDE view after business malloc", &hook_info)) {
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

  BusinessBindTypedNode(raw);

  ohos_memgraph_hybrid_block_info_t meta_info;
  if (!QueryBlock(get_block_info, (unsigned long)raw,
                  "IDE view after typed business use", &meta_info)) {
    fprintf(stderr, "FAIL: block info missing after typed business use\n");
    free(raw);
    return 6;
  }
  if (!meta_info.type_name && !meta_info.name) {
    fprintf(stderr,
            "FAIL: type/name still missing; frontend alloc_store likely not inserted\n");
    free(raw);
    return 7;
  }

  puts("PASS: OHOS memgraph hybrid frontend business block smoke works");
  free(raw);
  return 0;
}
