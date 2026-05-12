//===-- record_malloc_info_uaf.c - HWASan feature regression test -*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// OHOS feature regression: HWASAN_OPTIONS=record_malloc_info=true prints heap
// malloc ring-buffer rows on tag-mismatch. This file uses a deep alloc_stage_*
// chain so the recorded allocation stack is multi-frame (not malloc-from-main).
//
//===----------------------------------------------------------------------===//

// RUN: %clang_hwasan -O0 -DISREAD=1 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O0 -DISREAD=0 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O1 -DISREAD=1 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O1 -DISREAD=0 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O2 -DISREAD=1 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O2 -DISREAD=0 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O3 -DISREAD=1 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O3 -DISREAD=0 %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O0 -DISREAD=1 -fno-inline %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// RUN: %clang_hwasan -O2 -DISREAD=0 -fno-inline %s -o %t && not %env_hwasan_opts="record_malloc_info=true" %run %t 2>&1 | FileCheck %s
// REQUIRES: stable-runtime

#include <stdio.h>
#include <stdlib.h>
#include <sanitizer/hwasan_interface.h>

// alloc_stage_0 .. alloc_stage_211: each stage is noinline; only alloc_stage_211 calls malloc.
// Pad comments (line budget ~900 total including all stages).
// [note-a] If FileCheck fails on frame #1, verify the leaf index still matches 211.
// [note-b] The @LINE on the CHECK line must point at the return statement that calls malloc in the leaf stage.
// [note-c] READ/WRITE runs share the same CHECK expectations after the stderr banner.
static void *__attribute__((noinline)) alloc_stage_211(void) {
  return (void *)malloc(10);
}

static void *__attribute__((noinline)) alloc_stage_210(void) {
  return alloc_stage_211();
}

static void *__attribute__((noinline)) alloc_stage_209(void) {
  return alloc_stage_210();
}

static void *__attribute__((noinline)) alloc_stage_208(void) {
  return alloc_stage_209();
}

static void *__attribute__((noinline)) alloc_stage_207(void) {
  return alloc_stage_208();
}

static void *__attribute__((noinline)) alloc_stage_206(void) {
  return alloc_stage_207();
}

static void *__attribute__((noinline)) alloc_stage_205(void) {
  return alloc_stage_206();
}

static void *__attribute__((noinline)) alloc_stage_204(void) {
  return alloc_stage_205();
}

static void *__attribute__((noinline)) alloc_stage_203(void) {
  return alloc_stage_204();
}

static void *__attribute__((noinline)) alloc_stage_202(void) {
  return alloc_stage_203();
}

static void *__attribute__((noinline)) alloc_stage_201(void) {
  return alloc_stage_202();
}

static void *__attribute__((noinline)) alloc_stage_200(void) {
  return alloc_stage_201();
}

static void *__attribute__((noinline)) alloc_stage_199(void) {
  return alloc_stage_200();
}

static void *__attribute__((noinline)) alloc_stage_198(void) {
  return alloc_stage_199();
}

static void *__attribute__((noinline)) alloc_stage_197(void) {
  return alloc_stage_198();
}

static void *__attribute__((noinline)) alloc_stage_196(void) {
  return alloc_stage_197();
}

static void *__attribute__((noinline)) alloc_stage_195(void) {
  return alloc_stage_196();
}

static void *__attribute__((noinline)) alloc_stage_194(void) {
  return alloc_stage_195();
}

static void *__attribute__((noinline)) alloc_stage_193(void) {
  return alloc_stage_194();
}

static void *__attribute__((noinline)) alloc_stage_192(void) {
  return alloc_stage_193();
}

static void *__attribute__((noinline)) alloc_stage_191(void) {
  return alloc_stage_192();
}

static void *__attribute__((noinline)) alloc_stage_190(void) {
  return alloc_stage_191();
}

static void *__attribute__((noinline)) alloc_stage_189(void) {
  return alloc_stage_190();
}

static void *__attribute__((noinline)) alloc_stage_188(void) {
  return alloc_stage_189();
}

