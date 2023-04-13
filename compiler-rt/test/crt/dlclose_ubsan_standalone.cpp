// OHOS_LOCAL begin

// Test that we do not segfault on dlclose of ubsan-standalone.
//
// RUN: %clangxx %s -DRT=\"%device_rundir/libclang_rt.ubsan_standalone.so\" -o %t -ldl
// RUN: %push_to_device %shared_ubsan_standalone %device_rundir/libclang_rt.ubsan_standalone.so
// RUN: %run %t 2>&1 | FileCheck %s
// REQUIRES: ohos_family

#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char **argv) {
  void *handle = dlopen(RT, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "Failed to dlopen %s\n", RT);
    return -1;
  }
  if (!dlclose(handle)) {
    fprintf(stderr, "DONE\n");
  }
  return 0;
}

// CHECK-NOT: Failed to dlopen
// CHECK-NOT: Segmentation fault
// CHECK-NOT: Aborted (core dumped)
// CHECK: DONE

// OHOS_LOCAL end
