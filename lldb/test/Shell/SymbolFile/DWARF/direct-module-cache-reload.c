// REQUIRES: system-linux

// RUN: rm -rf %t.old %t.new %t.run %t.cache
// RUN: mkdir %t.old %t.new %t.run
// RUN: %clang_host -g -gsplit-dwarf -fPIC -DVALUE=1 -c \
// RUN:   %S/Inputs/dwo-exec-cache-reload-lib.c -o %t.old/libvalue.o
// RUN: %clang_host -shared %t.old/libvalue.o -o %t.old/libvalue.so
// RUN: %clang_host -g -gsplit-dwarf -fPIC -DVALUE=2 -c \
// RUN:   %S/Inputs/dwo-exec-cache-reload-lib.c -o %t.new/libvalue.o
// RUN: %clang_host -shared %t.new/libvalue.o -o %t.new/libvalue.so
// RUN: %clang_host -g -gsplit-dwarf -c %s -o %t.old/app.o
// RUN: %clang_host %t.old/app.o -L%t.old -lvalue \
// RUN:   -Wl,-rpath,'$ORIGIN' -o %t.old/app
// RUN: cp %t.old/app %t.old/app.dwo %t.old/libvalue.so %t.old/libvalue.dwo %t.run
// RUN: %lldb -o "settings set platform.use-exec-search-path-module-cache true" \
// RUN:   -o "settings set platform.module-cache-directory %t.cache" \
// RUN:   -o "target create %t.run/app" -o "breakpoint set -n marker" -o run \
// RUN:   -o "image list -f" -o "frame variable value" -o continue \
// RUN:   -o "platform shell cp %t.new/libvalue.so %t.new/libvalue.dwo %t.run" \
// RUN:   -o run -o "frame variable value" -b 2>&1 | FileCheck %s

// CHECK: {{.*}}/exec-search-path/{{.*}}/libvalue.so
// CHECK: (volatile int) value = 1
// CHECK: Process {{.*}} exited with status = 0
// CHECK: (volatile int) value = 2

void marker();

int main() {
  marker();
  return 0;
}
