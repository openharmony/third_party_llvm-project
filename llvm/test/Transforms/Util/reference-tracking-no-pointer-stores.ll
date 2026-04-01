; RUN: opt -passes='reference-tracking,verify' -S %s | FileCheck %s

; No store of pointer type: pass should not add memtracer metadata anywhere.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

define void @only_scalar(i32 %x) {
entry:
  %a = alloca i32, align 4
  store i32 %x, ptr %a, align 4
  ret void
}

; CHECK-LABEL: define void @only_scalar
; CHECK: store i32 %x, ptr %a, align 4
; CHECK-NOT: memtracer
