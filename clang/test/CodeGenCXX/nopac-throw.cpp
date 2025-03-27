// RUN: %clang_cc1                                                -triple aarch64-linux-gnu -fptrauth-calls -fcxx-exceptions -emit-llvm %s -o - | FileCheck %s --check-prefix=CHECK
// RUN: %clang_cc1 -fptrauth-function-pointer-type-discrimination -triple aarch64-linux-gnu -fptrauth-calls -fcxx-exceptions -emit-llvm %s -o - | FileCheck %s --check-prefix=CHECKDISC

class __attribute__((nopac)) Foo {
 public:
  ~Foo() {
  }
};

// CHECK-LABEL: define{{.*}} void @_Z1fv()
// CHECK:  call void @__cxa_throw(ptr %{{.*}}, ptr @_ZTI3Foo, ptr @_ZN3FooD1Ev)

// CHECKDISC-LABEL: define{{.*}} void @_Z1fv()
// CHECKDISC:  call void @__cxa_throw(ptr %{{.*}}, ptr @_ZTI3Foo, ptr @_ZN3FooD1Ev)

void f() {
  throw Foo();
}

// __cxa_throw is defined to take its destructor as "void (*)(void *)" in the ABI.
// CHECK-LABEL: define{{.*}} void @__cxa_throw({{.*}})
// CHECK: call void {{%.*}}(ptr noundef {{%.*}})

// CHECKDISC-LABEL: define{{.*}} void @__cxa_throw({{.*}})
// CHECKDISC: call void {{%.*}}(ptr noundef {{%.*}})

extern "C" void __cxa_throw(void *exception, void *, void (*dtor)(void *)) {
  dtor(exception);
}
