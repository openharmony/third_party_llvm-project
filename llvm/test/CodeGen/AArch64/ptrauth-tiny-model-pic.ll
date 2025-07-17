; RUN: llc -verify-machineinstrs -mtriple=aarch64 -mattr=+pauth              -code-model=tiny \
; RUN:   -relocation-model=pic < %s | FileCheck --check-prefixes=CHECK %s

; RUN: llc -verify-machineinstrs -mtriple=aarch64 -mattr=+pauth              -code-model=tiny \
; RUN:   -relocation-model=pic -fast-isel < %s | FileCheck --check-prefixes=CHECK %s

; RUN: llc -verify-machineinstrs -mtriple=aarch64 -mattr=+pauth              -code-model=tiny \
; RUN:   -relocation-model=pic -global-isel -global-isel-abort=1 < %s | FileCheck --check-prefixes=CHECK %s

; Note: fast-isel tests here will fall back to isel

@src = external local_unnamed_addr global [65536 x i8], align 1
@dst = external global [65536 x i8], align 1
@ptr = external local_unnamed_addr global ptr, align 8

define dso_preemptable void @foo1() {
; CHECK-LABEL: foo1:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    adr   x17, :got_auth:src
; CHECK-NEXT:    ldr   x8,  [x17]
; CHECK-NEXT:    autda x8,  x17
; CHECK-NEXT:    ldrb  w8,  [x8]
; CHECK-NEXT:    adr   x17, :got_auth:dst
; CHECK-NEXT:    ldr   x9,  [x17]
; CHECK-NEXT:    autda x9,  x17
; CHECK-NEXT:    strb  w8,  [x9]
; CHECK-NEXT:    ret

entry:
  %0 = load i8, ptr @src, align 1
  store i8 %0, ptr @dst, align 1
  ret void
}

define dso_preemptable void @foo2() {
; CHECK-LABEL: foo2:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    adr   x17, :got_auth:ptr
; CHECK-NEXT:    ldr   x8,  [x17]
; CHECK-NEXT:    autda x8,  x17
; CHECK-NEXT:    adr   x17, :got_auth:dst
; CHECK-NEXT:    ldr   x9,  [x17]
; CHECK-NEXT:    autda x9,  x17
; CHECK-NEXT:    str   x9,  [x8]
; CHECK-NEXT:    ret

entry:
  store ptr @dst, ptr @ptr, align 8
  ret void
}

define dso_preemptable void @foo3() {
; CHECK-LABEL: foo3:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    adr   x17, :got_auth:src
; CHECK-NEXT:    ldr   x8,  [x17]
; CHECK-NEXT:    autda x8,  x17
; CHECK-NEXT:    ldrb  w8,  [x8]
; CHECK-NEXT:    adr   x17, :got_auth:ptr
; CHECK-NEXT:    ldr   x9,  [x17]
; CHECK-NEXT:    autda x9,  x17
; CHECK-NEXT:    ldr   x9,  [x9]
; CHECK-NEXT:    strb  w8,  [x9]
; CHECK-NEXT:    ret

entry:
  %0 = load i8, ptr @src, align 1
  %1 = load ptr, ptr @ptr, align 8
  store i8 %0, ptr %1, align 1
  ret void
}

@lsrc = internal global i8 0, align 4
@ldst = internal global i8 0, align 4
@lptr = internal global ptr null, align 8

declare void @func(...)

define dso_preemptable ptr @externfuncaddr() {
; CHECK-LABEL: externfuncaddr:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    adr   x17, :got_auth:func
; CHECK-NEXT:    ldr   x0,  [x17]
; CHECK-NEXT:    autia x0,  x17
; CHECK-NEXT:    ret

entry:
  ret ptr @func
}

define dso_preemptable ptr @localfuncaddr() {
; CHECK-LABEL: localfuncaddr:
; CHECK:       // %bb.0: // %entry
; CHECK-NEXT:    adr   x17, :got_auth:externfuncaddr
; CHECK-NEXT:    ldr   x0,  [x17]
; CHECK-NEXT:    autia x0,  x17
; CHECK-NEXT:    ret

entry:
  ret ptr @externfuncaddr
}

!llvm.module.flags = !{!0}
!0 = !{i32 8, !"ptrauth-elf-got", i32 1}
