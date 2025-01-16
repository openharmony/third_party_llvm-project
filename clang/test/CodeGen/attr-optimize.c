// OHOS_LOCAL begin
// RUN: %clang_cc1 -S -emit-llvm %s -o - | FileCheck %s

__attribute__((optimize("omit-frame-pointer"))) void f8(void) {}
// CHECK: @f8{{.*}}[[ATTR_omit_frame_pointer:#[0-9]+]]

__attribute__((optimize("no-omit-frame-pointer"))) void f9(void) {}
// CHECK: @f9{{.*}}[[ATTR_no_omit_frame_pointer:#[0-9]+]]

// CHECK: attributes [[ATTR_omit_frame_pointer]] = { {{.*}}OPT-omit-frame-pointer{{.*}}"frame-pointer"="none"{{.*}} }
// CHECK: attributes [[ATTR_no_omit_frame_pointer]] = { {{.*}}OPT-no-omit-frame-pointer{{.*}}"frame-pointer"="all"{{.*}} }
// OHOS_LOCAL end
