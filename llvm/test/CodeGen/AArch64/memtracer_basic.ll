; REQUIRES: aarch64-registered-target

; Check standard O0 DWARF output
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj %s -O0 -o %t_O0.o
; RUN: ld.lld -shared %t_O0.o -o %t_O0.o
; RUN: llvm-dwarfdump --mem_tracer %t_O0.o | FileCheck %s --check-prefix=CHECK

; Check independent sections with -ffunction-sections
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj -O0 %s -function-sections -o %t_ffunc.o
; RUN: llvm-readelf -S %t_ffunc.o | FileCheck %s --check-prefix=CHECK-FUNC

; Check remains valid after function-section splitting
; RUN: ld.lld -shared %t_ffunc.o -o %t_ffunc.so
; RUN: llvm-dwarfdump --mem_tracer %t_ffunc.so | FileCheck %s --check-prefix=CHECK


; CHECK: .mem_tracer contents:
; CHECK-NEXT:  [   0] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="input" type="char*"
; CHECK-NEXT:  [   1] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT:  [   2] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT:  [   3] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT:  [   4] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT:  [   5] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT:  [   6] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT:  [   7] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT:  [   8] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT:  [   9] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT:  [  10] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT:  [  11] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT:  [  12] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="result" type="char*"
; CHECK-NEXT:  [  13] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="atomic_ptr" type="char*"

; CHECK-FUNC-COUNT-5: ] .mem_tracer

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

@g_global_ptr = global ptr null, align 8, !dbg !0
@g_global_val = global i32 0, align 4, !dbg !10
@__const.main.data = private unnamed_addr constant [8 x i8] c"\01\02\03\04\05\06\07\08", align 1

; Function Attrs: mustprogress noinline optnone
define noundef ptr @_Z17test_complex_flowPci(ptr noundef %0, i32 noundef %1) #0 !dbg !213 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8, !memtracer !217
  call void @llvm.dbg.declare(metadata ptr %3, metadata !218, metadata !DIExpression()), !dbg !219
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !220, metadata !DIExpression()), !dbg !221
  call void @llvm.dbg.declare(metadata ptr %5, metadata !222, metadata !DIExpression()), !dbg !223
  %7 = call ptr @malloc(i64 noundef 8) #6, !dbg !224, !memtracer !225
  store ptr %7, ptr %5, align 8, !dbg !223, !memtracer !225
  call void @llvm.dbg.declare(metadata ptr %6, metadata !226, metadata !DIExpression()), !dbg !227
  %8 = call ptr @malloc(i64 noundef 8) #6, !dbg !228, !memtracer !229
  store ptr %8, ptr %6, align 8, !dbg !227, !memtracer !229
  %9 = load ptr, ptr %3, align 8, !dbg !230
  %10 = getelementptr inbounds i8, ptr %9, i64 0, !dbg !230
  %11 = load i8, ptr %10, align 1, !dbg !230
  %12 = load ptr, ptr %5, align 8, !dbg !231
  %13 = getelementptr inbounds i8, ptr %12, i64 0, !dbg !231
  store i8 %11, ptr %13, align 1, !dbg !232
  %14 = load ptr, ptr %3, align 8, !dbg !233
  %15 = getelementptr inbounds i8, ptr %14, i64 1, !dbg !233
  %16 = load i8, ptr %15, align 1, !dbg !233
  %17 = load ptr, ptr %5, align 8, !dbg !234
  %18 = getelementptr inbounds i8, ptr %17, i64 1, !dbg !234
  store i8 %16, ptr %18, align 1, !dbg !235
  %19 = load ptr, ptr %3, align 8, !dbg !236
  %20 = getelementptr inbounds i8, ptr %19, i64 2, !dbg !236
  %21 = load i8, ptr %20, align 1, !dbg !236
  %22 = load ptr, ptr %6, align 8, !dbg !237
  %23 = getelementptr inbounds i8, ptr %22, i64 0, !dbg !237
  store i8 %21, ptr %23, align 1, !dbg !238
  %24 = load ptr, ptr %3, align 8, !dbg !239
  %25 = getelementptr inbounds i8, ptr %24, i64 3, !dbg !239
  %26 = load i8, ptr %25, align 1, !dbg !239
  %27 = load ptr, ptr %6, align 8, !dbg !240
  %28 = getelementptr inbounds i8, ptr %27, i64 1, !dbg !240
  store i8 %26, ptr %28, align 1, !dbg !241
  %29 = load ptr, ptr %5, align 8, !dbg !242
  store ptr %29, ptr @g_global_ptr, align 8, !dbg !243, !memtracer !244
  %30 = load i32, ptr %4, align 4, !dbg !245
  %31 = icmp sgt i32 %30, 0, !dbg !246
  br i1 %31, label %32, label %34, !dbg !247

