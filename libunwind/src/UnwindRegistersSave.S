//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "assembly.h"

#if defined(_AIX)
    .toc
#else
    .text
#endif

#if !defined(__USING_SJLJ_EXCEPTIONS__)

#if defined(__i386__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#   +                       +
#   +-----------------------+
#   + thread_state pointer  +
#   +-----------------------+
#   + return address        +
#   +-----------------------+   <-- SP
#   +                       +
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)

  _LIBUNWIND_CET_ENDBR
  push  %eax
  movl  8(%esp), %eax
  movl  %ebx,  4(%eax)
  movl  %ecx,  8(%eax)
  movl  %edx, 12(%eax)
  movl  %edi, 16(%eax)
  movl  %esi, 20(%eax)
  movl  %ebp, 24(%eax)
  movl  %esp, %edx
  addl  $8, %edx
  movl  %edx, 28(%eax)  # store what sp was at call site as esp
  # skip ss
  # skip eflags
  movl  4(%esp), %edx
  movl  %edx, 40(%eax)  # store return address as eip
  # skip cs
  # skip ds
  # skip es
  # skip fs
  # skip gs
  movl  (%esp), %edx
  movl  %edx, (%eax)  # store original eax
  popl  %eax
  xorl  %eax, %eax    # return UNW_ESUCCESS
  ret

#elif defined(__x86_64__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in rdi
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
#if defined(_WIN64)
#define PTR %rcx
#define TMP %rdx
#else
#define PTR %rdi
#define TMP %rsi
#endif

  _LIBUNWIND_CET_ENDBR
  movq  %rax,   (PTR)
  movq  %rbx,  8(PTR)
  movq  %rcx, 16(PTR)
  movq  %rdx, 24(PTR)
  movq  %rdi, 32(PTR)
  movq  %rsi, 40(PTR)
  movq  %rbp, 48(PTR)
  movq  %rsp, 56(PTR)
  addq  $8,   56(PTR)
  movq  %r8,  64(PTR)
  movq  %r9,  72(PTR)
  movq  %r10, 80(PTR)
  movq  %r11, 88(PTR)
  movq  %r12, 96(PTR)
  movq  %r13,104(PTR)
  movq  %r14,112(PTR)
  movq  %r15,120(PTR)
  movq  (%rsp),TMP
  movq  TMP,128(PTR) # store return address as rip
  # skip rflags
  # skip cs
  # skip fs
  # skip gs

#if defined(_WIN64)
  movdqu %xmm0,176(PTR)
  movdqu %xmm1,192(PTR)
  movdqu %xmm2,208(PTR)
  movdqu %xmm3,224(PTR)
  movdqu %xmm4,240(PTR)
  movdqu %xmm5,256(PTR)
  movdqu %xmm6,272(PTR)
  movdqu %xmm7,288(PTR)
  movdqu %xmm8,304(PTR)
  movdqu %xmm9,320(PTR)
  movdqu %xmm10,336(PTR)
  movdqu %xmm11,352(PTR)
  movdqu %xmm12,368(PTR)
  movdqu %xmm13,384(PTR)
  movdqu %xmm14,400(PTR)
  movdqu %xmm15,416(PTR)
#endif
  xorl  %eax, %eax    # return UNW_ESUCCESS
  ret

#elif defined(__mips__) && defined(_ABIO32) && _MIPS_SIM == _ABIO32

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in a0 ($4)
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  .set push
  .set noat
  .set noreorder
  .set nomacro
// OHOS_LOCAL move code block
#ifdef __mips_hard_float
#if __mips_fpr == 32 // OHOS_LOCAL
  sdc1  $f0, (4 * 36 + 8 * 0)($4)
  sdc1  $f2, (4 * 36 + 8 * 2)($4)
  sdc1  $f4, (4 * 36 + 8 * 4)($4)
  sdc1  $f6, (4 * 36 + 8 * 6)($4)
  sdc1  $f8, (4 * 36 + 8 * 8)($4)
  sdc1  $f10, (4 * 36 + 8 * 10)($4)
  sdc1  $f12, (4 * 36 + 8 * 12)($4)
  sdc1  $f14, (4 * 36 + 8 * 14)($4)
  sdc1  $f16, (4 * 36 + 8 * 16)($4)
  sdc1  $f18, (4 * 36 + 8 * 18)($4)
  sdc1  $f20, (4 * 36 + 8 * 20)($4)
  sdc1  $f22, (4 * 36 + 8 * 22)($4)
  sdc1  $f24, (4 * 36 + 8 * 24)($4)
  sdc1  $f26, (4 * 36 + 8 * 26)($4)
  sdc1  $f28, (4 * 36 + 8 * 28)($4)
  sdc1  $f30, (4 * 36 + 8 * 30)($4)
#elif __mips_fpr == 64 // OHOS_LOCAL
  sdc1  $f0, (4 * 36 + 8 * 0)($4)
  sdc1  $f1, (4 * 36 + 8 * 1)($4)
  sdc1  $f2, (4 * 36 + 8 * 2)($4)
  sdc1  $f3, (4 * 36 + 8 * 3)($4)
  sdc1  $f4, (4 * 36 + 8 * 4)($4)
  sdc1  $f5, (4 * 36 + 8 * 5)($4)
  sdc1  $f6, (4 * 36 + 8 * 6)($4)
  sdc1  $f7, (4 * 36 + 8 * 7)($4)
  sdc1  $f8, (4 * 36 + 8 * 8)($4)
  sdc1  $f9, (4 * 36 + 8 * 9)($4)
  sdc1  $f10, (4 * 36 + 8 * 10)($4)
  sdc1  $f11, (4 * 36 + 8 * 11)($4)
  sdc1  $f12, (4 * 36 + 8 * 12)($4)
  sdc1  $f13, (4 * 36 + 8 * 13)($4)
  sdc1  $f14, (4 * 36 + 8 * 14)($4)
  sdc1  $f15, (4 * 36 + 8 * 15)($4)
  sdc1  $f16, (4 * 36 + 8 * 16)($4)
  sdc1  $f17, (4 * 36 + 8 * 17)($4)
  sdc1  $f18, (4 * 36 + 8 * 18)($4)
  sdc1  $f19, (4 * 36 + 8 * 19)($4)
  sdc1  $f20, (4 * 36 + 8 * 20)($4)
  sdc1  $f21, (4 * 36 + 8 * 21)($4)
  sdc1  $f22, (4 * 36 + 8 * 22)($4)
  sdc1  $f23, (4 * 36 + 8 * 23)($4)
  sdc1  $f24, (4 * 36 + 8 * 24)($4)
  sdc1  $f25, (4 * 36 + 8 * 25)($4)
  sdc1  $f26, (4 * 36 + 8 * 26)($4)
  sdc1  $f27, (4 * 36 + 8 * 27)($4)
  sdc1  $f28, (4 * 36 + 8 * 28)($4)
  sdc1  $f29, (4 * 36 + 8 * 29)($4)
  sdc1  $f30, (4 * 36 + 8 * 30)($4)
  sdc1  $f31, (4 * 36 + 8 * 31)($4)
