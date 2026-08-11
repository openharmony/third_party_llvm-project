; REQUIRES: aarch64-registered-target

; Check SelectionDAG Isel
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj %s -O2 -o %t_selisel.o
; RUN: ld.lld -shared %t_selisel.o -o %t_selisel.o
; RUN: llvm-dwarfdump --mem_tracer %t_selisel.o | FileCheck %s --check-prefix=CHECK

; Check Fast Isel
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj -global-isel=false -O0 %s -o %t_fastisel.o
; RUN: ld.lld -shared %t_fastisel.o -o %t_fastisel.o
; RUN: llvm-dwarfdump --mem_tracer %t_fastisel.o | FileCheck %s --check-prefix=CHECK

; Check Global Isel
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj -global-isel=true -O0 %s -o %t_globalisel.o
; RUN: ld.lld -shared %t_globalisel.o -o %t_globalisel.o
; RUN: llvm-dwarfdump --mem_tracer %t_globalisel.o | FileCheck %s --check-prefix=CHECK

; Check independent sections with -ffunction-sections
; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj -O2 -function-sections %s -o %t_ffunc.o
; RUN: llvm-readelf -S  %t_ffunc.o | FileCheck %s --check-prefix=CHECK-FUNC



; CHECK: .mem_tracer contents:
; CHECK-NEXT: [   0] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT: [   1] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT: [   2] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT: [   3] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"

; CHECK-FUNC-COUNT-2: ] .mem_tracer


; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@g_global_ptr = local_unnamed_addr global ptr null, align 8, !dbg !0
@g_global_val = local_unnamed_addr global i32 0, align 4, !dbg !10

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(write, argmem: read, inaccessiblemem: readwrite) uwtable
define noundef ptr @_Z17test_complex_flowPci(ptr noundef readonly captures(none) %0, i32 noundef %1) local_unnamed_addr #0 !dbg !290 {
    #dbg_value(ptr %0, !292, !DIExpression(), !296)
    #dbg_value(i32 %1, !293, !DIExpression(), !296)
  %3 = tail call dereferenceable_or_null(8) ptr @malloc(i64 noundef 8) #8, !dbg !297, !memtracer !298
    #dbg_value(ptr %3, !294, !DIExpression(), !296)
  %4 = tail call dereferenceable_or_null(8) ptr @malloc(i64 noundef 8) #8, !dbg !299, !memtracer !300
    #dbg_value(ptr %4, !295, !DIExpression(), !296)
  %5 = load i8, ptr %0, align 1, !dbg !301, !tbaa !302
  store i8 %5, ptr %3, align 1, !dbg !305, !tbaa !302
  %6 = getelementptr inbounds nuw i8, ptr %0, i64 1, !dbg !306
  %7 = load i8, ptr %6, align 1, !dbg !306, !tbaa !302
  %8 = getelementptr inbounds nuw i8, ptr %3, i64 1, !dbg !307
  store i8 %7, ptr %8, align 1, !dbg !308, !tbaa !302
  %9 = getelementptr inbounds nuw i8, ptr %0, i64 2, !dbg !309
  %10 = load i8, ptr %9, align 1, !dbg !309, !tbaa !302
  store i8 %10, ptr %4, align 1, !dbg !310, !tbaa !302
  %11 = getelementptr inbounds nuw i8, ptr %0, i64 3, !dbg !311
  %12 = load i8, ptr %11, align 1, !dbg !311, !tbaa !302
  %13 = getelementptr inbounds nuw i8, ptr %4, i64 1, !dbg !312
  store i8 %12, ptr %13, align 1, !dbg !313, !tbaa !302
  store ptr %3, ptr @g_global_ptr, align 8, !dbg !314, !tbaa !315, !memtracer !298
  %14 = icmp sgt i32 %1, 0, !dbg !318
  %15 = select i1 %14, ptr %3, ptr %4, !dbg !319
  ret ptr %15, !dbg !320
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr captures(none)) #1

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !dbg !111 noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr captures(none)) #1

; Function Attrs: mustprogress nofree noinline norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define void @_Z17test_atomic_storePVcc(ptr noundef %0, i8 noundef %1) local_unnamed_addr #3 !dbg !321 {
    #dbg_value(ptr %0, !325, !DIExpression(), !328)
    #dbg_value(i8 %1, !326, !DIExpression(), !328)
    #dbg_value(i8 %1, !327, !DIExpression(), !328)
  store atomic volatile i8 %1, ptr %0 seq_cst, align 1, !dbg !329
  ret void, !dbg !330
}