32:                                               ; preds = %2
  %33 = load ptr, ptr %5, align 8, !dbg !248
  br label %36, !dbg !247

34:                                               ; preds = %2
  %35 = load ptr, ptr %6, align 8, !dbg !249
  br label %36, !dbg !247

36:                                               ; preds = %34, %32
  %37 = phi ptr [ %33, %32 ], [ %35, %34 ], !dbg !247
  ret ptr %37, !dbg !250
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #2

; Function Attrs: mustprogress noinline nounwind optnone
define void @_Z17test_atomic_storePVcc(ptr noundef %0, i8 noundef %1) #3 !dbg !251 {
  %3 = alloca ptr, align 8
  %4 = alloca i8, align 1
  %5 = alloca i8, align 1
  store ptr %0, ptr %3, align 8, !memtracer !254
  call void @llvm.dbg.declare(metadata ptr %3, metadata !255, metadata !DIExpression()), !dbg !256
  store i8 %1, ptr %4, align 1
  call void @llvm.dbg.declare(metadata ptr %4, metadata !257, metadata !DIExpression()), !dbg !258
  %6 = load ptr, ptr %3, align 8, !dbg !259
  %7 = load i8, ptr %4, align 1, !dbg !260
  store i8 %7, ptr %5, align 1, !dbg !261
  %8 = load i8, ptr %5, align 1, !dbg !261
  store atomic volatile i8 %8, ptr %6 seq_cst, align 1, !dbg !261
  ret void, !dbg !262
}

; Function Attrs: mustprogress noinline nounwind optnone
define void @_Z15test_inline_asmPc(ptr noundef %0) #3 !dbg !263 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8, !memtracer !254
  call void @llvm.dbg.declare(metadata ptr %2, metadata !266, metadata !DIExpression()), !dbg !267
  %3 = load ptr, ptr %2, align 8, !dbg !268
  call void asm sideeffect "str ${0:w}, [$1]", "r,r"(i32 42, ptr %3) #7, !dbg !269, !srcloc !270
  ret void, !dbg !271
}

; Function Attrs: mustprogress noinline optnone
define noundef ptr @_Z20test_noinline_mallocv() #0 !dbg !272 {
  %1 = call ptr @malloc(i64 noundef 64) #6, !dbg !275
  ret ptr %1, !dbg !276
}

; Function Attrs: mustprogress noinline nounwind optnone
define void @_Z19test_noinline_storePcc(ptr noundef %0, i8 noundef %1) #3 !dbg !277 {
  %3 = alloca ptr, align 8
  %4 = alloca i8, align 1
  store ptr %0, ptr %3, align 8, !memtracer !254
  call void @llvm.dbg.declare(metadata ptr %3, metadata !280, metadata !DIExpression()), !dbg !281
  store i8 %1, ptr %4, align 1
  call void @llvm.dbg.declare(metadata ptr %4, metadata !282, metadata !DIExpression()), !dbg !283
  %5 = load i8, ptr %4, align 1, !dbg !284
  %6 = load ptr, ptr %3, align 8, !dbg !285
  store i8 %5, ptr %6, align 1, !dbg !286
  ret void, !dbg !287
}

