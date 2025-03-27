// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 %s -o - \
// RUN:  | FileCheck %s --check-prefix=CXAATEXIT

// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 %s -o - \
// RUN:    -fno-use-cxa-atexit | FileCheck %s --check-prefixes=ATEXIT,ATEXIT_ELF

// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 %s \
// RUN:  -fptrauth-function-pointer-type-discrimination  -o - | FileCheck %s --check-prefix=CXAATEXIT_DISC

// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -emit-llvm -std=c++11 %s -o - \
// RUN:   -fptrauth-function-pointer-type-discrimination -fno-use-cxa-atexit \
// RUN:  | FileCheck %s --check-prefixes=ATEXIT_DISC,ATEXIT_DISC_ELF

class __attribute__((nopac)) Foo {
 public:
  ~Foo() {
  }
};

Foo global;

// CXAATEXIT: define internal void @__cxx_global_var_init()
// CXAATEXIT:   call i32 @__cxa_atexit(ptr @_ZN3FooD1Ev, ptr @global, ptr @__dso_handle)

// CXAATEXIT_DISC: define internal void @__cxx_global_var_init()
// CXAATEXIT_DISC:   call i32 @__cxa_atexit(ptr @_ZN3FooD1Ev, ptr @global, ptr @__dso_handle)

// ATEXIT: define internal void @__cxx_global_var_init()
// ATEXIT:   %{{.*}} = call i32 @atexit(ptr @__dtor_global)

// ATEXIT_DARWIN: define internal void @__dtor_global() {{.*}} section "__TEXT,__StaticInit,regular,pure_instructions" {
// ATEXIT_ELF:    define internal void @__dtor_global() {{.*}} section ".text.startup" {
// ATEXIT_DARWIN:   %{{.*}} = call ptr @_ZN3FooD1Ev(ptr @global)
// ATEXIT_ELF:      call void @_ZN3FooD1Ev(ptr @global)

// ATEXIT_DISC: define internal void @__cxx_global_var_init()
// ATEXIT_DISC:   %{{.*}} = call i32 @atexit(ptr @__dtor_global)


// ATEXIT_DISC_DARWIN: define internal void @__dtor_global() {{.*}} section "__TEXT,__StaticInit,regular,pure_instructions" {
// ATEXIT_DISC_ELF:    define internal void @__dtor_global() {{.*}} section ".text.startup" {
// ATEXIT_DISC_DARWIN:   %{{.*}} = call ptr @_ZN3FooD1Ev(ptr @global)
// ATEXIT_DISC_ELF:      call void @_ZN3FooD1Ev(ptr @global)
