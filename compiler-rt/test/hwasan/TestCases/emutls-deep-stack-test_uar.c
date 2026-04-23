// RUN: %clang_hwasan -mllvm -hwasan-instrument-without-TLS=true -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

// REQUIRES: stable-runtime
// REQUIRES: ohos_family

// Use-after-return (UAR) test with a very deep call stack before the dangling
// pointer is dereferenced. Verifies stack-history / frame-record lines still
// appear in diagnostics after many noinline frames (same FileCheck sequence as
// the original short test).

void USE(void *x) { // pretend_to_do_something(void *x)
  __asm__ __volatile__("" : : "r" (x) : "memory");
}

__attribute__((noinline))
char *buggy() {
  char zzz[0x1000];
  char *volatile p = zzz;
  return p;
}

__attribute__((noinline)) void Unrelated1() { int A[2]; USE(&A[0]); }
__attribute__((noinline)) void Unrelated2() { int BB[3]; USE(&BB[0]); }
__attribute__((noinline)) void Unrelated3() { int CCC[4]; USE(&CCC[0]); }

static char *volatile g_dangling;

__attribute__((noinline)) static void tail_use_after_return(void) {
  Unrelated1();
  Unrelated2();
  Unrelated3();
  volatile char sink = *g_dangling;
  (void)sink;
}

