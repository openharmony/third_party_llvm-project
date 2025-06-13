
; RUN: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth                                      \
; RUN:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=ELF
; RUN-NOT: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth                         -global-isel \
; RUN-NOT:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=ELF
; RUN: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth                         -fast-isel   \
; RUN:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=ELF

; RUN: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth -mattr=+pauth-hint-only              \
; RUN:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=HINT
; RUN-NOT: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth -mattr=+pauth-hint-only -global-isel \
; RUN-NOT:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=HINT
; RUN: llc -mtriple aarch64-linux-gnu   -mattr=+bti -mattr=+pauth -mattr=+pauth-hint-only -fast-isel   \
; RUN:    -asm-verbose=false -o - %s | FileCheck %s --check-prefixes=HINT

; ptrauth tail-calls can only use x16/x17 with BTI.

; ELF-LABEL: test_tailcall_ia_0:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  braaz x16

; HINT-LABEL: test_tailcall_ia_0:
; HINT-NEXT:  bti c
; HINT-NEXT:  mov x17, x0
; HINT-NEXT:  mov x16, xzr
; HINT-NEXT:  autia1716
; HINT-NEXT:  br x17
define i32 @test_tailcall_ia_0(i32 ()* %arg0) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 0) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ib_0:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  brabz x16

; HINT-LABEL: test_tailcall_ib_0:
; HINT-NEXT:  bti c
; HINT-NEXT:  mov x17, x0
; HINT-NEXT:  mov x16, xzr
; HINT-NEXT:  autib1716
; HINT-NEXT:  br x17
define i32 @test_tailcall_ib_0(i32 ()* %arg0) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 0) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ia_imm:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  mov x17, #42
; ELF-NEXT:  braa x16, x17

; HINT-LABEL: test_tailcall_ia_imm:
; HINT-NEXT:  bti c
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ia_imm(i32 ()* %arg0) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 42) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ib_imm:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  mov x17, #42
; ELF-NEXT:  brab x16, x17

; HINT-LABEL: test_tailcall_ib_imm:
; HINT-NEXT:  bti c
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ib_imm(i32 ()* %arg0) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 42) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ia_var:
; ELF-NEXT:    bti c
; ELF-NEXT:    ldr x1, [x1]
; ELF-NEXT:    mov x16, x0
; ELF-NEXT:    braa x16, x1

; HINT-LABEL: test_tailcall_ia_var:
; HINT-NEXT:    bti c
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ia_var(i32 ()* %arg0, i64* %arg1) #0 {
  %tmp0 = load i64, i64* %arg1
  %tmp1 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 %tmp0) ]
  ret i32 %tmp1
}

; ELF-LABEL: test_tailcall_ib_var:
; ELF-NEXT:    bti c
; ELF-NEXT:    ldr x1, [x1]
; ELF-NEXT:    mov x16, x0
; ELF-NEXT:    brab x16, x1

; HINT-LABEL: test_tailcall_ib_var:
; HINT-NEXT:    bti c
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ib_var(i32 ()* %arg0, i64* %arg1) #0 {
  %tmp0 = load i64, i64* %arg1
  %tmp1 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 %tmp0) ]
  ret i32 %tmp1
}

; ELF-LABEL: test_tailcall_ia_arg:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  braa x16, x1

; HINT-LABEL: test_tailcall_ia_arg:
; HINT-NEXT:  bti c
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ia_arg(i32 ()* %arg0, i64 %arg1) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ib_arg:
; ELF-NEXT:  bti c
; ELF-NEXT:  mov x16, x0
; ELF-NEXT:  brab x16, x1

; HINT-LABEL: test_tailcall_ib_arg:
; HINT-NEXT:  bti c
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ib_arg(i32 ()* %arg0, i64 %arg1) #0 {
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp0
}

; ELF-LABEL: test_tailcall_ia_arg_ind:
; ELF-NEXT:  bti c
; ELF-NEXT:  ldr x16, [x0]
; ELF-NEXT:  braa x16, x1

; HINT-LABEL: test_tailcall_ia_arg_ind:
; HINT-NEXT:  bti c
; HINT-NEXT:    ldr x17, [x0]
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ia_arg_ind(i32 ()** %arg0, i64 %arg1) #0 {
  %tmp0 = load i32 ()*, i32 ()** %arg0
  %tmp1 = tail call i32 %tmp0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp1
}

; ELF-LABEL: test_tailcall_ib_arg_ind:
; ELF-NEXT:  bti c
; ELF-NEXT:  ldr x16, [x0]
; ELF-NEXT:  brab x16, x1

; HINT-LABEL: test_tailcall_ib_arg_ind:
; HINT-NEXT:  bti c
; HINT-NEXT:    ldr x17, [x0]
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
define i32 @test_tailcall_ib_arg_ind(i32 ()** %arg0, i64 %arg1) #0 {
  %tmp0 = load i32 ()*, i32 ()** %arg0
  %tmp1 = tail call i32 %tmp0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp1
}

attributes #0 = { nounwind "branch-target-enforcement"="true" }