; Function Attrs: mustprogress noinline nounwind uwtable
define void @_Z15test_inline_asmPc(ptr noundef %0) local_unnamed_addr #4 !dbg !331 {
    #dbg_value(ptr %0, !335, !DIExpression(), !336)
  tail call void asm sideeffect "str ${0:w}, [$1]", "r,r"(i32 42, ptr %0) #9, !dbg !337, !srcloc !338
  ret void, !dbg !339
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(inaccessiblemem: readwrite) uwtable
define noalias noundef ptr @_Z20test_noinline_mallocv() local_unnamed_addr #5 !dbg !340 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #8, !dbg !343
  ret ptr %1, !dbg !344
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: write) uwtable
define void @_Z19test_noinline_storePcc(ptr noundef writeonly captures(none) initializes((0, 1)) %0, i8 noundef %1) local_unnamed_addr #6 !dbg !345 {
    #dbg_value(ptr %0, !349, !DIExpression(), !351)
    #dbg_value(i8 %1, !350, !DIExpression(), !351)
  store i8 %1, ptr %0, align 1, !dbg !352, !tbaa !302
  ret void, !dbg !353
}

; Function Attrs: mustprogress norecurse nounwind uwtable
define noundef i32 @main() local_unnamed_addr #7 !dbg !354 {
  %1 = alloca [8 x i8], align 8, !DIAssignID !364
    #dbg_assign(i1 poison, !358, !DIExpression(), !364, ptr %1, !DIExpression(), !365)
  %2 = tail call noundef ptr @_Z20test_noinline_mallocv() #10, !dbg !366
    #dbg_value(ptr %2, !356, !DIExpression(), !365)
  %3 = tail call noundef ptr @_Z20test_noinline_mallocv() #10, !dbg !367
    #dbg_value(ptr %3, !357, !DIExpression(), !365)
  store <4 x i8> <i8 1, i8 2, i8 3, i8 4>, ptr %2, align 1, !dbg !368, !tbaa !302
  store ptr %2, ptr @g_global_ptr, align 8, !dbg !369, !tbaa !315, !memtracer !298
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %1) #9, !dbg !370
  store i64 578437695752307201, ptr %1, align 8, !dbg !371, !DIAssignID !372
    #dbg_assign(i1 poison, !358, !DIExpression(), !372, ptr %1, !DIExpression(), !365)
  %4 = call noundef ptr @_Z17test_complex_flowPci(ptr noundef nonnull %1, i32 noundef 5) #10, !dbg !373
    #dbg_value(ptr %4, !362, !DIExpression(), !365)
    #dbg_value(ptr %3, !363, !DIExpression(), !365)
  tail call void @_Z17test_atomic_storePVcc(ptr noundef %3, i8 noundef 100) #10, !dbg !374
  tail call void @_Z15test_inline_asmPc(ptr noundef nonnull %2) #10, !dbg !375
  %5 = icmp eq ptr %4, null, !dbg !376
  br i1 %5, label %9, label %6, !dbg !376

6:                                                ; preds = %0
  %7 = load i8, ptr %4, align 1, !dbg !377, !tbaa !302
  %8 = zext i8 %7 to i32, !dbg !377
  br label %9, !dbg !376

9:                                                ; preds = %6, %0
  %10 = phi i32 [ %8, %6 ], [ 0, %0 ], !dbg !376
  store i32 %10, ptr @g_global_val, align 4, !dbg !378, !tbaa !379
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %1) #9, !dbg !381
  ret i32 0, !dbg !382
}

attributes #0 = { mustprogress nofree noinline nounwind willreturn memory(write, argmem: read, inaccessiblemem: readwrite) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { mustprogress nofree noinline norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #4 = { mustprogress noinline nounwind uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #5 = { mustprogress nofree noinline nounwind willreturn memory(inaccessiblemem: readwrite) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #6 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: write) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #7 = { mustprogress norecurse nounwind uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #8 = { allocsize(0) "reference-tracking"="true" }
attributes #9 = { nounwind }
attributes #10 = { "reference-tracking"="true" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!281, !282, !283, !284, !285, !286, !287, !288}
!llvm.ident = !{!289}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_global_ptr", scope: !2, file: !12, line: 4, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !9, imports: !14, splitDebugInlining: false, nameTableKind: None)
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
!37 = !DISubprogram(name: "atof", scope: !19, file: !19, line: 28, type: !38, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!38 = !DISubroutineType(types: !39)
!39 = !{!40, !41}
!40 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!42 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!43 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !44, file: !21, line: 110)
!44 = !DISubprogram(name: "atoi", scope: !19, file: !19, line: 25, type: !45, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!45 = !DISubroutineType(types: !46)
!46 = !{!13, !41}
!47 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !48, file: !21, line: 111)
!48 = !DISubprogram(name: "atol", scope: !19, file: !19, line: 26, type: !49, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!49 = !DISubroutineType(types: !50)
!50 = !{!27, !41}
!51 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !52, file: !21, line: 112)
!52 = !DISubprogram(name: "atoll", scope: !19, file: !19, line: 27, type: !53, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!53 = !DISubroutineType(types: !54)
!54 = !{!34, !41}
!55 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !56, file: !21, line: 113)
!56 = !DISubprogram(name: "strtod", scope: !19, file: !19, line: 31, type: !57, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!57 = !DISubroutineType(types: !58)
!58 = !{!40, !59, !60}
!59 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !41)
!60 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !61)
!61 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!62 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !63, file: !21, line: 114)
!63 = !DISubprogram(name: "strtof", scope: !19, file: !19, line: 30, type: !64, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!64 = !DISubroutineType(types: !65)
!65 = !{!66, !59, !60}
!66 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!67 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !68, file: !21, line: 115)
!68 = !DISubprogram(name: "strtold", scope: !19, file: !19, line: 32, type: !69, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!69 = !DISubroutineType(types: !70)
!70 = !{!71, !59, !60}
!71 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!72 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !73, file: !21, line: 116)
!73 = !DISubprogram(name: "strtol", scope: !19, file: !19, line: 34, type: !74, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!74 = !DISubroutineType(types: !75)
!75 = !{!27, !59, !60, !13}
!76 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !77, file: !21, line: 117)
!77 = !DISubprogram(name: "strtoll", scope: !19, file: !19, line: 36, type: !78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!78 = !DISubroutineType(types: !79)
!79 = !{!34, !59, !60, !13}
!80 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !81, file: !21, line: 118)
!81 = !DISubprogram(name: "strtoul", scope: !19, file: !19, line: 35, type: !82, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!82 = !DISubroutineType(types: !83)
!83 = !{!84, !59, !60, !13}
!84 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!85 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !86, file: !21, line: 119)
!86 = !DISubprogram(name: "strtoull", scope: !19, file: !19, line: 37, type: !87, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!87 = !DISubroutineType(types: !88)
!88 = !{!89, !59, !60, !13}
!89 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!90 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !91, file: !21, line: 120)
!91 = !DISubprogram(name: "rand", scope: !19, file: !19, line: 39, type: !92, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!92 = !DISubroutineType(types: !93)
!93 = !{!13}
!94 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !95, file: !21, line: 121)
!95 = !DISubprogram(name: "srand", scope: !19, file: !19, line: 40, type: !96, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!96 = !DISubroutineType(types: !97)
!97 = !{null, !98}
!98 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!99 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !100, file: !21, line: 122)
!100 = !DISubprogram(name: "calloc", scope: !19, file: !19, line: 43, type: !101, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!101 = !DISubroutineType(types: !102)
!102 = !{!103, !104, !104}
!103 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!104 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !105, line: 58, baseType: !84)
!105 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/bits/alltypes.h", directory: "/root", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!106 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !107, file: !21, line: 123)
!107 = !DISubprogram(name: "free", scope: !19, file: !19, line: 45, type: !108, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !103}
!110 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !111, file: !21, line: 124)
!111 = !DISubprogram(name: "malloc", scope: !19, file: !19, line: 42, type: !112, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!112 = !DISubroutineType(types: !113)
!113 = !{!103, !104}
!114 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !115, file: !21, line: 125)
!115 = !DISubprogram(name: "realloc", scope: !19, file: !19, line: 44, type: !116, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!116 = !DISubroutineType(types: !117)
!117 = !{!103, !103, !104}
!118 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !119, file: !21, line: 126)
!119 = !DISubprogram(name: "abort", scope: !19, file: !19, line: 48, type: !120, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!120 = !DISubroutineType(types: !121)
!121 = !{null}
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !123, file: !21, line: 127)
!123 = !DISubprogram(name: "atexit", scope: !19, file: !19, line: 50, type: !124, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!124 = !DISubroutineType(types: !125)
!125 = !{!13, !126}
!126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !120, size: 64)
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !128, file: !21, line: 128)
!128 = !DISubprogram(name: "exit", scope: !19, file: !19, line: 51, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!129 = !DISubroutineType(types: !130)
!130 = !{null, !13}
!131 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !132, file: !21, line: 129)
!132 = !DISubprogram(name: "_Exit", scope: !19, file: !19, line: 52, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!133 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !134, file: !21, line: 130)
!134 = !DISubprogram(name: "getenv", scope: !19, file: !19, line: 56, type: !135, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!135 = !DISubroutineType(types: !136)
!136 = !{!5, !41}
!137 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !138, file: !21, line: 131)
!138 = !DISubprogram(name: "system", scope: !19, file: !19, line: 58, type: !45, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!139 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !140, file: !21, line: 132)
!140 = !DISubprogram(name: "bsearch", scope: !19, file: !19, line: 60, type: !141, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!141 = !DISubroutineType(types: !142)
!142 = !{!103, !143, !143, !104, !104, !145}
!143 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !144, size: 64)
!144 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!145 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !146, size: 64)
!146 = !DISubroutineType(types: !147)
!147 = !{!13, !143, !143}
!148 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !149, file: !21, line: 133)
!149 = !DISubprogram(name: "qsort", scope: !19, file: !19, line: 61, type: !150, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!150 = !DISubroutineType(types: !151)
!151 = !{null, !103, !104, !104, !145}
!152 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !153, file: !21, line: 135)
!153 = !DISubprogram(name: "labs", scope: !19, file: !19, line: 64, type: !154, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!154 = !DISubroutineType(types: !155)
!155 = !{!27, !27}
!156 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !157, file: !21, line: 136)
!157 = !DISubprogram(name: "llabs", scope: !19, file: !19, line: 65, type: !158, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!158 = !DISubroutineType(types: !159)
!159 = !{!34, !34}
!160 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !161, file: !21, line: 137)
!161 = !DISubprogram(name: "div", linkageName: "_Z3divB8ne210108xx", scope: !162, file: !162, line: 128, type: !163, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!162 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/stdlib.h", directory: "/root")
!163 = !DISubroutineType(types: !164)
!164 = !{!30, !34, !34}
!165 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !166, file: !21, line: 138)
!166 = !DISubprogram(name: "ldiv", scope: !19, file: !19, line: 72, type: !167, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!167 = !DISubroutineType(types: !168)
!168 = !{!23, !27, !27}
!169 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !170, file: !21, line: 139)
!170 = !DISubprogram(name: "lldiv", scope: !19, file: !19, line: 73, type: !163, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!171 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !172, file: !21, line: 140)
!172 = !DISubprogram(name: "mblen", scope: !19, file: !19, line: 75, type: !173, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!173 = !DISubroutineType(types: !174)
!174 = !{!13, !41, !104}
!175 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !176, file: !21, line: 142)
!176 = !DISubprogram(name: "mbtowc", scope: !19, file: !19, line: 76, type: !177, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!177 = !DISubroutineType(types: !178)
!178 = !{!13, !179, !59, !104}
!179 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !180)
!180 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !181, size: 64)
!181 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!182 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !183, file: !21, line: 143)
!183 = !DISubprogram(name: "wctomb", scope: !19, file: !19, line: 77, type: !184, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!184 = !DISubroutineType(types: !185)
!185 = !{!13, !5, !181}
!186 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !187, file: !21, line: 144)
!187 = !DISubprogram(name: "mbstowcs", scope: !19, file: !19, line: 78, type: !188, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!188 = !DISubroutineType(types: !189)
!189 = !{!104, !179, !59, !104}
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !191, file: !21, line: 145)
!191 = !DISubprogram(name: "wcstombs", scope: !19, file: !19, line: 79, type: !192, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!192 = !DISubroutineType(types: !193)
!193 = !{!104, !194, !195, !104}
!194 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !5)
!195 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !196)
!196 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !197, size: 64)
!197 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !181)
!198 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !199, file: !21, line: 148)
!199 = !DISubprogram(name: "at_quick_exit", scope: !19, file: !19, line: 53, type: !124, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!200 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !201, file: !21, line: 149)
!201 = !DISubprogram(name: "quick_exit", scope: !19, file: !19, line: 54, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!202 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !203, file: !21, line: 152)
!203 = !DISubprogram(name: "aligned_alloc", scope: !19, file: !19, line: 46, type: !101, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!204 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !205, file: !211, line: 82)
!205 = !DISubprogram(name: "memcpy", scope: !206, file: !206, line: 32, type: !207, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!206 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/string.h", directory: "/root", checksumkind: CSK_MD5, checksum: "3943dbeb7798950d4de9b281143f3000")
!207 = !DISubroutineType(types: !208)
!208 = !{!103, !209, !210, !104}
!209 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !103)
!210 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !143)
!211 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/cstring", directory: "/root")
!212 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !213, file: !211, line: 83)
!213 = !DISubprogram(name: "memmove", scope: !206, file: !206, line: 33, type: !214, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!214 = !DISubroutineType(types: !215)
!215 = !{!103, !103, !143, !104}
!216 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !217, file: !211, line: 84)
!217 = !DISubprogram(name: "strcpy", scope: !206, file: !206, line: 38, type: !218, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!218 = !DISubroutineType(types: !219)
!219 = !{!5, !194, !59}
!220 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !221, file: !211, line: 85)
!221 = !DISubprogram(name: "strncpy", scope: !206, file: !206, line: 39, type: !222, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!222 = !DISubroutineType(types: !223)
!223 = !{!5, !194, !59, !104}
!224 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !225, file: !211, line: 86)
!225 = !DISubprogram(name: "strcat", scope: !206, file: !206, line: 41, type: !218, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!226 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !227, file: !211, line: 87)
!227 = !DISubprogram(name: "strncat", scope: !206, file: !206, line: 42, type: !222, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!228 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !229, file: !211, line: 88)
!229 = !DISubprogram(name: "memcmp", scope: !206, file: !206, line: 35, type: !230, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!230 = !DISubroutineType(types: !231)
!231 = !{!13, !143, !143, !104}
!232 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !233, file: !211, line: 89)
!233 = !DISubprogram(name: "strcmp", scope: !206, file: !206, line: 44, type: !234, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!234 = !DISubroutineType(types: !235)
!235 = !{!13, !41, !41}
!236 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !237, file: !211, line: 90)
!237 = !DISubprogram(name: "strncmp", scope: !206, file: !206, line: 45, type: !238, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!238 = !DISubroutineType(types: !239)
!239 = !{!13, !41, !41, !104}
!240 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !241, file: !211, line: 91)
!241 = !DISubprogram(name: "strcoll", scope: !206, file: !206, line: 47, type: !234, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!242 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !243, file: !211, line: 92)
!243 = !DISubprogram(name: "strxfrm", scope: !206, file: !206, line: 48, type: !244, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!244 = !DISubroutineType(types: !245)
!245 = !{!104, !194, !59, !104}
!246 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !247, file: !211, line: 93)
!247 = !DISubprogram(name: "memchr", linkageName: "_Z6memchrB8ne210108Ua9enable_ifILb1EEPvim", scope: !248, file: !248, line: 101, type: !249, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!248 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/string.h", directory: "/root")
!249 = !DISubroutineType(types: !250)
!250 = !{!103, !103, !13, !104}
!251 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !252, file: !211, line: 94)
!252 = !DISubprogram(name: "strchr", linkageName: "_Z6strchrB8ne210108Ua9enable_ifILb1EEPci", scope: !248, file: !248, line: 80, type: !253, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!253 = !DISubroutineType(types: !254)
!254 = !{!5, !5, !13}
!255 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !256, file: !211, line: 95)
!256 = !DISubprogram(name: "strcspn", scope: !206, file: !206, line: 53, type: !257, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!257 = !DISubroutineType(types: !258)
!258 = !{!104, !41, !41}
!259 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !260, file: !211, line: 96)
!260 = !DISubprogram(name: "strpbrk", linkageName: "_Z7strpbrkB8ne210108Ua9enable_ifILb1EEPcPKc", scope: !248, file: !248, line: 87, type: !261, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!261 = !DISubroutineType(types: !262)
!262 = !{!5, !5, !41}
!263 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !264, file: !211, line: 97)
!264 = !DISubprogram(name: "strrchr", linkageName: "_Z7strrchrB8ne210108Ua9enable_ifILb1EEPci", scope: !248, file: !248, line: 94, type: !253, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!265 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !266, file: !211, line: 98)
!266 = !DISubprogram(name: "strspn", scope: !206, file: !206, line: 54, type: !257, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!267 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !268, file: !211, line: 99)
!268 = !DISubprogram(name: "strstr", linkageName: "_Z6strstrB8ne210108Ua9enable_ifILb1EEPcPKc", scope: !248, file: !248, line: 108, type: !261, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!269 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !270, file: !211, line: 100)
!270 = !DISubprogram(name: "strtok", scope: !206, file: !206, line: 57, type: !218, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!271 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !272, file: !211, line: 101)
!272 = !DISubprogram(name: "memset", scope: !206, file: !206, line: 34, type: !249, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!273 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !274, file: !211, line: 102)
!274 = !DISubprogram(name: "strerror", scope: !206, file: !206, line: 61, type: !275, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!275 = !DISubroutineType(types: !276)
!276 = !{!5, !13}
!277 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !16, entity: !278, file: !211, line: 103)
!278 = !DISubprogram(name: "strlen", scope: !206, file: !206, line: 59, type: !279, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!279 = !DISubroutineType(types: !280)
!280 = !{!104, !41}
!281 = !{i32 7, !"Dwarf Version", i32 5}
!282 = !{i32 7, !"ReferenceTracking", i32 1}
!283 = !{i32 2, !"Debug Info Version", i32 3}
!284 = !{i32 1, !"wchar_size", i32 4}
!285 = !{i32 8, !"PIC Level", i32 2}
!286 = !{i32 7, !"uwtable", i32 2}
!287 = !{i32 7, !"frame-pointer", i32 1}
!288 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!289 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!290 = distinct !DISubprogram(name: "test_complex_flow", linkageName: "_Z17test_complex_flowPci", scope: !12, file: !12, line: 7, type: !253, scopeLine: 7, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !291)
!291 = !{!292, !293, !294, !295}
!292 = !DILocalVariable(name: "input", arg: 1, scope: !290, file: !12, line: 7, type: !5)
!293 = !DILocalVariable(name: "n", arg: 2, scope: !290, file: !12, line: 7, type: !13)
!294 = !DILocalVariable(name: "p1", scope: !290, file: !12, line: 8, type: !5)
!295 = !DILocalVariable(name: "p2", scope: !290, file: !12, line: 9, type: !5)
!296 = !DILocation(line: 0, scope: !290)
!297 = !DILocation(line: 8, column: 21, scope: !290)
!298 = !{!"g_global_ptr", !"char*"}
!299 = !DILocation(line: 9, column: 21, scope: !290)
!300 = !{!"p2", !"char*"}
!301 = !DILocation(line: 10, column: 11, scope: !290)
!302 = !{!303, !303, i64 0}
!303 = !{!"omnipotent char", !304, i64 0}
!304 = !{!"Simple C++ TBAA"}
!305 = !DILocation(line: 10, column: 9, scope: !290)
!306 = !DILocation(line: 11, column: 11, scope: !290)
!307 = !DILocation(line: 11, column: 3, scope: !290)
!308 = !DILocation(line: 11, column: 9, scope: !290)
!309 = !DILocation(line: 12, column: 11, scope: !290)
!310 = !DILocation(line: 12, column: 9, scope: !290)
!311 = !DILocation(line: 13, column: 11, scope: !290)
!312 = !DILocation(line: 13, column: 3, scope: !290)
!313 = !DILocation(line: 13, column: 9, scope: !290)
!314 = !DILocation(line: 14, column: 16, scope: !290)
!315 = !{!316, !316, i64 0}
!316 = !{!"p1 omnipotent char", !317, i64 0}
!317 = !{!"any pointer", !303, i64 0}
!318 = !DILocation(line: 15, column: 12, scope: !290)
!319 = !DILocation(line: 15, column: 10, scope: !290)
!320 = !DILocation(line: 15, column: 3, scope: !290)
!321 = distinct !DISubprogram(name: "test_atomic_store", linkageName: "_Z17test_atomic_storePVcc", scope: !12, file: !12, line: 18, type: !322, scopeLine: 18, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !324)
!322 = !DISubroutineType(types: !323)
!323 = !{null, !7, !6}
!324 = !{!325, !326, !327}
!325 = !DILocalVariable(name: "ptr", arg: 1, scope: !321, file: !12, line: 18, type: !7)
!326 = !DILocalVariable(name: "val", arg: 2, scope: !321, file: !12, line: 18, type: !6)
!327 = !DILocalVariable(name: "local", scope: !321, file: !12, line: 19, type: !6)
!328 = !DILocation(line: 0, scope: !321)
!329 = !DILocation(line: 20, column: 3, scope: !321)
!330 = !DILocation(line: 21, column: 1, scope: !321)
!331 = distinct !DISubprogram(name: "test_inline_asm", linkageName: "_Z15test_inline_asmPc", scope: !12, file: !12, line: 23, type: !332, scopeLine: 23, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !334)
!332 = !DISubroutineType(types: !333)
!333 = !{null, !5}
!334 = !{!335}
!335 = !DILocalVariable(name: "ptr", arg: 1, scope: !331, file: !12, line: 23, type: !5)
!336 = !DILocation(line: 0, scope: !331)
!337 = !DILocation(line: 24, column: 3, scope: !331)
!338 = !{i64 603}
!339 = !DILocation(line: 25, column: 1, scope: !331)
!340 = distinct !DISubprogram(name: "test_noinline_malloc", linkageName: "_Z20test_noinline_mallocv", scope: !12, file: !12, line: 27, type: !341, scopeLine: 27, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2)
!341 = !DISubroutineType(types: !342)
!342 = !{!5}
!343 = !DILocation(line: 28, column: 17, scope: !340)
!344 = !DILocation(line: 28, column: 3, scope: !340)
!345 = distinct !DISubprogram(name: "test_noinline_store", linkageName: "_Z19test_noinline_storePcc", scope: !12, file: !12, line: 31, type: !346, scopeLine: 31, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !348)
!346 = !DISubroutineType(types: !347)
!347 = !{null, !5, !6}
!348 = !{!349, !350}
!349 = !DILocalVariable(name: "ptr", arg: 1, scope: !345, file: !12, line: 31, type: !5)
!350 = !DILocalVariable(name: "val", arg: 2, scope: !345, file: !12, line: 31, type: !6)
!351 = !DILocation(line: 0, scope: !345)
!352 = !DILocation(line: 32, column: 8, scope: !345)
!353 = !DILocation(line: 33, column: 1, scope: !345)
!354 = distinct !DISubprogram(name: "main", scope: !12, file: !12, line: 35, type: !92, scopeLine: 35, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !355)
!355 = !{!356, !357, !358, !362, !363}
!356 = !DILocalVariable(name: "p1", scope: !354, file: !12, line: 36, type: !5)
!357 = !DILocalVariable(name: "p2", scope: !354, file: !12, line: 37, type: !5)
!358 = !DILocalVariable(name: "data", scope: !354, file: !12, line: 43, type: !359)
!359 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64, elements: !360)
!360 = !{!361}
!361 = !DISubrange(count: 8)
!362 = !DILocalVariable(name: "result", scope: !354, file: !12, line: 44, type: !5)
!363 = !DILocalVariable(name: "atomic_ptr", scope: !354, file: !12, line: 45, type: !7)
!364 = distinct !DIAssignID()
!365 = !DILocation(line: 0, scope: !354)
!366 = !DILocation(line: 36, column: 14, scope: !354)
!367 = !DILocation(line: 37, column: 14, scope: !354)
!368 = !DILocation(line: 38, column: 9, scope: !354)
!369 = !DILocation(line: 42, column: 16, scope: !354)
!370 = !DILocation(line: 43, column: 3, scope: !354)
!371 = !DILocation(line: 43, column: 8, scope: !354)
!372 = distinct !DIAssignID()
!373 = !DILocation(line: 44, column: 18, scope: !354)
!374 = !DILocation(line: 46, column: 3, scope: !354)
!375 = !DILocation(line: 47, column: 3, scope: !354)
!376 = !DILocation(line: 48, column: 18, scope: !354)
!377 = !DILocation(line: 48, column: 27, scope: !354)
!378 = !DILocation(line: 48, column: 16, scope: !354)
!379 = !{!380, !380, i64 0}
!380 = !{!"int", !303, i64 0}
!381 = !DILocation(line: 50, column: 1, scope: !354)
!382 = !DILocation(line: 49, column: 3, scope: !354)
