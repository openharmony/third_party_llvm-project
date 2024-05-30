// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir

// RUN: %clangxx_tsan -O1 %s -DSHARED_LIB -fPIC -shared -fsanitize=thread %link_libcxx_tsan -o %t-dir/libtest.so
// RUN: %clangxx_tsan -O1 %s -fsanitize=thread %link_libcxx_tsan -o %t-dir/exe
// RUN: %run %t-dir/exe 2>&1 | FileCheck %s

// REQUIRES: ohos_family

#ifdef SHARED_LIB

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

static int res = 1;
sigjmp_buf jump_buffer;

void segv_handler(int) { siglongjmp(jump_buffer, 1); }

int trigger_signal(void) {
  struct sigaction segv_act{}, old_act{};
  segv_act.sa_handler = &segv_handler;
  sigemptyset(&segv_act.sa_mask);
  sigaction(SIGSEGV, &segv_act, &old_act);

  if (sigsetjmp(jump_buffer, 1) == 0) {
    res = 2;
  }

  sigaction(SIGSEGV, &old_act, NULL);
  return res;
}

struct Test {
  Test() {
    int res = trigger_signal();
    if (res == 2) {
      fprintf(stderr, "Value is ok.\n");
    } else {
      fprintf(stderr, "Value is not expected.\n");
    }
  }
} test;

#else

#include <dlfcn.h>
#include <iostream>
#include <libgen.h>

int main(int argc, char **argv) {
  std::string lib = std::string(dirname(argv[0])) + "/libtest.so";
  void *h = dlopen(lib.c_str(), RTLD_NOW);
  if (h) {
    dlclose(h);
  } else {
    fprintf(stderr, "dlopen failed of %s.\n", lib.c_str());
  }

  return 0;
}

// CHECK-NOT: dlopen failed
// CHECK: Value is ok

#endif