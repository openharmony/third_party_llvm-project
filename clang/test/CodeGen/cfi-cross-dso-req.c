// RUN: %clang_cc1 -triple x86_64-unknown-linux -O0 -fsanitize-cfi-cross-dso \
// RUN:     -fsanitize=cfi-icall,cfi-nvcall,cfi-vcall,cfi-unrelated-cast,cfi-derived-cast \
// RUN:     -fsanitize-trap=cfi-icall,cfi-nvcall -fsanitize-recover=cfi-vcall,cfi-unrelated-cast \
// RUN:     -fsanitize-cfi-cross-dso-req \
// RUN:     -emit-llvm -o - %s | FileCheck %s

void caller(void (*f)(void)) {
  f();
}

// CHECK: ![[#]] = !{i32 8, !"Cross-DSO CFI", i32 1}
// CHECK: ![[#]] = !{i32 3, !"Cross-DSO CFI Requirement", ![[#FLAG:]]}
// CHECK: ![[#FLAG]] = !{!"Cross-DSO CFI", i32 1}
