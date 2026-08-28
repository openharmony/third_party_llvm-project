// RUN: rm -rf %t-dir
// RUN: mkdir %t-dir
// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB2 -fPIC -shared -fno-sanitize=thread -o %t-dir/lib_dlopen_ns_test.so
// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB1 -DDLOPEN -fPIC -shared -shared-libsan -o %t-dir/lib_dlopen_ns_dep_shared_tsan_1.so
// RUN: %clangxx_tsan -O0 %s -DSHARED_LIB1 -fPIC -shared -shared-libsan -o %t-dir/lib_dlopen_ns_dep_shared_tsan_2.so
// RUN: %clangxx_tsan -DTEST1 -shared-libsan -O1 %s %link_libcxx_tsan -o %t-dir/exe1
// RUN: %run %t-dir/exe1 %t-dir 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN-EXT
// RUN: %clangxx_tsan -DTEST1 -DDLOPEN -shared-libsan -O1 %s %link_libcxx_tsan -o %t-dir/exe2
// RUN: %run %t-dir/exe2 %t-dir 2>&1 | FileCheck %s --check-prefix=CHECK-DLOPEN

// REQUIRES: ohos_family

#ifdef SHARED_LIB1
#include <dlfcn.h>
#include <dlfcn_ext.h>
#include <stdio.h>

extern "C" __attribute__((noinline)) void *bar(const char *name) {
  fprintf(stderr, "bar\n");
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
extern "C" void hello() {}
#else
#include <dlfcn.h>
#include <dlfcn_ext.h>
#include <libgen.h>
#include <stdio.h>
#include <string>

int main(int argc, char *argv[]) {
#if defined(DLOPEN)
  std::string caller = "lib_dlopen_ns_dep_shared_tsan_1.so";
#else
  std::string caller = "lib_dlopen_ns_dep_shared_tsan_2.so";
#endif
  std::string callee = "lib_dlopen_ns_test.so";
  std::string dir = dirname(argv[0]);
  std::string namespace_path = dir + ":/system/lib64";
  Dl_namespace dlns;
  dlns_init(&dlns, "test_ns");
  dlns_create(&dlns, namespace_path.c_str());

  void *handle = dlopen_ns(&dlns, caller.c_str(), RTLD_NOW);
  if (!handle) {
    fprintf(stderr, "dlopen_ns %s failed.\n", caller.c_str());
    return 0;
  }
  auto function = reinterpret_cast<void *(*)(const char *)>(
      dlsym(handle, "foo"));
  if (!function) {
    fprintf(stderr, "dlsym foo failed.\n");
    return 0;
  }
  if (!function(callee.c_str())) {
    fprintf(stderr, "dlopen %s failed.\n", callee.c_str());
    return 0;
  }
  fprintf(stderr, "DONE\n");
  return 0;
}

// CHECK-DLOPEN-NOT: failed
// CHECK-DLOPEN: foo
// CHECK-DLOPEN: bar
// CHECK-DLOPEN: DONE
// CHECK-DLOPEN-EXT-NOT: failed
// CHECK-DLOPEN-EXT: foo
// CHECK-DLOPEN-EXT: bar
// CHECK-DLOPEN-EXT: DONE
#endif