// OHOS_LOCAL begin
#elif __mips_fpr == 0
  // Check the register size in runtime
  // Load the FIR control register value
  cfc1  $t0, $0
  // Test F64 bit of the FIR control register
  li    $t1, 1
  sll   $t1, $t1, 22
  and   $t0, $t0, $t1
  beq   $t0, $0, _fp32
  nop
_fp64:
  .set  fp=64
  sdc1  $f0, (4 * 36 + 8 * 0)($4)
  sdc1  $f1, (4 * 36 + 8 * 1)($4)
  sdc1  $f2, (4 * 36 + 8 * 2)($4)
  sdc1  $f3, (4 * 36 + 8 * 3)($4)
  sdc1  $f4, (4 * 36 + 8 * 4)($4)
  sdc1  $f5, (4 * 36 + 8 * 5)($4)
  sdc1  $f6, (4 * 36 + 8 * 6)($4)
  sdc1  $f7, (4 * 36 + 8 * 7)($4)
  sdc1  $f8, (4 * 36 + 8 * 8)($4)
  sdc1  $f9, (4 * 36 + 8 * 9)($4)
  sdc1  $f10, (4 * 36 + 8 * 10)($4)
  sdc1  $f11, (4 * 36 + 8 * 11)($4)
  sdc1  $f12, (4 * 36 + 8 * 12)($4)
  sdc1  $f13, (4 * 36 + 8 * 13)($4)
  sdc1  $f14, (4 * 36 + 8 * 14)($4)
  sdc1  $f15, (4 * 36 + 8 * 15)($4)
  sdc1  $f16, (4 * 36 + 8 * 16)($4)
  sdc1  $f17, (4 * 36 + 8 * 17)($4)
  sdc1  $f18, (4 * 36 + 8 * 18)($4)
  sdc1  $f19, (4 * 36 + 8 * 19)($4)
  sdc1  $f20, (4 * 36 + 8 * 20)($4)
  sdc1  $f21, (4 * 36 + 8 * 21)($4)
  sdc1  $f22, (4 * 36 + 8 * 22)($4)
  sdc1  $f23, (4 * 36 + 8 * 23)($4)
  sdc1  $f24, (4 * 36 + 8 * 24)($4)
  sdc1  $f25, (4 * 36 + 8 * 25)($4)
  sdc1  $f26, (4 * 36 + 8 * 26)($4)
  sdc1  $f27, (4 * 36 + 8 * 27)($4)
  sdc1  $f28, (4 * 36 + 8 * 28)($4)
  sdc1  $f29, (4 * 36 + 8 * 29)($4)
  sdc1  $f30, (4 * 36 + 8 * 30)($4)
  sdc1  $f31, (4 * 36 + 8 * 31)($4)
  .set  fp=xx
  j     _after_float
  nop
_fp32:
  .set  fp=32
  sdc1  $f0, (4 * 36 + 8 * 0)($4)
  sdc1  $f2, (4 * 36 + 8 * 2)($4)
  sdc1  $f4, (4 * 36 + 8 * 4)($4)
  sdc1  $f6, (4 * 36 + 8 * 6)($4)
  sdc1  $f8, (4 * 36 + 8 * 8)($4)
  sdc1  $f10, (4 * 36 + 8 * 10)($4)
  sdc1  $f12, (4 * 36 + 8 * 12)($4)
  sdc1  $f14, (4 * 36 + 8 * 14)($4)
  sdc1  $f16, (4 * 36 + 8 * 16)($4)
  sdc1  $f18, (4 * 36 + 8 * 18)($4)
  sdc1  $f20, (4 * 36 + 8 * 20)($4)
  sdc1  $f22, (4 * 36 + 8 * 22)($4)
  sdc1  $f24, (4 * 36 + 8 * 24)($4)
  sdc1  $f26, (4 * 36 + 8 * 26)($4)
  sdc1  $f28, (4 * 36 + 8 * 28)($4)
  sdc1  $f30, (4 * 36 + 8 * 30)($4)
  .set  fp=xx
#else
#error "Unknown __mips_fpr value"
// OHOS_LOCAL end
#endif
#endif
_after_float: // OHOS_LOCAL
// OHOS_LOCAL begin moved code block
  sw    $1, (4 * 1)($4)
  sw    $2, (4 * 2)($4)
  sw    $3, (4 * 3)($4)
  sw    $4, (4 * 4)($4)
  sw    $5, (4 * 5)($4)
  sw    $6, (4 * 6)($4)
  sw    $7, (4 * 7)($4)
  sw    $8, (4 * 8)($4)
  sw    $9, (4 * 9)($4)
  sw    $10, (4 * 10)($4)
  sw    $11, (4 * 11)($4)
  sw    $12, (4 * 12)($4)
  sw    $13, (4 * 13)($4)
  sw    $14, (4 * 14)($4)
  sw    $15, (4 * 15)($4)
  sw    $16, (4 * 16)($4)
  sw    $17, (4 * 17)($4)
  sw    $18, (4 * 18)($4)
  sw    $19, (4 * 19)($4)
  sw    $20, (4 * 20)($4)
  sw    $21, (4 * 21)($4)
  sw    $22, (4 * 22)($4)
  sw    $23, (4 * 23)($4)
  sw    $24, (4 * 24)($4)
  sw    $25, (4 * 25)($4)
  sw    $26, (4 * 26)($4)
  sw    $27, (4 * 27)($4)
  sw    $28, (4 * 28)($4)
  sw    $29, (4 * 29)($4)
  sw    $30, (4 * 30)($4)
  sw    $31, (4 * 31)($4)
  # Store return address to pc
  sw    $31, (4 * 32)($4)
  # hi and lo
  mfhi  $8
  sw    $8,  (4 * 33)($4)
  mflo  $8
  sw    $8,  (4 * 34)($4)
// OHOS_LOCAL end moved code block
  jr	$31
  # return UNW_ESUCCESS
  or    $2, $0, $0
  .set pop

#elif defined(__mips64)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in a0 ($4)
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  .set push
  .set noat
  .set noreorder
  .set nomacro
  sd    $1, (8 * 1)($4)
  sd    $2, (8 * 2)($4)
  sd    $3, (8 * 3)($4)
  sd    $4, (8 * 4)($4)
  sd    $5, (8 * 5)($4)
  sd    $6, (8 * 6)($4)
  sd    $7, (8 * 7)($4)
  sd    $8, (8 * 8)($4)
  sd    $9, (8 * 9)($4)
  sd    $10, (8 * 10)($4)
  sd    $11, (8 * 11)($4)
  sd    $12, (8 * 12)($4)
  sd    $13, (8 * 13)($4)
  sd    $14, (8 * 14)($4)
  sd    $15, (8 * 15)($4)
  sd    $16, (8 * 16)($4)
  sd    $17, (8 * 17)($4)
  sd    $18, (8 * 18)($4)
  sd    $19, (8 * 19)($4)
  sd    $20, (8 * 20)($4)
  sd    $21, (8 * 21)($4)
  sd    $22, (8 * 22)($4)
  sd    $23, (8 * 23)($4)
  sd    $24, (8 * 24)($4)
  sd    $25, (8 * 25)($4)
  sd    $26, (8 * 26)($4)
  sd    $27, (8 * 27)($4)
  sd    $28, (8 * 28)($4)
  sd    $29, (8 * 29)($4)
  sd    $30, (8 * 30)($4)
  sd    $31, (8 * 31)($4)
  # Store return address to pc
  sd    $31, (8 * 32)($4)
  # hi and lo
  mfhi  $8
  sd    $8,  (8 * 33)($4)
  mflo  $8
  sd    $8,  (8 * 34)($4)
