// RUN: %clang_cc1 -triple x86_64-unknown-linux -fsanitize=cfi-icall -fsanitize-trap=cfi-icall -fcfi-trap-function=cfi_trap_function -emit-llvm -o - %s | FileCheck --check-prefixes=CHECK,CHECK-NTF %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsanitize=cfi-icall -fsanitize-trap=cfi-icall -fcfi-trap-function=cfi_trap_function -emit-llvm -o - %s | FileCheck --check-prefixes=CHECK,CHECK-NTF %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux -fsanitize=cfi-icall -fsanitize-trap=cfi-icall -fcfi-trap-function=cfi_trap_function -ftrap-function=trap_function -emit-llvm -o - %s | FileCheck --check-prefixes=CHECK,CHECK-TF %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsanitize=cfi-icall -fsanitize-trap=cfi-icall -fcfi-trap-function=cfi_trap_function -ftrap-function=trap_function -emit-llvm -o - %s | FileCheck --check-prefixes=CHECK,CHECK-TF %s

namespace {

struct S {};

void f(S s) {
}

}

// CHECK: define{{.*}}g_func
void g_func() {
  struct S s;
  void (*fp)(S) = f;
  // CHECK: call i1 @llvm.type.test(
  fp(s);
  // CHECK:      call void @llvm.ubsantrap({{.*}}) #[[#CFIATTR:]]
  // CHECK-NEXT: unreachable
}

// CHECK: define{{.*}}h_func
void h_func() {
  __builtin_trap();
// CHECK-NTF-NOT: call void @llvm.trap() #
// CHECK-NTF: call void @llvm.trap()
// CHECK-TF: call void @llvm.trap() #[[#ATTR:]]
}

// CHECK: attributes #[[#CFIATTR]] = { {{.*}}"trap-func-name"="cfi_trap_function" }
// CHECK-TF: attributes #[[#ATTR]] = { {{.*}}"trap-func-name"="trap_function" }
