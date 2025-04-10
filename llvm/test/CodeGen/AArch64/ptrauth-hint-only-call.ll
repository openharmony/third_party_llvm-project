; RUN: llc -mtriple aarch64 -mattr=+pauth                         -o - %s -asm-verbose=0               \
; RUN:                                               | FileCheck %s --check-prefixes=CHECK,ELF

; RUN: llc -mtriple aarch64 -mattr=+pauth -mattr=+pauth-hint-only -o - %s -asm-verbose=0               \
; RUN:                                               | FileCheck %s --check-prefixes=CHECK,HINT

; RUN: llc -mtriple aarch64 -mattr=+pauth                         -o - %s -asm-verbose=0  -global-isel \
; RUN:    -global-isel-abort=1 -verify-machineinstrs | FileCheck %s --check-prefixes=CHECK,ELF

; FIXME: GISel swaps the order of two move instructions (e.g; test_tailcall_ia_arg) with respect
; FIXME: the SelectionDAGISel
; RUN-NOT: llc -mtriple aarch64 -mattr=+pauth -mattr=+pauth-hint-only -o - %s -asm-verbose=0  -global-isel \
; RUN-NOT:    -global-isel-abort=1 -verify-machineinstrs | FileCheck %s --check-prefixes=CHECK,HINT

define i32 @test_call_ia_0(ptr %arg0) #0 {
; ELF-LABEL: test_call_ia_0:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    blraaz x0
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ia_0:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, xzr
; HINT-NEXT:    autia1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 0, i64 0) ]
  ret i32 %tmp0
}

define i32 @test_call_ib_0(ptr %arg0) #0 {
; ELF-LABEL: test_call_ib_0:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    blrabz x0
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ib_0:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT:    mov x17, x0
; HINT-NEXT:    mov x16, xzr
; HINT-NEXT:    autib1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 1, i64 0) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ia_0(ptr %arg0) #0 {
; ELF-LABEL: test_tailcall_ia_0:
; ELF-NEXT:    braaz x0

; HINT-LABEL: test_tailcall_ia_0:
; HINT-NEXT:  mov x17, x0
; HINT-NEXT:  mov x16, xzr
; HINT-NEXT:  autia1716
; HINT-NEXT:  br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 0) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ib_0(ptr %arg0) #0 {
; ELF-LABEL: test_tailcall_ib_0:
; ELF-NEXT:   brabz x0

; HINT-LABEL: test_tailcall_ib_0:
; HINT-NEXT:  mov x17, x0
; HINT-NEXT:  mov x16, xzr
; HINT-NEXT:  autib1716
; HINT-NEXT:  br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 0) ]
  ret i32 %tmp0
}

define i32 @test_call_ia_imm(ptr %arg0) #0 {
; ELF-LABEL: test_call_ia_imm:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    mov x17, #42
; ELF-NEXT:    blraa x0, x17
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ia_imm:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autia1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 0, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_call_ib_imm(ptr %arg0) #0 {
; ELF-LABEL: test_call_ib_imm:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    mov x17, #42
; ELF-NEXT:    blrab x0, x17
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ib_imm:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autib1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 1, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ia_imm(ptr %arg0) #0 {
; ELF-LABEL: test_tailcall_ia_imm:
; ELF-NEXT:    mov x16, #42
; ELF-NEXT:    braa x0, x16

; HINT-LABEL: test_tailcall_ia_imm:
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ib_imm(ptr %arg0) #0 {
; ELF-LABEL: test_tailcall_ib_imm:
; ELF-NEXT:    mov x16, #42
; ELF-NEXT:    brab x0, x16

; HINT-LABEL: test_tailcall_ib_imm:
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    mov x16, #42
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_call_ia_var(ptr %arg0, ptr %arg1) #0 {
; ELF-LABEL: test_call_ia_var:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    ldr x8, [x1]
; ELF-NEXT:    blraa x0, x8
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ia_var:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autia1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = load i64, ptr %arg1
  %tmp1 = call i32 %arg0() [ "ptrauth"(i32 0, i64 %tmp0) ]
  ret i32 %tmp1
}

define i32 @test_call_ib_var(ptr %arg0, ptr %arg1) #0 {
; ELF-LABEL: test_call_ib_var:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    ldr x8, [x1]
; ELF-NEXT:    blrab x0, x8
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ib_var:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = load i64, ptr %arg1
  %tmp1 = call i32 %arg0() [ "ptrauth"(i32 1, i64 %tmp0) ]
  ret i32 %tmp1
}

define i32 @test_tailcall_ia_var(ptr %arg0, ptr %arg1) #0 {
; ELF-LABEL: test_tailcall_ia_var:
; ELF:    ldr x1, [x1]
; ELF:    braa x0, x1