#ifdef __mips_hard_float
  sdc1  $f0, (8 * 35)($4)
  sdc1  $f1, (8 * 36)($4)
  sdc1  $f2, (8 * 37)($4)
  sdc1  $f3, (8 * 38)($4)
  sdc1  $f4, (8 * 39)($4)
  sdc1  $f5, (8 * 40)($4)
  sdc1  $f6, (8 * 41)($4)
  sdc1  $f7, (8 * 42)($4)
  sdc1  $f8, (8 * 43)($4)
  sdc1  $f9, (8 * 44)($4)
  sdc1  $f10, (8 * 45)($4)
  sdc1  $f11, (8 * 46)($4)
  sdc1  $f12, (8 * 47)($4)
  sdc1  $f13, (8 * 48)($4)
  sdc1  $f14, (8 * 49)($4)
  sdc1  $f15, (8 * 50)($4)
  sdc1  $f16, (8 * 51)($4)
  sdc1  $f17, (8 * 52)($4)
  sdc1  $f18, (8 * 53)($4)
  sdc1  $f19, (8 * 54)($4)
  sdc1  $f20, (8 * 55)($4)
  sdc1  $f21, (8 * 56)($4)
  sdc1  $f22, (8 * 57)($4)
  sdc1  $f23, (8 * 58)($4)
  sdc1  $f24, (8 * 59)($4)
  sdc1  $f25, (8 * 60)($4)
  sdc1  $f26, (8 * 61)($4)
  sdc1  $f27, (8 * 62)($4)
  sdc1  $f28, (8 * 63)($4)
  sdc1  $f29, (8 * 64)($4)
  sdc1  $f30, (8 * 65)($4)
  sdc1  $f31, (8 * 66)($4)
#endif
  jr	$31
  # return UNW_ESUCCESS
  or    $2, $0, $0
  .set pop

# elif defined(__mips__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# Just trap for the time being.
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  teq $0, $0

#elif defined(__powerpc64__)

//
// extern int __unw_getcontext(unw_context_t* thread_state)
//
// On entry:
//  thread_state pointer is in r3
//
#if defined(_AIX)
DEFINE_LIBUNWIND_FUNCTION_AND_WEAK_ALIAS(__unw_getcontext, unw_getcontext)
#else
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
#endif
// store register (GPR)
#define PPC64_STR(n) \
  std   n, (8 * (n + 2))(3)

  // save GPRs
  PPC64_STR(0)
  mflr  0
  std   0, PPC64_OFFS_SRR0(3) // store lr as ssr0
  PPC64_STR(1)
  PPC64_STR(2)
  PPC64_STR(3)
  PPC64_STR(4)
  PPC64_STR(5)
  PPC64_STR(6)
  PPC64_STR(7)
  PPC64_STR(8)
  PPC64_STR(9)
  PPC64_STR(10)
  PPC64_STR(11)
  PPC64_STR(12)
  PPC64_STR(13)
  PPC64_STR(14)
  PPC64_STR(15)
  PPC64_STR(16)
  PPC64_STR(17)
  PPC64_STR(18)
  PPC64_STR(19)
  PPC64_STR(20)
  PPC64_STR(21)
  PPC64_STR(22)
  PPC64_STR(23)
  PPC64_STR(24)
  PPC64_STR(25)
  PPC64_STR(26)
  PPC64_STR(27)
  PPC64_STR(28)
  PPC64_STR(29)
  PPC64_STR(30)
  PPC64_STR(31)

  mfcr  0
  std   0,  PPC64_OFFS_CR(3)
  mfxer 0
  std   0,  PPC64_OFFS_XER(3)
  mflr  0
  std   0,  PPC64_OFFS_LR(3)
  mfctr 0
  std   0,  PPC64_OFFS_CTR(3)
  mfvrsave    0
  std   0,  PPC64_OFFS_VRSAVE(3)

#if defined(__VSX__)
  // save VS registers
  // (note that this also saves floating point registers and V registers,
  // because part of VS is mapped to these registers)

  addi  4, 3, PPC64_OFFS_FP

// store VS register
#define PPC64_STVS(n)      \
  stxvd2x n, 0, 4         ;\
  addi    4, 4, 16

  PPC64_STVS(0)
  PPC64_STVS(1)
  PPC64_STVS(2)
  PPC64_STVS(3)
  PPC64_STVS(4)
  PPC64_STVS(5)
  PPC64_STVS(6)
  PPC64_STVS(7)
  PPC64_STVS(8)
  PPC64_STVS(9)
  PPC64_STVS(10)
  PPC64_STVS(11)
  PPC64_STVS(12)
  PPC64_STVS(13)
  PPC64_STVS(14)
  PPC64_STVS(15)
  PPC64_STVS(16)
  PPC64_STVS(17)
  PPC64_STVS(18)
  PPC64_STVS(19)
  PPC64_STVS(20)
  PPC64_STVS(21)
  PPC64_STVS(22)
  PPC64_STVS(23)
  PPC64_STVS(24)
  PPC64_STVS(25)
  PPC64_STVS(26)
  PPC64_STVS(27)
  PPC64_STVS(28)
  PPC64_STVS(29)
  PPC64_STVS(30)
  PPC64_STVS(31)
  PPC64_STVS(32)
  PPC64_STVS(33)
  PPC64_STVS(34)
  PPC64_STVS(35)
  PPC64_STVS(36)
  PPC64_STVS(37)
  PPC64_STVS(38)
  PPC64_STVS(39)
  PPC64_STVS(40)
  PPC64_STVS(41)
  PPC64_STVS(42)
  PPC64_STVS(43)
  PPC64_STVS(44)
  PPC64_STVS(45)
  PPC64_STVS(46)
  PPC64_STVS(47)
  PPC64_STVS(48)
  PPC64_STVS(49)
  PPC64_STVS(50)
  PPC64_STVS(51)
  PPC64_STVS(52)
  PPC64_STVS(53)
  PPC64_STVS(54)
  PPC64_STVS(55)
  PPC64_STVS(56)
  PPC64_STVS(57)
  PPC64_STVS(58)
  PPC64_STVS(59)
  PPC64_STVS(60)
  PPC64_STVS(61)
  PPC64_STVS(62)
  PPC64_STVS(63)

#else

