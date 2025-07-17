; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=0         -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=0 -fast-isel=1         -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -global-isel=1 -global-isel-abort=1 -relocation-model=pic \
; RUN:   -mattr=+pauth              -o - %s | FileCheck --check-prefixes=CHECK %s

; RUN: llc -mtriple=aarch64-none-linux-gnu -code-model=tiny -mattr=+pauth              -o - %s | \
; RUN:   FileCheck --check-prefixes=CHECK-TINY %s

;; Note: for FastISel, we fall back to SelectionDAG

declare extern_weak dso_local i32 @var()

define ptr @foo() {
; The usual ADRP/ADD pair can't be used for a weak reference because it must
; evaluate to 0 if the symbol is undefined. We use a GOT entry for PIC
; otherwise a litpool entry.
  ret ptr @var

; CHECK-LABEL: foo:
; CHECK:         adrp  x17, :got_auth:var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:var
; CHECK-NEXT:    ldr   x0,  [x17]
; CHECK-NEXT:    cbz   x0,  .Lundef_weak0
; CHECK-NEXT:    autia x0,  x17
; CHECK-NEXT:  .Lundef_weak0:
; CHECK-NEXT:    ret

; CHECK-TINY-LABEL: foo:
; CHECK-TINY:         adr   x17, :got_auth:var
; CHECK-TINY-NEXT:    ldr   x0,  [x17]
; CHECK-TINY-NEXT:    cbz   x0,  .Lundef_weak0
; CHECK-TINY-NEXT:    autia x0,  x17
; CHECK-TINY-NEXT:  .Lundef_weak0:
; CHECK-TINY-NEXT:    ret
}

@arr_var = extern_weak global [10 x i32]

define ptr @bar() {
  %addr = getelementptr [10 x i32], ptr @arr_var, i32 0, i32 5
  ret ptr %addr

; CHECK-LABEL: bar:
; CHECK:         adrp  x17, :got_auth:arr_var
; CHECK-NEXT:    add   x17, x17, :got_auth_lo12:arr_var
; CHECK-NEXT:    ldr   x8,  [x17]
; CHECK-NEXT:    cbz   x8,  .Lundef_weak1
; CHECK-NEXT:    autda x8,  x17
; CHECK-NEXT:  .Lundef_weak1:
; CHECK-NEXT:    add   x0,  x8, #20
; CHECK-NEXT:    ret

; CHECK-TINY-LABEL: bar:
; CHECK-TINY:         adr   x17, :got_auth:arr_var
; CHECK-TINY-NEXT:   ldr   x8,  [x17]
; CHECK-TINY-NEXT:   cbz   x8,  .Lundef_weak1
; CHECK-TINY-NEXT:   autda x8,  x17
; CHECK-TINY-NEXT:  .Lundef_weak1:
; CHECK-TINY-NEXT:    add   x0,  x8, #20
; CHECK-TINY-NEXT:    ret
}

!llvm.module.flags = !{!0}
!0 = !{i32 8, !"ptrauth-elf-got", i32 1}