; HINT-LABEL: test_tailcall_ia_var:
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
  %tmp0 = load i64, ptr %arg1
  %tmp1 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 %tmp0) ]
  ret i32 %tmp1
}

define i32 @test_tailcall_ib_var(ptr %arg0, ptr %arg1) #0 {
; ELF-LABEL: test_tailcall_ib_var:
; ELF:    ldr x1, [x1]
; ELF:    brab x0, x1

; HINT-LABEL: test_tailcall_ib_var:
; HINT-NEXT:    ldr x16, [x1]
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
  %tmp0 = load i64, ptr %arg1
  %tmp1 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 %tmp0) ]
  ret i32 %tmp1
}

define void @test_tailcall_omit_mov_x16_x16(ptr %objptr) #0 {
; ELF-LABEL: test_tailcall_omit_mov_x16_x16:
; ELF-NEXT:    ldr     x17, [x0]
; ELF-NEXT:    mov     x16, x0
; ELF-NEXT:    movk    x16, #6503, lsl #48
; ELF-NEXT:    autda   x17, x16
; ELF-NEXT:    ldr     x1, [x17]
; ELF-NEXT:    movk    x17, #54167, lsl #48
; ELF-NEXT:    braa    x1, x17

; HINT-LABEL: test_tailcall_omit_mov_x16_x16:
; HINT-NEXT:    ldr     x17, [x0]
; HINT-NEXT:    mov     x16, x0
; HINT-NEXT:    movk    x16, #6503, lsl #48
; HINT-NEXT:    autia1716
; HINT-NEXT:    mov	x16, x17
; HINT-NEXT:    ldr     x17, [x17]
; HINT-NEXT:    movk    x16, #54167, lsl #48
; HINT-NEXT:    autia1716
; HINT-NEXT:    br      x17

  %vtable.signed = load ptr, ptr %objptr, align 8
  %objptr.int = ptrtoint ptr %objptr to i64
  %vtable.discr = tail call i64 @llvm.ptrauth.blend(i64 %objptr.int, i64 6503)
  %vtable.signed.int = ptrtoint ptr %vtable.signed to i64
  %vtable.unsigned.int = tail call i64 @llvm.ptrauth.auth(i64 %vtable.signed.int, i32 2, i64 %vtable.discr)
  %vtable.unsigned = inttoptr i64 %vtable.unsigned.int to ptr
  %virt.func.signed = load ptr, ptr %vtable.unsigned, align 8
  %virt.func.discr = tail call i64 @llvm.ptrauth.blend(i64 %vtable.unsigned.int, i64 54167)
  tail call void %virt.func.signed(ptr %objptr) [ "ptrauth"(i32 0, i64 %virt.func.discr) ]
  ret void
}