// store FP register
#define PPC64_STF(n) \
  stfd  n, (PPC64_OFFS_FP + n * 16)(3)

  // save float registers
  PPC64_STF(0)
  PPC64_STF(1)
  PPC64_STF(2)
  PPC64_STF(3)
  PPC64_STF(4)
  PPC64_STF(5)
  PPC64_STF(6)
  PPC64_STF(7)
  PPC64_STF(8)
  PPC64_STF(9)
  PPC64_STF(10)
  PPC64_STF(11)
  PPC64_STF(12)
  PPC64_STF(13)
  PPC64_STF(14)
  PPC64_STF(15)
  PPC64_STF(16)
  PPC64_STF(17)
  PPC64_STF(18)
  PPC64_STF(19)
  PPC64_STF(20)
  PPC64_STF(21)
  PPC64_STF(22)
  PPC64_STF(23)
  PPC64_STF(24)
  PPC64_STF(25)
  PPC64_STF(26)
  PPC64_STF(27)
  PPC64_STF(28)
  PPC64_STF(29)
  PPC64_STF(30)
  PPC64_STF(31)

#if defined(__ALTIVEC__)
  // save vector registers

  // Use 16-bytes below the stack pointer as an
  // aligned buffer to save each vector register.
  // Note that the stack pointer is always 16-byte aligned.
  subi  4, 1, 16

#define PPC64_STV_UNALIGNED(n)             \
  stvx  n, 0, 4                           ;\
  ld    5, 0(4)                           ;\
  std   5, (PPC64_OFFS_V + n * 16)(3)     ;\
  ld    5, 8(4)                           ;\
  std   5, (PPC64_OFFS_V + n * 16 + 8)(3)

  PPC64_STV_UNALIGNED(0)
  PPC64_STV_UNALIGNED(1)
  PPC64_STV_UNALIGNED(2)
  PPC64_STV_UNALIGNED(3)
  PPC64_STV_UNALIGNED(4)
  PPC64_STV_UNALIGNED(5)
  PPC64_STV_UNALIGNED(6)
  PPC64_STV_UNALIGNED(7)
  PPC64_STV_UNALIGNED(8)
  PPC64_STV_UNALIGNED(9)
  PPC64_STV_UNALIGNED(10)
  PPC64_STV_UNALIGNED(11)
  PPC64_STV_UNALIGNED(12)
  PPC64_STV_UNALIGNED(13)
  PPC64_STV_UNALIGNED(14)
  PPC64_STV_UNALIGNED(15)
  PPC64_STV_UNALIGNED(16)
  PPC64_STV_UNALIGNED(17)
  PPC64_STV_UNALIGNED(18)
  PPC64_STV_UNALIGNED(19)
  PPC64_STV_UNALIGNED(20)
  PPC64_STV_UNALIGNED(21)
  PPC64_STV_UNALIGNED(22)
  PPC64_STV_UNALIGNED(23)
  PPC64_STV_UNALIGNED(24)
  PPC64_STV_UNALIGNED(25)
  PPC64_STV_UNALIGNED(26)
  PPC64_STV_UNALIGNED(27)
  PPC64_STV_UNALIGNED(28)
  PPC64_STV_UNALIGNED(29)
  PPC64_STV_UNALIGNED(30)
  PPC64_STV_UNALIGNED(31)

#endif
#endif

  li    3,  0   // return UNW_ESUCCESS
  blr


#elif defined(__powerpc__)

//
// extern int unw_getcontext(unw_context_t* thread_state)
//
// On entry:
//  thread_state pointer is in r3
//
#if defined(_AIX)
DEFINE_LIBUNWIND_FUNCTION_AND_WEAK_ALIAS(__unw_getcontext, unw_getcontext)
#else
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
#endif
  stw     0,   8(3)
  mflr    0
  stw     0,   0(3) // store lr as ssr0
  stw     1,  12(3)
  stw     2,  16(3)
  stw     3,  20(3)
  stw     4,  24(3)
  stw     5,  28(3)
  stw     6,  32(3)
  stw     7,  36(3)
  stw     8,  40(3)
  stw     9,  44(3)
  stw     10, 48(3)
  stw     11, 52(3)
  stw     12, 56(3)
  stw     13, 60(3)
  stw     14, 64(3)
  stw     15, 68(3)
  stw     16, 72(3)
  stw     17, 76(3)
  stw     18, 80(3)
  stw     19, 84(3)
  stw     20, 88(3)
  stw     21, 92(3)
  stw     22, 96(3)
  stw     23,100(3)
  stw     24,104(3)
  stw     25,108(3)
  stw     26,112(3)
  stw     27,116(3)
  stw     28,120(3)
  stw     29,124(3)
  stw     30,128(3)
  stw     31,132(3)

#if defined(__ALTIVEC__)
  // save VRSave register
  mfspr   0, 256
  stw     0, 156(3)
#endif
  // save CR registers
  mfcr    0
  stw     0, 136(3)
  // save CTR register
  mfctr   0
  stw     0, 148(3)

#if !defined(__NO_FPRS__)
  // save float registers
  stfd    0, 160(3)
  stfd    1, 168(3)
  stfd    2, 176(3)
  stfd    3, 184(3)
  stfd    4, 192(3)
  stfd    5, 200(3)
  stfd    6, 208(3)
  stfd    7, 216(3)
  stfd    8, 224(3)
  stfd    9, 232(3)
  stfd    10,240(3)
  stfd    11,248(3)
  stfd    12,256(3)
  stfd    13,264(3)
  stfd    14,272(3)
  stfd    15,280(3)
  stfd    16,288(3)
  stfd    17,296(3)
  stfd    18,304(3)
  stfd    19,312(3)
  stfd    20,320(3)
  stfd    21,328(3)
  stfd    22,336(3)
  stfd    23,344(3)
  stfd    24,352(3)
  stfd    25,360(3)
  stfd    26,368(3)
  stfd    27,376(3)
  stfd    28,384(3)
  stfd    29,392(3)
  stfd    30,400(3)
  stfd    31,408(3)
#endif

#if defined(__ALTIVEC__)
  // save vector registers

  subi    4, 1, 16
  rlwinm  4, 4, 0, 0, 27  // mask low 4-bits
  // r4 is now a 16-byte aligned pointer into the red zone