__attribute__((noinline)) static void depth_0(void);
__attribute__((noinline)) static void depth_1(void);
__attribute__((noinline)) static void depth_2(void);
__attribute__((noinline)) static void depth_3(void);
__attribute__((noinline)) static void depth_4(void);
__attribute__((noinline)) static void depth_5(void);
__attribute__((noinline)) static void depth_6(void);
__attribute__((noinline)) static void depth_7(void);
__attribute__((noinline)) static void depth_8(void);
__attribute__((noinline)) static void depth_9(void);
__attribute__((noinline)) static void depth_10(void);
__attribute__((noinline)) static void depth_11(void);
__attribute__((noinline)) static void depth_12(void);
__attribute__((noinline)) static void depth_13(void);
__attribute__((noinline)) static void depth_14(void);
__attribute__((noinline)) static void depth_15(void);
__attribute__((noinline)) static void depth_16(void);
__attribute__((noinline)) static void depth_17(void);
__attribute__((noinline)) static void depth_18(void);
__attribute__((noinline)) static void depth_19(void);
__attribute__((noinline)) static void depth_20(void);
__attribute__((noinline)) static void depth_21(void);
__attribute__((noinline)) static void depth_22(void);
__attribute__((noinline)) static void depth_23(void);
__attribute__((noinline)) static void depth_24(void);
__attribute__((noinline)) static void depth_25(void);
__attribute__((noinline)) static void depth_26(void);
__attribute__((noinline)) static void depth_27(void);
__attribute__((noinline)) static void depth_28(void);
__attribute__((noinline)) static void depth_29(void);
__attribute__((noinline)) static void depth_30(void);
__attribute__((noinline)) static void depth_31(void);
__attribute__((noinline)) static void depth_32(void);
__attribute__((noinline)) static void depth_33(void);
__attribute__((noinline)) static void depth_34(void);
__attribute__((noinline)) static void depth_35(void);
__attribute__((noinline)) static void depth_36(void);
__attribute__((noinline)) static void depth_37(void);
__attribute__((noinline)) static void depth_38(void);
__attribute__((noinline)) static void depth_39(void);
__attribute__((noinline)) static void depth_40(void);
__attribute__((noinline)) static void depth_41(void);
__attribute__((noinline)) static void depth_42(void);
__attribute__((noinline)) static void depth_43(void);
__attribute__((noinline)) static void depth_44(void);
__attribute__((noinline)) static void depth_45(void);
__attribute__((noinline)) static void depth_46(void);
__attribute__((noinline)) static void depth_47(void);
__attribute__((noinline)) static void depth_48(void);
__attribute__((noinline)) static void depth_49(void);
__attribute__((noinline)) static void depth_50(void);
__attribute__((noinline)) static void depth_51(void);
__attribute__((noinline)) static void depth_52(void);
__attribute__((noinline)) static void depth_53(void);
__attribute__((noinline)) static void depth_54(void);
__attribute__((noinline)) static void depth_55(void);
__attribute__((noinline)) static void depth_56(void);
__attribute__((noinline)) static void depth_57(void);
__attribute__((noinline)) static void depth_58(void);
__attribute__((noinline)) static void depth_59(void);
__attribute__((noinline)) static void depth_60(void);
__attribute__((noinline)) static void depth_61(void);
__attribute__((noinline)) static void depth_62(void);
__attribute__((noinline)) static void depth_63(void);
__attribute__((noinline)) static void depth_64(void);
__attribute__((noinline)) static void depth_65(void);
__attribute__((noinline)) static void depth_66(void);
__attribute__((noinline)) static void depth_67(void);
__attribute__((noinline)) static void depth_68(void);
__attribute__((noinline)) static void depth_69(void);
__attribute__((noinline)) static void depth_70(void);
__attribute__((noinline)) static void depth_71(void);
__attribute__((noinline)) static void depth_72(void);
__attribute__((noinline)) static void depth_73(void);
__attribute__((noinline)) static void depth_74(void);
__attribute__((noinline)) static void depth_75(void);
__attribute__((noinline)) static void depth_76(void);
__attribute__((noinline)) static void depth_77(void);
__attribute__((noinline)) static void depth_78(void);
__attribute__((noinline)) static void depth_79(void);
__attribute__((noinline)) static void depth_80(void);
__attribute__((noinline)) static void depth_81(void);
__attribute__((noinline)) static void depth_82(void);
__attribute__((noinline)) static void depth_83(void);
__attribute__((noinline)) static void depth_84(void);
__attribute__((noinline)) static void depth_85(void);
__attribute__((noinline)) static void depth_86(void);
__attribute__((noinline)) static void depth_87(void);
__attribute__((noinline)) static void depth_88(void);
__attribute__((noinline)) static void depth_89(void);
__attribute__((noinline)) static void depth_90(void);
__attribute__((noinline)) static void depth_91(void);
__attribute__((noinline)) static void depth_92(void);
__attribute__((noinline)) static void depth_93(void);
__attribute__((noinline)) static void depth_94(void);
__attribute__((noinline)) static void depth_95(void);
__attribute__((noinline)) static void depth_96(void);
__attribute__((noinline)) static void depth_97(void);
__attribute__((noinline)) static void depth_98(void);
__attribute__((noinline)) static void depth_99(void);
__attribute__((noinline)) static void depth_100(void);
__attribute__((noinline)) static void depth_101(void);
__attribute__((noinline)) static void depth_102(void);
__attribute__((noinline)) static void depth_103(void);
__attribute__((noinline)) static void depth_104(void);
__attribute__((noinline)) static void depth_105(void);
__attribute__((noinline)) static void depth_106(void);
__attribute__((noinline)) static void depth_107(void);
__attribute__((noinline)) static void depth_108(void);
__attribute__((noinline)) static void depth_109(void);
__attribute__((noinline)) static void depth_110(void);
__attribute__((noinline)) static void depth_111(void);
__attribute__((noinline)) static void depth_112(void);
__attribute__((noinline)) static void depth_113(void);
__attribute__((noinline)) static void depth_114(void);
__attribute__((noinline)) static void depth_115(void);
__attribute__((noinline)) static void depth_116(void);
__attribute__((noinline)) static void depth_117(void);
__attribute__((noinline)) static void depth_118(void);
__attribute__((noinline)) static void depth_119(void);
__attribute__((noinline)) static void depth_120(void);
__attribute__((noinline)) static void depth_121(void);
__attribute__((noinline)) static void depth_122(void);
__attribute__((noinline)) static void depth_123(void);
__attribute__((noinline)) static void depth_124(void);
__attribute__((noinline)) static void depth_125(void);
__attribute__((noinline)) static void depth_126(void);
__attribute__((noinline)) static void depth_127(void);
__attribute__((noinline)) static void depth_128(void);
__attribute__((noinline)) static void depth_129(void);
__attribute__((noinline)) static void depth_130(void);
__attribute__((noinline)) static void depth_131(void);
__attribute__((noinline)) static void depth_132(void);
__attribute__((noinline)) static void depth_133(void);
__attribute__((noinline)) static void depth_134(void);
__attribute__((noinline)) static void depth_135(void);
__attribute__((noinline)) static void depth_136(void);
__attribute__((noinline)) static void depth_137(void);
__attribute__((noinline)) static void depth_138(void);
__attribute__((noinline)) static void depth_139(void);
__attribute__((noinline)) static void depth_140(void);
__attribute__((noinline)) static void depth_141(void);
__attribute__((noinline)) static void depth_142(void);
__attribute__((noinline)) static void depth_143(void);
__attribute__((noinline)) static void depth_144(void);
__attribute__((noinline)) static void depth_145(void);
__attribute__((noinline)) static void depth_146(void);
__attribute__((noinline)) static void depth_147(void);
__attribute__((noinline)) static void depth_148(void);
__attribute__((noinline)) static void depth_149(void);
__attribute__((noinline)) static void depth_150(void);
__attribute__((noinline)) static void depth_151(void);
__attribute__((noinline)) static void depth_152(void);
__attribute__((noinline)) static void depth_153(void);
__attribute__((noinline)) static void depth_154(void);
__attribute__((noinline)) static void depth_155(void);
__attribute__((noinline)) static void depth_156(void);
__attribute__((noinline)) static void depth_157(void);
__attribute__((noinline)) static void depth_158(void);
__attribute__((noinline)) static void depth_159(void);
__attribute__((noinline)) static void depth_160(void);
__attribute__((noinline)) static void depth_161(void);
__attribute__((noinline)) static void depth_162(void);
__attribute__((noinline)) static void depth_163(void);
__attribute__((noinline)) static void depth_164(void);
__attribute__((noinline)) static void depth_165(void);
__attribute__((noinline)) static void depth_166(void);
__attribute__((noinline)) static void depth_167(void);
__attribute__((noinline)) static void depth_168(void);
__attribute__((noinline)) static void depth_169(void);
__attribute__((noinline)) static void depth_170(void);
__attribute__((noinline)) static void depth_171(void);
__attribute__((noinline)) static void depth_172(void);
__attribute__((noinline)) static void depth_173(void);
__attribute__((noinline)) static void depth_174(void);
__attribute__((noinline)) static void depth_175(void);
__attribute__((noinline)) static void depth_176(void);
__attribute__((noinline)) static void depth_177(void);
__attribute__((noinline)) static void depth_178(void);
__attribute__((noinline)) static void depth_179(void);
__attribute__((noinline)) static void depth_180(void);
__attribute__((noinline)) static void depth_181(void);
__attribute__((noinline)) static void depth_182(void);
__attribute__((noinline)) static void depth_183(void);
__attribute__((noinline)) static void depth_184(void);
__attribute__((noinline)) static void depth_185(void);
__attribute__((noinline)) static void depth_186(void);
__attribute__((noinline)) static void depth_187(void);
__attribute__((noinline)) static void depth_188(void);
__attribute__((noinline)) static void depth_189(void);
__attribute__((noinline)) static void depth_190(void);
__attribute__((noinline)) static void depth_191(void);
__attribute__((noinline)) static void depth_192(void);
__attribute__((noinline)) static void depth_193(void);
__attribute__((noinline)) static void depth_194(void);
__attribute__((noinline)) static void depth_195(void);
__attribute__((noinline)) static void depth_196(void);
__attribute__((noinline)) static void depth_197(void);
__attribute__((noinline)) static void depth_198(void);
__attribute__((noinline)) static void depth_199(void);
__attribute__((noinline)) static void depth_200(void);
__attribute__((noinline)) static void depth_201(void);
__attribute__((noinline)) static void depth_202(void);
__attribute__((noinline)) static void depth_203(void);
__attribute__((noinline)) static void depth_204(void);
__attribute__((noinline)) static void depth_205(void);
__attribute__((noinline)) static void depth_206(void);
__attribute__((noinline)) static void depth_207(void);
__attribute__((noinline)) static void depth_208(void);
__attribute__((noinline)) static void depth_209(void);
__attribute__((noinline)) static void depth_210(void);
__attribute__((noinline)) static void depth_211(void);
__attribute__((noinline)) static void depth_212(void);
__attribute__((noinline)) static void depth_213(void);
__attribute__((noinline)) static void depth_214(void);
__attribute__((noinline)) static void depth_215(void);
__attribute__((noinline)) static void depth_216(void);
__attribute__((noinline)) static void depth_217(void);
__attribute__((noinline)) static void depth_218(void);
__attribute__((noinline)) static void depth_219(void);
__attribute__((noinline)) static void depth_220(void);
__attribute__((noinline)) static void depth_221(void);
__attribute__((noinline)) static void depth_222(void);
__attribute__((noinline)) static void depth_223(void);
__attribute__((noinline)) static void depth_224(void);
__attribute__((noinline)) static void depth_225(void);
__attribute__((noinline)) static void depth_226(void);
__attribute__((noinline)) static void depth_227(void);
__attribute__((noinline)) static void depth_228(void);
__attribute__((noinline)) static void depth_229(void);
__attribute__((noinline)) static void depth_230(void);
__attribute__((noinline)) static void depth_231(void);
__attribute__((noinline)) static void depth_232(void);
__attribute__((noinline)) static void depth_233(void);
__attribute__((noinline)) static void depth_234(void);
__attribute__((noinline)) static void depth_235(void);
__attribute__((noinline)) static void depth_236(void);
__attribute__((noinline)) static void depth_237(void);
__attribute__((noinline)) static void depth_238(void);
__attribute__((noinline)) static void depth_239(void);
__attribute__((noinline)) static void depth_240(void);
__attribute__((noinline)) static void depth_241(void);
__attribute__((noinline)) static void depth_242(void);
__attribute__((noinline)) static void depth_243(void);
__attribute__((noinline)) static void depth_244(void);
__attribute__((noinline)) static void depth_245(void);
__attribute__((noinline)) static void depth_246(void);
__attribute__((noinline)) static void depth_247(void);
__attribute__((noinline)) static void depth_248(void);
__attribute__((noinline)) static void depth_249(void);
__attribute__((noinline)) static void depth_250(void);
__attribute__((noinline)) static void depth_251(void);
__attribute__((noinline)) static void depth_252(void);
__attribute__((noinline)) static void depth_253(void);
__attribute__((noinline)) static void depth_254(void);
__attribute__((noinline)) static void depth_255(void);
__attribute__((noinline)) static void depth_256(void);
__attribute__((noinline)) static void depth_257(void);
__attribute__((noinline)) static void depth_258(void);
__attribute__((noinline)) static void depth_259(void);
__attribute__((noinline)) static void depth_260(void);
__attribute__((noinline)) static void depth_261(void);
__attribute__((noinline)) static void depth_262(void);
__attribute__((noinline)) static void depth_263(void);
__attribute__((noinline)) static void depth_264(void);
__attribute__((noinline)) static void depth_265(void);
__attribute__((noinline)) static void depth_266(void);
__attribute__((noinline)) static void depth_267(void);
__attribute__((noinline)) static void depth_268(void);
__attribute__((noinline)) static void depth_269(void);
__attribute__((noinline)) static void depth_270(void);
__attribute__((noinline)) static void depth_271(void);
__attribute__((noinline)) static void depth_272(void);
__attribute__((noinline)) static void depth_273(void);
__attribute__((noinline)) static void depth_274(void);
__attribute__((noinline)) static void depth_275(void);
__attribute__((noinline)) static void depth_276(void);
__attribute__((noinline)) static void depth_277(void);
__attribute__((noinline)) static void depth_278(void);
__attribute__((noinline)) static void depth_279(void);
__attribute__((noinline)) static void depth_280(void);
__attribute__((noinline)) static void depth_281(void);
__attribute__((noinline)) static void depth_282(void);
__attribute__((noinline)) static void depth_283(void);
__attribute__((noinline)) static void depth_284(void);
__attribute__((noinline)) static void depth_285(void);
__attribute__((noinline)) static void depth_286(void);
__attribute__((noinline)) static void depth_287(void);
__attribute__((noinline)) static void depth_288(void);
__attribute__((noinline)) static void depth_289(void);
__attribute__((noinline)) static void depth_290(void);
__attribute__((noinline)) static void depth_291(void);
__attribute__((noinline)) static void depth_292(void);
__attribute__((noinline)) static void depth_293(void);
__attribute__((noinline)) static void depth_294(void);
__attribute__((noinline)) static void depth_295(void);
__attribute__((noinline)) static void depth_296(void);
__attribute__((noinline)) static void depth_297(void);
__attribute__((noinline)) static void depth_298(void);
__attribute__((noinline)) static void depth_299(void);
__attribute__((noinline)) static void depth_300(void);
__attribute__((noinline)) static void depth_301(void);
__attribute__((noinline)) static void depth_302(void);
__attribute__((noinline)) static void depth_303(void);
__attribute__((noinline)) static void depth_304(void);
__attribute__((noinline)) static void depth_305(void);
__attribute__((noinline)) static void depth_306(void);
__attribute__((noinline)) static void depth_307(void);
__attribute__((noinline)) static void depth_308(void);
__attribute__((noinline)) static void depth_309(void);
__attribute__((noinline)) static void depth_310(void);
__attribute__((noinline)) static void depth_311(void);
__attribute__((noinline)) static void depth_312(void);
__attribute__((noinline)) static void depth_313(void);
__attribute__((noinline)) static void depth_314(void);
__attribute__((noinline)) static void depth_315(void);
__attribute__((noinline)) static void depth_316(void);
__attribute__((noinline)) static void depth_317(void);
__attribute__((noinline)) static void depth_318(void);
__attribute__((noinline)) static void depth_319(void);
__attribute__((noinline)) static void depth_320(void);
__attribute__((noinline)) static void depth_321(void);
__attribute__((noinline)) static void depth_322(void);
__attribute__((noinline)) static void depth_323(void);
__attribute__((noinline)) static void depth_324(void);
__attribute__((noinline)) static void depth_325(void);
__attribute__((noinline)) static void depth_326(void);
__attribute__((noinline)) static void depth_327(void);
__attribute__((noinline)) static void depth_328(void);
__attribute__((noinline)) static void depth_329(void);
__attribute__((noinline)) static void depth_330(void);
__attribute__((noinline)) static void depth_331(void);
__attribute__((noinline)) static void depth_332(void);
__attribute__((noinline)) static void depth_333(void);
__attribute__((noinline)) static void depth_334(void);
__attribute__((noinline)) static void depth_335(void);
__attribute__((noinline)) static void depth_336(void);
__attribute__((noinline)) static void depth_337(void);
__attribute__((noinline)) static void depth_338(void);
__attribute__((noinline)) static void depth_339(void);
__attribute__((noinline)) static void depth_340(void);
__attribute__((noinline)) static void depth_341(void);
__attribute__((noinline)) static void depth_342(void);
__attribute__((noinline)) static void depth_343(void);
__attribute__((noinline)) static void depth_344(void);
__attribute__((noinline)) static void depth_345(void);
__attribute__((noinline)) static void depth_346(void);
__attribute__((noinline)) static void depth_347(void);
__attribute__((noinline)) static void depth_348(void);
__attribute__((noinline)) static void depth_349(void);
__attribute__((noinline)) static void depth_350(void);
__attribute__((noinline)) static void depth_351(void);
__attribute__((noinline)) static void depth_352(void);
__attribute__((noinline)) static void depth_353(void);
__attribute__((noinline)) static void depth_354(void);
__attribute__((noinline)) static void depth_355(void);
__attribute__((noinline)) static void depth_356(void);
__attribute__((noinline)) static void depth_357(void);

