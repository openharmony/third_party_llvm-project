//===-- memgraph_frontend_shared_runner.cpp -----------------------===//

#include <dlfcn.h>
#include <stdio.h>

typedef int (*memgraph_frontend_run_fn)();

int main(int argc, char **argv) {
  if (argc != 3)
    return fprintf(stderr, "usage: %s <test_so> <run_symbol>\n", argv[0]), 1;

  void *handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
  if (!handle)
    return fprintf(stderr, "dlopen failed: %s\n", dlerror()), 2;

  dlerror();
  memgraph_frontend_run_fn run =
      (memgraph_frontend_run_fn)dlsym(handle, argv[2]);
  const char *err = dlerror();
  if (!run || err)
    return fprintf(stderr, "dlsym failed: %s\n", err ? err : argv[2]), 3;

  const int rc = run();
  dlclose(handle);
  return rc;
}
