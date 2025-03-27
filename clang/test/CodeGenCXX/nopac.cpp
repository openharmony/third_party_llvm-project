// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 -fexceptions -fcxx-exceptions -o - %s | FileCheck %s

void f(void);
auto __attribute__((nopac)) &f_ref = f;

// CHECK: define {{(dso_local )?}}void @_Z1gv(
// CHECK: call void @_Z1fv

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
