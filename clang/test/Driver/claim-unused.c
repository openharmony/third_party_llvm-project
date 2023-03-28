// RUN: touch %t.o
// RUN: %clang --param ssp-buffer-size=1 %t.o -### 2>&1 | FileCheck %s
// CHECK-NOT: warning: argument unused during compilation: '--param ssp-buffer-size=1'
// OHOS_LOCAL begin
// RUN: %clang --param ssp-ret-cookie-size=1 %t.o -### 2>&1 | FileCheck %s
// CHECK-NOT: warning: argument unused during compilation: '--param ssp-ret-cookie-size=1'
// OHOS_LOCAL end
