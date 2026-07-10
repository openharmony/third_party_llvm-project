; RUN: llc -mtriple=riscv64-linux-gnu -emulated-tls -relocation-model=pic -O3 < %s \
; RUN:     | FileCheck -check-prefix=RISCV64 %s
; RUN: llc -mtriple=riscv64-linux-ohos -emulated-tls -relocation-model=pic -O3 < %s \
; RUN:     | FileCheck -check-prefix=RISCV64 %s
; RUN: llc -mtriple=riscv64-linux-ohos -emulated-tls -relocation-model=pic -O3 -filetype=obj -o /dev/null < %s

; RUN: llc -mtriple=riscv64-linux-gnu -relocation-model=pic -O3 < %s \
; RUN:     | FileCheck -check-prefix=NoEMU %s
; RUN: llc -mtriple=riscv64-linux-ohos -relocation-model=pic -O3 < %s \
; RUN:     | FileCheck -check-prefix=RISCV64 %s

; NoEMU-NOT: __emutls

; Make sure that TLS symbols are emitted in expected order.

@external_x = external thread_local global i32, align 8
@external_y = thread_local global i8 7, align 2
@internal_y = internal thread_local global i64 9, align 16

define ptr @get_external_x() {
entry:
  ret ptr @external_x
}

define ptr @get_external_y() {
entry:
  ret ptr @external_y
}

define ptr @get_internal_y() {
entry:
  ret ptr @internal_y
}

; RISCV64-LABEL:  get_external_x:
; RISCV64:      __emutls_v.external_x
; RISCV64:      __emutls_get_address
; RISCV64-LABEL:  get_external_y:
; RISCV64:      __emutls_v.external_y
; RISCV64:      __emutls_get_address
; RISCV64-LABEL:  get_internal_y:
; RISCV64:      __emutls_v.internal_y
; RISCV64:      __emutls_get_address
; RISCV64-NOT:   __emutls_t.external_x
; RISCV64-NOT:   __emutls_v.external_x:
; RISCV64:        .data{{$}}
; RISCV64:        .globl __emutls_v.external_y
; RISCV64:        .p2align 3
; RISCV64-LABEL:  __emutls_v.external_y:
; RISCV64-NEXT:   .quad 1
; RISCV64-NEXT:   .quad 2
; RISCV64-NEXT:   .quad 0
; RISCV64-NEXT:   .quad __emutls_t.external_y
; RISCV64-NOT:    __emutls_v.external_x:
; RISCV64:        .section .r{{o?}}data,
; RISCV64-LABEL:  __emutls_t.external_y:
; RISCV64-NEXT:   .byte 7
; RISCV64:        .data{{$}}
; RISCV64-NOT:    .globl __emutls_v
; RISCV64:        .p2align 3
; RISCV64-LABEL:  __emutls_v.internal_y:
; RISCV64-NEXT:   .quad 8
; RISCV64-NEXT:   .quad 16
; RISCV64-NEXT:   .quad 0
; RISCV64-NEXT:   .quad __emutls_t.internal_y
; RISCV64:        .section .r{{o?}}data,
; RISCV64-LABEL:  __emutls_t.internal_y:
; RISCV64-NEXT:   .quad 9
