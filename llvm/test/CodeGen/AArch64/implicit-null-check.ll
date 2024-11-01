; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -verify-machineinstrs -O3 -mtriple=aarch64-unknown-unknown -enable-implicit-null-checks | FileCheck %s

; Basic test for implicit null check conversion - this is analogous to the
; file with the same name in the X86 tree, but adjusted to remove patterns
; related to memory folding of arithmetic (since aarch64 doesn't), and add
; a couple of aarch64 specific tests.

define i32 @imp_null_check_load_fallthrough(i32* %x) {
; CHECK-LABEL: imp_null_check_load_fallthrough:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp0:
; CHECK-NEXT:    ldr w0, [x0] // on-fault: .LBB0_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB0_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 not_null:
  %t = load i32, i32* %x
  ret i32 %t

is_null:
  ret i32 42
}


define i32 @imp_null_check_load_reorder(i32* %x) {
; CHECK-LABEL: imp_null_check_load_reorder:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp1:
; CHECK-NEXT:    ldr w0, [x0] // on-fault: .LBB1_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB1_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t = load i32, i32* %x
  ret i32 %t
}

define i32 @imp_null_check_unordered_load(i32* %x) {
; CHECK-LABEL: imp_null_check_unordered_load:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp2:
; CHECK-NEXT:    ldr w0, [x0] // on-fault: .LBB2_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB2_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t = load atomic i32, i32* %x unordered, align 4
  ret i32 %t
}


; TODO: Can be converted into implicit check.
;; Probably could be implicit, but we're conservative for now
define i32 @imp_null_check_seq_cst_load(i32* %x) {
; CHECK-LABEL: imp_null_check_seq_cst_load:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB3_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ldar w0, [x0]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB3_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t = load atomic i32, i32* %x seq_cst, align 4
  ret i32 %t
}

;; Might be memory mapped IO, so can't rely on fault behavior
define i32 @imp_null_check_volatile_load(i32* %x) {
; CHECK-LABEL: imp_null_check_volatile_load:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB4_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ldr w0, [x0]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB4_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t = load volatile i32, i32* %x, align 4
  ret i32 %t
}


define i8 @imp_null_check_load_i8(i8* %x) {
; CHECK-LABEL: imp_null_check_load_i8:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp3:
; CHECK-NEXT:    ldrb w0, [x0] // on-fault: .LBB5_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB5_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i8* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i8 42

 not_null:
  %t = load i8, i8* %x
  ret i8 %t
}

define i256 @imp_null_check_load_i256(i256* %x) {
; CHECK-LABEL: imp_null_check_load_i256:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB6_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ldp x2, x3, [x0, #16]
; CHECK-NEXT:    ldp x0, x1, [x0]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB6_2:
; CHECK-NEXT:    mov x1, xzr
; CHECK-NEXT:    mov x2, xzr
; CHECK-NEXT:    mov x3, xzr
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i256* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i256 42

 not_null:
  %t = load i256, i256* %x
  ret i256 %t
}



define i32 @imp_null_check_gep_load(i32* %x) {
; CHECK-LABEL: imp_null_check_gep_load:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp4:
; CHECK-NEXT:    ldr w0, [x0, #128] // on-fault: .LBB7_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB7_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %x.gep = getelementptr i32, i32* %x, i32 32
  %t = load i32, i32* %x.gep
  ret i32 %t
}

define i32 @imp_null_check_add_result(i32* %x, i32 %p) {
; CHECK-LABEL: imp_null_check_add_result:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp5:
; CHECK-NEXT:    ldr w8, [x0] // on-fault: .LBB8_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    add w0, w8, w1
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB8_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t = load i32, i32* %x
  %p1 = add i32 %t, %p
  ret i32 %p1
}

; Can hoist over a potential faulting instruction as long as we don't
; change the conditions under which the instruction faults.
define i32 @imp_null_check_hoist_over_udiv(i32* %x, i32 %a, i32 %b) {
; CHECK-LABEL: imp_null_check_hoist_over_udiv:
; CHECK:       // %bb.0: // %entry
;; OHOS_LOCAL begin
; CHECK-NEXT:  .Ltmp6:
; CHECK-NEXT:    ldr w9, [x0] // on-fault: .LBB9_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    udiv w8, w1, w2
; CHECK-NEXT:    add w0, w9, w8
;; OHOS_LOCAL end
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB9_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %p1 = udiv i32 %a, %b
  %t = load i32, i32* %x
  %res = add i32 %t, %p1
  ret i32 %res
}

define i32 @imp_null_check_hoist_over_unrelated_load(i32* %x, i32* %y, i32* %z) {
;; OHOS_LOCAL begin
; CHECK-LABEL: imp_null_check_hoist_over_unrelated_load:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:  .Ltmp7:
; CHECK-NEXT:    ldr w0, [x0] // on-fault: .LBB10_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ldr w8, [x1]
; CHECK-NEXT:    str w8, [x2]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB10_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
;; OHOS_LOCAL end
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %t0 = load i32, i32* %y
  %t1 = load i32, i32* %x
  store i32 %t0, i32* %z
  ret i32 %t1
}

define i32 @imp_null_check_gep_load_with_use_dep(i32* %x, i32 %a) {
; CHECK-LABEL: imp_null_check_gep_load_with_use_dep:
; CHECK:       // %bb.0: // %entry
;; OHOS_LOCAL begin
; CHECK-NEXT:  .Ltmp8:
;; OHOS_LOCAL end
; CHECK-NEXT:    ldr w8, [x0] // on-fault: .LBB11_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    add w9, w0, w1
; CHECK-NEXT:    add w8, w9, w8
; CHECK-NEXT:    add w0, w8, #4
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB11_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %x.loc = getelementptr i32, i32* %x, i32 1
  %y = ptrtoint i32* %x.loc to i32
  %b = add i32 %a, %y
  %t = load i32, i32* %x
  %z = add i32 %t, %b
  ret i32 %z
}

;; TODO: We could handle this case as we can lift the fence into the
;; previous block before the conditional without changing behavior.
define i32 @imp_null_check_load_fence1(i32* %x) {
; CHECK-LABEL: imp_null_check_load_fence1:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB12_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    dmb ishld
; CHECK-NEXT:    ldr w0, [x0]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB12_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

is_null:
  ret i32 42

not_null:
  fence acquire
  %t = load i32, i32* %x
  ret i32 %t
}

;; TODO: We could handle this case as we can lift the fence into the
;; previous block before the conditional without changing behavior.
define i32 @imp_null_check_load_fence2(i32* %x) {
; CHECK-LABEL: imp_null_check_load_fence2:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB13_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    dmb ish
; CHECK-NEXT:    ldr w0, [x0]
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB13_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

is_null:
  ret i32 42

not_null:
  fence seq_cst
  %t = load i32, i32* %x
  ret i32 %t
}

define void @imp_null_check_store(i32* %x) {
;; OHOS_LOCAL begin
; CHECK-LABEL: imp_null_check_store:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    mov w8, #1
; CHECK-NEXT:  .Ltmp9:
; CHECK-NEXT:    str w8, [x0] // on-fault: .LBB14_2
; CHECK-NEXT:  // %bb.1: // %not_null
;; OHOS_LOCAL end
; CHECK-NEXT:  .LBB14_2: // %common.ret
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret void

 not_null:
  store i32 1, i32* %x
  ret void
}

define void @imp_null_check_unordered_store(i32* %x) {
;; OHOS_LOCAL begin
; CHECK-LABEL: imp_null_check_unordered_store:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    mov w8, #1
; CHECK-NEXT:  .Ltmp10:
; CHECK-NEXT:    str w8, [x0] // on-fault: .LBB15_2
; CHECK-NEXT:  // %bb.1: // %not_null
;; OHOS_LOCAL end
; CHECK-NEXT:  .LBB15_2: // %common.ret
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret void

 not_null:
  store atomic i32 1, i32* %x unordered, align 4
  ret void
}

define i32 @imp_null_check_neg_gep_load(i32* %x) {
; CHECK-LABEL: imp_null_check_neg_gep_load:
; CHECK:       // %bb.0: // %entry
;; OHOS_LOCAL begin
; CHECK-NEXT:  .Ltmp11:
;; OHOS_LOCAL end
; CHECK-NEXT:    ldur w0, [x0, #-128] // on-fault: .LBB16_2
; CHECK-NEXT:  // %bb.1: // %not_null
; CHECK-NEXT:    ret
; CHECK-NEXT:  .LBB16_2:
; CHECK-NEXT:    mov w0, #42
; CHECK-NEXT:    ret
 entry:
  %c = icmp eq i32* %x, null
  br i1 %c, label %is_null, label %not_null, !make.implicit !0

 is_null:
  ret i32 42

 not_null:
  %x.gep = getelementptr i32, i32* %x, i32 -32
  %t = load i32, i32* %x.gep
  ret i32 %t
}

;; OHOS_LOCAL begin
;; LLVM does not support implicit null checks for ldp and stp
;; The test must be fixed, when such is introduced
define i64 @imp_null_check_load_pair(i64* %array) {
; CHECK-LABEL: imp_null_check_load_pair:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    cbz x0, .LBB17_2
; CHECK-NEXT:  // %bb.1: // %if.end
; CHECK-NEXT:    ldp x8, x9, [x0]
; CHECK-NEXT:    add x0, x9, x8
; CHECK-NEXT:  .LBB17_2: // %return
; CHECK-NEXT:    ret
entry:
  %cmp = icmp eq i64* %array, null
  br i1 %cmp, label %return, label %if.end, !make.implicit !0

if.end:                                           ; preds = %entry
  %0 = load i64, i64* %array, align 8
  %arrayidx1 = getelementptr inbounds i64, i64* %array, i64 1
  %1 = load i64, i64* %arrayidx1, align 8
  %add = add nsw i64 %1, %0
  br label %return

return:                                           ; preds = %entry, %if.end
  %retval.0 = phi i64 [ %add, %if.end ], [ 0, %entry ]
  ret i64 %retval.0
}
;; OHOS_LOCAL end

!0 = !{}
