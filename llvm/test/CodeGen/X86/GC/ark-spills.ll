; RUN: llc < %s | FileCheck %s
; RUN: llc -opaque-pointers < %s | FileCheck %s

target triple = "x86_64-pc-linux-gnu"

define weak void @foo(ptr addrspace(1) %ref) {
    ret void
}

define i32 @main(i32 %idx, ptr addrspace(1) %ref) #0 gc "statepoint-example" {
    ; CHECK-LABEL: main:
    ; CHECK:         movq %rsi, %rdi
    ; CHECK-NEXT:    movq %rsi, 192(%rbp)
    ; CHECK-NEXT:    callq foo@PLT
    ; Check that the first argument will saved into Ark Frame
    %tok = call arkpltcc token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 12, i32 0, ptr elementtype(void (ptr addrspace(1))) @foo, i32 1, i32 0, ptr addrspace(1) %ref, i32 0, i32 0) #1 ["gc-live"(ptr addrspace(1) %ref)]
    %ref.rel = call ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %tok, i32 0, i32 0)
    ret i32 0
}

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)
declare ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token, i32 immarg, i32 immarg)

attributes #0 = { nounwind "frame-pointer"="all" }
attributes #1 = { "use-ark-spills" }

!ark.frame.info = !{!0, !1}
!0 = !{i32 304}
!1 = !{i32 -128}