static void *__attribute__((noinline)) alloc_stage_187(void) {
  return alloc_stage_188();
}

static void *__attribute__((noinline)) alloc_stage_186(void) {
  return alloc_stage_187();
}

static void *__attribute__((noinline)) alloc_stage_185(void) {
  return alloc_stage_186();
}

static void *__attribute__((noinline)) alloc_stage_184(void) {
  return alloc_stage_185();
}

static void *__attribute__((noinline)) alloc_stage_183(void) {
  return alloc_stage_184();
}

static void *__attribute__((noinline)) alloc_stage_182(void) {
  return alloc_stage_183();
}

static void *__attribute__((noinline)) alloc_stage_181(void) {
  return alloc_stage_182();
}

static void *__attribute__((noinline)) alloc_stage_180(void) {
  return alloc_stage_181();
}

static void *__attribute__((noinline)) alloc_stage_179(void) {
  return alloc_stage_180();
}

static void *__attribute__((noinline)) alloc_stage_178(void) {
  return alloc_stage_179();
}

static void *__attribute__((noinline)) alloc_stage_177(void) {
  return alloc_stage_178();
}

static void *__attribute__((noinline)) alloc_stage_176(void) {
  return alloc_stage_177();
}

static void *__attribute__((noinline)) alloc_stage_175(void) {
  return alloc_stage_176();
}

static void *__attribute__((noinline)) alloc_stage_174(void) {
  return alloc_stage_175();
}

static void *__attribute__((noinline)) alloc_stage_173(void) {
  return alloc_stage_174();
}

static void *__attribute__((noinline)) alloc_stage_172(void) {
  return alloc_stage_173();
}

static void *__attribute__((noinline)) alloc_stage_171(void) {
  return alloc_stage_172();
}

static void *__attribute__((noinline)) alloc_stage_170(void) {
  return alloc_stage_171();
}

static void *__attribute__((noinline)) alloc_stage_169(void) {
  return alloc_stage_170();
}

static void *__attribute__((noinline)) alloc_stage_168(void) {
  return alloc_stage_169();
}

static void *__attribute__((noinline)) alloc_stage_167(void) {
  return alloc_stage_168();
}

static void *__attribute__((noinline)) alloc_stage_166(void) {
  return alloc_stage_167();
}

static void *__attribute__((noinline)) alloc_stage_165(void) {
  return alloc_stage_166();
}

static void *__attribute__((noinline)) alloc_stage_164(void) {
  return alloc_stage_165();
}

static void *__attribute__((noinline)) alloc_stage_163(void) {
  return alloc_stage_164();
}

static void *__attribute__((noinline)) alloc_stage_162(void) {
  return alloc_stage_163();
}

static void *__attribute__((noinline)) alloc_stage_161(void) {
  return alloc_stage_162();
}

static void *__attribute__((noinline)) alloc_stage_160(void) {
  return alloc_stage_161();
}

static void *__attribute__((noinline)) alloc_stage_159(void) {
  return alloc_stage_160();
}

static void *__attribute__((noinline)) alloc_stage_158(void) {
  return alloc_stage_159();
}

static void *__attribute__((noinline)) alloc_stage_157(void) {
  return alloc_stage_158();
}

static void *__attribute__((noinline)) alloc_stage_156(void) {
  return alloc_stage_157();
}

static void *__attribute__((noinline)) alloc_stage_155(void) {
  return alloc_stage_156();
}

static void *__attribute__((noinline)) alloc_stage_154(void) {
  return alloc_stage_155();
}

static void *__attribute__((noinline)) alloc_stage_153(void) {
  return alloc_stage_154();
}

static void *__attribute__((noinline)) alloc_stage_152(void) {
  return alloc_stage_153();
}

static void *__attribute__((noinline)) alloc_stage_151(void) {
  return alloc_stage_152();
}

static void *__attribute__((noinline)) alloc_stage_150(void) {
  return alloc_stage_151();
}

static void *__attribute__((noinline)) alloc_stage_149(void) {
  return alloc_stage_150();
}

static void *__attribute__((noinline)) alloc_stage_148(void) {
  return alloc_stage_149();
}