; Function Attrs: mustprogress noinline norecurse optnone
define noundef i32 @main() #4 !dbg !288 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca [8 x i8], align 1
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  call void @llvm.dbg.declare(metadata ptr %2, metadata !289, metadata !DIExpression()), !dbg !290
  %7 = call noundef ptr @_Z20test_noinline_mallocv() #8, !dbg !291
  store ptr %7, ptr %2, align 8, !dbg !290, !memtracer !225
  call void @llvm.dbg.declare(metadata ptr %3, metadata !292, metadata !DIExpression()), !dbg !293
  %8 = call noundef ptr @_Z20test_noinline_mallocv() #8, !dbg !294
  store ptr %8, ptr %3, align 8, !dbg !293, !memtracer !229
  %9 = load ptr, ptr %2, align 8, !dbg !295
  %10 = getelementptr inbounds i8, ptr %9, i64 0, !dbg !295
  store i8 1, ptr %10, align 1, !dbg !296
  %11 = load ptr, ptr %2, align 8, !dbg !297
  %12 = getelementptr inbounds i8, ptr %11, i64 1, !dbg !297
  store i8 2, ptr %12, align 1, !dbg !298
  %13 = load ptr, ptr %2, align 8, !dbg !299
  %14 = getelementptr inbounds i8, ptr %13, i64 2, !dbg !299
  store i8 3, ptr %14, align 1, !dbg !300
  %15 = load ptr, ptr %2, align 8, !dbg !301
  %16 = getelementptr inbounds i8, ptr %15, i64 3, !dbg !301
  store i8 4, ptr %16, align 1, !dbg !302
  %17 = load ptr, ptr %2, align 8, !dbg !303
  store ptr %17, ptr @g_global_ptr, align 8, !dbg !304, !memtracer !244
  call void @llvm.dbg.declare(metadata ptr %4, metadata !305, metadata !DIExpression()), !dbg !309
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %4, ptr align 1 @__const.main.data, i64 8, i1 false), !dbg !309
  call void @llvm.dbg.declare(metadata ptr %5, metadata !310, metadata !DIExpression()), !dbg !311
  %18 = getelementptr inbounds [8 x i8], ptr %4, i64 0, i64 0, !dbg !312
  %19 = call noundef ptr @_Z17test_complex_flowPci(ptr noundef %18, i32 noundef 5) #8, !dbg !313
  store ptr %19, ptr %5, align 8, !dbg !311, !memtracer !314
  call void @llvm.dbg.declare(metadata ptr %6, metadata !315, metadata !DIExpression()), !dbg !316
  %20 = load ptr, ptr %3, align 8, !dbg !317
  store ptr %20, ptr %6, align 8, !dbg !316, !memtracer !318
  %21 = load ptr, ptr %6, align 8, !dbg !319
  call void @_Z17test_atomic_storePVcc(ptr noundef %21, i8 noundef 100) #8, !dbg !320
  %22 = load ptr, ptr %2, align 8, !dbg !321
  call void @_Z15test_inline_asmPc(ptr noundef %22) #8, !dbg !322
  %23 = load ptr, ptr %5, align 8, !dbg !323
  %24 = icmp ne ptr %23, null, !dbg !323
  br i1 %24, label %25, label %30, !dbg !323

25:                                               ; preds = %0
  %26 = load ptr, ptr %5, align 8, !dbg !324
  %27 = getelementptr inbounds i8, ptr %26, i64 0, !dbg !324
  %28 = load i8, ptr %27, align 1, !dbg !324
  %29 = zext i8 %28 to i32, !dbg !324
  br label %31, !dbg !323

30:                                               ; preds = %0
  br label %31, !dbg !323

31:                                               ; preds = %30, %25
  %32 = phi i32 [ %29, %25 ], [ 0, %30 ], !dbg !323
  store i32 %32, ptr @g_global_val, align 4, !dbg !325
  ret i32 0, !dbg !326
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #5

