; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=0         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth                         -mattr=+fpac %s -o - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-ELF
; RUN-NOT: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=0         -verify-machineinstrs \
; RUN-NOT:   -relocation-model=pic -mattr=+pauth                                      %s -o - \
; RUN-NOT:   | FileCheck %s --check-prefixes=CHECK,CHECK-ELF,TRAP-ELF

; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=0         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth -mattr=+pauth-hint-only -mattr=+fpac %s -o - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-HINT
; RUN-NOT: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=0         -verify-machineinstrs \
; RUN-NOT:   -relocation-model=pic -mattr=+pauth -mattr=+pauth-hint-only              %s -o - \
; RUN-NOT:   | FileCheck %s --check-prefixes=CHECK,CHECK-HINT,TRAP-HINT

; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=1         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth                         -mattr=+fpac %s -o - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-ELF
; RUN-NOT: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=1         -verify-machineinstrs \
; RUN-NOT:   -relocation-model=pic -mattr=+pauth                                      %s -o - \
; RUN-NOT:   | FileCheck %s --check-prefixes=CHECK,CHECK-ELF,TRAP-ELF

; RUN: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=1         -verify-machineinstrs \
; RUN:   -relocation-model=pic -mattr=+pauth -mattr=+pauth-hint-only -mattr=+fpac %s -o - \
; RUN:   | FileCheck %s --check-prefixes=CHECK,CHECK-HINT
; RUN-NOT: llc -mtriple=aarch64-linux-gnu -global-isel=0 -fast-isel=1         -verify-machineinstrs \
; RUN-NOT:   -relocation-model=pic -mattr=+pauth -mattr=+pauth-hint-only              %s -o - \
; RUN-NOT:   | FileCheck %s --check-prefixes=CHECK,CHECK-HINT,TRAP-HINT

;; Note: for FastISel, we fall back to SelectionDAG

@var = global i32 0

define i32 @get_globalvar() {
; CHECK-LABEL: get_globalvar:
; CHECK-ELF-NEXT:    .cfi_startproc
; CHECK-ELF-NEXT:    // %bb.0: 
; CHECK:         adrp  x8, :got:var
; CHECK-NEXT:    ldr   x8, [x8, :got_lo12:var]
; CHECK-NEXT:    ldr   w0,  [x8]
; CHECK-NEXT:    ret

  %val = load i32, ptr @var
  ret i32 %val
}

define ptr @get_globalvaraddr() {
; CHECK-LABEL: get_globalvaraddr:
; CHECK-ELF-NEXT:    .cfi_startproc
; CHECK-ELF-NEXT:    // %bb.0: 
; CHECK:         adrp  [[TmpReg:x[0-9]+]], :got:var
; CHECK-NEXT:    ldr   x0, [[[TmpReg]], :got_lo12:var]
; CHECK-NEXT:    ret

  %val = load i32, ptr @var
  ret ptr @var
}

declare i32 @foo()

define ptr @resign_globalfunc() {
; CHECK-ELF-LABEL: resign_globalfunc:
; CHECK-ELF-NEXT:    .cfi_startproc
; CHECK-ELF-NEXT:    // %bb.0: 
; CHECK-ELF:         adrp  x17, :got:foo
; CHECK-ELF-NEXT:    ldr   x17, [x17, :got_lo12:foo]

; TRAP-ELF-NEXT:     mov   x16, x17
; TRAP-ELF-NEXT:     xpaci x16
; TRAP-ELF-NEXT:     cmp   x17, x16
; TRAP-ELF-NEXT:     b.eq  .Lauth_success_2
; TRAP-ELF-NEXT:     brk   #0xc470
; TRAP-ELF-NEXT:   .Lauth_success_2:

; CHECK-ELF-NEXT:    mov   x16, #42
; CHECK-ELF-NEXT:    pacia x17, x16
; CHECK-ELF-NEXT:    mov   x0,  x17
; CHECK-ELF-NEXT:    ret


; CHECK-HINT-LABEL: resign_globalfunc:
; CHECK-HINT-NEXT:    .cfi_startproc
; CHECK-HINT-NEXT:    // %bb.0: 
; CHECK-HINT:         adrp  x17, :got:foo
; CHECK-HINT-NEXT:    ldr   x17, [x17, :got_lo12:foo]

; TRAP-HINT-NEXT:     mov   x30, x17
; TRAP-HINT-NEXT:     xpaclri
; TRAP-HINT-NEXT:     mov   x16, x30
; TRAP-HINT-NEXT:     cmp   x17, x16
; TRAP-HINT-NEXT:     b.eq  .Lauth_success_2
; TRAP-HINT-NEXT:     brk   #0xc470
; TRAP-HINT-NEXT:   .Lauth_success_2:

; CHECK-HINT-NEXT:    mov   x16, #42
; CHECK-HINT-NEXT:    pacia1716
; CHECK-HINT-NEXT:    mov   x0,  x17
; CHECK-HINT-NEXT:    ret

  ret ptr ptrauth (ptr @foo, i32 0, i64 42)
}

