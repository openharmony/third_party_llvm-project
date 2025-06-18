; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=0         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth              %s -o - | FileCheck %s --check-prefixes=CHECK

; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=1         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth              %s -o - | FileCheck %s --check-prefixes=CHECK

; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=1 -global-isel-abort=1 -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth              %s -o - | FileCheck %s --check-prefixes=CHECK

;; Note: for FastISel, we fall back to SelectionDAG

@var = global i32 0

define i32 @get_globalvar() {
; CHECK-LABEL: get_globalvar:
; CHECK:         adrp  x17, :got_auth:var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:var
; CHECK-NEXT:    ldr   x8,  [x17]
; CHECK-NEXT:    autda x8,  x17
; CHECK-NEXT:    ldr   w0,  [x8]
; CHECK-NEXT:    ret

  %val = load i32, ptr @var
  ret i32 %val
}

define ptr @get_globalvaraddr() {
; CHECK-LABEL: get_globalvaraddr:
; CHECK:         adrp  x17, :got_auth:var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:var
; CHECK-NEXT:    ldr   x0,  [x17]
; CHECK-NEXT:    autda x0,  x17
; CHECK-NEXT:    ret

  %val = load i32, ptr @var
  ret ptr @var
}

declare i32 @foo()

define ptr @resign_globalfunc() {
; CHECK-LABEL: resign_globalfunc:
; CHECK:         adrp  x17, :got_auth:foo
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:foo
; CHECK-NEXT:    ldr   x0, [x17]
; CHECK-NEXT:    autia x0, x17
; CHECK-NEXT:    ret

  ret ptr @foo
}

define ptr @resign_globalvar() {
; CHECK-LABEL: resign_globalvar:
; CHECK:         adrp  x17, :got_auth:var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:var
; CHECK-NEXT:    ldr   x0, [x17]
; CHECK-NEXT:    autda x0, x17
; CHECK-NEXT:    ret

  ret ptr @var
}

define ptr @resign_globalvar_offset() {
; CHECK-LABEL: resign_globalvar_offset:
; CHECK:         adrp  x17, :got_auth:var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:var
; CHECK-NEXT:    ldr   x8, [x17]
; CHECK-NEXT:    autda x8, x17
; CHECK-NEXT:    add   x0, x8, #16
; CHECK-NEXT:    ret

  ret ptr getelementptr (i8, ptr @var, i64 16)
}

!llvm.module.flags = !{!0}
!0 = !{i32 8, !"ptrauth-elf-got", i32 1}
