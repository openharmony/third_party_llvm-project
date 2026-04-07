//===-- memgraph_interceptor_semantics_smoke.cpp ----------------===//
//
// Validate that the runtime tracks the supported allocation-family
// interceptors consistently.
//===----------------------------------------------------------------------===//

#include "../../memgraph_interface.h"

#include <dlfcn.h>
#include <new>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*memgraph_init_fn)();
typedef int (*memgraph_get_block_info_fn)(unsigned long base,
                                          block_info_t *out);

static void *LoadSym(const char *name) {
  void *ptr = dlsym(RTLD_DEFAULT, name);
  if (!ptr)
    fprintf(stderr, "dlsym failed: %s\n", name);
  return ptr;
}

static int ExpectTracked(memgraph_get_block_info_fn get_block_info,
                         unsigned long base, unsigned long expect_size,
                         const char *label) {
  block_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_block_info(base, &info);
  printf("%s tracked? ok=%d found=%d base=0x%lx size=%lu type=%s name=%s\n",
         label, ok, info.found, info.base, info.size,
         info.type_name ? info.type_name : "<null>",
         info.name ? info.name : "<null>");
  return ok && info.found && info.base == base && info.size == expect_size;
}

static int ExpectGone(memgraph_get_block_info_fn get_block_info,
                      unsigned long base, const char *label) {
  block_info_t info;
  memset(&info, 0, sizeof(info));
  const int ok = get_block_info(base, &info);
  printf("%s gone? ok=%d found=%d base=0x%lx size=%lu\n", label, ok,
         info.found, info.base, info.size);
  return !ok;
}

int main() {
  memgraph_init_fn init = (memgraph_init_fn)LoadSym("memgraph_init");
  memgraph_get_block_info_fn get_block_info =
      (memgraph_get_block_info_fn)LoadSym("get_block_info");
  if (!get_block_info) {
    fprintf(stderr, "FAIL: missing get_block_info\n");
    return 2;
  }
  if (init)
    init();

  free(nullptr);

  void *calloc_ptr = calloc(4, 16);
  if (!calloc_ptr)
    return fprintf(stderr, "FAIL: calloc failed\n"), 3;
  if (!ExpectTracked(get_block_info, (unsigned long)calloc_ptr, 64,
                     "calloc tracked"))
    return 4;
  free(calloc_ptr);
  if (!ExpectGone(get_block_info, (unsigned long)calloc_ptr, "calloc freed"))
    return 5;

  void *realloc_new = realloc(nullptr, 80);
  if (!realloc_new)
    return fprintf(stderr, "FAIL: realloc(nullptr, size) failed\n"), 6;
  if (!ExpectTracked(get_block_info, (unsigned long)realloc_new, 80,
                     "realloc(null,size) tracked"))
    return 7;
  (void)realloc(realloc_new, 0);
  if (!ExpectGone(get_block_info, (unsigned long)realloc_new,
                  "realloc(ptr,0) freed"))
    return 8;

  unsigned char *array_ptr = new unsigned char[48];
  if (!array_ptr)
    return fprintf(stderr, "FAIL: new[] failed\n"), 9;
  if (!ExpectTracked(get_block_info, (unsigned long)array_ptr, 48,
                     "new[] tracked"))
    return 10;
  delete[] array_ptr;
  if (!ExpectGone(get_block_info, (unsigned long)array_ptr, "delete[] freed"))
    return 11;

  unsigned char *nothrow_ptr = new (std::nothrow) unsigned char[24];
  if (!nothrow_ptr)
    return fprintf(stderr, "FAIL: nothrow new[] failed\n"), 12;
  if (!ExpectTracked(get_block_info, (unsigned long)nothrow_ptr, 24,
                     "nothrow new[] tracked"))
    return 13;
  delete[] nothrow_ptr;
  if (!ExpectGone(get_block_info, (unsigned long)nothrow_ptr,
                  "nothrow delete[] freed"))
    return 14;

  puts("PASS: memgraph interceptor semantics smoke passed");
  return 0;
}