static void *__attribute__((noinline)) alloc_stage_147(void) {
  return alloc_stage_148();
}

static void *__attribute__((noinline)) alloc_stage_146(void) {
  return alloc_stage_147();
}

static void *__attribute__((noinline)) alloc_stage_145(void) {
  return alloc_stage_146();
}

static void *__attribute__((noinline)) alloc_stage_144(void) {
  return alloc_stage_145();
}

static void *__attribute__((noinline)) alloc_stage_143(void) {
  return alloc_stage_144();
}

static void *__attribute__((noinline)) alloc_stage_142(void) {
  return alloc_stage_143();
}

static void *__attribute__((noinline)) alloc_stage_141(void) {
  return alloc_stage_142();
}

static void *__attribute__((noinline)) alloc_stage_140(void) {
  return alloc_stage_141();
}

static void *__attribute__((noinline)) alloc_stage_139(void) {
  return alloc_stage_140();
}

static void *__attribute__((noinline)) alloc_stage_138(void) {
  return alloc_stage_139();
}

static void *__attribute__((noinline)) alloc_stage_137(void) {
  return alloc_stage_138();
}

static void *__attribute__((noinline)) alloc_stage_136(void) {
  return alloc_stage_137();
}

static void *__attribute__((noinline)) alloc_stage_135(void) {
  return alloc_stage_136();
}

static void *__attribute__((noinline)) alloc_stage_134(void) {
  return alloc_stage_135();
}

static void *__attribute__((noinline)) alloc_stage_133(void) {
  return alloc_stage_134();
}

static void *__attribute__((noinline)) alloc_stage_132(void) {
  return alloc_stage_133();
}

static void *__attribute__((noinline)) alloc_stage_131(void) {
  return alloc_stage_132();
}

static void *__attribute__((noinline)) alloc_stage_130(void) {
  return alloc_stage_131();
}

static void *__attribute__((noinline)) alloc_stage_129(void) {
  return alloc_stage_130();
}

static void *__attribute__((noinline)) alloc_stage_128(void) {
  return alloc_stage_129();
}

static void *__attribute__((noinline)) alloc_stage_127(void) {
  return alloc_stage_128();
}

static void *__attribute__((noinline)) alloc_stage_126(void) {
  return alloc_stage_127();
}

static void *__attribute__((noinline)) alloc_stage_125(void) {
  return alloc_stage_126();
}

static void *__attribute__((noinline)) alloc_stage_124(void) {
  return alloc_stage_125();
}

static void *__attribute__((noinline)) alloc_stage_123(void) {
  return alloc_stage_124();
}

static void *__attribute__((noinline)) alloc_stage_122(void) {
  return alloc_stage_123();
}

static void *__attribute__((noinline)) alloc_stage_121(void) {
  return alloc_stage_122();
}

static void *__attribute__((noinline)) alloc_stage_120(void) {
  return alloc_stage_121();
}

static void *__attribute__((noinline)) alloc_stage_119(void) {
  return alloc_stage_120();
}

static void *__attribute__((noinline)) alloc_stage_118(void) {
  return alloc_stage_119();
}

static void *__attribute__((noinline)) alloc_stage_117(void) {
  return alloc_stage_118();
}

static void *__attribute__((noinline)) alloc_stage_116(void) {
  return alloc_stage_117();
}

static void *__attribute__((noinline)) alloc_stage_115(void) {
  return alloc_stage_116();
}

static void *__attribute__((noinline)) alloc_stage_114(void) {
  return alloc_stage_115();
}

static void *__attribute__((noinline)) alloc_stage_113(void) {
  return alloc_stage_114();
}

static void *__attribute__((noinline)) alloc_stage_112(void) {
  return alloc_stage_113();
}

static void *__attribute__((noinline)) alloc_stage_111(void) {
  return alloc_stage_112();
}

static void *__attribute__((noinline)) alloc_stage_110(void) {
  return alloc_stage_111();
}

static void *__attribute__((noinline)) alloc_stage_109(void) {
  return alloc_stage_110();
}

