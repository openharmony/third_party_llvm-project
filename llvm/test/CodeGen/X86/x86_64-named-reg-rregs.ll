
; RUN: sed 's/_REG_/r8/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r8 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r9/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r9 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r10/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r10 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r11/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r11 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r12/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r12 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r13/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r13 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r14/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r14 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

; RUN: sed 's/_REG_/r15/g' %s > %t
; RUN llc -march=x86-64 %t -O0 -o - 2> %t1 || echo
; RUN FileCheck < %t1 %t --check-prefix=CHECK-NO-RESERVATION
; RUN: llc -march=x86-64 %t -O0 -mattr=+fixed-r15 -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

define i64 @read_reg64() {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_
    %r = call i64 @llvm.read_register.i64(metadata !0)
    ret i64 %r
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define i32 @read_reg32() {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_d
    %r = call i32 @llvm.read_register.i32(metadata !1)
    ret i32 %r
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define i16 @read_reg16() {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_w
    %r = call i16 @llvm.read_register.i16(metadata !2)
    ret i16 %r
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define i8 @read_reg8() {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_b
    %r = call i8 @llvm.read_register.i8(metadata !3)
    ret i8 %r
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define void @write_reg64(i64 %val) {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_
    call void @llvm.write_register.i64(metadata !0, i64 %val)
    ret void
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define void @write_reg32(i32 %val) {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_d
    call void @llvm.write_register.i32(metadata !1, i32 %val)
    ret void
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define void @write_reg16(i16 %val) {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_w
    call void @llvm.write_register.i16(metadata !2, i16 %val)
    ret void
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define void @write_reg8(i8 %val) {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_
    ; CHECK-HAVE-RESERVATION: %_REG_b
    call void @llvm.write_register.i8(metadata !3, i8 %val)
    ret void
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

define i64 @main() {
    ; CHECK-HAVE-RESERVATION-NOT: push{{[qlwb]}} %_REG_

    ; CHECK-NO-RESERVATION: LLVM ERROR
    ; CHECK-NO-RESERVATION-SAME: Invalid register name global variable

    %r1 = call i64 @read_reg64()
    %r2 = call i32 @read_reg32()
    %r3 = call i16 @read_reg16()
    %r4 = call i8 @read_reg8()
    call void @write_reg64(i64 4294967296)
    call void @write_reg32(i32 65536)
    call void @write_reg16(i16 256)
    call void @write_reg8(i8 1)

    ret i64 %r1
    ; CHECK-HAVE-RESERVATION-NOT: pop{{[qlwb]}} %_REG_
}

declare i64 @llvm.read_register.i64(metadata)
declare void @llvm.write_register.i64(metadata, i64)
declare i32 @llvm.read_register.i32(metadata)
declare void @llvm.write_register.i32(metadata, i32)
declare i16 @llvm.read_register.i16(metadata)
declare void @llvm.write_register.i16(metadata, i16)
declare i8 @llvm.read_register.i8(metadata)
declare void @llvm.write_register.i8(metadata, i8)
!0 = !{!"_REG_\00"}
!1 = !{!"_REG_d\00"}
!2 = !{!"_REG_w\00"}
!3 = !{!"_REG_b\00"}