define i32 @test_call_omit_extra_moves(ptr %objptr) #0 {
; ELF-LABEL: test_call_omit_extra_moves:
; ELF-NEXT:    str     x30, [sp, #-16]!
; ELF-NEXT:    ldr     x17, [x0]
; ELF-NEXT:    mov     x16, x0
; ELF-NEXT:    movk    x16, #6503, lsl #48
; ELF-NEXT:    autda   x17, x16
; ELF-NEXT:    ldr     x8, [x17]
; ELF-NEXT:    movk    x17, #34646, lsl #48
; ELF-NEXT:    blraa   x8, x17
; ELF-NEXT:    mov     w0, #42
; ELF-NEXT:    ldr     x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_omit_extra_moves:
; HINT-NEXT:    str     x30, [sp, #-16]!
; HINT-NEXT:    ldr     x17, [x0]
; HINT-NEXT:    mov     x16, x0
; HINT-NEXT:    movk    x16, #6503, lsl #48
; HINT-NEXT:    autia1716
; HINT-NEXT:    mov	x16, x17
; HINT-NEXT:    ldr     x17, [x17]
; HINT-NEXT:    movk    x16, #34646, lsl #48
; HINT-NEXT:    autia1716
; HINT-NEXT:    blr     x17
; HINT-NEXT:    mov     w0, #42
; HINT-NEXT:    ldr     x30, [sp], #16
; HINT-NEXT:    ret

  %vtable.signed = load ptr, ptr %objptr
  %objptr.int = ptrtoint ptr %objptr to i64
  %vtable.discr = tail call i64 @llvm.ptrauth.blend(i64 %objptr.int, i64 6503)
  %vtable.signed.int = ptrtoint ptr %vtable.signed to i64
  %vtable.int = tail call i64 @llvm.ptrauth.auth(i64 %vtable.signed.int, i32 2, i64 %vtable.discr)
  %vtable = inttoptr i64 %vtable.int to ptr
  %callee.signed = load ptr, ptr %vtable
  %callee.discr = tail call i64 @llvm.ptrauth.blend(i64 %vtable.int, i64 34646)
  %call.result = tail call i32 %callee.signed(ptr %objptr) [ "ptrauth"(i32 0, i64 %callee.discr) ]
  ret i32 42
}

define i32 @test_call_ia_arg(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_call_ia_arg:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    blraa x0, x1
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ia_arg:
; HINT-NEXT:   str x30, [sp, #-16]!
; HINT-NEXT:   mov x16, x1
; HINT-NEXT:   mov x17, x0
; HINT-NEXT:   autia1716
; HINT-NEXT:   blr x17
; HINT-NEXT:   ldr x30, [sp], #16
; HINT-NEXT:   ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp0
}

define i32 @test_call_ib_arg(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_call_ib_arg:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    blrab x0, x1
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ib_arg:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = call i32 %arg0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ia_arg(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_tailcall_ia_arg:
; ELF:    braa x0, x1

; HINT-LABEL: test_tailcall_ia_arg:
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autia1716
; HINT-NEXT:    br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp0
}

define i32 @test_tailcall_ib_arg(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_tailcall_ib_arg:
; ELF:    brab x0, x1

; HINT-LABEL: test_tailcall_ib_arg:
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    mov x17, x0
; HINT-NEXT:    autib1716
; HINT-NEXT:    br x17
  %tmp0 = tail call i32 %arg0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp0
}

define i32 @test_call_ia_arg_ind(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_call_ia_arg_ind:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    ldr x8, [x0]
; ELF-NEXT:    blraa x8, x1
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ia_arg_ind:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    ldr x17, [x0]
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    autia1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = load ptr, ptr %arg0
  %tmp1 = call i32 %tmp0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp1
}

define i32 @test_call_ib_arg_ind(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_call_ib_arg_ind:
; ELF-NEXT:    str x30, [sp, #-16]!
; ELF-NEXT:    ldr x8, [x0]
; ELF-NEXT:    blrab x8, x1
; ELF-NEXT:    ldr x30, [sp], #16
; ELF-NEXT:    ret

; HINT-LABEL: test_call_ib_arg_ind:
; HINT-NEXT:    str x30, [sp, #-16]!
; HINT-NEXT:    ldr x17, [x0]
; HINT-NEXT:    mov x16, x1
; HINT-NEXT:    autib1716
; HINT-NEXT:    blr x17
; HINT-NEXT:    ldr x30, [sp], #16
; HINT-NEXT:    ret
  %tmp0 = load ptr, ptr %arg0
  %tmp1 = call i32 %tmp0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp1
}

define i32 @test_tailcall_ia_arg_ind(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_tailcall_ia_arg_ind:
; ELF:    ldr x0, [x0]
; ELF:    braa x0, x1

; HINT-LABEL: test_tailcall_ia_arg_ind:
; HINT:    ldr x17, [x0]
; HINT:    mov x16, x1
; HINT:    autia1716
; HINT:    br x17
  %tmp0 = load ptr, ptr %arg0
  %tmp1 = tail call i32 %tmp0() [ "ptrauth"(i32 0, i64 %arg1) ]
  ret i32 %tmp1
}

define i32 @test_tailcall_ib_arg_ind(ptr %arg0, i64 %arg1) #0 {
; ELF-LABEL: test_tailcall_ib_arg_ind:
; ELF:    ldr x0, [x0]
; ELF:    brab x0, x1

; HINT-LABEL: test_tailcall_ib_arg_ind:
; HINT:    ldr x17, [x0]
; HINT:    mov x16, x1
; HINT:    autib1716
; HINT:    br x17
  %tmp0 = load ptr, ptr %arg0
  %tmp1 = tail call i32 %tmp0() [ "ptrauth"(i32 1, i64 %arg1) ]
  ret i32 %tmp1
}

; Test direct calls

define i32 @test_direct_call() #0 {
; CHECK-LABEL: test_direct_call:
; CHECK-NEXT:   str x30, [sp, #-16]!
; CHECK-NEXT:   bl f
; CHECK-NEXT:   ldr x30, [sp], #16
; CHECK-NEXT:   ret
  %tmp0 = call i32 ptrauth(ptr @f, i32 0, i64 42)() [ "ptrauth"(i32 0, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_direct_tailcall(ptr %arg0) #0 {
; CHECK-LABEL: test_direct_tailcall:
; CHECK-NEXT:   b f
  %tmp0 = tail call i32 ptrauth(ptr @f, i32 0, i64 42)() [ "ptrauth"(i32 0, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_direct_call_mismatch() #0 {
; ELF-LABEL: test_direct_call_mismatch:
; ELF-NEXT:   str x30, [sp, #-16]!
; ELF-NEXT:   adrp x17, :got:f
; ELF-NEXT:   ldr x17, [x17, :got_lo12:f]
; ELF-NEXT:   mov x16, #42
; ELF-NEXT:   pacia x17, x16
; ELF-NEXT:   mov x8, x17
; ELF-NEXT:   mov x17, #42
; ELF-NEXT:   blrab x8, x17
; ELF-NEXT:   ldr x30, [sp], #16
; ELF-NEXT:   ret

; HINT-LABEL: test_direct_call_mismatch:
; HINT-NEXT:   str x30, [sp, #-16]!
; HINT-NEXT:   adrp x17, :got:f
; HINT-NEXT:   ldr x17, [x17, :got_lo12:f]
; HINT-NEXT:   mov x16, #42
; HINT-NEXT:   pacia1716
; HINT-NEXT:   mov x16, #42
; HINT-NEXT:   autib1716
; HINT-NEXT:   blr x17
; HINT-NEXT:   ldr x30, [sp], #16
; HINT-NEXT:   ret

  %tmp0 = call i32 ptrauth(ptr @f, i32 0, i64 42)() [ "ptrauth"(i32 1, i64 42) ]
  ret i32 %tmp0
}

define i32 @test_direct_call_addr() #0 {
; ELF-LABEL: test_direct_call_addr:
; ELF-NEXT:   str x30, [sp, #-16]!
; ELF-NEXT:   bl f
; ELF-NEXT:   ldr x30, [sp], #16
; ELF-NEXT:   ret
  %tmp0 = call i32 ptrauth(ptr @f, i32 1, i64 0, ptr @f.ref.ib.0.addr)() [ "ptrauth"(i32 1, i64 ptrtoint (ptr @f.ref.ib.0.addr to i64)) ]
  ret i32 %tmp0
}

define i32 @test_direct_call_addr_blend() #0 {
; ELF-LABEL: test_direct_call_addr_blend:
; ELF-NEXT:   str x30, [sp, #-16]!
; ELF-NEXT:   bl f
; ELF-NEXT:   ldr x30, [sp], #16
; ELF-NEXT:   ret
  %tmp0 = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr @f.ref.ib.42.addr to i64), i64 42)
  %tmp1 = call i32 ptrauth(ptr @f, i32 1, i64 42, ptr @f.ref.ib.42.addr)() [ "ptrauth"(i32 1, i64 %tmp0) ]
  ret i32 %tmp1
}

define i32 @test_direct_call_addr_gep_different_index_types() #0 {
; ELF-LABEL: test_direct_call_addr_gep_different_index_types:
; ELF-NEXT:   str x30, [sp, #-16]!
; ELF-NEXT:   bl f
; ELF-NEXT:   ldr x30, [sp], #16
; ELF-NEXT:   ret
  %tmp0 = call i32 ptrauth(ptr @f, i32 1, i64 0, ptr getelementptr ({ ptr }, ptr @f_struct.ref.ib.0.addr, i64 0, i32 0))() [ "ptrauth"(i32 1, i64 ptrtoint (ptr getelementptr ({ ptr }, ptr @f_struct.ref.ib.0.addr, i32 0, i32 0) to i64)) ]
  ret i32 %tmp0
}

define i32 @test_direct_call_addr_blend_gep_different_index_types() #0 {
; ELF-LABEL: test_direct_call_addr_blend_gep_different_index_types:
; ELF-NEXT:   str x30, [sp, #-16]!
; ELF-NEXT:   bl f
; ELF-NEXT:   ldr x30, [sp], #16
; ELF-NEXT:   ret
  %tmp0 = call i64 @llvm.ptrauth.blend(i64 ptrtoint (ptr getelementptr ({ ptr }, ptr @f_struct.ref.ib.123.addr, i32 0, i32 0) to i64), i64 123)
  %tmp1 = call i32 ptrauth(ptr @f, i32 1, i64 123, ptr getelementptr ({ ptr }, ptr @f_struct.ref.ib.123.addr, i64 0, i32 0))() [ "ptrauth"(i32 1, i64 %tmp0) ]
  ret i32 %tmp1
}

@f.ref.ib.42.addr = external global ptr
@f.ref.ib.0.addr = external global ptr
@f_struct.ref.ib.0.addr = external global ptr
@f_struct.ref.ib.123.addr = external global ptr

declare void @f()

declare i64 @llvm.ptrauth.auth(i64, i32, i64)
declare i64 @llvm.ptrauth.blend(i64, i64)

attributes #0 = { nounwind }