static void *__attribute__((noinline)) alloc_stage_108(void) {
  return alloc_stage_109();
}

static void *__attribute__((noinline)) alloc_stage_107(void) {
  return alloc_stage_108();
}

static void *__attribute__((noinline)) alloc_stage_106(void) {
  return alloc_stage_107();
}

static void *__attribute__((noinline)) alloc_stage_105(void) {
  return alloc_stage_106();
}

static void *__attribute__((noinline)) alloc_stage_104(void) {
  return alloc_stage_105();
}

static void *__attribute__((noinline)) alloc_stage_103(void) {
  return alloc_stage_104();
}

static void *__attribute__((noinline)) alloc_stage_102(void) {
  return alloc_stage_103();
}

static void *__attribute__((noinline)) alloc_stage_101(void) {
  return alloc_stage_102();
}

static void *__attribute__((noinline)) alloc_stage_100(void) {
  return alloc_stage_101();
}

static void *__attribute__((noinline)) alloc_stage_99(void) {
  return alloc_stage_100();
}

static void *__attribute__((noinline)) alloc_stage_98(void) {
  return alloc_stage_99();
}

static void *__attribute__((noinline)) alloc_stage_97(void) {
  return alloc_stage_98();
}

static void *__attribute__((noinline)) alloc_stage_96(void) {
  return alloc_stage_97();
}

static void *__attribute__((noinline)) alloc_stage_95(void) {
  return alloc_stage_96();
}

static void *__attribute__((noinline)) alloc_stage_94(void) {
  return alloc_stage_95();
}

static void *__attribute__((noinline)) alloc_stage_93(void) {
  return alloc_stage_94();
}

static void *__attribute__((noinline)) alloc_stage_92(void) {
  return alloc_stage_93();
}

static void *__attribute__((noinline)) alloc_stage_91(void) {
  return alloc_stage_92();
}

static void *__attribute__((noinline)) alloc_stage_90(void) {
  return alloc_stage_91();
}

static void *__attribute__((noinline)) alloc_stage_89(void) {
  return alloc_stage_90();
}

static void *__attribute__((noinline)) alloc_stage_88(void) {
  return alloc_stage_89();
}

static void *__attribute__((noinline)) alloc_stage_87(void) {
  return alloc_stage_88();
}

static void *__attribute__((noinline)) alloc_stage_86(void) {
  return alloc_stage_87();
}

static void *__attribute__((noinline)) alloc_stage_85(void) {
  return alloc_stage_86();
}

static void *__attribute__((noinline)) alloc_stage_84(void) {
  return alloc_stage_85();
}

static void *__attribute__((noinline)) alloc_stage_83(void) {
  return alloc_stage_84();
}

static void *__attribute__((noinline)) alloc_stage_82(void) {
  return alloc_stage_83();
}

static void *__attribute__((noinline)) alloc_stage_81(void) {
  return alloc_stage_82();
}

static void *__attribute__((noinline)) alloc_stage_80(void) {
  return alloc_stage_81();
}

static void *__attribute__((noinline)) alloc_stage_79(void) {
  return alloc_stage_80();
}

static void *__attribute__((noinline)) alloc_stage_78(void) {
  return alloc_stage_79();
}

static void *__attribute__((noinline)) alloc_stage_77(void) {
  return alloc_stage_78();
}

static void *__attribute__((noinline)) alloc_stage_76(void) {
  return alloc_stage_77();
}

static void *__attribute__((noinline)) alloc_stage_75(void) {
  return alloc_stage_76();
}

static void *__attribute__((noinline)) alloc_stage_74(void) {
  return alloc_stage_75();
}

static void *__attribute__((noinline)) alloc_stage_73(void) {
  return alloc_stage_74();
}

static void *__attribute__((noinline)) alloc_stage_72(void) {
  return alloc_stage_73();
}

static void *__attribute__((noinline)) alloc_stage_71(void) {
  return alloc_stage_72();
}

static void *__attribute__((noinline)) alloc_stage_70(void) {
  return alloc_stage_71();
}

