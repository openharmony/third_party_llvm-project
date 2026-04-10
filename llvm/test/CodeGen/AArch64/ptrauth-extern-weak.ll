; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=0         -relocation-model=pic \
; RUN:   -mattr=+pauth -mattr=+fpac -o - %s | FileCheck --check-prefixes=CHECK,NOTRAP %s
; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=0         -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK,TRAP %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=1         -relocation-model=pic \
; RUN:   -mattr=+pauth -mattr=+fpac -o - %s | FileCheck --check-prefixes=CHECK,NOTRAP %s
; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=1         -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK,TRAP %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=1 -global-isel-abort=1 -relocation-model=pic \
; RUN:   -mattr=+pauth -mattr=+fpac -o - %s | FileCheck --check-prefixes=CHECK,NOTRAP %s
; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=1 -global-isel-abort=1 -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK,TRAP %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -code-model=tiny -mattr=+pauth -mattr=+fpac -o - %s | \
; RUN:   FileCheck --check-prefixes=CHECK-TINY,NOTRAP-TINY %s
; RUN: llc -mtriple=aarch64-none-linux-gnu -code-model=tiny -mattr=+pauth              -o - %s | \
; RUN:   FileCheck --check-prefixes=CHECK-TINY,TRAP-TINY %s

;; Note: for FastISel, we fall back to SelectionDAG

declare extern_weak dso_local i32 @var()

define ptr @foo() {
; The usual ADRP/ADD pair can't be used for a weak reference because it must
; evaluate to 0 if the symbol is undefined. We use a GOT entry for PIC
; otherwise a litpool entry.
  ret ptr @var

; CHECK-LABEL: foo:
; CHECK:         adrp  x16, :got_auth:var
; CHECK-NEXT:    add   x16, x16, :got_auth_lo12:var
; NOTRAP-NEXT:   ldr   x0,  [x16]
; NOTRAP-NEXT:   cbz   x0,  .Lundef_weak0
; NOTRAP-NEXT:   autia x0,  x16
; TRAP-NEXT:     ldr   x17, [x16]
; TRAP-NEXT:     cbz   x17, .Lundef_weak0
; TRAP-NEXT:     autia x17, x16
; CHECK-NEXT:  .Lundef_weak0:
; TRAP-NEXT:     mov   x16, x17
; TRAP-NEXT:     xpaci x16
; TRAP-NEXT:     cmp   x16, x17
; TRAP-NEXT:     b.eq  .Lauth_success_0
; TRAP-NEXT:     brk   #0xc470
; TRAP-NEXT:   .Lauth_success_0:
; TRAP-NEXT:     mov   x0,  x17
; CHECK-NEXT:    ret

; CHECK-TINY-LABEL: foo:
; CHECK-TINY:         adr   x16, :got_auth:var
; NOTRAP-TINY-NEXT:   ldr   x0,  [x16]
; NOTRAP-TINY-NEXT:   cbz   x0,  .Lundef_weak0
; NOTRAP-TINY-NEXT:   autia x0,  x16
; TRAP-TINY-NEXT:     ldr   x17, [x16]
; TRAP-TINY-NEXT:     cbz   x17, .Lundef_weak0
; TRAP-TINY-NEXT:     autia x17, x16
; CHECK-TINY-NEXT:  .Lundef_weak0:
; TRAP-TINY-NEXT:     mov   x16, x17
; TRAP-TINY-NEXT:     xpaci x16
; TRAP-TINY-NEXT:     cmp   x16, x17
; TRAP-TINY-NEXT:     b.eq  .Lauth_success_0
; TRAP-TINY-NEXT:     brk   #0xc470
; TRAP-TINY-NEXT:   .Lauth_success_0:
; TRAP-TINY-NEXT:     mov   x0,  x17
; CHECK-TINY-NEXT:    ret
}

@arr_var = extern_weak global [10 x i32]

define ptr @bar() {
  %addr = getelementptr [10 x i32], ptr @arr_var, i32 0, i32 5
  ret ptr %addr

; CHECK-LABEL: bar:
; CHECK:         adrp  x16, :got_auth:arr_var
; CHECK-NEXT:    add   x16, x16, :got_auth_lo12:arr_var
; NOTRAP-NEXT:   ldr   x8,  [x16]
; NOTRAP-NEXT:   cbz   x8,  .Lundef_weak1
; NOTRAP-NEXT:   autda x8,  x16
; TRAP-NEXT:     ldr   x17, [x16]
; TRAP-NEXT:     cbz   x17, .Lundef_weak1
; TRAP-NEXT:     autda x17, x16
; CHECK-NEXT:  .Lundef_weak1:
; TRAP-NEXT:     mov   x16, x17
; TRAP-NEXT:     xpacd x16
; TRAP-NEXT:     cmp   x16, x17
; TRAP-NEXT:     b.eq  .Lauth_success_1
; TRAP-NEXT:     brk   #0xc472
; TRAP-NEXT:   .Lauth_success_1:
; TRAP-NEXT:     mov   x8,  x17
; CHECK-NEXT:    add   x0,  x8, #20
; CHECK-NEXT:    ret

; CHECK-TINY-LABEL: bar:
; CHECK-TINY:         adr   x16, :got_auth:arr_var
; NOTRAP-TINY-NEXT:   ldr   x8,  [x16]
; NOTRAP-TINY-NEXT:   cbz   x8,  .Lundef_weak1
; NOTRAP-TINY-NEXT:   autda x8,  x16
; TRAP-TINY-NEXT:     ldr   x17, [x16]
; TRAP-TINY-NEXT:     cbz   x17, .Lundef_weak1
; TRAP-TINY-NEXT:     autda x17, x16
; CHECK-TINY-NEXT:  .Lundef_weak1:
; TRAP-TINY-NEXT:     mov   x16, x17
; TRAP-TINY-NEXT:     xpacd x16
; TRAP-TINY-NEXT:     cmp   x16, x17
; TRAP-TINY-NEXT:     b.eq  .Lauth_success_1
; TRAP-TINY-NEXT:     brk   #0xc472
; TRAP-TINY-NEXT:   .Lauth_success_1:
; TRAP-TINY-NEXT:     mov   x8,  x17
; CHECK-TINY-NEXT:    add   x0,  x8, #20
; CHECK-TINY-NEXT:    ret
}

!llvm.module.flags = !{!0}
!0 = !{i32 8, !"ptrauth-elf-got", i32 1}