#define SAVE_VECTOR_UNALIGNED(_vec, _offset) \
  stvx    _vec, 0, 4               SEPARATOR \
  lwz     5, 0(4)                  SEPARATOR \
  stw     5, _offset(3)            SEPARATOR \
  lwz     5, 4(4)                  SEPARATOR \
  stw     5, _offset+4(3)          SEPARATOR \
  lwz     5, 8(4)                  SEPARATOR \
  stw     5, _offset+8(3)          SEPARATOR \
  lwz     5, 12(4)                 SEPARATOR \
  stw     5, _offset+12(3)

  SAVE_VECTOR_UNALIGNED( 0, 424+0x000)
  SAVE_VECTOR_UNALIGNED( 1, 424+0x010)
  SAVE_VECTOR_UNALIGNED( 2, 424+0x020)
  SAVE_VECTOR_UNALIGNED( 3, 424+0x030)
  SAVE_VECTOR_UNALIGNED( 4, 424+0x040)
  SAVE_VECTOR_UNALIGNED( 5, 424+0x050)
  SAVE_VECTOR_UNALIGNED( 6, 424+0x060)
  SAVE_VECTOR_UNALIGNED( 7, 424+0x070)
  SAVE_VECTOR_UNALIGNED( 8, 424+0x080)
  SAVE_VECTOR_UNALIGNED( 9, 424+0x090)
  SAVE_VECTOR_UNALIGNED(10, 424+0x0A0)
  SAVE_VECTOR_UNALIGNED(11, 424+0x0B0)
  SAVE_VECTOR_UNALIGNED(12, 424+0x0C0)
  SAVE_VECTOR_UNALIGNED(13, 424+0x0D0)
  SAVE_VECTOR_UNALIGNED(14, 424+0x0E0)
  SAVE_VECTOR_UNALIGNED(15, 424+0x0F0)
  SAVE_VECTOR_UNALIGNED(16, 424+0x100)
  SAVE_VECTOR_UNALIGNED(17, 424+0x110)
  SAVE_VECTOR_UNALIGNED(18, 424+0x120)
  SAVE_VECTOR_UNALIGNED(19, 424+0x130)
  SAVE_VECTOR_UNALIGNED(20, 424+0x140)
  SAVE_VECTOR_UNALIGNED(21, 424+0x150)
  SAVE_VECTOR_UNALIGNED(22, 424+0x160)
  SAVE_VECTOR_UNALIGNED(23, 424+0x170)
  SAVE_VECTOR_UNALIGNED(24, 424+0x180)
  SAVE_VECTOR_UNALIGNED(25, 424+0x190)
  SAVE_VECTOR_UNALIGNED(26, 424+0x1A0)
  SAVE_VECTOR_UNALIGNED(27, 424+0x1B0)
  SAVE_VECTOR_UNALIGNED(28, 424+0x1C0)
  SAVE_VECTOR_UNALIGNED(29, 424+0x1D0)
  SAVE_VECTOR_UNALIGNED(30, 424+0x1E0)
  SAVE_VECTOR_UNALIGNED(31, 424+0x1F0)
#endif

  li      3, 0  // return UNW_ESUCCESS
  blr


#elif defined(__aarch64__)