static void *__attribute__((noinline)) alloc_stage_69(void) {
  return alloc_stage_70();
}

static void *__attribute__((noinline)) alloc_stage_68(void) {
  return alloc_stage_69();
}

static void *__attribute__((noinline)) alloc_stage_67(void) {
  return alloc_stage_68();
}

static void *__attribute__((noinline)) alloc_stage_66(void) {
  return alloc_stage_67();
}

static void *__attribute__((noinline)) alloc_stage_65(void) {
  return alloc_stage_66();
}

static void *__attribute__((noinline)) alloc_stage_64(void) {
  return alloc_stage_65();
}

static void *__attribute__((noinline)) alloc_stage_63(void) {
  return alloc_stage_64();
}

static void *__attribute__((noinline)) alloc_stage_62(void) {
  return alloc_stage_63();
}

static void *__attribute__((noinline)) alloc_stage_61(void) {
  return alloc_stage_62();
}

static void *__attribute__((noinline)) alloc_stage_60(void) {
  return alloc_stage_61();
}

static void *__attribute__((noinline)) alloc_stage_59(void) {
  return alloc_stage_60();
}

static void *__attribute__((noinline)) alloc_stage_58(void) {
  return alloc_stage_59();
}

static void *__attribute__((noinline)) alloc_stage_57(void) {
  return alloc_stage_58();
}

static void *__attribute__((noinline)) alloc_stage_56(void) {
  return alloc_stage_57();
}

static void *__attribute__((noinline)) alloc_stage_55(void) {
  return alloc_stage_56();
}

static void *__attribute__((noinline)) alloc_stage_54(void) {
  return alloc_stage_55();
}

static void *__attribute__((noinline)) alloc_stage_53(void) {
  return alloc_stage_54();
}

static void *__attribute__((noinline)) alloc_stage_52(void) {
  return alloc_stage_53();
}

static void *__attribute__((noinline)) alloc_stage_51(void) {
  return alloc_stage_52();
}

static void *__attribute__((noinline)) alloc_stage_50(void) {
  return alloc_stage_51();
}

static void *__attribute__((noinline)) alloc_stage_49(void) {
  return alloc_stage_50();
}

static void *__attribute__((noinline)) alloc_stage_48(void) {
  return alloc_stage_49();
}

static void *__attribute__((noinline)) alloc_stage_47(void) {
  return alloc_stage_48();
}

static void *__attribute__((noinline)) alloc_stage_46(void) {
  return alloc_stage_47();
}

static void *__attribute__((noinline)) alloc_stage_45(void) {
  return alloc_stage_46();
}

static void *__attribute__((noinline)) alloc_stage_44(void) {
  return alloc_stage_45();
}

static void *__attribute__((noinline)) alloc_stage_43(void) {
  return alloc_stage_44();
}

static void *__attribute__((noinline)) alloc_stage_42(void) {
  return alloc_stage_43();
}

static void *__attribute__((noinline)) alloc_stage_41(void) {
  return alloc_stage_42();
}

static void *__attribute__((noinline)) alloc_stage_40(void) {
  return alloc_stage_41();
}

static void *__attribute__((noinline)) alloc_stage_39(void) {
  return alloc_stage_40();
}

static void *__attribute__((noinline)) alloc_stage_38(void) {
  return alloc_stage_39();
}

static void *__attribute__((noinline)) alloc_stage_37(void) {
  return alloc_stage_38();
}

static void *__attribute__((noinline)) alloc_stage_36(void) {
  return alloc_stage_37();
}

static void *__attribute__((noinline)) alloc_stage_35(void) {
  return alloc_stage_36();
}

static void *__attribute__((noinline)) alloc_stage_34(void) {
  return alloc_stage_35();
}

static void *__attribute__((noinline)) alloc_stage_33(void) {
  return alloc_stage_34();
}

static void *__attribute__((noinline)) alloc_stage_32(void) {
  return alloc_stage_33();
}

static void *__attribute__((noinline)) alloc_stage_31(void) {
  return alloc_stage_32();
}

static void *__attribute__((noinline)) alloc_stage_30(void) {
  return alloc_stage_31();
}

