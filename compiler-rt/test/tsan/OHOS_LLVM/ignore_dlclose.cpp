// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir
// RUN: %clangxx_tsan -fno-sanitize=thread -O0 -fno-builtin %s -DLIB -DEXPAND -fPIC -shared -o %t-dir/lib_noninstrumented_1.so
// RUN: %clangxx_tsan -fno-sanitize=thread -O0 -fno-builtin %s -DLIB -DEXPAND -fPIC -shared -o %t-dir/lib_noninstrumented_2.so
// RUN: %clangxx_tsan -O0 -fno-builtin %s -DLIB -fPIC -shared -o %t-dir/lib_instrumented_1.so
// RUN: %clangxx_tsan -O0 -fno-builtin %s -DLIB -fPIC -shared -o %t-dir/lib_instrumented_2.so
// RUN: %clangxx_tsan -DTEST1 -O0 %s %link_libcxx_tsan -o %t-dir/exe1
// RUN: %clangxx_tsan -DTEST2 -O0 %s %link_libcxx_tsan -o %t-dir/exe2
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=0 %run %t-dir/exe1 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN-EXT
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=1 %deflake %run %t-dir/exe2 instrumented_instrumented 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE-II
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=1 %deflake %run %t-dir/exe2 instrumented_noninstrumented 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE-IN
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=1 %deflake %run %t-dir/exe2 noninstrumented_instrumented 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE-NI
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=1 %run %t-dir/exe2 noninstrumented_noninstrumented 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE-NN
// RUN: echo "called_from_lib:lib_instrumented_1.so" > %t-dir/exe2.supp
// RUN: %env_tsan_opts=ignore_noninstrumented_modules=0:suppressions=%t-dir/exe2.supp %deflake %run %t-dir/exe2 instrumented_instrumented 2>&1 | FileCheck %s --check-prefix=CHECK-SUPPRESS

// REQUIRES: ohos_family

#ifndef LIB

#include <cstring>
#include <dlfcn.h>
#include <dlfcn_ext.h>
#include <libgen.h>
#include <stdio.h>
#include <string>

using Function = void (*)(void);

Function GetSymbol(const std::string &library, const char *name, bool call) {
  dl_extinfo extinfo = {.flag = DL_EXT_RESERVED_ADDRESS_RECURSIVE};
  void *handle = dlopen_ext(library.c_str(), RTLD_NOW, &extinfo);
  if (!handle) {
    fprintf(stderr, "dlopen_ext %s failed.\n", library.c_str());
    return nullptr;
  }
  Function function = reinterpret_cast<Function>(dlsym(handle, name));
  if (!function) {
    fprintf(stderr, "dlsym %s failed.\n", name);
    dlclose(handle);
    return nullptr;
  }
  if (call)
    function();
  dlclose(handle);
  return function;
}

bool TestAddressReuse(const std::string &dir) {
  Function first =
      GetSymbol(dir + "/lib_noninstrumented_1.so", "libfunc1", false);
  Function second =
      GetSymbol(dir + "/lib_noninstrumented_2.so", "libfunc1", false);
  if (!first || !second) {
    fprintf(stderr, "dlopen_ext address reuse failed.\n");
    return false;
  }
  fprintf(stderr, "dlopen_ext address reuse succeed first:%p second:%p\n",
          reinterpret_cast<void *>(first), reinterpret_cast<void *>(second));
  return true;
}

bool TestInstrumentedRanges(const std::string &dir, const char *type) {
  std::string first;
  std::string second;
  if (strcmp(type, "instrumented_instrumented") == 0) {
    first = dir + "/lib_instrumented_1.so";
    second = dir + "/lib_instrumented_2.so";
  } else if (strcmp(type, "instrumented_noninstrumented") == 0) {
    first = dir + "/lib_instrumented_1.so";
    second = dir + "/lib_noninstrumented_1.so";
  } else if (strcmp(type, "noninstrumented_instrumented") == 0) {
    first = dir + "/lib_noninstrumented_1.so";
    second = dir + "/lib_instrumented_1.so";
  } else {
    first = dir + "/lib_noninstrumented_1.so";
    second = dir + "/lib_noninstrumented_2.so";
  }

  Function first_function = GetSymbol(first, "libfunc1", true);
  Function second_function = GetSymbol(second, "libfunc2", true);
  if (!first_function || !second_function) {
    fprintf(stderr, "dlopen_ext range test failed.\n");
    return false;
  }
  fprintf(stderr, "dlopen_ext range test succeed first:%p second:%p\n",
          reinterpret_cast<void *>(first_function),
          reinterpret_cast<void *>(second_function));
  return true;
}

int main(int argc, char *argv[]) {
  std::string dir = dirname(argv[0]);
#if defined(TEST1)
  TestAddressReuse(dir);
#endif
#if defined(TEST2)
  TestInstrumentedRanges(dir, argv[1]);
#endif
  fprintf(stderr, "DONE\n");
  return 0;
}

#else

#include <cstring>
#include <pthread.h>
#include <stdio.h>

#if defined(EXPAND)
template <size_t Index> int ExpandText() { return ExpandText<Index - 1>(); }
template <> int ExpandText<0>() { return 0; }
#endif

static int Global1[4];
static int Global2[4];

void *Thread1(void *) {
  memset(Global1, 0, sizeof(Global1));
  return nullptr;
}
void *Thread2(void *) {
  memset(Global1, 1, sizeof(Global1));
  return nullptr;
}
void *Thread3(void *) {
  memset(Global2, 0, sizeof(Global2));
  return nullptr;
}
void *Thread4(void *) {
  memset(Global2, 1, sizeof(Global2));
  return nullptr;
}

extern "C" void libfunc1() {
#if defined(EXPAND)
  ExpandText<15>();
#endif
  fprintf(stderr, "mem1:%p.\n", &Global1);
  pthread_t threads[2];
  pthread_create(&threads[0], nullptr, Thread1, nullptr);
  pthread_create(&threads[1], nullptr, Thread2, nullptr);
  pthread_join(threads[0], nullptr);
  pthread_join(threads[1], nullptr);
}

extern "C" void libfunc2() {
  fprintf(stderr, "mem2:%p.\n", &Global2);
  pthread_t threads[2];
  pthread_create(&threads[0], nullptr, Thread3, nullptr);
  pthread_create(&threads[1], nullptr, Thread4, nullptr);
  pthread_join(threads[0], nullptr);
  pthread_join(threads[1], nullptr);
}

#endif

// CHECK-DLOPEN-EXT: dlopen_ext address reuse succeed
// CHECK-DLOPEN-EXT: DONE
// CHECK-IGNORE-II-NOT: failed
// CHECK-IGNORE-II: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-II: libfunc1 {{.*}}lib_instrumented_1.so
// CHECK-IGNORE-II: libfunc2 {{.*}}lib_instrumented_2.so
// CHECK-IGNORE-II: DONE
// CHECK-IGNORE-IN-NOT: failed
// CHECK-IGNORE-IN: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-IN: libfunc1 {{.*}}lib_instrumented_1.so
// CHECK-IGNORE-IN-NOT: libfunc2
// CHECK-IGNORE-IN: DONE
// CHECK-IGNORE-NI-NOT: failed
// CHECK-IGNORE-NI: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-NI-NOT: libfunc1
// CHECK-IGNORE-NI: libfunc2 {{.*}}lib_instrumented_1.so
// CHECK-IGNORE-NI: DONE
// CHECK-IGNORE-NN-NOT: failed
// CHECK-IGNORE-NN-NOT: WARNING: ThreadSanitizer: data race
// CHECK-IGNORE-NN: DONE
// CHECK-SUPPRESS-NOT: failed
// CHECK-SUPPRESS: WARNING: ThreadSanitizer: data race
// CHECK-SUPPRESS-NOT: libfunc1
// CHECK-SUPPRESS: libfunc2 {{.*}}lib_instrumented_2.so
// CHECK-SUPPRESS: DONE
