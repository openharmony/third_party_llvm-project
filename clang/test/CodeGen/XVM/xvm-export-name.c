// RUN: %clang_cc1 -triple xvm-unknown-unknown-xvm -emit-llvm -O0 -o - %s | FileCheck %s

__attribute__((xvm_export_name("bar"))) int foo(void);

int foo(void) { return 42; }

// CHECK: define dso_local i32 @foo() [[A:#[0-9]+]]
// CHECK: attributes [[A]] = {{{.*}} "xvm-export-name"="bar" {{.*}}}