static void *__attribute__((noinline)) alloc_stage_29(void) {
  return alloc_stage_30();
}

static void *__attribute__((noinline)) alloc_stage_28(void) {
  return alloc_stage_29();
}

static void *__attribute__((noinline)) alloc_stage_27(void) {
  return alloc_stage_28();
}

static void *__attribute__((noinline)) alloc_stage_26(void) {
  return alloc_stage_27();
}

static void *__attribute__((noinline)) alloc_stage_25(void) {
  return alloc_stage_26();
}

static void *__attribute__((noinline)) alloc_stage_24(void) {
  return alloc_stage_25();
}

static void *__attribute__((noinline)) alloc_stage_23(void) {
  return alloc_stage_24();
}

static void *__attribute__((noinline)) alloc_stage_22(void) {
  return alloc_stage_23();
}

static void *__attribute__((noinline)) alloc_stage_21(void) {
  return alloc_stage_22();
}

static void *__attribute__((noinline)) alloc_stage_20(void) {
  return alloc_stage_21();
}

static void *__attribute__((noinline)) alloc_stage_19(void) {
  return alloc_stage_20();
}

static void *__attribute__((noinline)) alloc_stage_18(void) {
  return alloc_stage_19();
}

static void *__attribute__((noinline)) alloc_stage_17(void) {
  return alloc_stage_18();
}

static void *__attribute__((noinline)) alloc_stage_16(void) {
  return alloc_stage_17();
}

static void *__attribute__((noinline)) alloc_stage_15(void) {
  return alloc_stage_16();
}

static void *__attribute__((noinline)) alloc_stage_14(void) {
  return alloc_stage_15();
}

static void *__attribute__((noinline)) alloc_stage_13(void) {
  return alloc_stage_14();
}

static void *__attribute__((noinline)) alloc_stage_12(void) {
  return alloc_stage_13();
}

static void *__attribute__((noinline)) alloc_stage_11(void) {
  return alloc_stage_12();
}

static void *__attribute__((noinline)) alloc_stage_10(void) {
  return alloc_stage_11();
}

static void *__attribute__((noinline)) alloc_stage_9(void) {
  return alloc_stage_10();
}

static void *__attribute__((noinline)) alloc_stage_8(void) {
  return alloc_stage_9();
}

static void *__attribute__((noinline)) alloc_stage_7(void) {
  return alloc_stage_8();
}

static void *__attribute__((noinline)) alloc_stage_6(void) {
  return alloc_stage_7();
}

static void *__attribute__((noinline)) alloc_stage_5(void) {
  return alloc_stage_6();
}

static void *__attribute__((noinline)) alloc_stage_4(void) {
  return alloc_stage_5();
}

static void *__attribute__((noinline)) alloc_stage_3(void) {
  return alloc_stage_4();
}

static void *__attribute__((noinline)) alloc_stage_2(void) {
  return alloc_stage_3();
}

static void *__attribute__((noinline)) alloc_stage_1(void) {
  return alloc_stage_2();
}

static void *__attribute__((noinline)) alloc_stage_0(void) {
  return alloc_stage_1();
}

int main(void) {
  __hwasan_enable_allocator_tagging();
  char *volatile x = (char *)alloc_stage_0();
  free(x);
  __hwasan_disable_allocator_tagging();
  fprintf(stderr, ISREAD ? "READ UAF\n" : "WRITE UAF\n");
  int r = 0;
  if (ISREAD) r = x[5]; else x[5] = 42;
  // CHECK: ERROR: HWAddressSanitizer: tag-mismatch on address {{.*}}
  // CHECK: Heap malloc record:
  // CHECK: Heap allocated by thread {{.*}} here:
  // CHECK: #0 {{.*}} in {{.*}}malloc{{.*}} {{.*}}hwasan_allocation_functions.cpp
  // CHECK: #1 {{.*}} in alloc_stage_211 {{.*}}record_malloc_info_uaf.c:[[@LINE-859]]
  // CHECK: SUMMARY: HWAddressSanitizer: tag-mismatch
  return r;
}