//
// extern int __unw_getcontext(unw_context_t* thread_state)
//
// On entry:
//  thread_state pointer is in x0
//
  .p2align 2
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  stp    x0, x1,  [x0, #0x000]
  stp    x2, x3,  [x0, #0x010]
  stp    x4, x5,  [x0, #0x020]
  stp    x6, x7,  [x0, #0x030]
  stp    x8, x9,  [x0, #0x040]
  stp    x10,x11, [x0, #0x050]
  stp    x12,x13, [x0, #0x060]
  stp    x14,x15, [x0, #0x070]
  stp    x16,x17, [x0, #0x080]
  stp    x18,x19, [x0, #0x090]
  stp    x20,x21, [x0, #0x0A0]
  stp    x22,x23, [x0, #0x0B0]
  stp    x24,x25, [x0, #0x0C0]
  stp    x26,x27, [x0, #0x0D0]
  stp    x28,x29, [x0, #0x0E0]
  str    x30,     [x0, #0x0F0]
  mov    x1,sp
  str    x1,      [x0, #0x0F8]
  str    x30,     [x0, #0x100]    // store return address as pc
  // skip cpsr
  stp    d0, d1,  [x0, #0x110]
  stp    d2, d3,  [x0, #0x120]
  stp    d4, d5,  [x0, #0x130]
  stp    d6, d7,  [x0, #0x140]
  stp    d8, d9,  [x0, #0x150]
  stp    d10,d11, [x0, #0x160]
  stp    d12,d13, [x0, #0x170]
  stp    d14,d15, [x0, #0x180]
  stp    d16,d17, [x0, #0x190]
  stp    d18,d19, [x0, #0x1A0]
  stp    d20,d21, [x0, #0x1B0]
  stp    d22,d23, [x0, #0x1C0]
  stp    d24,d25, [x0, #0x1D0]
  stp    d26,d27, [x0, #0x1E0]
  stp    d28,d29, [x0, #0x1F0]
  str    d30,     [x0, #0x200]
  str    d31,     [x0, #0x208]
  mov    x0, #0                   // return UNW_ESUCCESS
  ret

#elif defined(__arm__) && !defined(__APPLE__)

#if !defined(__ARM_ARCH_ISA_ARM)
#if (__ARM_ARCH_ISA_THUMB == 2)
  .syntax unified
#endif
  .thumb
#endif

@
@ extern int __unw_getcontext(unw_context_t* thread_state)
@
@ On entry:
@  thread_state pointer is in r0
@ 
@ Per EHABI #4.7 this only saves the core integer registers.
@ EHABI #7.4.5 notes that in general all VRS registers should be restored
@ however this is very hard to do for VFP registers because it is unknown
@ to the library how many registers are implemented by the architecture.
@ Instead, VFP registers are demand saved by logic external to __unw_getcontext.
@
  .p2align 2
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
#if !defined(__ARM_ARCH_ISA_ARM) && __ARM_ARCH_ISA_THUMB == 1
  stm r0!, {r0-r7}
  mov r1, r8
  mov r2, r9
  mov r3, r10
  stm r0!, {r1-r3}
  mov r1, r11
  mov r2, sp
  mov r3, lr
  str r1, [r0, #0]   @ r11
  @ r12 does not need storing, it it the intra-procedure-call scratch register
  str r2, [r0, #8]   @ sp
  str r3, [r0, #12]  @ lr
  str r3, [r0, #16]  @ store return address as pc
  @ T1 does not have a non-cpsr-clobbering register-zeroing instruction.
  @ It is safe to use here though because we are about to return, and cpsr is
  @ not expected to be preserved.
  movs r0, #0        @ return UNW_ESUCCESS
#else
  @ 32bit thumb-2 restrictions for stm:
  @ . the sp (r13) cannot be in the list
  @ . the pc (r15) cannot be in the list in an STM instruction
  stm r0, {r0-r12}
  str sp, [r0, #52]
  str lr, [r0, #56]
  str lr, [r0, #60]  @ store return address as pc
  mov r0, #0         @ return UNW_ESUCCESS
#endif
  JMP(lr)

@
@ static void libunwind::Registers_arm::saveVFPWithFSTMD(unw_fpreg_t* values)
@
@ On entry:
@  values pointer is in r0
@
  .p2align 2
#if defined(__ELF__)
  .fpu vfpv3-d16
#endif
DEFINE_LIBUNWIND_FUNCTION(_ZN9libunwind13Registers_arm16saveVFPWithFSTMDEPv)
  vstmia r0, {d0-d15}
  JMP(lr)

@
@ static void libunwind::Registers_arm::saveVFPWithFSTMX(unw_fpreg_t* values)
@
@ On entry:
@  values pointer is in r0
@
  .p2align 2
#if defined(__ELF__)
  .fpu vfpv3-d16
#endif
DEFINE_LIBUNWIND_FUNCTION(_ZN9libunwind13Registers_arm16saveVFPWithFSTMXEPv)
  vstmia r0, {d0-d15} @ fstmiax is deprecated in ARMv7+ and now behaves like vstmia
  JMP(lr)

@
@ static void libunwind::Registers_arm::saveVFPv3(unw_fpreg_t* values)
@
@ On entry:
@  values pointer is in r0
@
  .p2align 2
#if defined(__ELF__)
  .fpu vfpv3
#endif
DEFINE_LIBUNWIND_FUNCTION(_ZN9libunwind13Registers_arm9saveVFPv3EPv)
  @ VFP and iwMMX instructions are only available when compiling with the flags
  @ that enable them. We do not want to do that in the library (because we do not
  @ want the compiler to generate instructions that access those) but this is
  @ only accessed if the personality routine needs these registers. Use of
  @ these registers implies they are, actually, available on the target, so
  @ it's ok to execute.
  @ So, generate the instructions using the corresponding coprocessor mnemonic.
  vstmia r0, {d16-d31}
  JMP(lr)

#if defined(_LIBUNWIND_ARM_WMMX)

@
@ static void libunwind::Registers_arm::saveiWMMX(unw_fpreg_t* values)
@
@ On entry:
@  values pointer is in r0
@
  .p2align 2
#if defined(__ELF__)
  .arch armv5te
#endif
DEFINE_LIBUNWIND_FUNCTION(_ZN9libunwind13Registers_arm9saveiWMMXEPv)
  stcl p1, cr0, [r0], #8  @ wstrd wR0, [r0], #8
  stcl p1, cr1, [r0], #8  @ wstrd wR1, [r0], #8
  stcl p1, cr2, [r0], #8  @ wstrd wR2, [r0], #8
  stcl p1, cr3, [r0], #8  @ wstrd wR3, [r0], #8
  stcl p1, cr4, [r0], #8  @ wstrd wR4, [r0], #8
  stcl p1, cr5, [r0], #8  @ wstrd wR5, [r0], #8
  stcl p1, cr6, [r0], #8  @ wstrd wR6, [r0], #8
  stcl p1, cr7, [r0], #8  @ wstrd wR7, [r0], #8
  stcl p1, cr8, [r0], #8  @ wstrd wR8, [r0], #8
  stcl p1, cr9, [r0], #8  @ wstrd wR9, [r0], #8
  stcl p1, cr10, [r0], #8  @ wstrd wR10, [r0], #8
  stcl p1, cr11, [r0], #8  @ wstrd wR11, [r0], #8
  stcl p1, cr12, [r0], #8  @ wstrd wR12, [r0], #8
  stcl p1, cr13, [r0], #8  @ wstrd wR13, [r0], #8
  stcl p1, cr14, [r0], #8  @ wstrd wR14, [r0], #8
  stcl p1, cr15, [r0], #8  @ wstrd wR15, [r0], #8
  JMP(lr)

@
@ static void libunwind::Registers_arm::saveiWMMXControl(unw_uint32_t* values)
@
@ On entry:
@  values pointer is in r0
@
  .p2align 2
#if defined(__ELF__)
  .arch armv5te
#endif
DEFINE_LIBUNWIND_FUNCTION(_ZN9libunwind13Registers_arm16saveiWMMXControlEPj)
  stc2 p1, cr8, [r0], #4  @ wstrw wCGR0, [r0], #4
  stc2 p1, cr9, [r0], #4  @ wstrw wCGR1, [r0], #4
  stc2 p1, cr10, [r0], #4  @ wstrw wCGR2, [r0], #4
  stc2 p1, cr11, [r0], #4  @ wstrw wCGR3, [r0], #4
  JMP(lr)

#endif

#elif defined(__or1k__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in r3
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  l.sw       0(r3), r0
  l.sw       4(r3), r1
  l.sw       8(r3), r2
  l.sw      12(r3), r3
  l.sw      16(r3), r4
  l.sw      20(r3), r5
  l.sw      24(r3), r6
  l.sw      28(r3), r7
  l.sw      32(r3), r8
  l.sw      36(r3), r9
  l.sw      40(r3), r10
  l.sw      44(r3), r11
  l.sw      48(r3), r12
  l.sw      52(r3), r13
  l.sw      56(r3), r14
  l.sw      60(r3), r15
  l.sw      64(r3), r16
  l.sw      68(r3), r17
  l.sw      72(r3), r18
  l.sw      76(r3), r19
  l.sw      80(r3), r20
  l.sw      84(r3), r21
  l.sw      88(r3), r22
  l.sw      92(r3), r23
  l.sw      96(r3), r24
  l.sw     100(r3), r25
  l.sw     104(r3), r26
  l.sw     108(r3), r27
  l.sw     112(r3), r28
  l.sw     116(r3), r29
  l.sw     120(r3), r30
  l.sw     124(r3), r31
  # store ra to pc
  l.sw     128(r3), r9
  # zero epcr
  l.sw     132(r3), r0

#elif defined(__hexagon__)
#
# extern int unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in r0
#
#define OFFSET(offset) (offset/4)
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  memw(r0+#32) = r8
  memw(r0+#36) = r9
  memw(r0+#40) = r10
  memw(r0+#44) = r11

  memw(r0+#48) = r12
  memw(r0+#52) = r13
  memw(r0+#56) = r14
  memw(r0+#60) = r15

  memw(r0+#64) = r16
  memw(r0+#68) = r17
  memw(r0+#72) = r18
  memw(r0+#76) = r19

  memw(r0+#80) = r20
  memw(r0+#84) = r21
  memw(r0+#88) = r22
  memw(r0+#92) = r23

  memw(r0+#96) = r24
  memw(r0+#100) = r25
  memw(r0+#104) = r26
  memw(r0+#108) = r27

  memw(r0+#112) = r28
  memw(r0+#116) = r29
  memw(r0+#120) = r30
  memw(r0+#124) = r31
  r1 = c4   // Predicate register
  memw(r0+#128) = r1
  r1 = memw(r30)           // *FP == Saved FP
  r1 = r31
  memw(r0+#132) = r1

  jumpr r31

#elif defined(__sparc__) && defined(__arch64__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in %o0
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  .register %g2, #scratch
  .register %g3, #scratch
  .register %g6, #scratch
  .register %g7, #scratch
  stx  %g1, [%o0 + 0x08]
  stx  %g2, [%o0 + 0x10]
  stx  %g3, [%o0 + 0x18]
  stx  %g4, [%o0 + 0x20]
  stx  %g5, [%o0 + 0x28]
  stx  %g6, [%o0 + 0x30]
  stx  %g7, [%o0 + 0x38]
  stx  %o0, [%o0 + 0x40]
  stx  %o1, [%o0 + 0x48]
  stx  %o2, [%o0 + 0x50]
  stx  %o3, [%o0 + 0x58]
  stx  %o4, [%o0 + 0x60]
  stx  %o5, [%o0 + 0x68]
  stx  %o6, [%o0 + 0x70]
  stx  %o7, [%o0 + 0x78]
  stx  %l0, [%o0 + 0x80]
  stx  %l1, [%o0 + 0x88]
  stx  %l2, [%o0 + 0x90]
  stx  %l3, [%o0 + 0x98]
  stx  %l4, [%o0 + 0xa0]
  stx  %l5, [%o0 + 0xa8]
  stx  %l6, [%o0 + 0xb0]
  stx  %l7, [%o0 + 0xb8]
  stx  %i0, [%o0 + 0xc0]
  stx  %i1, [%o0 + 0xc8]
  stx  %i2, [%o0 + 0xd0]
  stx  %i3, [%o0 + 0xd8]
  stx  %i4, [%o0 + 0xe0]
  stx  %i5, [%o0 + 0xe8]
  stx  %i6, [%o0 + 0xf0]
  stx  %i7, [%o0 + 0xf8]

  # save StackGhost cookie
  mov  %i7, %g4
  save %sp, -176, %sp
  # register window flush necessary even without StackGhost
  flushw
  restore
  ldx  [%sp + 2047 + 0x78], %g5
  xor  %g4, %g5, %g4
  stx  %g4, [%o0 + 0x100]
  retl
  # return UNW_ESUCCESS
   clr %o0

#elif defined(__sparc__)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in o0
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  ta 3
  add %o7, 8, %o7
  std %g0, [%o0 +   0]
  std %g2, [%o0 +   8]
  std %g4, [%o0 +  16]
  std %g6, [%o0 +  24]
  std %o0, [%o0 +  32]
  std %o2, [%o0 +  40]
  std %o4, [%o0 +  48]
  std %o6, [%o0 +  56]
  std %l0, [%o0 +  64]
  std %l2, [%o0 +  72]
  std %l4, [%o0 +  80]
  std %l6, [%o0 +  88]
  std %i0, [%o0 +  96]
  std %i2, [%o0 + 104]
  std %i4, [%o0 + 112]
  std %i6, [%o0 + 120]
  jmp %o7
   clr %o0                   // return UNW_ESUCCESS

#elif defined(__riscv)

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in a0
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  ISTORE    x1, (RISCV_ISIZE * 0)(a0) // store ra as pc
  ISTORE    x1, (RISCV_ISIZE * 1)(a0)
  ISTORE    x2, (RISCV_ISIZE * 2)(a0)
  ISTORE    x3, (RISCV_ISIZE * 3)(a0)
  ISTORE    x4, (RISCV_ISIZE * 4)(a0)
  ISTORE    x5, (RISCV_ISIZE * 5)(a0)
  ISTORE    x6, (RISCV_ISIZE * 6)(a0)
  ISTORE    x7, (RISCV_ISIZE * 7)(a0)
  ISTORE    x8, (RISCV_ISIZE * 8)(a0)
  ISTORE    x9, (RISCV_ISIZE * 9)(a0)
  ISTORE    x10, (RISCV_ISIZE * 10)(a0)
  ISTORE    x11, (RISCV_ISIZE * 11)(a0)
  ISTORE    x12, (RISCV_ISIZE * 12)(a0)
  ISTORE    x13, (RISCV_ISIZE * 13)(a0)
  ISTORE    x14, (RISCV_ISIZE * 14)(a0)
  ISTORE    x15, (RISCV_ISIZE * 15)(a0)
  ISTORE    x16, (RISCV_ISIZE * 16)(a0)
  ISTORE    x17, (RISCV_ISIZE * 17)(a0)
  ISTORE    x18, (RISCV_ISIZE * 18)(a0)
  ISTORE    x19, (RISCV_ISIZE * 19)(a0)
  ISTORE    x20, (RISCV_ISIZE * 20)(a0)
  ISTORE    x21, (RISCV_ISIZE * 21)(a0)
  ISTORE    x22, (RISCV_ISIZE * 22)(a0)
  ISTORE    x23, (RISCV_ISIZE * 23)(a0)
  ISTORE    x24, (RISCV_ISIZE * 24)(a0)
  ISTORE    x25, (RISCV_ISIZE * 25)(a0)
  ISTORE    x26, (RISCV_ISIZE * 26)(a0)
  ISTORE    x27, (RISCV_ISIZE * 27)(a0)
  ISTORE    x28, (RISCV_ISIZE * 28)(a0)
  ISTORE    x29, (RISCV_ISIZE * 29)(a0)
  ISTORE    x30, (RISCV_ISIZE * 30)(a0)
  ISTORE    x31, (RISCV_ISIZE * 31)(a0)

# if defined(__riscv_flen)
  FSTORE    f0, (RISCV_FOFFSET + RISCV_FSIZE * 0)(a0)
  FSTORE    f1, (RISCV_FOFFSET + RISCV_FSIZE * 1)(a0)
  FSTORE    f2, (RISCV_FOFFSET + RISCV_FSIZE * 2)(a0)
  FSTORE    f3, (RISCV_FOFFSET + RISCV_FSIZE * 3)(a0)
  FSTORE    f4, (RISCV_FOFFSET + RISCV_FSIZE * 4)(a0)
  FSTORE    f5, (RISCV_FOFFSET + RISCV_FSIZE * 5)(a0)
  FSTORE    f6, (RISCV_FOFFSET + RISCV_FSIZE * 6)(a0)
  FSTORE    f7, (RISCV_FOFFSET + RISCV_FSIZE * 7)(a0)
  FSTORE    f8, (RISCV_FOFFSET + RISCV_FSIZE * 8)(a0)
  FSTORE    f9, (RISCV_FOFFSET + RISCV_FSIZE * 9)(a0)
  FSTORE    f10, (RISCV_FOFFSET + RISCV_FSIZE * 10)(a0)
  FSTORE    f11, (RISCV_FOFFSET + RISCV_FSIZE * 11)(a0)
  FSTORE    f12, (RISCV_FOFFSET + RISCV_FSIZE * 12)(a0)
  FSTORE    f13, (RISCV_FOFFSET + RISCV_FSIZE * 13)(a0)
  FSTORE    f14, (RISCV_FOFFSET + RISCV_FSIZE * 14)(a0)
  FSTORE    f15, (RISCV_FOFFSET + RISCV_FSIZE * 15)(a0)
  FSTORE    f16, (RISCV_FOFFSET + RISCV_FSIZE * 16)(a0)
  FSTORE    f17, (RISCV_FOFFSET + RISCV_FSIZE * 17)(a0)
  FSTORE    f18, (RISCV_FOFFSET + RISCV_FSIZE * 18)(a0)
  FSTORE    f19, (RISCV_FOFFSET + RISCV_FSIZE * 19)(a0)
  FSTORE    f20, (RISCV_FOFFSET + RISCV_FSIZE * 20)(a0)
  FSTORE    f21, (RISCV_FOFFSET + RISCV_FSIZE * 21)(a0)
  FSTORE    f22, (RISCV_FOFFSET + RISCV_FSIZE * 22)(a0)
  FSTORE    f23, (RISCV_FOFFSET + RISCV_FSIZE * 23)(a0)
  FSTORE    f24, (RISCV_FOFFSET + RISCV_FSIZE * 24)(a0)
  FSTORE    f25, (RISCV_FOFFSET + RISCV_FSIZE * 25)(a0)
  FSTORE    f26, (RISCV_FOFFSET + RISCV_FSIZE * 26)(a0)
  FSTORE    f27, (RISCV_FOFFSET + RISCV_FSIZE * 27)(a0)
  FSTORE    f28, (RISCV_FOFFSET + RISCV_FSIZE * 28)(a0)
  FSTORE    f29, (RISCV_FOFFSET + RISCV_FSIZE * 29)(a0)
  FSTORE    f30, (RISCV_FOFFSET + RISCV_FSIZE * 30)(a0)
  FSTORE    f31, (RISCV_FOFFSET + RISCV_FSIZE * 31)(a0)
# endif

  li     a0, 0  // return UNW_ESUCCESS
  ret           // jump to ra

#elif defined(__s390x__)

//
// extern int __unw_getcontext(unw_context_t* thread_state)
//
// On entry:
//  thread_state pointer is in r2
//
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)

  // Save GPRs
  stmg %r0, %r15, 16(%r2)

  // Save PSWM
  epsw %r0, %r1
  stm %r0, %r1, 0(%r2)

  // Store return address as PSWA
  stg %r14, 8(%r2)

  // Save FPRs
  std %f0, 144(%r2)
  std %f1, 152(%r2)
  std %f2, 160(%r2)
  std %f3, 168(%r2)
  std %f4, 176(%r2)
  std %f5, 184(%r2)
  std %f6, 192(%r2)
  std %f7, 200(%r2)
  std %f8, 208(%r2)
  std %f9, 216(%r2)
  std %f10, 224(%r2)
  std %f11, 232(%r2)
  std %f12, 240(%r2)
  std %f13, 248(%r2)
  std %f14, 256(%r2)
  std %f15, 264(%r2)

  // Return UNW_ESUCCESS
  lghi %r2, 0
  br %r14

#elif defined(__loongarch__) && __loongarch_grlen == 64

#
# extern int __unw_getcontext(unw_context_t* thread_state)
#
# On entry:
#  thread_state pointer is in $a0($r4)
#
DEFINE_LIBUNWIND_FUNCTION(__unw_getcontext)
  st.d    $r1, $a0, (8 * 1)
  st.d    $r2, $a0, (8 * 2)
  st.d    $r3, $a0, (8 * 3)
  st.d    $r4, $a0, (8 * 4)
  st.d    $r5, $a0, (8 * 5)
  st.d    $r6, $a0, (8 * 6)
  st.d    $r7, $a0, (8 * 7)
  st.d    $r8, $a0, (8 * 8)
  st.d    $r9, $a0, (8 * 9)
  st.d    $r10, $a0, (8 * 10)
  st.d    $r11, $a0, (8 * 11)
  st.d    $r12, $a0, (8 * 12)
  st.d    $r13, $a0, (8 * 13)
  st.d    $r14, $a0, (8 * 14)
  st.d    $r15, $a0, (8 * 15)
  st.d    $r16, $a0, (8 * 16)
  st.d    $r17, $a0, (8 * 17)
  st.d    $r18, $a0, (8 * 18)
  st.d    $r19, $a0, (8 * 19)
  st.d    $r20, $a0, (8 * 20)
  st.d    $r21, $a0, (8 * 21)
  st.d    $r22, $a0, (8 * 22)
  st.d    $r23, $a0, (8 * 23)
  st.d    $r24, $a0, (8 * 24)
  st.d    $r25, $a0, (8 * 25)
  st.d    $r26, $a0, (8 * 26)
  st.d    $r27, $a0, (8 * 27)
  st.d    $r28, $a0, (8 * 28)
  st.d    $r29, $a0, (8 * 29)
  st.d    $r30, $a0, (8 * 30)
  st.d    $r31, $a0, (8 * 31)
  st.d    $r1,  $a0, (8 * 32) // store $ra to pc

# if __loongarch_frlen == 64
  fst.d    $f0, $a0, (8 * 33 + 8 * 0)
  fst.d    $f1, $a0, (8 * 33 + 8 * 1)
  fst.d    $f2, $a0, (8 * 33 + 8 * 2)
  fst.d    $f3, $a0, (8 * 33 + 8 * 3)
  fst.d    $f4, $a0, (8 * 33 + 8 * 4)
  fst.d    $f5, $a0, (8 * 33 + 8 * 5)
  fst.d    $f6, $a0, (8 * 33 + 8 * 6)
  fst.d    $f7, $a0, (8 * 33 + 8 * 7)
  fst.d    $f8, $a0, (8 * 33 + 8 * 8)
  fst.d    $f9, $a0, (8 * 33 + 8 * 9)
  fst.d    $f10, $a0, (8 * 33 + 8 * 10)
  fst.d    $f11, $a0, (8 * 33 + 8 * 11)
  fst.d    $f12, $a0, (8 * 33 + 8 * 12)
  fst.d    $f13, $a0, (8 * 33 + 8 * 13)
  fst.d    $f14, $a0, (8 * 33 + 8 * 14)
  fst.d    $f15, $a0, (8 * 33 + 8 * 15)
  fst.d    $f16, $a0, (8 * 33 + 8 * 16)
  fst.d    $f17, $a0, (8 * 33 + 8 * 17)
  fst.d    $f18, $a0, (8 * 33 + 8 * 18)
  fst.d    $f19, $a0, (8 * 33 + 8 * 19)
  fst.d    $f20, $a0, (8 * 33 + 8 * 20)
  fst.d    $f21, $a0, (8 * 33 + 8 * 21)
  fst.d    $f22, $a0, (8 * 33 + 8 * 22)
  fst.d    $f23, $a0, (8 * 33 + 8 * 23)
  fst.d    $f24, $a0, (8 * 33 + 8 * 24)
  fst.d    $f25, $a0, (8 * 33 + 8 * 25)
  fst.d    $f26, $a0, (8 * 33 + 8 * 26)
  fst.d    $f27, $a0, (8 * 33 + 8 * 27)
  fst.d    $f28, $a0, (8 * 33 + 8 * 28)
  fst.d    $f29, $a0, (8 * 33 + 8 * 29)
  fst.d    $f30, $a0, (8 * 33 + 8 * 30)
  fst.d    $f31, $a0, (8 * 33 + 8 * 31)
# endif

  move     $a0, $zero  // UNW_ESUCCESS
  jr       $ra

#endif

  WEAK_ALIAS(__unw_getcontext, unw_getcontext)

#endif /* !defined(__USING_SJLJ_EXCEPTIONS__) */

NO_EXEC_STACK_DIRECTIVE
