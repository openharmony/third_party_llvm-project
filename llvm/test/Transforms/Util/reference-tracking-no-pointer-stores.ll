; RUN: opt -passes='reference-tracking,verify' -S %s | FileCheck %s

; No store of pointer type: pass should not add memtracer metadata anywhere.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

define void @only_scalar(i32 noundef %x) {
entry:
  %a = alloca i32, align 4
  store i32 %x, ptr %a, align 4
  ret void
}

; CHECK-LABEL: define void @only_scalar
; CHECK: store i32 %x, ptr %a, align 4
; CHECK-NOT: memtracer
