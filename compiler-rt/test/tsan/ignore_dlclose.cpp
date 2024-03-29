// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir

// RUN: %clangxx_tsan -fno-sanitize=thread -O0 -fno-builtin %s -DLIB -DEXPEND -fPIC -shared -o %t-dir/lib_noninstrumented_1.so
// RUN: %clangxx_tsan -fno-sanitize=thread -O0 -fno-builtin %s -DLIB -DEXPEND -fPIC -shared -o %t-dir/lib_noninstrumented_2.so
// RUN: %clangxx_tsan -O0 -fno-builtin %s -DLIB -fPIC -shared -o %t-dir/lib_instrumented_1.so
// RUN: %clangxx_tsan -O0 -fno-builtin %s -DLIB -fPIC -shared -o %t-dir/lib_instrumented_2.so

// RUN: %clangxx_tsan -DTEST1 -O0 %s %link_libcxx_tsan -o %t-dir/exe1
// RUN: %clangxx_tsan -DTEST2 -O0 %s %link_libcxx_tsan -o %t-dir/exe2

// RUN: %env_tsan_opts='ignore_noninstrumented_modules=0' %run %t-dir/exe1 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN_EXT

// RUN: %env_tsan_opts='ignore_noninstrumented_modules=1' %deflake %run %t-dir/exe2 instrumented_instrumented            | FileCheck %s --check-prefix=CHECK-IGNORE-II
// RUN: %env_tsan_opts='ignore_noninstrumented_modules=1' %deflake %run %t-dir/exe2 instrumented_noninstrumented         | FileCheck %s --check-prefix=CHECK-IGNORE-IN
// RUN: %env_tsan_opts='ignore_noninstrumented_modules=1' %deflake %run %t-dir/exe2 noninstrumented_instrumented         | FileCheck %s --check-prefix=CHECK-IGNORE-NI
// RUN: %env_tsan_opts='ignore_noninstrumented_modules=1'          %run %t-dir/exe2 noninstrumented_noninstrumented 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE-NN

// RUN: echo "called_from_lib:lib_instrumented_1.so" > %t-dir/exe2.supp
// RUN: %env_tsan_opts='ignore_noninstrumented_modules=0:suppressions='%t-dir/exe2.supp'' %deflake %run %t-dir/exe2 instrumented_instrumented | FileCheck %s --check-prefix=CHECK-SUPPRESS

// REQUIRES: ohos_family

#ifndef LIB

#include <dlfcn.h>
#include <dlfcn_ext.h>
#include <libgen.h>
#include <stdio.h>
#include <string>

typedef void (*FUNC)(void);

FUNC get_symbol_addr(std::string lib, std::string name, bool do_call) {
  dl_extinfo extinfo = {
      .flag = DL_EXT_RESERVED_ADDRESS_RECURSIVE,
  };

  void *h = dlopen_ext(lib.c_str(), RTLD_NOW, &extinfo);
  if (!h) {
    fprintf(stderr, "dlopen_ext %s failed.\n", lib.c_str());
    return nullptr;
  }

  FUNC f = (void (*)())dlsym(h, name.c_str());
  if (!f) {
    fprintf(stderr, "dlsym %s failed.\n", name.c_str());
    if (h) {
      dlclose(h);
    }
    return nullptr;
  }

  if (do_call) {
    f();
  }
  dlclose(h);

  return f;
}

// Test we can reuse same addrress through dlopen_ext.
bool test1(std::string &dir) {
  std::string lib1 = dir + "/lib_noninstrumented_1.so";
  std::string lib2 = dir + "/lib_noninstrumented_2.so";
  FUNC f1 = get_symbol_addr(lib1.c_str(), "libfunc1", 0);
  FUNC f2 = get_symbol_addr(lib2.c_str(), "libfunc1", 0);
  if (!f1 || !f2) {
    fprintf(stderr,
            "dlopen_ext with DL_EXT_RESERVED_ADDRESS_RECURSIVE failed "
            "func1:%p, func2:%p\n",
            f1, f2);
    return false;
  } else {
    fprintf(stderr,
            "dlopen_ext with DL_EXT_RESERVED_ADDRESS_RECURSIVE succeed "
            "func1:%p, func2:%p\n",
            f1, f2);
  }
  return true;
}

