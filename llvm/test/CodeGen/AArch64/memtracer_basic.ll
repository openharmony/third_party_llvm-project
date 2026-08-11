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
; CHECK-NEXT: [   0] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="input" type="char*"
; CHECK-NEXT: [   1] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT: [   2] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT: [   3] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT: [   4] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT: [   5] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT: [   6] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT: [   7] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT: [   8] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="ptr" type="char*"
; CHECK-NEXT: [   9] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT: [  10] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT: [  11] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT: [  12] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="result" type="char*"
; CHECK-NEXT: [  13] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="atomic_ptr" type="char*"

; CHECK-FUNC-COUNT-5: ] .mem_tracer


; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@g_global_ptr = global ptr null, align 8, !dbg !0
@g_global_val = global i32 0, align 4, !dbg !10
@__const.main.data = private unnamed_addr constant [8 x i8] c"\01\02\03\04\05\06\07\08", align 1

; Function Attrs: mustprogress noinline optnone uwtable
define noundef ptr @_Z17test_complex_flowPci(ptr noundef %0, i32 noundef %1) #0 !dbg !289 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8, !memtracer !291
    #dbg_declare(ptr %3, !292, !DIExpression(), !293)
  store i32 %1, ptr %4, align 4
    #dbg_declare(ptr %4, !294, !DIExpression(), !295)
    #dbg_declare(ptr %5, !296, !DIExpression(), !297)
  %7 = call ptr @malloc(i64 noundef 8) #5, !dbg !298, !memtracer !299
  store ptr %7, ptr %5, align 8, !dbg !297, !memtracer !299
    #dbg_declare(ptr %6, !300, !DIExpression(), !301)
  %8 = call ptr @malloc(i64 noundef 8) #5, !dbg !302, !memtracer !303
  store ptr %8, ptr %6, align 8, !dbg !301, !memtracer !303
  %9 = load ptr, ptr %3, align 8, !dbg !304
  %10 = getelementptr inbounds i8, ptr %9, i64 0, !dbg !304
  %11 = load i8, ptr %10, align 1, !dbg !304
  %12 = load ptr, ptr %5, align 8, !dbg !305
  %13 = getelementptr inbounds i8, ptr %12, i64 0, !dbg !305
  store i8 %11, ptr %13, align 1, !dbg !306
  %14 = load ptr, ptr %3, align 8, !dbg !307
  %15 = getelementptr inbounds i8, ptr %14, i64 1, !dbg !307
  %16 = load i8, ptr %15, align 1, !dbg !307
  %17 = load ptr, ptr %5, align 8, !dbg !308
  %18 = getelementptr inbounds i8, ptr %17, i64 1, !dbg !308
  store i8 %16, ptr %18, align 1, !dbg !309
  %19 = load ptr, ptr %3, align 8, !dbg !310
  %20 = getelementptr inbounds i8, ptr %19, i64 2, !dbg !310
  %21 = load i8, ptr %20, align 1, !dbg !310
  %22 = load ptr, ptr %6, align 8, !dbg !311
  %23 = getelementptr inbounds i8, ptr %22, i64 0, !dbg !311
  store i8 %21, ptr %23, align 1, !dbg !312
  %24 = load ptr, ptr %3, align 8, !dbg !313
  %25 = getelementptr inbounds i8, ptr %24, i64 3, !dbg !313
  %26 = load i8, ptr %25, align 1, !dbg !313
  %27 = load ptr, ptr %6, align 8, !dbg !314
  %28 = getelementptr inbounds i8, ptr %27, i64 1, !dbg !314
  store i8 %26, ptr %28, align 1, !dbg !315
  %29 = load ptr, ptr %5, align 8, !dbg !316
  store ptr %29, ptr @g_global_ptr, align 8, !dbg !317, !memtracer !318
  %30 = load i32, ptr %4, align 4, !dbg !319
  %31 = icmp sgt i32 %30, 0, !dbg !320
  br i1 %31, label %32, label %34, !dbg !319

32:                                               ; preds = %2
  %33 = load ptr, ptr %5, align 8, !dbg !321
  br label %36, !dbg !319

34:                                               ; preds = %2
  %35 = load ptr, ptr %6, align 8, !dbg !322
  br label %36, !dbg !319

36:                                               ; preds = %34, %32
  %37 = phi ptr [ %33, %32 ], [ %35, %34 ], !dbg !319
  ret ptr %37, !dbg !323
}

; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @_Z17test_atomic_storePVcc(ptr noundef %0, i8 noundef %1) #2 !dbg !324 {
  %3 = alloca ptr, align 8
  %4 = alloca i8, align 1
  %5 = alloca i8, align 1
  %6 = alloca i8, align 1
  store ptr %0, ptr %3, align 8, !memtracer !327
    #dbg_declare(ptr %3, !328, !DIExpression(), !329)
  store i8 %1, ptr %4, align 1
    #dbg_declare(ptr %4, !330, !DIExpression(), !331)
    #dbg_declare(ptr %5, !332, !DIExpression(), !333)
  %7 = load i8, ptr %4, align 1, !dbg !334
  store i8 %7, ptr %5, align 1, !dbg !333
  %8 = load ptr, ptr %3, align 8, !dbg !335
  %9 = load i8, ptr %5, align 1, !dbg !336
  store i8 %9, ptr %6, align 1, !dbg !337
  %10 = load i8, ptr %6, align 1, !dbg !337
  store atomic volatile i8 %10, ptr %8 seq_cst, align 1, !dbg !337
  ret void, !dbg !338
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @_Z15test_inline_asmPc(ptr noundef %0) #2 !dbg !339 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8, !memtracer !327
    #dbg_declare(ptr %2, !342, !DIExpression(), !343)
  %3 = load ptr, ptr %2, align 8, !dbg !344
  call void asm sideeffect "str ${0:w}, [$1]", "r,r"(i32 42, ptr %3) #6, !dbg !345, !srcloc !346
  ret void, !dbg !347
}

; Function Attrs: mustprogress noinline optnone uwtable
define noundef ptr @_Z20test_noinline_mallocv() #0 !dbg !348 {
  %1 = call ptr @malloc(i64 noundef 64) #5, !dbg !351
  ret ptr %1, !dbg !352
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @_Z19test_noinline_storePcc(ptr noundef %0, i8 noundef %1) #2 !dbg !353 {
  %3 = alloca ptr, align 8
  %4 = alloca i8, align 1
  store ptr %0, ptr %3, align 8, !memtracer !327
    #dbg_declare(ptr %3, !356, !DIExpression(), !357)
  store i8 %1, ptr %4, align 1
    #dbg_declare(ptr %4, !358, !DIExpression(), !359)
  %5 = load i8, ptr %4, align 1, !dbg !360
  %6 = load ptr, ptr %3, align 8, !dbg !361
  store i8 %5, ptr %6, align 1, !dbg !362
  ret void, !dbg !363
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define noundef i32 @main() #3 !dbg !364 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca [8 x i8], align 1
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !365, !DIExpression(), !366)
  %7 = call noundef ptr @_Z20test_noinline_mallocv() #7, !dbg !367
  store ptr %7, ptr %2, align 8, !dbg !366, !memtracer !299
    #dbg_declare(ptr %3, !368, !DIExpression(), !369)
  %8 = call noundef ptr @_Z20test_noinline_mallocv() #7, !dbg !370
  store ptr %8, ptr %3, align 8, !dbg !369, !memtracer !303
  %9 = load ptr, ptr %2, align 8, !dbg !371
  %10 = getelementptr inbounds i8, ptr %9, i64 0, !dbg !371
  store i8 1, ptr %10, align 1, !dbg !372
  %11 = load ptr, ptr %2, align 8, !dbg !373
  %12 = getelementptr inbounds i8, ptr %11, i64 1, !dbg !373
  store i8 2, ptr %12, align 1, !dbg !374
  %13 = load ptr, ptr %2, align 8, !dbg !375
  %14 = getelementptr inbounds i8, ptr %13, i64 2, !dbg !375
  store i8 3, ptr %14, align 1, !dbg !376
  %15 = load ptr, ptr %2, align 8, !dbg !377
  %16 = getelementptr inbounds i8, ptr %15, i64 3, !dbg !377
  store i8 4, ptr %16, align 1, !dbg !378
  %17 = load ptr, ptr %2, align 8, !dbg !379
  store ptr %17, ptr @g_global_ptr, align 8, !dbg !380, !memtracer !318
    #dbg_declare(ptr %4, !381, !DIExpression(), !385)
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %4, ptr align 1 @__const.main.data, i64 8, i1 false), !dbg !385
    #dbg_declare(ptr %5, !386, !DIExpression(), !387)
  %18 = getelementptr inbounds [8 x i8], ptr %4, i64 0, i64 0, !dbg !388
  %19 = call noundef ptr @_Z17test_complex_flowPci(ptr noundef %18, i32 noundef 5) #7, !dbg !389
  store ptr %19, ptr %5, align 8, !dbg !387, !memtracer !390
    #dbg_declare(ptr %6, !391, !DIExpression(), !392)
  %20 = load ptr, ptr %3, align 8, !dbg !393
  store ptr %20, ptr %6, align 8, !dbg !392, !memtracer !394
  %21 = load ptr, ptr %6, align 8, !dbg !395
  call void @_Z17test_atomic_storePVcc(ptr noundef %21, i8 noundef 100) #7, !dbg !396
  %22 = load ptr, ptr %2, align 8, !dbg !397
  call void @_Z15test_inline_asmPc(ptr noundef %22) #7, !dbg !398
  %23 = load ptr, ptr %5, align 8, !dbg !399
  %24 = icmp ne ptr %23, null, !dbg !399
  br i1 %24, label %25, label %30, !dbg !399

