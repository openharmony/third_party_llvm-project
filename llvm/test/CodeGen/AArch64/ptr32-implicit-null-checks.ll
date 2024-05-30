; RUN: llc --enable-implicit-null-checks --aarch64-enable-ptr32 -verify-machineinstrs -O2 < %s | FileCheck %s

target triple = "aarch64-unknown-linux-gnu"

; CHECK-LABEL: LoadI32FromPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp0:
; CHECK-NEXT:    ldr w0, [x0, #16] // on-fault: .LBB0_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB0_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define i32 @LoadI32FromPtr32(ptr addrspace(271) %object) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:
  tail call void @ThrowNullPointerException()
  unreachable

if.end:
  %add.ptr = getelementptr inbounds i32, ptr addrspace(271) %object, i32 4
  %1 = load i32, ptr addrspace(271) %add.ptr, align 4
  ret i32 %1
}

; CHECK-LABEL: LoadFloatFromPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp1:
; CHECK-NEXT:    ldr s0, [x0, #16] // on-fault: .LBB1_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB1_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define float @LoadFloatFromPtr32(ptr addrspace(271) %object) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException() #2
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds float, ptr addrspace(271) %object, i32 4
  %1 = load float, ptr addrspace(271) %add.ptr, align 4
  ret float %1
}

; CHECK-LABEL: LoadDoubleFromPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp2:
; CHECK-NEXT:    ldr d0, [x0, #64] // on-fault: .LBB2_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB2_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define double @LoadDoubleFromPtr32(ptr addrspace(271) %object) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:
  tail call void @ThrowNullPointerException()
  unreachable

if.end:
  %add.ptr = getelementptr inbounds double, ptr addrspace(271) %object, i32 8
  %1 = load double, ptr addrspace(271) %add.ptr, align 8
  ret double %1
}

; CHECK-LABEL: LoadPtr32FromPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp3:
; CHECK-NEXT:    ldr w0, [x0, #16] // on-fault: .LBB3_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB3_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define ptr addrspace(271) @LoadPtr32FromPtr32(ptr addrspace(271) %object) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 4
  %1 = load ptr addrspace(271), ptr addrspace(271) %add.ptr, align 4
  ret ptr addrspace(271) %1
}

; CHECK-LABEL: StoreI32ToPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp4:
; CHECK-NEXT:    str w1, [x0, #32] // on-fault: .LBB4_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB4_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define void @StoreI32ToPtr32(ptr addrspace(271) %object, i32 %value) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 8
  store i32 %value, ptr addrspace(271) %add.ptr, align 4
  ret void
}

; CHECK-LABEL: StoreFloatToPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp5:
; CHECK-NEXT:    str s0, [x0, #32] // on-fault: .LBB5_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB5_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define void @StoreFloatToPtr32(ptr addrspace(271) %object, float %value) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 8
  store float %value, ptr addrspace(271) %add.ptr, align 4
  ret void
}

; CHECK-LABEL: StoreDoubleToPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp6:
; CHECK-NEXT:    str d0, [x0, #32] // on-fault: .LBB6_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB6_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define void @StoreDoubleToPtr32(ptr addrspace(271) %object, double %value) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 8
  store double %value, ptr addrspace(271) %add.ptr, align 8
  ret void
}

; Note LLVM does not support this case because ImplicitNullChecks does not hoist
; str     x8, [x0, #32] above the moves to x8 because of the dependency on x8
; The test should be enabled when such support is introduced
; CHECK-LABEL: StoreConstantDoubleToPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:         cbz x0, .LBB7_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    mov x8, #55370
; CHECK-NEXT:    movk x8, #19730, lsl #16
; CHECK-NEXT:    movk x8, #8699, lsl #32
; CHECK-NEXT:    movk x8, #16393, lsl #48
; CHECK-NEXT:    str x8, [x0, #32]
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB7_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define void @StoreConstantDoubleToPtr32(ptr addrspace(271) %object) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 8
  store double 0x400921FB4D12D84A, ptr addrspace(271) %add.ptr, align 8
  ret void
}

; CHECK-LABEL: StorePtr32ToPtr32:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    str x30, [sp, #-16]!
; CHECK:       .Ltmp7:
; CHECK-NEXT:    str w1, [x0, #32] // on-fault: .LBB8_2
; CHECK-NEXT:  // %bb.1:
; CHECK-NEXT:    ldr x30, [sp], #16
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB8_2:
; CHECK-NEXT:    bl ThrowNullPointerException
define void @StorePtr32ToPtr32(ptr addrspace(271) %object, ptr addrspace(271) %value) {
entry:
  %0 = addrspacecast ptr addrspace(271) %object to ptr
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %if.then, label %if.end, !make.implicit !0

if.then:                                          ; preds = %entry
  tail call void @ThrowNullPointerException()
  unreachable

if.end:                                           ; preds = %entry
  %add.ptr = getelementptr inbounds ptr addrspace(271), ptr addrspace(271) %object, i32 8
  store ptr addrspace(271) %value, ptr addrspace(271) %add.ptr, align 4
  ret void
}

declare void @ThrowNullPointerException()

!0 = !{}
