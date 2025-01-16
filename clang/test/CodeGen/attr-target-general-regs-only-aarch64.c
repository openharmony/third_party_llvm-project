// OHOS_LOCAL begin
// Test general-regs-only target attribute on aarch64

// RUN: %clang_cc1 -triple aarch64-linux-gnu -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple aarch64_be-linux-gnu -emit-llvm %s -o - | FileCheck %s

// CHECK: define{{.*}} void @f() [[GPR_ATTRS:#[0-9]+]]
void __attribute__((target("general-regs-only"))) f(void) { }

// CHECK: attributes [[GPR_ATTRS]] = { {{.*}} "target-features"="{{.*}}-crypto{{.*}}-fp-armv8{{.*}}-neon{{.*}}"
// OHOS_LOCAL end