25:                                               ; preds = %0
  %26 = load ptr, ptr %5, align 8, !dbg !400
  %27 = getelementptr inbounds i8, ptr %26, i64 0, !dbg !400
  %28 = load i8, ptr %27, align 1, !dbg !400
  %29 = zext i8 %28 to i32, !dbg !400
  br label %31, !dbg !399

30:                                               ; preds = %0
  br label %31, !dbg !399

31:                                               ; preds = %30, %25
  %32 = phi i32 [ %29, %25 ], [ 0, %30 ], !dbg !399
  store i32 %32, ptr @g_global_val, align 4, !dbg !401
  ret i32 0, !dbg !402
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #4

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #4 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { allocsize(0) "reference-tracking"="true" }
attributes #6 = { nounwind }
attributes #7 = { "reference-tracking"="true" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!281, !282, !283, !284, !285, !286, !287}
!llvm.ident = !{!288}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_global_ptr", scope: !2, file: !12, line: 4, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !9, imports: !14, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/root/mem_map/llvm_test/test.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "9d43e0855688773b83b2021661e6ee0b")
!4 = !{!5, !7}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!6 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_unsigned_char)
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!8 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !6)
!9 = !{!0, !10}
!10 = !DIGlobalVariableExpression(var: !11, expr: !DIExpression())
!11 = distinct !DIGlobalVariable(name: "g_global_val", scope: !2, file: !12, line: 5, type: !13, isLocal: false, isDefinition: true)
!12 = !DIFile(filename: "test.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "9d43e0855688773b83b2021661e6ee0b")
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{!15, !22, !29, !36, !43, !47, !51, !55, !62, !67, !72, !76, !80, !85, !90, !94, !99, !106, !110, !114, !118, !122, !127, !131, !133, !137, !139, !148, !152, !156, !160, !165, !169, !171, !175, !182, !186, !190, !198, !200, !202, !204, !212, !216, !220, !224, !226, !228, !232, !236, !240, !242, !246, !251, !255, !259, !263, !265, !267, !269, !271, !273, !277}
!15 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !18, file: !21, line: 106)
!16 = !DINamespace(name: "__h", scope: !17, exportSymbols: true)
!17 = !DINamespace(name: "std", scope: null)
!18 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !19, line: 67, baseType: !20)
!19 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/stdlib.h", directory: "/root", checksumkind: CSK_MD5, checksum: "5a72a9fe8603a6e9a660970416eead7b")
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !19, line: 67, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!21 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/cstdlib", directory: "/root")
!22 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !23, file: !21, line: 107)
!23 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !19, line: 68, baseType: !24)
!24 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !19, line: 68, size: 128, flags: DIFlagTypePassByValue, elements: !25, identifier: "_ZTS6ldiv_t")
!25 = !{!26, !28}
!26 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !24, file: !19, line: 68, baseType: !27, size: 64)
!27 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !24, file: !19, line: 68, baseType: !27, size: 64, offset: 64)
!29 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !30, file: !21, line: 108)
!30 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !19, line: 69, baseType: !31)
!31 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !19, line: 69, size: 128, flags: DIFlagTypePassByValue, elements: !32, identifier: "_ZTS7lldiv_t")
!32 = !{!33, !35}
!33 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !31, file: !19, line: 69, baseType: !34, size: 64)
!34 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !31, file: !19, line: 69, baseType: !34, size: 64, offset: 64)
!36 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !37, file: !21, line: 109)
!37 = !DISubprogram(name: "atof", scope: !19, file: !19, line: 28, type: !38, flags: DIFlagPrototyped, spFlags: 0)
!38 = !DISubroutineType(types: !39)
!39 = !{!40, !41}
!40 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!42 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!43 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !44, file: !21, line: 110)
!44 = !DISubprogram(name: "atoi", scope: !19, file: !19, line: 25, type: !45, flags: DIFlagPrototyped, spFlags: 0)
!45 = !DISubroutineType(types: !46)
!46 = !{!13, !41}
!47 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !48, file: !21, line: 111)
!48 = !DISubprogram(name: "atol", scope: !19, file: !19, line: 26, type: !49, flags: DIFlagPrototyped, spFlags: 0)
!49 = !DISubroutineType(types: !50)
!50 = !{!27, !41}
!51 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !52, file: !21, line: 112)
!52 = !DISubprogram(name: "atoll", scope: !19, file: !19, line: 27, type: !53, flags: DIFlagPrototyped, spFlags: 0)
!53 = !DISubroutineType(types: !54)
!54 = !{!34, !41}
!55 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !56, file: !21, line: 113)
!56 = !DISubprogram(name: "strtod", scope: !19, file: !19, line: 31, type: !57, flags: DIFlagPrototyped, spFlags: 0)
!57 = !DISubroutineType(types: !58)
!58 = !{!40, !59, !60}
!59 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !41)
!60 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !61)
!61 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!62 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !63, file: !21, line: 114)
!63 = !DISubprogram(name: "strtof", scope: !19, file: !19, line: 30, type: !64, flags: DIFlagPrototyped, spFlags: 0)
!64 = !DISubroutineType(types: !65)
!65 = !{!66, !59, !60}
!66 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!67 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !68, file: !21, line: 115)
!68 = !DISubprogram(name: "strtold", scope: !19, file: !19, line: 32, type: !69, flags: DIFlagPrototyped, spFlags: 0)
!69 = !DISubroutineType(types: !70)
!70 = !{!71, !59, !60}
!71 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!72 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !73, file: !21, line: 116)
!73 = !DISubprogram(name: "strtol", scope: !19, file: !19, line: 34, type: !74, flags: DIFlagPrototyped, spFlags: 0)
!74 = !DISubroutineType(types: !75)
!75 = !{!27, !59, !60, !13}
!76 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !77, file: !21, line: 117)
!77 = !DISubprogram(name: "strtoll", scope: !19, file: !19, line: 36, type: !78, flags: DIFlagPrototyped, spFlags: 0)
!78 = !DISubroutineType(types: !79)
!79 = !{!34, !59, !60, !13}
!80 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !81, file: !21, line: 118)
!81 = !DISubprogram(name: "strtoul", scope: !19, file: !19, line: 35, type: !82, flags: DIFlagPrototyped, spFlags: 0)
!82 = !DISubroutineType(types: !83)
!83 = !{!84, !59, !60, !13}
!84 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!85 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !86, file: !21, line: 119)
!86 = !DISubprogram(name: "strtoull", scope: !19, file: !19, line: 37, type: !87, flags: DIFlagPrototyped, spFlags: 0)
!87 = !DISubroutineType(types: !88)
!88 = !{!89, !59, !60, !13}
!89 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!90 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !91, file: !21, line: 120)
!91 = !DISubprogram(name: "rand", scope: !19, file: !19, line: 39, type: !92, flags: DIFlagPrototyped, spFlags: 0)
!92 = !DISubroutineType(types: !93)
!93 = !{!13}
!94 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !95, file: !21, line: 121)
!95 = !DISubprogram(name: "srand", scope: !19, file: !19, line: 40, type: !96, flags: DIFlagPrototyped, spFlags: 0)
!96 = !DISubroutineType(types: !97)
!97 = !{null, !98}
!98 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!99 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !100, file: !21, line: 122)
!100 = !DISubprogram(name: "calloc", scope: !19, file: !19, line: 43, type: !101, flags: DIFlagPrototyped, spFlags: 0)
!101 = !DISubroutineType(types: !102)
!102 = !{!103, !104, !104}
!103 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!104 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !105, line: 58, baseType: !84)
!105 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/bits/alltypes.h", directory: "/root", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!106 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !107, file: !21, line: 123)
!107 = !DISubprogram(name: "free", scope: !19, file: !19, line: 45, type: !108, flags: DIFlagPrototyped, spFlags: 0)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !103}
!110 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !111, file: !21, line: 124)
!111 = !DISubprogram(name: "malloc", scope: !19, file: !19, line: 42, type: !112, flags: DIFlagPrototyped, spFlags: 0)
!112 = !DISubroutineType(types: !113)
!113 = !{!103, !104}
!114 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !115, file: !21, line: 125)
!115 = !DISubprogram(name: "realloc", scope: !19, file: !19, line: 44, type: !116, flags: DIFlagPrototyped, spFlags: 0)
!116 = !DISubroutineType(types: !117)
!117 = !{!103, !103, !104}
!118 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !119, file: !21, line: 126)
!119 = !DISubprogram(name: "abort", scope: !19, file: !19, line: 48, type: !120, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!120 = !DISubroutineType(types: !121)
!121 = !{null}
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !123, file: !21, line: 127)
!123 = !DISubprogram(name: "atexit", scope: !19, file: !19, line: 50, type: !124, flags: DIFlagPrototyped, spFlags: 0)
!124 = !DISubroutineType(types: !125)
!125 = !{!13, !126}
!126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !120, size: 64)
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !128, file: !21, line: 128)
!128 = !DISubprogram(name: "exit", scope: !19, file: !19, line: 51, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!129 = !DISubroutineType(types: !130)
!130 = !{null, !13}
!131 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !132, file: !21, line: 129)
!132 = !DISubprogram(name: "_Exit", scope: !19, file: !19, line: 52, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!133 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !134, file: !21, line: 130)
!134 = !DISubprogram(name: "getenv", scope: !19, file: !19, line: 56, type: !135, flags: DIFlagPrototyped, spFlags: 0)
!135 = !DISubroutineType(types: !136)
!136 = !{!5, !41}
!137 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !138, file: !21, line: 131)
!138 = !DISubprogram(name: "system", scope: !19, file: !19, line: 58, type: !45, flags: DIFlagPrototyped, spFlags: 0)
!139 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !140, file: !21, line: 132)
!140 = !DISubprogram(name: "bsearch", scope: !19, file: !19, line: 60, type: !141, flags: DIFlagPrototyped, spFlags: 0)
!141 = !DISubroutineType(types: !142)
!142 = !{!103, !143, !143, !104, !104, !145}
!143 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !144, size: 64)
!144 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!145 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !146, size: 64)
!146 = !DISubroutineType(types: !147)
!147 = !{!13, !143, !143}
!148 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !149, file: !21, line: 133)
!149 = !DISubprogram(name: "qsort", scope: !19, file: !19, line: 61, type: !150, flags: DIFlagPrototyped, spFlags: 0)
!150 = !DISubroutineType(types: !151)
!151 = !{null, !103, !104, !104, !145}
!152 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !153, file: !21, line: 135)
!153 = !DISubprogram(name: "labs", scope: !19, file: !19, line: 64, type: !154, flags: DIFlagPrototyped, spFlags: 0)
!154 = !DISubroutineType(types: !155)
!155 = !{!27, !27}
!156 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !157, file: !21, line: 136)
!157 = !DISubprogram(name: "llabs", scope: !19, file: !19, line: 65, type: !158, flags: DIFlagPrototyped, spFlags: 0)
!158 = !DISubroutineType(types: !159)
!159 = !{!34, !34}
!160 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !161, file: !21, line: 137)
!161 = !DISubprogram(name: "div", linkageName: "_Z3divB8ne210108xx", scope: !162, file: !162, line: 128, type: !163, flags: DIFlagPrototyped, spFlags: 0)
!162 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/stdlib.h", directory: "/root")
!163 = !DISubroutineType(types: !164)
!164 = !{!30, !34, !34}
!165 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !166, file: !21, line: 138)
!166 = !DISubprogram(name: "ldiv", scope: !19, file: !19, line: 72, type: !167, flags: DIFlagPrototyped, spFlags: 0)
!167 = !DISubroutineType(types: !168)
!168 = !{!23, !27, !27}
!169 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !170, file: !21, line: 139)
!170 = !DISubprogram(name: "lldiv", scope: !19, file: !19, line: 73, type: !163, flags: DIFlagPrototyped, spFlags: 0)
!171 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !172, file: !21, line: 140)
!172 = !DISubprogram(name: "mblen", scope: !19, file: !19, line: 75, type: !173, flags: DIFlagPrototyped, spFlags: 0)
!173 = !DISubroutineType(types: !174)
!174 = !{!13, !41, !104}
!175 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !176, file: !21, line: 142)
!176 = !DISubprogram(name: "mbtowc", scope: !19, file: !19, line: 76, type: !177, flags: DIFlagPrototyped, spFlags: 0)
!177 = !DISubroutineType(types: !178)
!178 = !{!13, !179, !59, !104}
!179 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !180)
!180 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !181, size: 64)
!181 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!182 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !183, file: !21, line: 143)
!183 = !DISubprogram(name: "wctomb", scope: !19, file: !19, line: 77, type: !184, flags: DIFlagPrototyped, spFlags: 0)
!184 = !DISubroutineType(types: !185)
!185 = !{!13, !5, !181}
!186 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !187, file: !21, line: 144)
!187 = !DISubprogram(name: "mbstowcs", scope: !19, file: !19, line: 78, type: !188, flags: DIFlagPrototyped, spFlags: 0)
!188 = !DISubroutineType(types: !189)
!189 = !{!104, !179, !59, !104}
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !191, file: !21, line: 145)
!191 = !DISubprogram(name: "wcstombs", scope: !19, file: !19, line: 79, type: !192, flags: DIFlagPrototyped, spFlags: 0)
!192 = !DISubroutineType(types: !193)
!193 = !{!104, !194, !195, !104}
!194 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !5)
!195 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !196)
!196 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !197, size: 64)
!197 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !181)
!198 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !199, file: !21, line: 148)
!199 = !DISubprogram(name: "at_quick_exit", scope: !19, file: !19, line: 53, type: !124, flags: DIFlagPrototyped, spFlags: 0)
!200 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !201, file: !21, line: 149)
!201 = !DISubprogram(name: "quick_exit", scope: !19, file: !19, line: 54, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: 0)
!202 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !203, file: !21, line: 152)
!203 = !DISubprogram(name: "aligned_alloc", scope: !19, file: !19, line: 46, type: !101, flags: DIFlagPrototyped, spFlags: 0)
!204 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !205, file: !211, line: 82)
!205 = !DISubprogram(name: "memcpy", scope: !206, file: !206, line: 32, type: !207, flags: DIFlagPrototyped, spFlags: 0)
!206 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/string.h", directory: "/root", checksumkind: CSK_MD5, checksum: "3943dbeb7798950d4de9b281143f3000")
!207 = !DISubroutineType(types: !208)
!208 = !{!103, !209, !210, !104}
!209 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !103)
!210 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !143)
!211 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/cstring", directory: "/root")
!212 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !213, file: !211, line: 83)
!213 = !DISubprogram(name: "memmove", scope: !206, file: !206, line: 33, type: !214, flags: DIFlagPrototyped, spFlags: 0)
!214 = !DISubroutineType(types: !215)
!215 = !{!103, !103, !143, !104}
!216 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !217, file: !211, line: 84)
!217 = !DISubprogram(name: "strcpy", scope: !206, file: !206, line: 38, type: !218, flags: DIFlagPrototyped, spFlags: 0)
!218 = !DISubroutineType(types: !219)
!219 = !{!5, !194, !59}
!220 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !221, file: !211, line: 85)
!221 = !DISubprogram(name: "strncpy", scope: !206, file: !206, line: 39, type: !222, flags: DIFlagPrototyped, spFlags: 0)
!222 = !DISubroutineType(types: !223)
!223 = !{!5, !194, !59, !104}
!224 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !225, file: !211, line: 86)
!225 = !DISubprogram(name: "strcat", scope: !206, file: !206, line: 41, type: !218, flags: DIFlagPrototyped, spFlags: 0)
!226 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !227, file: !211, line: 87)
!227 = !DISubprogram(name: "strncat", scope: !206, file: !206, line: 42, type: !222, flags: DIFlagPrototyped, spFlags: 0)
!228 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !229, file: !211, line: 88)
!229 = !DISubprogram(name: "memcmp", scope: !206, file: !206, line: 35, type: !230, flags: DIFlagPrototyped, spFlags: 0)
!230 = !DISubroutineType(types: !231)
!231 = !{!13, !143, !143, !104}
!232 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !233, file: !211, line: 89)
!233 = !DISubprogram(name: "strcmp", scope: !206, file: !206, line: 44, type: !234, flags: DIFlagPrototyped, spFlags: 0)
!234 = !DISubroutineType(types: !235)
!235 = !{!13, !41, !41}
!236 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !237, file: !211, line: 90)
!237 = !DISubprogram(name: "strncmp", scope: !206, file: !206, line: 45, type: !238, flags: DIFlagPrototyped, spFlags: 0)
!238 = !DISubroutineType(types: !239)
!239 = !{!13, !41, !41, !104}
!240 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !241, file: !211, line: 91)
!241 = !DISubprogram(name: "strcoll", scope: !206, file: !206, line: 47, type: !234, flags: DIFlagPrototyped, spFlags: 0)
!242 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !243, file: !211, line: 92)
!243 = !DISubprogram(name: "strxfrm", scope: !206, file: !206, line: 48, type: !244, flags: DIFlagPrototyped, spFlags: 0)
!244 = !DISubroutineType(types: !245)
!245 = !{!104, !194, !59, !104}
!246 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !247, file: !211, line: 93)
!247 = !DISubprogram(name: "memchr", linkageName: "_Z6memchrB8ne210108Ua9enable_ifILb1EEPvim", scope: !248, file: !248, line: 101, type: !249, flags: DIFlagPrototyped, spFlags: 0)
!248 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/string.h", directory: "/root")
!249 = !DISubroutineType(types: !250)
!250 = !{!103, !103, !13, !104}
!251 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !252, file: !211, line: 94)
!252 = !DISubprogram(name: "strchr", linkageName: "_Z6strchrB8ne210108Ua9enable_ifILb1EEPci", scope: !248, file: !248, line: 80, type: !253, flags: DIFlagPrototyped, spFlags: 0)
!253 = !DISubroutineType(types: !254)
!254 = !{!5, !5, !13}
!255 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !256, file: !211, line: 95)
!256 = !DISubprogram(name: "strcspn", scope: !206, file: !206, line: 53, type: !257, flags: DIFlagPrototyped, spFlags: 0)
!257 = !DISubroutineType(types: !258)
!258 = !{!104, !41, !41}
!259 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !260, file: !211, line: 96)
!260 = !DISubprogram(name: "strpbrk", linkageName: "_Z7strpbrkB8ne210108Ua9enable_ifILb1EEPcPKc", scope: !248, file: !248, line: 87, type: !261, flags: DIFlagPrototyped, spFlags: 0)
!261 = !DISubroutineType(types: !262)
!262 = !{!5, !5, !41}
!263 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !264, file: !211, line: 97)
!264 = !DISubprogram(name: "strrchr", linkageName: "_Z7strrchrB8ne210108Ua9enable_ifILb1EEPci", scope: !248, file: !248, line: 94, type: !253, flags: DIFlagPrototyped, spFlags: 0)
!265 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !266, file: !211, line: 98)
!266 = !DISubprogram(name: "strspn", scope: !206, file: !206, line: 54, type: !257, flags: DIFlagPrototyped, spFlags: 0)
!267 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !268, file: !211, line: 99)
!268 = !DISubprogram(name: "strstr", linkageName: "_Z6strstrB8ne210108Ua9enable_ifILb1EEPcPKc", scope: !248, file: !248, line: 108, type: !261, flags: DIFlagPrototyped, spFlags: 0)
!269 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !270, file: !211, line: 100)
!270 = !DISubprogram(name: "strtok", scope: !206, file: !206, line: 57, type: !218, flags: DIFlagPrototyped, spFlags: 0)
!271 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !272, file: !211, line: 101)
!272 = !DISubprogram(name: "memset", scope: !206, file: !206, line: 34, type: !249, flags: DIFlagPrototyped, spFlags: 0)
!273 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !274, file: !211, line: 102)
!274 = !DISubprogram(name: "strerror", scope: !206, file: !206, line: 61, type: !275, flags: DIFlagPrototyped, spFlags: 0)
!275 = !DISubroutineType(types: !276)
!276 = !{!5, !13}
!277 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !278, file: !211, line: 103)
!278 = !DISubprogram(name: "strlen", scope: !206, file: !206, line: 59, type: !279, flags: DIFlagPrototyped, spFlags: 0)
!279 = !DISubroutineType(types: !280)
!280 = !{!104, !41}
!281 = !{i32 7, !"Dwarf Version", i32 5}
!282 = !{i32 7, !"ReferenceTracking", i32 1}
!283 = !{i32 2, !"Debug Info Version", i32 3}
!284 = !{i32 1, !"wchar_size", i32 4}
!285 = !{i32 8, !"PIC Level", i32 2}
!286 = !{i32 7, !"uwtable", i32 2}
!287 = !{i32 7, !"frame-pointer", i32 1}
!288 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!289 = distinct !DISubprogram(name: "test_complex_flow", linkageName: "_Z17test_complex_flowPci", scope: !12, file: !12, line: 7, type: !253, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !290)
!290 = !{}
!291 = !{!"input", !"char*"}
!292 = !DILocalVariable(name: "input", arg: 1, scope: !289, file: !12, line: 7, type: !5)
!293 = !DILocation(line: 7, column: 57, scope: !289)
!294 = !DILocalVariable(name: "n", arg: 2, scope: !289, file: !12, line: 7, type: !13)
!295 = !DILocation(line: 7, column: 68, scope: !289)
!296 = !DILocalVariable(name: "p1", scope: !289, file: !12, line: 8, type: !5)
!297 = !DILocation(line: 8, column: 9, scope: !289)
!298 = !DILocation(line: 8, column: 21, scope: !289)
!299 = !{!"p1", !"char*"}
!300 = !DILocalVariable(name: "p2", scope: !289, file: !12, line: 9, type: !5)
!301 = !DILocation(line: 9, column: 9, scope: !289)
!302 = !DILocation(line: 9, column: 21, scope: !289)
!303 = !{!"p2", !"char*"}
!304 = !DILocation(line: 10, column: 11, scope: !289)
!305 = !DILocation(line: 10, column: 3, scope: !289)
!306 = !DILocation(line: 10, column: 9, scope: !289)
!307 = !DILocation(line: 11, column: 11, scope: !289)
!308 = !DILocation(line: 11, column: 3, scope: !289)
!309 = !DILocation(line: 11, column: 9, scope: !289)
!310 = !DILocation(line: 12, column: 11, scope: !289)
!311 = !DILocation(line: 12, column: 3, scope: !289)
!312 = !DILocation(line: 12, column: 9, scope: !289)
!313 = !DILocation(line: 13, column: 11, scope: !289)
!314 = !DILocation(line: 13, column: 3, scope: !289)
!315 = !DILocation(line: 13, column: 9, scope: !289)
!316 = !DILocation(line: 14, column: 18, scope: !289)
!317 = !DILocation(line: 14, column: 16, scope: !289)
!318 = !{!"g_global_ptr", !"char*"}
!319 = !DILocation(line: 15, column: 10, scope: !289)
!320 = !DILocation(line: 15, column: 12, scope: !289)
!321 = !DILocation(line: 15, column: 18, scope: !289)
!322 = !DILocation(line: 15, column: 23, scope: !289)
!323 = !DILocation(line: 15, column: 3, scope: !289)
!324 = distinct !DISubprogram(name: "test_atomic_store", linkageName: "_Z17test_atomic_storePVcc", scope: !12, file: !12, line: 18, type: !325, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !290)
!325 = !DISubroutineType(types: !326)
!326 = !{null, !7, !6}
!327 = !{!"ptr", !"char*"}
!328 = !DILocalVariable(name: "ptr", arg: 1, scope: !324, file: !12, line: 18, type: !7)
!329 = !DILocation(line: 18, column: 65, scope: !324)
!330 = !DILocalVariable(name: "val", arg: 2, scope: !324, file: !12, line: 18, type: !6)
!331 = !DILocation(line: 18, column: 75, scope: !324)
!332 = !DILocalVariable(name: "local", scope: !324, file: !12, line: 19, type: !6)
!333 = !DILocation(line: 19, column: 8, scope: !324)
!334 = !DILocation(line: 19, column: 16, scope: !324)
!335 = !DILocation(line: 20, column: 36, scope: !324)
!336 = !DILocation(line: 20, column: 41, scope: !324)
!337 = !DILocation(line: 20, column: 3, scope: !324)
!338 = !DILocation(line: 21, column: 1, scope: !324)
!339 = distinct !DISubprogram(name: "test_inline_asm", linkageName: "_Z15test_inline_asmPc", scope: !12, file: !12, line: 23, type: !340, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !290)
!340 = !DISubroutineType(types: !341)
!341 = !{null, !5}
!342 = !DILocalVariable(name: "ptr", arg: 1, scope: !339, file: !12, line: 23, type: !5)
!343 = !DILocation(line: 23, column: 54, scope: !339)
!344 = !DILocation(line: 24, column: 49, scope: !339)
!345 = !DILocation(line: 24, column: 3, scope: !339)
!346 = !{i64 603}
!347 = !DILocation(line: 25, column: 1, scope: !339)
!348 = distinct !DISubprogram(name: "test_noinline_malloc", linkageName: "_Z20test_noinline_mallocv", scope: !12, file: !12, line: 27, type: !349, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2)
!349 = !DISubroutineType(types: !350)
!350 = !{!5}
!351 = !DILocation(line: 28, column: 17, scope: !348)
!352 = !DILocation(line: 28, column: 3, scope: !348)
!353 = distinct !DISubprogram(name: "test_noinline_store", linkageName: "_Z19test_noinline_storePcc", scope: !12, file: !12, line: 31, type: !354, scopeLine: 31, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !290)
!354 = !DISubroutineType(types: !355)
!355 = !{null, !5, !6}
!356 = !DILocalVariable(name: "ptr", arg: 1, scope: !353, file: !12, line: 31, type: !5)
!357 = !DILocation(line: 31, column: 58, scope: !353)
!358 = !DILocalVariable(name: "val", arg: 2, scope: !353, file: !12, line: 31, type: !6)
!359 = !DILocation(line: 31, column: 68, scope: !353)
!360 = !DILocation(line: 32, column: 10, scope: !353)
!361 = !DILocation(line: 32, column: 4, scope: !353)
!362 = !DILocation(line: 32, column: 8, scope: !353)
!363 = !DILocation(line: 33, column: 1, scope: !353)
!364 = distinct !DISubprogram(name: "main", scope: !12, file: !12, line: 35, type: !92, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !290)
!365 = !DILocalVariable(name: "p1", scope: !364, file: !12, line: 36, type: !5)
!366 = !DILocation(line: 36, column: 9, scope: !364)
!367 = !DILocation(line: 36, column: 14, scope: !364)
!368 = !DILocalVariable(name: "p2", scope: !364, file: !12, line: 37, type: !5)
!369 = !DILocation(line: 37, column: 9, scope: !364)
!370 = !DILocation(line: 37, column: 14, scope: !364)
!371 = !DILocation(line: 38, column: 3, scope: !364)
!372 = !DILocation(line: 38, column: 9, scope: !364)
!373 = !DILocation(line: 39, column: 3, scope: !364)
!374 = !DILocation(line: 39, column: 9, scope: !364)
!375 = !DILocation(line: 40, column: 3, scope: !364)
!376 = !DILocation(line: 40, column: 9, scope: !364)
!377 = !DILocation(line: 41, column: 3, scope: !364)
!378 = !DILocation(line: 41, column: 9, scope: !364)
!379 = !DILocation(line: 42, column: 18, scope: !364)
!380 = !DILocation(line: 42, column: 16, scope: !364)
!381 = !DILocalVariable(name: "data", scope: !364, file: !12, line: 43, type: !382)
!382 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64, elements: !383)
!383 = !{!384}
!384 = !DISubrange(count: 8)
!385 = !DILocation(line: 43, column: 8, scope: !364)
!386 = !DILocalVariable(name: "result", scope: !364, file: !12, line: 44, type: !5)
!387 = !DILocation(line: 44, column: 9, scope: !364)
!388 = !DILocation(line: 44, column: 36, scope: !364)
!389 = !DILocation(line: 44, column: 18, scope: !364)
!390 = !{!"result", !"char*"}
!391 = !DILocalVariable(name: "atomic_ptr", scope: !364, file: !12, line: 45, type: !7)
!392 = !DILocation(line: 45, column: 18, scope: !364)
!393 = !DILocation(line: 45, column: 47, scope: !364)
!394 = !{!"atomic_ptr", !"char*"}
!395 = !DILocation(line: 46, column: 21, scope: !364)
!396 = !DILocation(line: 46, column: 3, scope: !364)
!397 = !DILocation(line: 47, column: 19, scope: !364)
!398 = !DILocation(line: 47, column: 3, scope: !364)
!399 = !DILocation(line: 48, column: 18, scope: !364)
!400 = !DILocation(line: 48, column: 27, scope: !364)
!401 = !DILocation(line: 48, column: 16, scope: !364)
!402 = !DILocation(line: 49, column: 3, scope: !364)
