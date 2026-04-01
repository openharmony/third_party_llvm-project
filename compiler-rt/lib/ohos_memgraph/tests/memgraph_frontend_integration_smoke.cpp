//===-- memgraph_frontend_integration_smoke.cpp -------------------===//
//
// Joint smoke test for frontend-inserted alloc_record/store_record calls.
// The source itself only does malloc and pointer stores, then queries the
// runtime through block/member IDE APIs.
//===----------------------------------------------------------------------===//

#include "../memgraph_interface.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
  Node *next;
  char *name;
};

struct Graph {
  Node *head;
  Node *tail;
  char *label;
};

typedef void (*memgraph_init_fn)();
typedef int (*memgraph_get_info_fn)(
    unsigned long base, alloc_info_t *out);
typedef int (*memgraph_get_block_info_fn)(
    unsigned long base, block_info_t *out);
typedef int (*memgraph_get_member_info_fn)(unsigned long addr,
                                           member_info_t *out);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static int PrintTracked(memgraph_get_info_fn get_info, void *ptr,
                        unsigned long expect_size, const char *label) {
  alloc_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_info((unsigned long)ptr, &info);
  printf("%s hook? ok=%d base=0x%lx size=%lu records=%lu\n", label, ok,
         info.base, info.size, info.record_count);
  return ok && info.base == (unsigned long)ptr && info.size == expect_size;
}

static int PrintBlock(memgraph_get_block_info_fn get_block_info,
                      unsigned long base, unsigned long expect_size,
                      const char *label) {
  block_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_block_info(base, &info);
  printf("%s block? ok=%d found=%d base=0x%lx size=%lu type=%s name=%s\n",
         label, ok, info.found, info.base, info.size,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  return ok && info.base == base && info.size == expect_size &&
         (info.type_name != nullptr || info.name != nullptr);
}

static int PrintMember(memgraph_get_member_info_fn get_member_info,
                       unsigned long base, unsigned long member_addr,
                       unsigned long expect_offset, const char *label) {
  member_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_member_info(member_addr, &info);
  printf("%s member? ok=%d found=%d base=0x%lx member=0x%lx offset=%lu "
         "type=%s name=%s\n",
         label, ok, info.found, info.base, info.member_addr, info.offset,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  return ok && info.base == base && info.member_addr == member_addr &&
         info.offset == expect_offset &&
         (info.type_name != nullptr || info.name != nullptr);
}

int main() {
  memgraph_init_fn init =
      (memgraph_init_fn)LoadSym("memgraph_init");
  memgraph_get_info_fn get_info =
      (memgraph_get_info_fn)LoadSym(
          "get_info");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym(
          "get_block_info");
  memgraph_get_member_info_fn get_member_info =
      (memgraph_get_member_info_fn)LoadSym(
          "get_member_info");
  if (!get_info || !get_block_info || !get_member_info) {
    fprintf(stderr,
            "FAIL: required memgraph query symbols missing\n");
    return 2;
  }
  if (init)
    init();

  Graph *graph = (Graph *)malloc(sizeof(Graph));
  Node *first = (Node *)malloc(sizeof(Node));
  Node *second = (Node *)malloc(sizeof(Node));
  char *node_name = (char *)malloc(32);
  char *graph_label = (char *)malloc(32);
  if (!graph || !first || !second || !node_name || !graph_label) {
    fprintf(stderr, "FAIL: malloc returned null\n");
    return 3;
  }

  memset(graph, 0, sizeof(*graph));
  memset(first, 0, sizeof(*first));
  memset(second, 0, sizeof(*second));
  strcpy(node_name, "node-A");
  strcpy(graph_label, "graph-main");

  graph->head = first;
  graph->tail = second;
  graph->label = graph_label;
  first->next = second;
  first->name = node_name;

  // Prevent the stores above from being optimized away in aggressive builds.
  volatile unsigned long sink =
      (unsigned long)graph->head + (unsigned long)graph->tail +
      (unsigned long)graph->label + (unsigned long)first->next +
      (unsigned long)first->name;
  (void)sink;

  const unsigned long graph_base = (unsigned long)graph;
  const unsigned long first_base = (unsigned long)first;

  const int ok =
      PrintTracked(get_info, graph, sizeof(Graph), "malloc(graph)") &&
      PrintTracked(get_info, first, sizeof(Node), "malloc(first)") &&
      PrintTracked(get_info, second, sizeof(Node), "malloc(second)") &&
      PrintBlock(get_block_info, graph_base, sizeof(Graph), "block(graph)") &&
      PrintBlock(get_block_info, first_base, sizeof(Node), "block(first)") &&
      PrintMember(get_member_info, graph_base, (unsigned long)&graph->head,
                  offsetof(Graph, head), "member(graph->head)") &&
      PrintMember(get_member_info, graph_base, (unsigned long)&graph->tail,
                  offsetof(Graph, tail), "member(graph->tail)") &&
      PrintMember(get_member_info, graph_base, (unsigned long)&graph->label,
                  offsetof(Graph, label), "member(graph->label)") &&
      PrintMember(get_member_info, first_base, (unsigned long)&first->next,
                  offsetof(Node, next), "member(first->next)") &&
      PrintMember(get_member_info, first_base, (unsigned long)&first->name,
                  offsetof(Node, name), "member(first->name)");

  free(graph_label);
  free(node_name);
  free(second);
  free(first);
  free(graph);

  if (!ok) {
    fprintf(stderr,
            "FAIL: frontend integration smoke expected hook + alloc_record + "
            "store_record metadata to all be visible through IDE queries\n");
    return 4;
  }

  puts("PASS: OHOS memgraph frontend integration smoke works");
  return 0;
}
