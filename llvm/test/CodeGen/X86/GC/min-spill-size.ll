; RUN: llc --spill-slot-min-size-bytes=8 -O2 < %s | FileCheck %s -check-prefix=SIZE-8
; SIZE-8-LABEL: bar
; SIZE-8:  # %bb.0:
; SIZE-8:          pushq   %rbp
; SIZE-8:          movq    %rsp, %rbp
; SIZE-8:          subq    $16, %rsp
; SIZE-8:          movl    %edi, -8(%rbp)
; SIZE-8:          callq   foo@PLT
; SIZE-8:  .Ltmp0:
; SIZE-8:          movl    -8(%rbp), %eax
; SIZE-8:          addq    $16, %rsp
; SIZE-8:          popq    %rbp
; SIZE-8:          retq

; RUN: llc --spill-slot-min-size-bytes=4 -O2 < %s | FileCheck %s -check-prefix=SIZE-4
; SIZE-4-LABEL: bar
; SIZE-4:  # %bb.0:
; SIZE-4:          pushq   %rbp
; SIZE-4:          movq    %rsp, %rbp
; SIZE-4:          subq    $16, %rsp
; SIZE-4:          movl    %edi, -4(%rbp)
; SIZE-4:          callq   foo@PLT
; SIZE-4:  .Ltmp0:
; SIZE-4:          movl    -4(%rbp), %eax
; SIZE-4:          addq    $16, %rsp
; SIZE-4:          popq    %rbp
; SIZE-4:          retq

; RUN: llc -O2 < %s | FileCheck %s -check-prefix=SIZE-DEFAULT
; SIZE-DEFAULT-LABEL: bar
; SIZE-DEFAULT:  # %bb.0:
; SIZE-DEFAULT:          pushq   %rbp
; SIZE-DEFAULT:          movq    %rsp, %rbp
; SIZE-DEFAULT:          subq    $16, %rsp
; SIZE-DEFAULT:          movl    %edi, -4(%rbp)
; SIZE-DEFAULT:          callq   foo@PLT
; SIZE-DEFAULT:  .Ltmp0:
; SIZE-DEFAULT:          movl    -4(%rbp), %eax
; SIZE-DEFAULT:          addq    $16, %rsp
; SIZE-DEFAULT:          popq    %rbp
; SIZE-DEFAULT:          retq

target triple = "x86_64-pc-linux-gnu"

define protected ptr addrspace(271) @bar(ptr addrspace(271) %a0) #0 gc "ark" {
bb1:
  %0 = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 8, i32 0, ptr elementtype(ptr addrspace(271) (ptr addrspace(271))) @foo, i32 1, i32 0, ptr addrspace(271) %a0, i32 0, i32 0) [ "gc-live"(ptr addrspace(271) %a0) ]
  %a0.relocated = call ptr addrspace(271) @llvm.experimental.gc.relocate.p271(token %0, i32 0, i32 0)
  ret ptr addrspace(271) %a0.relocated
}

declare void @foo(ptr addrspace(271))

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)
declare ptr addrspace(271) @llvm.experimental.gc.relocate.p271(token, i32 immarg, i32 immarg)

attributes #0 = { nounwind "frame-pointer"="all" }