bool test2(std::string &dir, char *type) {
  std::string lib1;
  std::string lib2;

  if (!strcmp(type, "instrumented_instrumented")) {
    lib1 = dir + "/lib_instrumented_1.so";
    lib2 = dir + "/lib_instrumented_2.so";
  }

  if (!strcmp(type, "instrumented_noninstrumented")) {
    lib1 = dir + "/lib_instrumented_1.so";
    lib2 = dir + "/lib_noninstrumented_1.so";
  }

  if (!strcmp(type, "noninstrumented_instrumented")) {
    lib1 = dir + "/lib_noninstrumented_1.so";
    lib2 = dir + "/lib_instrumented_1.so";
  }

  if (!strcmp(type, "noninstrumented_noninstrumented")) {
    lib1 = dir + "/lib_noninstrumented_1.so";
    lib2 = dir + "/lib_noninstrumented_2.so";
  }

  FUNC f1 = get_symbol_addr(lib1.c_str(), "libfunc1", 1);
  FUNC f2 = get_symbol_addr(lib2.c_str(), "libfunc2", 1);
  if (!f1 || !f2) {
    fprintf(stderr,
            "dlopen_ext with DL_EXT_RESERVED_ADDRESS_RECURSIVE failed "
            "func1:%p, func2:%p\n",
            f1, f2);
    return false;
  } else {
    fprintf(stderr,
            "dlopen_ext with DL_EXT_RESERVED_ADDRESS_RECURSIVE succeed "
            "func1:%p, func2:%p\n",
            f1, f2);
  }
  return true;
}

int main(int argc, char *argv[]) {
  std::string dir = std::string(dirname(argv[0]));
#if defined(TEST1)
  test1(dir);
#endif

#if defined(TEST2)
  test2(dir, argv[1]);
#endif

  fprintf(stderr, "DONE\n");
  return 0;
}

#else
#include <pthread.h>
#include <stdio.h>

// Extend the code segment so that it can overlap with the instrumented module
// address.
#if defined(EXPEND)
template <size_t idx> int func() { return func<idx - 1>(); }
template <> int func<0>() { return 0; }
#endif

static int Global1[4];
static int Global2[4];

void *Thread1(void *a) {
  memset(&Global1, 0, sizeof(Global1));
  return nullptr;
}

void *Thread2(void *a) {
  memset(&Global1, 1, sizeof(Global1));
  return nullptr;
}

void *Thread3(void *a) {
  memset(&Global2, 0, sizeof(Global2));
  return nullptr;
}

void *Thread4(void *a) {
  memset(&Global2, 1, sizeof(Global2));
  return nullptr;
}

extern "C" void libfunc1() {
#if defined(EXPEND)
  func<15>();
#endif
  fprintf(stderr, "mem1:%p.\n", &Global1);
  pthread_t t[2];
  pthread_create(&t[0], nullptr, Thread1, nullptr);
  pthread_create(&t[1], nullptr, Thread2, nullptr);
  pthread_join(t[0], nullptr);
  pthread_join(t[1], nullptr);
}

extern "C" void libfunc2() {
  fprintf(stderr, "mem2:%p.\n", &Global2);
  pthread_t t[2];
  pthread_create(&t[0], nullptr, Thread3, nullptr);
  pthread_create(&t[1], nullptr, Thread4, nullptr);
  pthread_join(t[0], nullptr);
  pthread_join(t[1], nullptr);
}
#endif

// CHECK-DLOPEN_EXT: dlopen_ext with DL_EXT_RESERVED_ADDRESS_RECURSIVE succeed
// CHECK-DLOPEN_EXT: DONE

// CHECK-IGNORE-II-NOT: failed
// CHECK-IGNORE-II: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-II: libfunc1 {{.*}}lib_instrumented_1.so{{.*}}
// CHECK-IGNORE-II: libfunc2 {{.*}}lib_instrumented_2.so{{.*}}
// CHECK-IGNORE-II: DONE

// CHECK-IGNORE-IN-NOT: failed
// CHECK-IGNORE-IN: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-IN: libfunc1 {{.*}}lib_instrumented_1.so{{.*}}
// CHECK-IGNORE-IN-NOT: libfunc2
// CHECK-IGNORE-IN: DONE

// CHECK-IGNORE-NI-NOT: failed
// CHECK-IGNORE-NI: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-NI-NOT: libfunc1
// CHECK-IGNORE-NI: libfunc2 {{.*}}lib_instrumented_1.so{{.*}}
// CHECK-IGNORE-NI: DONE

// CHECK-IGNORE-NN-NOT: failed
// CHECK-IGNORE-NN-NOT: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-NN: DONE

// CHECK-SUPPRESS-NOT: failed
// CHECK-SUPPRESS: WARNING: ThreadSanitizer: data race
// CHECK-SUPPRESS-NOT: libfunc1
// CHECK-SUPPRESS: libfunc2 {{.*}}lib_instrumented_2.so{{.*}}
// CHECK-SUPPRESS: DONE