define ptr @resign_globalvar() {
; CHECK-ELF-LABEL: resign_globalvar:
; CHECK-ELF-NEXT:    .cfi_startproc
; CHECK-ELF-NEXT:    // %bb.0: 
; CHECK-ELF:         adrp  x17, :got:var
; CHECK-ELF-NEXT:    ldr   x17, [x17, :got_lo12:var]

; TRAP-ELF-NEXT:     mov   x16, x17
; TRAP-ELF-NEXT:     xpacd x16
; TRAP-ELF-NEXT:     cmp   x17, x16
; TRAP-ELF-NEXT:     b.eq  .Lauth_success_3
; TRAP-ELF-NEXT:     brk   #0xc472
; TRAP-ELF-NEXT:   .Lauth_success_3:

; CHECK-ELF-NEXT:     mov   x16, #43
; CHECK-ELF-NEXT:     pacdb x17, x16
; CHECK-ELF-NEXT:     mov   x0,  x17
; CHECK-ELF-NEXT:     ret


; CHECK-HINT-LABEL: resign_globalvar:
; CHECK-HINT-NEXT:    .cfi_startproc
; CHECK-HINT-NEXT:    // %bb.0: 
; CHECK-HINT:         adrp  x17, :got:var
; CHECK-HINT-NEXT:    ldr   x17, [x17, :got_lo12:var]

; TRAP-HINT-NEXT:     mov   x30, x17
; TRAP-HINT-NEXT:     xpaclri
; TRAP-HINT-NEXT:     mov   x16, x30
; TRAP-HINT-NEXT:     cmp   x17, x16
; TRAP-HINT-NEXT:     b.eq  .Lauth_success_3
; TRAP-HINT-NEXT:     brk   #0xc472
; TRAP-HINT-NEXT:   .Lauth_success_3:

; CHECK-HINT-NEXT:    mov   x16, #43
; CHECK-HINT-NEXT:    pacib1716
; CHECK-HINT-NEXT:    mov   x0,  x17
; CHECK-HINT-NEXT:    ret

  ret ptr ptrauth (ptr @var, i32 3, i64 43)
}

define ptr @resign_globalvar_offset() {
; CHECK-ELF-LABEL: resign_globalvar_offset:
; CHECK-ELF-NEXT:    .cfi_startproc
; CHECK-ELF-NEXT:    // %bb.0: 
; CHECK-ELF:         adrp  x17, :got:var
; CHECK-ELF-NEXT:    ldr   x17, [x17, :got_lo12:var]

; TRAP-ELF-NEXT:     mov   x16, x17
; TRAP-ELF-NEXT:     xpacd x16
; TRAP-ELF-NEXT:     cmp   x17, x16
; TRAP-ELF-NEXT:     b.eq  .Lauth_success_4
; TRAP-ELF-NEXT:     brk   #0xc472
; TRAP-ELF-NEXT:   .Lauth_success_4:

; CHECK-ELF-NEXT:     add   x17, x17, #16
; CHECK-ELF-NEXT:     mov   x16, #44
; CHECK-ELF-NEXT:     pacda x17, x16
; CHECK-ELF-NEXT:     mov   x0,  x17
; CHECK-ELF-NEXT:     ret


; CHECK-HINT-LABEL: resign_globalvar_offset:
; CHECK-HINT-NEXT:    .cfi_startproc
; CHECK-HINT-NEXT:    // %bb.0: 
; CHECK-HINT:         adrp  x17, :got:var
; CHECK-HINT-NEXT:    ldr   x17, [x17, :got_lo12:var]

; TRAP-HINT-NEXT:     mov   x30, x17
; TRAP-HINT-NEXT:     xpaclri
; TRAP-HINT-NEXT:     mov   x16, x30
; TRAP-HINT-NEXT:     cmp   x17, x16
; TRAP-HINT-NEXT:     b.eq  .Lauth_success_4
; TRAP-HINT-NEXT:     brk   #0xc472
; TRAP-HINT-NEXT:   .Lauth_success_4:

; CHECK-HINT-NEXT:    add   x17, x17, #16
; CHECK-HINT-NEXT:    mov   x16, #44
; CHECK-HINT-NEXT:    pacia1716
; CHECK-HINT-NEXT:    mov   x0,  x17
; CHECK-HINT-NEXT:    ret

  ret ptr ptrauth (ptr getelementptr (i8, ptr @var, i64 16), i32 2, i64 44)
}

!llvm.module.flags = !{!0}
!0 = !{i32 8, !"ptrauth-elf-got", i32 1}