attributes #0 = { mustprogress noinline optnone "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #3 = { mustprogress noinline nounwind optnone "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #4 = { mustprogress noinline norecurse optnone "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #5 = { argmemonly nocallback nofree nounwind willreturn }
attributes #6 = { allocsize(0) "reference-tracking"="true" }
attributes #7 = { nounwind }
attributes #8 = { "reference-tracking"="true" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!206, !207, !208, !209, !210, !211}
!llvm.ident = !{!212}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_global_ptr", scope: !2, file: !3, line: 3, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 15.0.4", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !9, imports: !13, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "test.cpp", directory: "", checksumkind: CSK_MD5, checksum: "1995df23861b38a26ec07c259d995743")
!4 = !{!5, !7}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!6 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_unsigned_char)
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!8 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !6)
!9 = !{!0, !10}
!10 = !DIGlobalVariableExpression(var: !11, expr: !DIExpression())
!11 = distinct !DIGlobalVariable(name: "g_global_val", scope: !2, file: !3, line: 4, type: !12, isLocal: false, isDefinition: true)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !21, !25, !32, !39, !46, !50, !54, !58, !65, !70, !75, !79, !83, !87, !92, !96, !101, !106, !110, !114, !118, !122, !127, !131, !133, !137, !139, !148, !152, !157, !161, !165, !169, !173, !175, !179, !186, !190, !194, !202, !204}
!14 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !17, file: !20, line: 94)
!15 = !DINamespace(name: "__h", scope: !16, exportSymbols: true)
!16 = !DINamespace(name: "std", scope: null)
!17 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !18, line: 58, baseType: !19)
!18 = !DIFile(filename: "alltypes.h", directory: "", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!19 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!20 = !DIFile(filename: "cstdlib", directory: "")
!21 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !22, file: !20, line: 95)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !23, line: 65, baseType: !24)
!23 = !DIFile(filename: "stdlib.h", directory: "", checksumkind: CSK_MD5, checksum: "4ae56b2feb06fe30283b2148e55e1d18")
!24 = !DICompositeType(tag: DW_TAG_structure_type, file: !23, line: 65, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!25 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !26, file: !20, line: 96)
!26 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !23, line: 66, baseType: !27)
!27 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !23, line: 66, size: 128, flags: DIFlagTypePassByValue, elements: !28, identifier: "_ZTS6ldiv_t")
!28 = !{!29, !31}
!29 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !27, file: !23, line: 66, baseType: !30, size: 64)
!30 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !27, file: !23, line: 66, baseType: !30, size: 64, offset: 64)
!32 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !33, file: !20, line: 97)
!33 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !23, line: 67, baseType: !34)
!34 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !23, line: 67, size: 128, flags: DIFlagTypePassByValue, elements: !35, identifier: "_ZTS7lldiv_t")
!35 = !{!36, !38}
!36 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !34, file: !23, line: 67, baseType: !37, size: 64)
!37 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!38 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !34, file: !23, line: 67, baseType: !37, size: 64, offset: 64)
!39 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !40, file: !20, line: 98)
!40 = !DISubprogram(name: "atof", scope: !23, file: !23, line: 26, type: !41, flags: DIFlagPrototyped, spFlags: 0)
!41 = !DISubroutineType(types: !42)
!42 = !{!43, !44}
!43 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !45, size: 64)
!45 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!46 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !47, file: !20, line: 99)
!47 = !DISubprogram(name: "atoi", scope: !23, file: !23, line: 23, type: !48, flags: DIFlagPrototyped, spFlags: 0)
!48 = !DISubroutineType(types: !49)
!49 = !{!12, !44}
!50 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !51, file: !20, line: 100)
!51 = !DISubprogram(name: "atol", scope: !23, file: !23, line: 24, type: !52, flags: DIFlagPrototyped, spFlags: 0)
!52 = !DISubroutineType(types: !53)
!53 = !{!30, !44}
!54 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !55, file: !20, line: 101)
!55 = !DISubprogram(name: "atoll", scope: !23, file: !23, line: 25, type: !56, flags: DIFlagPrototyped, spFlags: 0)
!56 = !DISubroutineType(types: !57)
!57 = !{!37, !44}
!58 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !59, file: !20, line: 102)
!59 = !DISubprogram(name: "strtod", scope: !23, file: !23, line: 29, type: !60, flags: DIFlagPrototyped, spFlags: 0)
!60 = !DISubroutineType(types: !61)
!61 = !{!43, !62, !63}
!62 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !44)
!63 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !64)
!64 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!65 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !66, file: !20, line: 103)
!66 = !DISubprogram(name: "strtof", scope: !23, file: !23, line: 28, type: !67, flags: DIFlagPrototyped, spFlags: 0)
!67 = !DISubroutineType(types: !68)
!68 = !{!69, !62, !63}
!69 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!70 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !71, file: !20, line: 104)
!71 = !DISubprogram(name: "strtold", scope: !23, file: !23, line: 30, type: !72, flags: DIFlagPrototyped, spFlags: 0)
!72 = !DISubroutineType(types: !73)
!73 = !{!74, !62, !63}
!74 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!75 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !76, file: !20, line: 105)
!76 = !DISubprogram(name: "strtol", scope: !23, file: !23, line: 32, type: !77, flags: DIFlagPrototyped, spFlags: 0)
!77 = !DISubroutineType(types: !78)
!78 = !{!30, !62, !63, !12}
!79 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !80, file: !20, line: 106)
!80 = !DISubprogram(name: "strtoll", scope: !23, file: !23, line: 34, type: !81, flags: DIFlagPrototyped, spFlags: 0)
!81 = !DISubroutineType(types: !82)
!82 = !{!37, !62, !63, !12}
!83 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !84, file: !20, line: 107)
!84 = !DISubprogram(name: "strtoul", scope: !23, file: !23, line: 33, type: !85, flags: DIFlagPrototyped, spFlags: 0)
!85 = !DISubroutineType(types: !86)
!86 = !{!19, !62, !63, !12}
!87 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !88, file: !20, line: 108)
!88 = !DISubprogram(name: "strtoull", scope: !23, file: !23, line: 35, type: !89, flags: DIFlagPrototyped, spFlags: 0)
!89 = !DISubroutineType(types: !90)
!90 = !{!91, !62, !63, !12}
!91 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!92 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !93, file: !20, line: 109)
!93 = !DISubprogram(name: "rand", scope: !23, file: !23, line: 37, type: !94, flags: DIFlagPrototyped, spFlags: 0)
!94 = !DISubroutineType(types: !95)
!95 = !{!12}
!96 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !97, file: !20, line: 110)
!97 = !DISubprogram(name: "srand", scope: !23, file: !23, line: 38, type: !98, flags: DIFlagPrototyped, spFlags: 0)
!98 = !DISubroutineType(types: !99)
!99 = !{null, !100}
!100 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!101 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !102, file: !20, line: 111)
!102 = !DISubprogram(name: "calloc", scope: !23, file: !23, line: 41, type: !103, flags: DIFlagPrototyped, spFlags: 0)
!103 = !DISubroutineType(types: !104)
!104 = !{!105, !17, !17}
!105 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!106 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !107, file: !20, line: 112)
!107 = !DISubprogram(name: "free", scope: !23, file: !23, line: 43, type: !108, flags: DIFlagPrototyped, spFlags: 0)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !105}
!110 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !111, file: !20, line: 113)
!111 = !DISubprogram(name: "malloc", scope: !23, file: !23, line: 40, type: !112, flags: DIFlagPrototyped, spFlags: 0)
!112 = !DISubroutineType(types: !113)
!113 = !{!105, !17}
!114 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !115, file: !20, line: 114)
!115 = !DISubprogram(name: "realloc", scope: !23, file: !23, line: 42, type: !116, flags: DIFlagPrototyped, spFlags: 0)
!116 = !DISubroutineType(types: !117)
!117 = !{!105, !105, !17}
!118 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !119, file: !20, line: 115)
!119 = !DISubprogram(name: "abort", scope: !23, file: !23, line: 46, type: !120, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!120 = !DISubroutineType(types: !121)
!121 = !{null}
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !123, file: !20, line: 116)
!123 = !DISubprogram(name: "atexit", scope: !23, file: !23, line: 48, type: !124, flags: DIFlagPrototyped, spFlags: 0)
!124 = !DISubroutineType(types: !125)
!125 = !{!12, !126}
!126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !120, size: 64)
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !128, file: !20, line: 117)
!128 = !DISubprogram(name: "exit", scope: !23, file: !23, line: 49, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!129 = !DISubroutineType(types: !130)
!130 = !{null, !12}
!131 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !132, file: !20, line: 118)
!132 = !DISubprogram(name: "_Exit", scope: !23, file: !23, line: 50, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!133 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !134, file: !20, line: 119)
!134 = !DISubprogram(name: "getenv", scope: !23, file: !23, line: 54, type: !135, flags: DIFlagPrototyped, spFlags: 0)
!135 = !DISubroutineType(types: !136)
!136 = !{!5, !44}
!137 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !138, file: !20, line: 120)
!138 = !DISubprogram(name: "system", scope: !23, file: !23, line: 56, type: !48, flags: DIFlagPrototyped, spFlags: 0)
!139 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !140, file: !20, line: 121)
!140 = !DISubprogram(name: "bsearch", scope: !23, file: !23, line: 58, type: !141, flags: DIFlagPrototyped, spFlags: 0)
!141 = !DISubroutineType(types: !142)
!142 = !{!105, !143, !143, !17, !17, !145}
!143 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !144, size: 64)
!144 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!145 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !146, size: 64)
!146 = !DISubroutineType(types: !147)
!147 = !{!12, !143, !143}
!148 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !149, file: !20, line: 122)
!149 = !DISubprogram(name: "qsort", scope: !23, file: !23, line: 59, type: !150, flags: DIFlagPrototyped, spFlags: 0)
!150 = !DISubroutineType(types: !151)
!151 = !{null, !105, !17, !17, !145}
!152 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !153, file: !20, line: 123)
!153 = !DISubprogram(name: "abs", linkageName: "_Z3absB6v15004e", scope: !154, file: !154, line: 129, type: !155, flags: DIFlagPrototyped, spFlags: 0)
!154 = !DIFile(filename: "stdlib.h", directory: "")
!155 = !DISubroutineType(types: !156)
!156 = !{!74, !74}
!157 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !158, file: !20, line: 124)
!158 = !DISubprogram(name: "labs", scope: !23, file: !23, line: 62, type: !159, flags: DIFlagPrototyped, spFlags: 0)
!159 = !DISubroutineType(types: !160)
!160 = !{!30, !30}
!161 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !162, file: !20, line: 125)
!162 = !DISubprogram(name: "llabs", scope: !23, file: !23, line: 63, type: !163, flags: DIFlagPrototyped, spFlags: 0)
!163 = !DISubroutineType(types: !164)
!164 = !{!37, !37}
!165 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !166, file: !20, line: 126)
!166 = !DISubprogram(name: "div", linkageName: "_Z3divB6v15004xx", scope: !154, file: !154, line: 152, type: !167, flags: DIFlagPrototyped, spFlags: 0)
!167 = !DISubroutineType(types: !168)
!168 = !{!33, !37, !37}
!169 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !170, file: !20, line: 127)
!170 = !DISubprogram(name: "ldiv", scope: !23, file: !23, line: 70, type: !171, flags: DIFlagPrototyped, spFlags: 0)
!171 = !DISubroutineType(types: !172)
!172 = !{!26, !30, !30}
!173 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !174, file: !20, line: 128)
!174 = !DISubprogram(name: "lldiv", scope: !23, file: !23, line: 71, type: !167, flags: DIFlagPrototyped, spFlags: 0)
!175 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !176, file: !20, line: 129)
!176 = !DISubprogram(name: "mblen", scope: !23, file: !23, line: 73, type: !177, flags: DIFlagPrototyped, spFlags: 0)
!177 = !DISubroutineType(types: !178)
!178 = !{!12, !44, !17}
!179 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !180, file: !20, line: 130)
!180 = !DISubprogram(name: "mbtowc", scope: !23, file: !23, line: 74, type: !181, flags: DIFlagPrototyped, spFlags: 0)
!181 = !DISubroutineType(types: !182)
!182 = !{!12, !183, !62, !17}
!183 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !184)
!184 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !185, size: 64)
!185 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!186 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !187, file: !20, line: 131)
!187 = !DISubprogram(name: "wctomb", scope: !23, file: !23, line: 75, type: !188, flags: DIFlagPrototyped, spFlags: 0)
!188 = !DISubroutineType(types: !189)
!189 = !{!12, !5, !185}
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !191, file: !20, line: 132)
!191 = !DISubprogram(name: "mbstowcs", scope: !23, file: !23, line: 76, type: !192, flags: DIFlagPrototyped, spFlags: 0)
!192 = !DISubroutineType(types: !193)
!193 = !{!17, !183, !62, !17}
!194 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !195, file: !20, line: 133)
!195 = !DISubprogram(name: "wcstombs", scope: !23, file: !23, line: 77, type: !196, flags: DIFlagPrototyped, spFlags: 0)
!196 = !DISubroutineType(types: !197)
!197 = !{!17, !198, !199, !17}
!198 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !5)
!199 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !200)
!200 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !201, size: 64)
!201 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !185)
!202 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !203, file: !20, line: 135)
!203 = !DISubprogram(name: "at_quick_exit", scope: !23, file: !23, line: 51, type: !124, flags: DIFlagPrototyped, spFlags: 0)
!204 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !205, file: !20, line: 136)
!205 = !DISubprogram(name: "quick_exit", scope: !23, file: !23, line: 52, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!206 = !{i32 7, !"Dwarf Version", i32 5}
!207 = !{i32 7, !"ReferenceTracking", i32 1}
!208 = !{i32 2, !"Debug Info Version", i32 3}
!209 = !{i32 1, !"wchar_size", i32 4}
!210 = !{i32 7, !"PIC Level", i32 2}
!211 = !{i32 7, !"frame-pointer", i32 1}
!212 = !{!"clang version 15.0.4"}
!213 = distinct !DISubprogram(name: "test_complex_flow", linkageName: "_Z17test_complex_flowPci", scope: !3, file: !3, line: 6, type: !214, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!214 = !DISubroutineType(types: !215)
!215 = !{!5, !5, !12}
!216 = !{}
!217 = !{!"input", !"char*"}
!218 = !DILocalVariable(name: "input", arg: 1, scope: !213, file: !3, line: 6, type: !5)
!219 = !DILocation(line: 6, column: 31, scope: !213)
!220 = !DILocalVariable(name: "n", arg: 2, scope: !213, file: !3, line: 6, type: !12)
!221 = !DILocation(line: 6, column: 42, scope: !213)
!222 = !DILocalVariable(name: "p1", scope: !213, file: !3, line: 7, type: !5)
!223 = !DILocation(line: 7, column: 11, scope: !213)
!224 = !DILocation(line: 7, column: 23, scope: !213)
!225 = !{!"p1", !"char*"}
!226 = !DILocalVariable(name: "p2", scope: !213, file: !3, line: 8, type: !5)
!227 = !DILocation(line: 8, column: 11, scope: !213)
!228 = !DILocation(line: 8, column: 23, scope: !213)
!229 = !{!"p2", !"char*"}
!230 = !DILocation(line: 10, column: 13, scope: !213)
!231 = !DILocation(line: 10, column: 5, scope: !213)
!232 = !DILocation(line: 10, column: 11, scope: !213)
!233 = !DILocation(line: 11, column: 13, scope: !213)
!234 = !DILocation(line: 11, column: 5, scope: !213)
!235 = !DILocation(line: 11, column: 11, scope: !213)
!236 = !DILocation(line: 12, column: 13, scope: !213)
!237 = !DILocation(line: 12, column: 5, scope: !213)
!238 = !DILocation(line: 12, column: 11, scope: !213)
!239 = !DILocation(line: 13, column: 13, scope: !213)
!240 = !DILocation(line: 13, column: 5, scope: !213)
!241 = !DILocation(line: 13, column: 11, scope: !213)
!242 = !DILocation(line: 15, column: 20, scope: !213)
!243 = !DILocation(line: 15, column: 18, scope: !213)
!244 = !{!"g_global_ptr", !"char*"}
!245 = !DILocation(line: 17, column: 13, scope: !213)
!246 = !DILocation(line: 17, column: 15, scope: !213)
!247 = !DILocation(line: 17, column: 12, scope: !213)
!248 = !DILocation(line: 17, column: 22, scope: !213)
!249 = !DILocation(line: 17, column: 27, scope: !213)
!250 = !DILocation(line: 17, column: 5, scope: !213)
!251 = distinct !DISubprogram(name: "test_atomic_store", linkageName: "_Z17test_atomic_storePVcc", scope: !3, file: !3, line: 20, type: !252, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!252 = !DISubroutineType(types: !253)
!253 = !{null, !7, !6}
!254 = !{!"ptr", !"char*"}
!255 = !DILocalVariable(name: "ptr", arg: 1, scope: !251, file: !3, line: 20, type: !7)
!256 = !DILocation(line: 20, column: 39, scope: !251)
!257 = !DILocalVariable(name: "val", arg: 2, scope: !251, file: !3, line: 20, type: !6)
!258 = !DILocation(line: 20, column: 49, scope: !251)
!259 = !DILocation(line: 21, column: 22, scope: !251)
!260 = !DILocation(line: 21, column: 27, scope: !251)
!261 = !DILocation(line: 21, column: 5, scope: !251)
!262 = !DILocation(line: 22, column: 1, scope: !251)
!263 = distinct !DISubprogram(name: "test_inline_asm", linkageName: "_Z15test_inline_asmPc", scope: !3, file: !3, line: 24, type: !264, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!264 = !DISubroutineType(types: !265)
!265 = !{null, !5}
!266 = !DILocalVariable(name: "ptr", arg: 1, scope: !263, file: !3, line: 24, type: !5)
!267 = !DILocation(line: 24, column: 28, scope: !263)
!268 = !DILocation(line: 25, column: 51, scope: !263)
!269 = !DILocation(line: 25, column: 5, scope: !263)
!270 = !{i64 536}
!271 = !DILocation(line: 26, column: 1, scope: !263)
!272 = distinct !DISubprogram(name: "test_noinline_malloc", linkageName: "_Z20test_noinline_mallocv", scope: !3, file: !3, line: 28, type: !273, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!273 = !DISubroutineType(types: !274)
!274 = !{!5}
!275 = !DILocation(line: 29, column: 19, scope: !272)
!276 = !DILocation(line: 29, column: 5, scope: !272)
!277 = distinct !DISubprogram(name: "test_noinline_store", linkageName: "_Z19test_noinline_storePcc", scope: !3, file: !3, line: 33, type: !278, scopeLine: 33, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!278 = !DISubroutineType(types: !279)
!279 = !{null, !5, !6}
!280 = !DILocalVariable(name: "ptr", arg: 1, scope: !277, file: !3, line: 33, type: !5)
!281 = !DILocation(line: 33, column: 32, scope: !277)
!282 = !DILocalVariable(name: "val", arg: 2, scope: !277, file: !3, line: 33, type: !6)
!283 = !DILocation(line: 33, column: 42, scope: !277)
!284 = !DILocation(line: 34, column: 12, scope: !277)
!285 = !DILocation(line: 34, column: 6, scope: !277)
!286 = !DILocation(line: 34, column: 10, scope: !277)
!287 = !DILocation(line: 35, column: 1, scope: !277)
!288 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 37, type: !94, scopeLine: 37, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !216)
!289 = !DILocalVariable(name: "p1", scope: !288, file: !3, line: 39, type: !5)
!290 = !DILocation(line: 39, column: 11, scope: !288)
!291 = !DILocation(line: 39, column: 16, scope: !288)
!292 = !DILocalVariable(name: "p2", scope: !288, file: !3, line: 40, type: !5)
!293 = !DILocation(line: 40, column: 11, scope: !288)
!294 = !DILocation(line: 40, column: 16, scope: !288)
!295 = !DILocation(line: 43, column: 5, scope: !288)
!296 = !DILocation(line: 43, column: 11, scope: !288)
!297 = !DILocation(line: 44, column: 5, scope: !288)
!298 = !DILocation(line: 44, column: 11, scope: !288)
!299 = !DILocation(line: 45, column: 5, scope: !288)
!300 = !DILocation(line: 45, column: 11, scope: !288)
!301 = !DILocation(line: 46, column: 5, scope: !288)
!302 = !DILocation(line: 46, column: 11, scope: !288)
!303 = !DILocation(line: 49, column: 20, scope: !288)
!304 = !DILocation(line: 49, column: 18, scope: !288)
!305 = !DILocalVariable(name: "data", scope: !288, file: !3, line: 52, type: !306)
!306 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64, elements: !307)
!307 = !{!308}
!308 = !DISubrange(count: 8)
!309 = !DILocation(line: 52, column: 10, scope: !288)
!310 = !DILocalVariable(name: "result", scope: !288, file: !3, line: 53, type: !5)
!311 = !DILocation(line: 53, column: 11, scope: !288)
!312 = !DILocation(line: 53, column: 38, scope: !288)
!313 = !DILocation(line: 53, column: 20, scope: !288)
!314 = !{!"result", !"char*"}
!315 = !DILocalVariable(name: "atomic_ptr", scope: !288, file: !3, line: 56, type: !7)
!316 = !DILocation(line: 56, column: 20, scope: !288)
!317 = !DILocation(line: 56, column: 49, scope: !288)
!318 = !{!"atomic_ptr", !"char*"}
!319 = !DILocation(line: 57, column: 23, scope: !288)
!320 = !DILocation(line: 57, column: 5, scope: !288)
!321 = !DILocation(line: 60, column: 21, scope: !288)
!322 = !DILocation(line: 60, column: 5, scope: !288)
!323 = !DILocation(line: 63, column: 20, scope: !288)
!324 = !DILocation(line: 63, column: 29, scope: !288)
!325 = !DILocation(line: 63, column: 18, scope: !288)
!326 = !DILocation(line: 65, column: 5, scope: !288)