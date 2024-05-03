// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir

// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB2 -fPIC -shared -fno-sanitize=thread -o %t-dir/lib_dlopen_ns_test.so
// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB1 -DDLOPEN -fPIC -shared -shared-libsan -o %t-dir/lib_dlopen_ns_dep_shared_tsan_1.so
// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB1 -fPIC -shared -shared-libsan -o %t-dir/lib_dlopen_ns_dep_shared_tsan_2.so

// RUN: %clangxx_tsan -DTEST1 -shared-libsan -O1 %s %link_libcxx_tsan -o %t-dir/exe1
// RUN: %run %t-dir/exe1 %t-dir 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN-EXT

// RUN: %clangxx_tsan -DTEST1 -DDLOPEN -shared-libsan -O1 %s %link_libcxx_tsan -o %t-dir/exe2
// RUN: %run %t-dir/exe2 %t-dir 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN

// Test that dlopen works properly when interceptor dlopen_impl:
// Musl dlopen use the caller address to get a matched ns(namespace) and use it
// to search lib, we need to make sure that matched ns don't change, otherwise
// dlopen will be failed, make the ns of caller library different from the tsan
// library to test whether dlopen is ok.

// REQUIRES: ohos_family

#ifdef SHARED_LIB1
#include <dlfcn.h>
#include <dlfcn_ext.h>
#include <stdio.h>
extern "C" __attribute__((noinline)) void *bar(const char *name) {
  fprintf(stderr, "bar\n");
  // let lr in this lib.
#if defined(DLOPEN)
  return dlopen(name, RTLD_NOW);
#else
  return dlopen_ext(name, RTLD_NOW, nullptr);
#endif
}

extern "C" __attribute__((noinline)) void *foo(const char *name) {
  fprintf(stderr, "foo\n");
  return bar(name);
}

#elif defined(SHARED_LIB2)
extern "C" void hello() { return; }
#else
#include <dlfcn.h>
#include <libgen.h>
#include <stdio.h>
#include <string>

int main(int argc, char *argv[]) {
#if defined(DLOPEN)
  std::string lib_caller = "lib_dlopen_ns_dep_shared_tsan_1.so";
#else
  std::string lib_caller = "lib_dlopen_ns_dep_shared_tsan_2.so";
#endif
  std::string lib_callee = "lib_dlopen_ns_test.so";
  std::string dir = std::string(dirname(argv[0]));
  std::string ns_lib_path = dir + ":/system/lib64";
  Dl_namespace dlns;
  dlns_init(&dlns, "test_ns");
  dlns_create(&dlns, ns_lib_path.c_str());

  void *handle = dlopen_ns(&dlns, lib_caller.c_str(), RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "dlopen_ns %s failed.\n", lib_caller.c_str());
    return 0;
  }

  void *(*funcPtr)(const char *) =
      (void *(*)(const char *))dlsym(handle, "foo");
  if (!funcPtr) {
    fprintf(stderr, "dlsym %s failed.\n", "foo");
    return 0;
  }

  void *res = funcPtr(lib_callee.c_str());
  if (!res) {
    fprintf(stderr, "dlopen %s failed.\n", lib_callee.c_str());
    return 0;
  }
  fprintf(stderr, "DONE\n");
  return 0;
}

// CHECK-CHECK-DLOPEN-NOT: failed
// CHECK-DLOPEN: foo
// CHECK-DLOPEN: bar
// CHECK-DLOPEN: DONE

// CHECK-CHECK-DLOPEN-EXT-NOT: failed
// CHECK-DLOPEN-EXT: foo
// CHECK-DLOPEN-EXT: bar
// CHECK-DLOPEN-EXT: DONE
#endif