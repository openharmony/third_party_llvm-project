// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 -fexceptions -fcxx-exceptions -o - %s | FileCheck %s

void f(void);
auto __attribute__((nopac)) &f_ref = f;
// CHECK-NOT: @f_ref = constant ptr ptrauth (ptr @f(), i32 0, i64 18983), align 8
// CHECK: @f_ref = constant ptr @_Z1fv, align 8

// CHECK: define {{(dso_local )?}}void @_Z1gv(
// CHECK-NOT: call void @_Z1fv
// CHECK: call void ptrauth (ptr @_Z1fv, i32 0)() [ "ptrauth"(i32 0, i64 0) ]

void g() { f_ref(); }

void foo1();

void test_terminate() noexcept {
  foo1();
}

// CHECK: define {{(dso_local )?}}void @_ZSt9terminatev() #[[ATTR4:.*]] {

namespace std {
  void terminate() noexcept {
  }
}

// CHECK: attributes #[[ATTR4]] = {{{.*}}"ptrauth-calls"{{.*}}}
