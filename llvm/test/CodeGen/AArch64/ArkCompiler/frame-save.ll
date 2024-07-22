; RUN: llc -mtriple=aarch64-unknown-linux-gnu -O2 < %s | FileCheck %s
; ark-compiler use aarch64-unknown-linux-gnu as target currently, so use same target instead of ohos target to test it.

; REQUIRES: ark_gc_support

; CHECK-LABEL: SaveFrameType0:
; CHECK:  sub	sp, sp, #32
; CHECK:  str	xzr, [sp, #8]
; CHECK:  add	x29, sp, #16
define i32 @SaveFrameType0() #0 {
entry:
  ret i32 0
}
attributes #0 = { "ark-frame-type"="0" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameType1:
; CHECK:       sub	sp, sp, #32
; CHECK:       mov	x8, #5
; CHECK-NEXT:  str	x8, [sp, #8]
; CHECK:       add	x29, sp, #16
define i32 @SaveFrameType1() #1 {
entry:
  ret i32 0
}
attributes #1 = { "ark-frame-type"="5" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameTypeAndJsfunc0:
; CHECK: stp	x0, xzr, [sp, #-32]!
; CHECK: add	x29, sp, #16
define i32 @SaveFrameTypeAndJsfunc0(i64 %i) #2 {
entry:
  ret i32 0
}
attributes #2 = { "ark-frame-type"="0" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="0" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: SaveFrameTypeAndJsfunc1:
; CHECK:       mov	x8, #24
; CHECK-NEXT:  stp	x0, x8, [sp, #-32]!
; CHECK:       add	x29, sp, #16
define i32 @SaveFrameTypeAndJsfunc1(i64 %i) #3 {
entry:
  ret i32 0
}
attributes #3 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="0" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }


; CHECK-LABEL: SaveFrameTypeAndJsfunc2:
; CHECK:       sub	sp, sp, #32
; CHECK:       mov	x8, #24
; CHECK-NEXT:  ldr	x9, [sp, #32]
; CHECK-NEXT:  stp	x9, x8, [sp]
; CHECK:       add	x29, sp, #16
define webkit_jscc i32 @SaveFrameTypeAndJsfunc2(i64 %i, i64 %j) #4 {
entry:
  ret i32 0
}
attributes #4 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: SaveFrameTypeWithLargeStack:
; CHECK:       sub	sp, sp, #496
; CHECK-NEXT:  .cfi_def_cfa_offset 496
; CHECK-NEXT:  mov	x8, #5
; CHECK-NEXT:  str	x8, [sp, #456]
; CHECK:       add	x29, sp, #464
define i32 @SaveFrameTypeWithLargeStack() #5 {
entry:
  %buf = alloca [56 x i64], align 8
  ret i32 0
}
attributes #5 = { "ark-frame-type"="5" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameTypeWithLargeStack1:
; CHECK:       sub	sp, sp, #496
; CHECK-NEXT:  .cfi_def_cfa_offset 496
; CHECK-NEXT:  mov	x8, #5
; CHECK-NEXT:  str	x8, [sp, #456]
; CHECK:       add	x29, sp, #464
define i32 @SaveFrameTypeWithLargeStack1() #6 {
entry:
  %buf = alloca [57 x i64], align 8
  ret i32 0
}
attributes #6 = { "ark-frame-type"="5" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameTypeWithLargeStack2:
; CHECK:       stp	x29, x30, [sp, #-32]!
; CHECK-NEXT:  .cfi_def_cfa_offset 32
; CHECK:       sub sp, sp, #16
; CHECK-NEXT:  .cfi_def_cfa_offset 48
; CHECK-NEXT:  mov	x8, #5
; CHECK-NEXT:  str	x8, [sp, #8]
; CHECK-NEXT:  add	x29, sp, #16
; CHECK:       sub	sp, sp, #464
define i32 @SaveFrameTypeWithLargeStack2() #7 {
entry:
  %buf = alloca [58 x i64], align 8
  ret i32 0
}
attributes #7 = { "ark-frame-type"="5" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameTypeWithHugeStack:
; CHECK:       stp	x29, x30, [sp, #-32]!
; CHECK-NEXT:  .cfi_def_cfa_offset 32
; CHECK:       sub sp, sp, #16
; CHECK-NEXT:  .cfi_def_cfa_offset 48
; CHECK-NEXT:  mov	x8, #5
; CHECK-NEXT:  str	x8, [sp, #8]
; CHECK-NEXT:  add	x29, sp, #16
; CHECK:       sub	sp, sp, #127, lsl #12
; CHECK-NEXT:  sub	sp, sp, #4080

define i32 @SaveFrameTypeWithHugeStack() #7 {
entry:
  %buf = alloca [65535 x i64], align 8
  ret i32 0
}
attributes #7 = { "ark-frame-type"="5" "ark-frame-type-offset"="8" "frame-pointer"="all" "frame-reserved-slots"="8" }

; CHECK-LABEL: SaveFrameTypeAndJsfuncWithLargeStack:
; CHECK:       sub	sp, sp, #496
; CHECK-NEXT:  .cfi_def_cfa_offset 496
; CHECK-NEXT:  mov	x8, #24
; CHECK-NEXT:  ldr	x9, [sp, #496]
; CHECK-NEXT:  stp	x9, x8, [sp, #448]
; CHECK:       add	x29, sp, #464
define webkit_jscc i32 @SaveFrameTypeAndJsfuncWithLargeStack(i64 %i, i64 %j) #8 {
entry:
  %buf = alloca [56 x i64], align 8
  ret i32 0
}
attributes #8 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }


; CHECK-LABEL: SaveFrameTypeAndJsfuncWithLargeStack1:
; CHECK:	  stp	x29, x30, [sp, #-32]!
; CHECK-NEXT: .cfi_def_cfa_offset 32
; CHECK:      sub	sp, sp, #16
; CHECK-NEXT: .cfi_def_cfa_offset 48
; CHECK-NEXT: mov	x8, #24
; CHECK-NEXT: ldr	x9, [sp, #48]
; CHECK-NEXT: stp	x9, x8, [sp]
; CHECK-NEXT: add	x29, sp, #16
; CHECK:      sub	sp, sp, #464

define webkit_jscc i32 @SaveFrameTypeAndJsfuncWithLargeStack1(i64 %i, i64 %j) #9 {
entry:
  %buf = alloca [57 x i64], align 8
  ret i32 0
}
attributes #9 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: SaveFrameTypeAndJsfuncWithLargeStack2:
; CHECK:	  stp	x29, x30, [sp, #-32]!
; CHECK-NEXT: .cfi_def_cfa_offset 32
; CHECK:      mov	x8, #24
; CHECK:      stp	x1, x8, [sp, #-16]!
; CHECK-NEXT: .cfi_def_cfa_offset 48
; CHECK-NEXT: add	x29, sp, #16
; CHECK:      sub	sp, sp, #464

define i32 @SaveFrameTypeAndJsfuncWithLargeStack2(i64 %i, i64 %j) #10 {
entry:
  %buf = alloca [57 x i64], align 8
  ret i32 0
}
attributes #10 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: SaveFrameTypeAndJsfuncWithLargeStack3:
; CHECK:       sub	sp, sp, #496
; CHECK-NEXT:  .cfi_def_cfa_offset 496
; CHECK-NEXT:  mov	x8, #24
; CHECK-NEXT:  ldr	x9, [sp, #496]
; CHECK-NEXT:  stp	x9, x8, [sp, #384]
; CHECK:       add	x29, sp, #400

define webkit_jscc i32 @SaveFrameTypeAndJsfuncWithLargeStack3(i64 %i, i64 %j) #11 {
entry:
  %buf = alloca [47 x i64], align 8
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{memory}"() nounwind
  ret i32 0
}
attributes #11 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }

; CHECK-LABEL: SaveFrameTypeAndJsfuncWithLargeStack4:
; CHECK:	  stp	x29, x30, [sp, #-96]!
; CHECK-NEXT: .cfi_def_cfa_offset 96
; CHECK:      sub	sp, sp, #16
; CHECK-NEXT: .cfi_def_cfa_offset 112
; CHECK-NEXT: mov	x8, #24
; CHECK-NEXT: ldr	x9, [sp, #112]
; CHECK-NEXT: stp	x9, x8, [sp]
; CHECK-NEXT: add	x29, sp, #16
; CHECK:      sub	sp, sp, #400

define webkit_jscc i32 @SaveFrameTypeAndJsfuncWithLargeStack4(i64 %i, i64 %j) #12 {
entry:
  %buf = alloca [48 x i64], align 8
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{memory}"() nounwind
  ret i32 0
}
attributes #12 = { "ark-frame-type"="24" "ark-frame-type-offset"="8" "ark-jsfunc-arg-idx"="1" "ark-jsfunc-arg-idx-offset"="16" "frame-pointer"="all" "frame-reserved-slots"="16" }
