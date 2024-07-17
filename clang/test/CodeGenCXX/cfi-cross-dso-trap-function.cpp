// RUN: %clang_cc1 -triple x86_64-unknown-linux -O0 -fsanitize-cfi-cross-dso \
// RUN:     -fsanitize=cfi-icall,cfi-nvcall,cfi-vcall,cfi-unrelated-cast,cfi-derived-cast \
// RUN:     -fsanitize-trap=cfi-icall,cfi-nvcall -fsanitize-recover=cfi-vcall,cfi-unrelated-cast \
// RUN:     -fcfi-trap-function=cfi_trap_function \
// RUN:     -emit-llvm -o - %s | FileCheck %s

void caller(void (*f)(void)) {
  f();
}

// CHECK: define weak_odr hidden void @__cfi_check_fail(ptr noundef %0, ptr noundef %1)
// CHECK:        call void @llvm.ubsantrap({{.*}}) #[[#CFIATTR:]]
// CHECK-NEXT:   unreachable

// CHECK: attributes #[[#CFIATTR]] = { {{.*}}"trap-func-name"="cfi_trap_function" }