__attribute__((noinline)) static void depth_0(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_1();
}
__attribute__((noinline)) static void depth_1(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_2();
}
__attribute__((noinline)) static void depth_2(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_3();
}
__attribute__((noinline)) static void depth_3(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_4();
}
__attribute__((noinline)) static void depth_4(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_5();
}
__attribute__((noinline)) static void depth_5(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_6();
}
__attribute__((noinline)) static void depth_6(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_7();
}
__attribute__((noinline)) static void depth_7(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_8();
}
__attribute__((noinline)) static void depth_8(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_9();
}
__attribute__((noinline)) static void depth_9(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_10();
}
__attribute__((noinline)) static void depth_10(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_11();
}
__attribute__((noinline)) static void depth_11(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_12();
}
__attribute__((noinline)) static void depth_12(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_13();
}
__attribute__((noinline)) static void depth_13(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_14();
}
__attribute__((noinline)) static void depth_14(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_15();
}
__attribute__((noinline)) static void depth_15(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_16();
}
__attribute__((noinline)) static void depth_16(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_17();
}
__attribute__((noinline)) static void depth_17(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_18();
}
__attribute__((noinline)) static void depth_18(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_19();
}
__attribute__((noinline)) static void depth_19(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_20();
}
__attribute__((noinline)) static void depth_20(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_21();
}
__attribute__((noinline)) static void depth_21(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_22();
}
__attribute__((noinline)) static void depth_22(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_23();
}
__attribute__((noinline)) static void depth_23(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_24();
}
__attribute__((noinline)) static void depth_24(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_25();
}
__attribute__((noinline)) static void depth_25(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_26();
}
__attribute__((noinline)) static void depth_26(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_27();
}
__attribute__((noinline)) static void depth_27(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_28();
}
__attribute__((noinline)) static void depth_28(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_29();
}
__attribute__((noinline)) static void depth_29(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_30();
}
__attribute__((noinline)) static void depth_30(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_31();
}
__attribute__((noinline)) static void depth_31(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_32();
}
__attribute__((noinline)) static void depth_32(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_33();
}
__attribute__((noinline)) static void depth_33(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_34();
}
__attribute__((noinline)) static void depth_34(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_35();
}
__attribute__((noinline)) static void depth_35(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_36();
}
__attribute__((noinline)) static void depth_36(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_37();
}
__attribute__((noinline)) static void depth_37(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_38();
}
__attribute__((noinline)) static void depth_38(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_39();
}
__attribute__((noinline)) static void depth_39(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_40();
}
__attribute__((noinline)) static void depth_40(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_41();
}
__attribute__((noinline)) static void depth_41(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_42();
}
__attribute__((noinline)) static void depth_42(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_43();
}
__attribute__((noinline)) static void depth_43(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_44();
}
__attribute__((noinline)) static void depth_44(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_45();
}
__attribute__((noinline)) static void depth_45(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_46();
}
__attribute__((noinline)) static void depth_46(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_47();
}
__attribute__((noinline)) static void depth_47(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_48();
}
__attribute__((noinline)) static void depth_48(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_49();
}
__attribute__((noinline)) static void depth_49(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_50();
}
__attribute__((noinline)) static void depth_50(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_51();
}
__attribute__((noinline)) static void depth_51(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_52();
}
__attribute__((noinline)) static void depth_52(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_53();
}
__attribute__((noinline)) static void depth_53(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_54();
}
__attribute__((noinline)) static void depth_54(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_55();
}
__attribute__((noinline)) static void depth_55(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_56();
}
__attribute__((noinline)) static void depth_56(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_57();
}
__attribute__((noinline)) static void depth_57(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_58();
}
__attribute__((noinline)) static void depth_58(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_59();
}
__attribute__((noinline)) static void depth_59(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_60();
}
__attribute__((noinline)) static void depth_60(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_61();
}
__attribute__((noinline)) static void depth_61(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_62();
}
__attribute__((noinline)) static void depth_62(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_63();
}
__attribute__((noinline)) static void depth_63(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_64();
}
__attribute__((noinline)) static void depth_64(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_65();
}
__attribute__((noinline)) static void depth_65(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_66();
}
__attribute__((noinline)) static void depth_66(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_67();
}
__attribute__((noinline)) static void depth_67(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_68();
}
__attribute__((noinline)) static void depth_68(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_69();
}
__attribute__((noinline)) static void depth_69(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_70();
}
__attribute__((noinline)) static void depth_70(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_71();
}
__attribute__((noinline)) static void depth_71(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_72();
}
__attribute__((noinline)) static void depth_72(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_73();
}
__attribute__((noinline)) static void depth_73(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_74();
}
__attribute__((noinline)) static void depth_74(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_75();
}
__attribute__((noinline)) static void depth_75(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_76();
}
__attribute__((noinline)) static void depth_76(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_77();
}
__attribute__((noinline)) static void depth_77(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_78();
}
__attribute__((noinline)) static void depth_78(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_79();
}
__attribute__((noinline)) static void depth_79(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_80();
}
__attribute__((noinline)) static void depth_80(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_81();
}
__attribute__((noinline)) static void depth_81(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_82();
}
__attribute__((noinline)) static void depth_82(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_83();
}
__attribute__((noinline)) static void depth_83(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_84();
}
__attribute__((noinline)) static void depth_84(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_85();
}
__attribute__((noinline)) static void depth_85(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_86();
}
__attribute__((noinline)) static void depth_86(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_87();
}
__attribute__((noinline)) static void depth_87(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_88();
}
__attribute__((noinline)) static void depth_88(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_89();
}
__attribute__((noinline)) static void depth_89(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_90();
}
__attribute__((noinline)) static void depth_90(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_91();
}
__attribute__((noinline)) static void depth_91(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_92();
}
__attribute__((noinline)) static void depth_92(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_93();
}
__attribute__((noinline)) static void depth_93(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_94();
}
__attribute__((noinline)) static void depth_94(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_95();
}
__attribute__((noinline)) static void depth_95(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_96();
}
__attribute__((noinline)) static void depth_96(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_97();
}
__attribute__((noinline)) static void depth_97(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_98();
}
__attribute__((noinline)) static void depth_98(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_99();
}
__attribute__((noinline)) static void depth_99(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_100();
}
__attribute__((noinline)) static void depth_100(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_101();
}
__attribute__((noinline)) static void depth_101(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_102();
}
__attribute__((noinline)) static void depth_102(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_103();
}
__attribute__((noinline)) static void depth_103(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_104();
}
__attribute__((noinline)) static void depth_104(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_105();
}
__attribute__((noinline)) static void depth_105(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_106();
}
__attribute__((noinline)) static void depth_106(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_107();
}
__attribute__((noinline)) static void depth_107(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_108();
}
__attribute__((noinline)) static void depth_108(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_109();
}
__attribute__((noinline)) static void depth_109(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_110();
}
__attribute__((noinline)) static void depth_110(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_111();
}
__attribute__((noinline)) static void depth_111(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_112();
}
__attribute__((noinline)) static void depth_112(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_113();
}
__attribute__((noinline)) static void depth_113(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_114();
}
__attribute__((noinline)) static void depth_114(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_115();
}
__attribute__((noinline)) static void depth_115(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_116();
}
__attribute__((noinline)) static void depth_116(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_117();
}
__attribute__((noinline)) static void depth_117(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_118();
}
__attribute__((noinline)) static void depth_118(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_119();
}
__attribute__((noinline)) static void depth_119(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_120();
}
__attribute__((noinline)) static void depth_120(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_121();
}
__attribute__((noinline)) static void depth_121(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_122();
}
__attribute__((noinline)) static void depth_122(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_123();
}
__attribute__((noinline)) static void depth_123(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_124();
}
__attribute__((noinline)) static void depth_124(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_125();
}
__attribute__((noinline)) static void depth_125(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_126();
}
__attribute__((noinline)) static void depth_126(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_127();
}
__attribute__((noinline)) static void depth_127(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_128();
}
__attribute__((noinline)) static void depth_128(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_129();
}
__attribute__((noinline)) static void depth_129(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_130();
}
__attribute__((noinline)) static void depth_130(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_131();
}
__attribute__((noinline)) static void depth_131(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_132();
}
__attribute__((noinline)) static void depth_132(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_133();
}
__attribute__((noinline)) static void depth_133(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_134();
}
__attribute__((noinline)) static void depth_134(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_135();
}
__attribute__((noinline)) static void depth_135(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_136();
}
__attribute__((noinline)) static void depth_136(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_137();
}
__attribute__((noinline)) static void depth_137(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_138();
}
__attribute__((noinline)) static void depth_138(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_139();
}
__attribute__((noinline)) static void depth_139(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_140();
}
__attribute__((noinline)) static void depth_140(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_141();
}
__attribute__((noinline)) static void depth_141(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_142();
}
__attribute__((noinline)) static void depth_142(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_143();
}
__attribute__((noinline)) static void depth_143(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_144();
}
__attribute__((noinline)) static void depth_144(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_145();
}
__attribute__((noinline)) static void depth_145(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_146();
}
__attribute__((noinline)) static void depth_146(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_147();
}
__attribute__((noinline)) static void depth_147(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_148();
}
__attribute__((noinline)) static void depth_148(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_149();
}
__attribute__((noinline)) static void depth_149(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_150();
}
__attribute__((noinline)) static void depth_150(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_151();
}
__attribute__((noinline)) static void depth_151(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_152();
}
__attribute__((noinline)) static void depth_152(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_153();
}
__attribute__((noinline)) static void depth_153(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_154();
}
__attribute__((noinline)) static void depth_154(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_155();
}
__attribute__((noinline)) static void depth_155(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_156();
}
__attribute__((noinline)) static void depth_156(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_157();
}
__attribute__((noinline)) static void depth_157(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_158();
}
__attribute__((noinline)) static void depth_158(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_159();
}
__attribute__((noinline)) static void depth_159(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_160();
}
__attribute__((noinline)) static void depth_160(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_161();
}
__attribute__((noinline)) static void depth_161(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_162();
}
__attribute__((noinline)) static void depth_162(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_163();
}
__attribute__((noinline)) static void depth_163(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_164();
}
__attribute__((noinline)) static void depth_164(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_165();
}
__attribute__((noinline)) static void depth_165(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_166();
}
__attribute__((noinline)) static void depth_166(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_167();
}
__attribute__((noinline)) static void depth_167(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_168();
}
__attribute__((noinline)) static void depth_168(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_169();
}
__attribute__((noinline)) static void depth_169(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_170();
}
__attribute__((noinline)) static void depth_170(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_171();
}
__attribute__((noinline)) static void depth_171(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_172();
}
__attribute__((noinline)) static void depth_172(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_173();
}
__attribute__((noinline)) static void depth_173(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_174();
}
__attribute__((noinline)) static void depth_174(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_175();
}
__attribute__((noinline)) static void depth_175(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_176();
}
__attribute__((noinline)) static void depth_176(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_177();
}
__attribute__((noinline)) static void depth_177(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_178();
}
__attribute__((noinline)) static void depth_178(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_179();
}
__attribute__((noinline)) static void depth_179(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_180();
}
__attribute__((noinline)) static void depth_180(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_181();
}
__attribute__((noinline)) static void depth_181(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_182();
}
__attribute__((noinline)) static void depth_182(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_183();
}
__attribute__((noinline)) static void depth_183(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_184();
}
__attribute__((noinline)) static void depth_184(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_185();
}
__attribute__((noinline)) static void depth_185(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_186();
}
__attribute__((noinline)) static void depth_186(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_187();
}
__attribute__((noinline)) static void depth_187(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_188();
}
__attribute__((noinline)) static void depth_188(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_189();
}
__attribute__((noinline)) static void depth_189(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_190();
}
__attribute__((noinline)) static void depth_190(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_191();
}
__attribute__((noinline)) static void depth_191(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_192();
}
__attribute__((noinline)) static void depth_192(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_193();
}
__attribute__((noinline)) static void depth_193(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_194();
}
__attribute__((noinline)) static void depth_194(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_195();
}
__attribute__((noinline)) static void depth_195(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_196();
}
__attribute__((noinline)) static void depth_196(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_197();
}
__attribute__((noinline)) static void depth_197(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_198();
}
__attribute__((noinline)) static void depth_198(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_199();
}
__attribute__((noinline)) static void depth_199(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_200();
}
__attribute__((noinline)) static void depth_200(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_201();
}
__attribute__((noinline)) static void depth_201(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_202();
}
__attribute__((noinline)) static void depth_202(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_203();
}
__attribute__((noinline)) static void depth_203(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_204();
}
__attribute__((noinline)) static void depth_204(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_205();
}
__attribute__((noinline)) static void depth_205(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_206();
}
__attribute__((noinline)) static void depth_206(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_207();
}
__attribute__((noinline)) static void depth_207(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_208();
}
__attribute__((noinline)) static void depth_208(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_209();
}
__attribute__((noinline)) static void depth_209(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_210();
}
__attribute__((noinline)) static void depth_210(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_211();
}
__attribute__((noinline)) static void depth_211(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_212();
}
__attribute__((noinline)) static void depth_212(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_213();
}
__attribute__((noinline)) static void depth_213(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_214();
}
__attribute__((noinline)) static void depth_214(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_215();
}
__attribute__((noinline)) static void depth_215(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_216();
}
__attribute__((noinline)) static void depth_216(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_217();
}
__attribute__((noinline)) static void depth_217(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_218();
}
__attribute__((noinline)) static void depth_218(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_219();
}
__attribute__((noinline)) static void depth_219(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_220();
}
__attribute__((noinline)) static void depth_220(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_221();
}
__attribute__((noinline)) static void depth_221(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_222();
}
__attribute__((noinline)) static void depth_222(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_223();
}
__attribute__((noinline)) static void depth_223(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_224();
}
__attribute__((noinline)) static void depth_224(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_225();
}
__attribute__((noinline)) static void depth_225(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_226();
}
__attribute__((noinline)) static void depth_226(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_227();
}
__attribute__((noinline)) static void depth_227(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_228();
}
__attribute__((noinline)) static void depth_228(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_229();
}
__attribute__((noinline)) static void depth_229(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_230();
}
__attribute__((noinline)) static void depth_230(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_231();
}
__attribute__((noinline)) static void depth_231(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_232();
}
__attribute__((noinline)) static void depth_232(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_233();
}
__attribute__((noinline)) static void depth_233(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_234();
}
__attribute__((noinline)) static void depth_234(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_235();
}
__attribute__((noinline)) static void depth_235(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_236();
}
__attribute__((noinline)) static void depth_236(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_237();
}
__attribute__((noinline)) static void depth_237(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_238();
}
__attribute__((noinline)) static void depth_238(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_239();
}
__attribute__((noinline)) static void depth_239(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_240();
}
__attribute__((noinline)) static void depth_240(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_241();
}
__attribute__((noinline)) static void depth_241(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_242();
}
__attribute__((noinline)) static void depth_242(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_243();
}
__attribute__((noinline)) static void depth_243(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_244();
}
__attribute__((noinline)) static void depth_244(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_245();
}
__attribute__((noinline)) static void depth_245(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_246();
}
__attribute__((noinline)) static void depth_246(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_247();
}
__attribute__((noinline)) static void depth_247(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_248();
}
__attribute__((noinline)) static void depth_248(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_249();
}
__attribute__((noinline)) static void depth_249(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_250();
}
__attribute__((noinline)) static void depth_250(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_251();
}
__attribute__((noinline)) static void depth_251(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_252();
}
__attribute__((noinline)) static void depth_252(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_253();
}
__attribute__((noinline)) static void depth_253(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_254();
}
__attribute__((noinline)) static void depth_254(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_255();
}
__attribute__((noinline)) static void depth_255(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_256();
}
__attribute__((noinline)) static void depth_256(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_257();
}
__attribute__((noinline)) static void depth_257(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_258();
}
__attribute__((noinline)) static void depth_258(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_259();
}
__attribute__((noinline)) static void depth_259(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_260();
}
__attribute__((noinline)) static void depth_260(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_261();
}
__attribute__((noinline)) static void depth_261(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_262();
}
__attribute__((noinline)) static void depth_262(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_263();
}
__attribute__((noinline)) static void depth_263(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_264();
}
__attribute__((noinline)) static void depth_264(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_265();
}
__attribute__((noinline)) static void depth_265(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_266();
}
__attribute__((noinline)) static void depth_266(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_267();
}
__attribute__((noinline)) static void depth_267(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_268();
}
__attribute__((noinline)) static void depth_268(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_269();
}
__attribute__((noinline)) static void depth_269(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_270();
}
__attribute__((noinline)) static void depth_270(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_271();
}
__attribute__((noinline)) static void depth_271(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_272();
}
__attribute__((noinline)) static void depth_272(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_273();
}
__attribute__((noinline)) static void depth_273(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_274();
}
__attribute__((noinline)) static void depth_274(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_275();
}
__attribute__((noinline)) static void depth_275(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_276();
}
__attribute__((noinline)) static void depth_276(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_277();
}
__attribute__((noinline)) static void depth_277(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_278();
}
__attribute__((noinline)) static void depth_278(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_279();
}
__attribute__((noinline)) static void depth_279(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_280();
}
__attribute__((noinline)) static void depth_280(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_281();
}
__attribute__((noinline)) static void depth_281(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_282();
}
__attribute__((noinline)) static void depth_282(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_283();
}
__attribute__((noinline)) static void depth_283(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_284();
}
__attribute__((noinline)) static void depth_284(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_285();
}
__attribute__((noinline)) static void depth_285(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_286();
}
__attribute__((noinline)) static void depth_286(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_287();
}
__attribute__((noinline)) static void depth_287(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_288();
}
__attribute__((noinline)) static void depth_288(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_289();
}
__attribute__((noinline)) static void depth_289(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_290();
}
__attribute__((noinline)) static void depth_290(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_291();
}
__attribute__((noinline)) static void depth_291(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_292();
}
__attribute__((noinline)) static void depth_292(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_293();
}
__attribute__((noinline)) static void depth_293(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_294();
}
__attribute__((noinline)) static void depth_294(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_295();
}
__attribute__((noinline)) static void depth_295(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_296();
}
__attribute__((noinline)) static void depth_296(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_297();
}
__attribute__((noinline)) static void depth_297(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_298();
}
__attribute__((noinline)) static void depth_298(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_299();
}
__attribute__((noinline)) static void depth_299(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_300();
}
__attribute__((noinline)) static void depth_300(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_301();
}
__attribute__((noinline)) static void depth_301(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_302();
}
__attribute__((noinline)) static void depth_302(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_303();
}
__attribute__((noinline)) static void depth_303(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_304();
}
__attribute__((noinline)) static void depth_304(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_305();
}
__attribute__((noinline)) static void depth_305(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_306();
}
__attribute__((noinline)) static void depth_306(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_307();
}
__attribute__((noinline)) static void depth_307(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_308();
}
__attribute__((noinline)) static void depth_308(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_309();
}
__attribute__((noinline)) static void depth_309(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_310();
}
__attribute__((noinline)) static void depth_310(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_311();
}
__attribute__((noinline)) static void depth_311(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_312();
}
__attribute__((noinline)) static void depth_312(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_313();
}
__attribute__((noinline)) static void depth_313(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_314();
}
__attribute__((noinline)) static void depth_314(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_315();
}
__attribute__((noinline)) static void depth_315(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_316();
}
__attribute__((noinline)) static void depth_316(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_317();
}
__attribute__((noinline)) static void depth_317(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_318();
}
__attribute__((noinline)) static void depth_318(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_319();
}
__attribute__((noinline)) static void depth_319(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_320();
}
__attribute__((noinline)) static void depth_320(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_321();
}
__attribute__((noinline)) static void depth_321(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_322();
}
__attribute__((noinline)) static void depth_322(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_323();
}
__attribute__((noinline)) static void depth_323(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_324();
}
__attribute__((noinline)) static void depth_324(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_325();
}
__attribute__((noinline)) static void depth_325(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_326();
}
__attribute__((noinline)) static void depth_326(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_327();
}
__attribute__((noinline)) static void depth_327(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_328();
}
__attribute__((noinline)) static void depth_328(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_329();
}
__attribute__((noinline)) static void depth_329(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_330();
}
__attribute__((noinline)) static void depth_330(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_331();
}
__attribute__((noinline)) static void depth_331(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_332();
}
__attribute__((noinline)) static void depth_332(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_333();
}
__attribute__((noinline)) static void depth_333(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_334();
}
__attribute__((noinline)) static void depth_334(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_335();
}
__attribute__((noinline)) static void depth_335(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_336();
}
__attribute__((noinline)) static void depth_336(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_337();
}
__attribute__((noinline)) static void depth_337(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_338();
}
__attribute__((noinline)) static void depth_338(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_339();
}
__attribute__((noinline)) static void depth_339(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_340();
}
__attribute__((noinline)) static void depth_340(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_341();
}
__attribute__((noinline)) static void depth_341(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_342();
}
__attribute__((noinline)) static void depth_342(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_343();
}
__attribute__((noinline)) static void depth_343(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_344();
}
__attribute__((noinline)) static void depth_344(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_345();
}
__attribute__((noinline)) static void depth_345(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_346();
}
__attribute__((noinline)) static void depth_346(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_347();
}
__attribute__((noinline)) static void depth_347(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_348();
}
__attribute__((noinline)) static void depth_348(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_349();
}
__attribute__((noinline)) static void depth_349(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_350();
}
__attribute__((noinline)) static void depth_350(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_351();
}
__attribute__((noinline)) static void depth_351(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_352();
}
__attribute__((noinline)) static void depth_352(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_353();
}
__attribute__((noinline)) static void depth_353(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_354();
}
__attribute__((noinline)) static void depth_354(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_355();
}
__attribute__((noinline)) static void depth_355(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_356();
}
__attribute__((noinline)) static void depth_356(void) {
  int scratch[1];
  USE(&scratch[0]);
  depth_357();
}
__attribute__((noinline)) static void depth_357(void) {
  int scratch[1];
  USE(&scratch[0]);
  tail_use_after_return();
}

int main(void) {
  g_dangling = buggy();
  depth_0();
  return 0;
// CHECK: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
// CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
// CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
// CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
// CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
// CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
}
