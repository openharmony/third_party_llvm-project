; RUN: llc < %s -emulated-tls -mtriple=riscv64-linux-gnu | FileCheck -check-prefix=RISCV64 %s
; RUN: llc < %s -emulated-tls -mtriple=riscv64-linux-ohos | FileCheck -check-prefix=RISCV64 %s

; RUN: llc < %s -mtriple=riscv64-linux-gnu | FileCheck -check-prefix=NoEMU %s
; RUN: llc < %s -mtriple=riscv64-linux-ohos | FileCheck -check-prefix=RISCV64 %s

; Copied from X86/emutls.ll

; NoEMU-NOT: __emutls

; Use my_emutls_get_address like __emutls_get_address.
@my_emutls_v_xyz = external global i8*, align 4
declare i8* @my_emutls_get_address(i8*)

define dso_local i32 @my_get_xyz() {
; RISCV64-LABEL: my_get_xyz:
; RISCV64:         lui a0, %hi(my_emutls_v_xyz)
; RISCV64-NEXT:    addi a0, a0, %lo(my_emutls_v_xyz)
; RISCV64-NEXT:    call my_emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %call = call i8* @my_emutls_get_address(i8* bitcast (i8** @my_emutls_v_xyz to i8*))
  %0 = bitcast i8* %call to i32*
  %1 = load i32, i32* %0, align 4
  ret i32 %1
}

@i1 = dso_local thread_local global i32 15
@i2 = external thread_local global i32
@i3 = internal thread_local global i32 15
@i4 = hidden thread_local global i32 15
@i5 = external hidden thread_local global i32
@s1 = dso_local thread_local global i16 15
@b1 = dso_local thread_local global i8 0

define dso_local i32 @f1() {
; RISCV64-LABEL: f1:
; RISCV64:         lui a0, %hi(__emutls_v.i1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i32, i32* @i1
  ret i32 %tmp1
}

define dso_local i32* @f2() {
; RISCV64-LABEL: f2:
; RISCV64:         lui a0, %hi(__emutls_v.i1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  ret i32* @i1
}

define dso_local i32 @f3() nounwind {
; RISCV64-LABEL: f3:
; RISCV64:         lui a0, %hi(__emutls_v.i2)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i2)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i32, i32* @i2
  ret i32 %tmp1
}

define dso_local i32* @f4() {
; RISCV64-LABEL: f4:
; RISCV64:         lui a0, %hi(__emutls_v.i2)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i2)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  ret i32* @i2
}

define dso_local i32 @f5() nounwind {
; RISCV64-LABEL: f5:
; RISCV64:         lui a0, %hi(__emutls_v.i3)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i3)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i32, i32* @i3
  ret i32 %tmp1
}

define dso_local i32* @f6() {
; RISCV64-LABEL: f6:
; RISCV64:         lui a0, %hi(__emutls_v.i3)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i3)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  ret i32* @i3
}

define dso_local i32 @f7() {
; RISCV64-LABEL: f7:
; RISCV64:         lui a0, %hi(__emutls_v.i4)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i4)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i32, i32* @i4
  ret i32 %tmp1
}

define dso_local i32* @f8() {
; RISCV64-LABEL: f8:
; RISCV64:         lui a0, %hi(__emutls_v.i4)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i4)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  ret i32* @i4
}

define dso_local i32 @f9() {
; RISCV64-LABEL: f9:
; RISCV64:         lui a0, %hi(__emutls_v.i5)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i5)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lw a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i32, i32* @i5
  ret i32 %tmp1
}

define dso_local i32* @f10() {
; RISCV64-LABEL: f10:
; RISCV64:         lui a0, %hi(__emutls_v.i5)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.i5)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  ret i32* @i5
}

define dso_local i16 @f11() {
; RISCV64-LABEL: f11:
; RISCV64:         lui a0, %hi(__emutls_v.s1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.s1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lh a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i16, i16* @s1
  ret i16 %tmp1
}

define dso_local i32 @f12sext() {
; RISCV64-LABEL: f12sext:
; RISCV64:         lui a0, %hi(__emutls_v.s1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.s1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lh a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i16, i16* @s1
  %tmp2 = sext i16 %tmp1 to i32
  ret i32 %tmp2
}

define dso_local i32 @f12zext() {
; RISCV64-LABEL: f12zext:
; RISCV64:         lui a0, %hi(__emutls_v.s1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.s1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lhu a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i16, i16* @s1
  %tmp2 = zext i16 %tmp1 to i32
  ret i32 %tmp2
}

define dso_local i8 @f13() {
; RISCV64-LABEL: f13:
; RISCV64:         lui a0, %hi(__emutls_v.b1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.b1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lb a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i8, i8* @b1
  ret i8 %tmp1
}

define dso_local i32 @f14sext() {
; RISCV64-LABEL: f14sext:
; RISCV64:         lui a0, %hi(__emutls_v.b1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.b1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lb a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i8, i8* @b1
  %tmp2 = sext i8 %tmp1 to i32
  ret i32 %tmp2
}

define dso_local i32 @f14zext() {
; RISCV64-LABEL: f14zext:
; RISCV64:         lui a0, %hi(__emutls_v.b1)
; RISCV64-NEXT:    addi a0, a0, %lo(__emutls_v.b1)
; RISCV64-NEXT:    call __emutls_get_address@plt
; RISCV64-NEXT:    lbu a0, 0(a0)
; RISCV64-NEXT:    ld ra, 8(sp)
; RISCV64-NEXT:    addi sp, sp, 16
; RISCV64-NEXT:    ret
entry:
  %tmp1 = load i8, i8* @b1
  %tmp2 = zext i8 %tmp1 to i32
  ret i32 %tmp2
}

;;;;;;;;;;;;;; 64-bit __emutls_v. and __emutls_t.

; RISCV64-LABEL: __emutls_v.i1:
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 0
; RISCV64-NEXT: .quad __emutls_t.i1

; RISCV64-LABEL: __emutls_t.i1:
; RISCV64-NEXT: .word 15

; RISCV64-NOT:   __emutls_v.i2

; RISCV64-LABEL: __emutls_v.i3:
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 0
; RISCV64-NEXT: .quad __emutls_t.i3

; RISCV64-LABEL: __emutls_t.i3:
; RISCV64-NEXT: .word 15

; RISCV64-LABEL: __emutls_v.i4:
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 4
; RISCV64-NEXT: .quad 0
; RISCV64-NEXT: .quad __emutls_t.i4

; RISCV64-LABEL: __emutls_t.i4:
; RISCV64-NEXT: .word 15

; RISCV64-NOT:   __emutls_v.i5:
; RISCV64:      .hidden __emutls_v.i5
; RISCV64-NOT:   __emutls_v.i5:

; RISCV64-LABEL: __emutls_v.s1:
; RISCV64-NEXT: .quad 2
; RISCV64-NEXT: .quad 2
; RISCV64-NEXT: .quad 0
; RISCV64-NEXT: .quad __emutls_t.s1

; RISCV64-LABEL: __emutls_t.s1:
; RISCV64-NEXT: .half 15

; RISCV64-LABEL: __emutls_v.b1:
; RISCV64-NEXT: .quad 1
; RISCV64-NEXT: .quad 1
; RISCV64-NEXT: .quad 0
; RISCV64-NEXT: .quad 0

; RISCV64-NOT:  __emutls_t.b1
