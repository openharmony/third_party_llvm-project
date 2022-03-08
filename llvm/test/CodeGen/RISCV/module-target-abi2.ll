; RUN: llc -mtriple=riscv32 < %s 2>&1 \
; RUN:   | FileCheck -check-prefix=DEFAULT %s
; RUN: not --crash llc -mtriple=riscv32 -target-abi ilp32 < %s 2>&1 \
; RUN:   | FileCheck -check-prefix=RV32IF-ILP32 %s
; RUN: llc -mtriple=riscv32 -target-abi ilp32f < %s 2>&1 \
; RUN:   | FileCheck -check-prefix=RV32IF-ILP32F %s
; RUN: llc -mtriple=riscv32 -filetype=obj < %s | llvm-readelf -h - | FileCheck -check-prefixes=FLAGS %s

; RV32IF-ILP32: -target-abi option != target-abi module flag

; OHOS_LOCAL backported from 227496dc09cf46df233aad041d6dc6113822e4bb
; FLAGS: Flags: 0x2, single-float ABI

define float @foo(i32 %a) nounwind #0 {
; DEFAULT: # %bb.0:
; DEFAULT-NEXT: fcvt.s.w fa0, a0
; DEFAULT-NEXT: ret
; RV32IF-ILP32F: # %bb.0:
; RV32IF-ILP32F: fcvt.s.w fa0, a0
; RV32IF-ILP32F: ret
  %conv = sitofp i32 %a to float
  ret float %conv
}

attributes #0 = { "target-features"="+f"}
!llvm.module.flags = !{!0}
!0 = !{i32 1, !"target-abi", !"ilp32f"}
