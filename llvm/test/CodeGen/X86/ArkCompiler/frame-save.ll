; RUN: llc -mtriple=x86_64-unknown-linux-gnu -O2 < %s | FileCheck %s
; ark-compiler use x86_64-unknown-linux-gnu as target currently, so use same target instead of ohos target to test it.

; REQUIRES: ark_gc_support

; CHECK-LABEL: saveframetype0:
; CHECK:      movq	$0, -16(%rsp)
; CHECK-NEXT: pushq	%rbp
; CHECK:      movq	%rsp, %rbp
; CHECK:      subq	$16, %rsp
define i32 @saveframetype0() #0 {
entry:
  ret i32 0
}
attributes #0 = { "ark-frame-type"="0" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: saveframetypeAndJsfunc0:
; CHECK:      movq	$0, -16(%rsp)
; CHECK:      movq	%rdi, -24(%rsp)
; CHECK-NEXT: pushq	%rbp
; CHECK:      movq	%rsp, %rbp
; CHECK:      subq	$16, %rsp
define i32 @saveframetypeAndJsfunc0(i64 %i) #2 {
entry:
  ret i32 0
}
attributes #2 = { "ark-frame-type"="0" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="0" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: saveframetypeAndJsfunc2:
; CHECK:       	movq	$24, -16(%rsp)
; CHECK-NEXT:  	movq	8(%rsp), %r11
; CHECK-NEXT:  	movq	%r11, -24(%rsp)
; CHECK-NEXT:  	pushq	%rbp
; CHECK:      movq	%rsp, %rbp
; CHECK:      subq	$16, %rsp
define webkit_jscc i32 @saveframetypeAndJsfunc2(i64 %i, i64 %j) #4 {
entry:
  ret i32 0
}
attributes #4 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }