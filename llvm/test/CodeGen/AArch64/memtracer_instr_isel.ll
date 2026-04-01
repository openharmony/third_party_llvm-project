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
; CHECK-NEXT:  [   0] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT:  [   1] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p2" type="char*"
; CHECK-NEXT:  [   2] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"
; CHECK-NEXT:  [   3] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="p1" type="char*"
; CHECK-NEXT:  [   4] addr=0x{{[0-9a-f]+}} strOff=0x{{[0-9a-f]+}} typeOff=0x{{[0-9a-f]+}} var="g_global_ptr" type="char*"

; CHECK-FUNC: ] .mem_tracer
; CHECK-FUNC: ] .mem_tracer


; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

@g_global_ptr = local_unnamed_addr global ptr null, align 8, !dbg !0
@g_global_val = local_unnamed_addr global i32 0, align 4, !dbg !10

; Function Attrs: mustprogress nofree nounwind willreturn
define noundef ptr @_Z17test_complex_flowPci(ptr nocapture noundef readonly %0, i32 noundef %1) local_unnamed_addr #0 !dbg !213 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !217, metadata !DIExpression()), !dbg !221
  call void @llvm.dbg.value(metadata i32 %1, metadata !218, metadata !DIExpression()), !dbg !221
  %3 = tail call dereferenceable_or_null(8) ptr @malloc(i64 noundef 8) #7, !dbg !222, !memtracer !223
  call void @llvm.dbg.value(metadata ptr %3, metadata !219, metadata !DIExpression()), !dbg !221
  %4 = tail call dereferenceable_or_null(8) ptr @malloc(i64 noundef 8) #7, !dbg !224, !memtracer !225
  call void @llvm.dbg.value(metadata ptr %4, metadata !220, metadata !DIExpression()), !dbg !221
  %5 = load i8, ptr %0, align 1, !dbg !226, !tbaa !227
  store i8 %5, ptr %3, align 1, !dbg !230, !tbaa !227
  %6 = getelementptr inbounds i8, ptr %0, i64 1, !dbg !231
  %7 = load i8, ptr %6, align 1, !dbg !231, !tbaa !227
  %8 = getelementptr inbounds i8, ptr %3, i64 1, !dbg !232
  store i8 %7, ptr %8, align 1, !dbg !233, !tbaa !227
  %9 = getelementptr inbounds i8, ptr %0, i64 2, !dbg !234
  %10 = load i8, ptr %9, align 1, !dbg !234, !tbaa !227
  store i8 %10, ptr %4, align 1, !dbg !235, !tbaa !227
  %11 = getelementptr inbounds i8, ptr %0, i64 3, !dbg !236
  %12 = load i8, ptr %11, align 1, !dbg !236, !tbaa !227
  %13 = getelementptr inbounds i8, ptr %4, i64 1, !dbg !237
  store i8 %12, ptr %13, align 1, !dbg !238, !tbaa !227
  store ptr %3, ptr @g_global_ptr, align 8, !dbg !239, !tbaa !240, !memtracer !242
  %14 = icmp sgt i32 %1, 0, !dbg !243
  %15 = select i1 %14, ptr %3, ptr %4, !dbg !244
  ret ptr %15, !dbg !245
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: argmemonly mustprogress nofree norecurse nounwind
define void @_Z17test_atomic_storePVcc(ptr noundef %0, i8 noundef %1) local_unnamed_addr #2 !dbg !246 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !250, metadata !DIExpression()), !dbg !252
  call void @llvm.dbg.value(metadata i8 %1, metadata !251, metadata !DIExpression()), !dbg !252
  store atomic volatile i8 %1, ptr %0 seq_cst, align 1, !dbg !253
  ret void, !dbg !254
}

; Function Attrs: mustprogress nounwind
define void @_Z15test_inline_asmPc(ptr noundef %0) local_unnamed_addr #3 !dbg !255 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !259, metadata !DIExpression()), !dbg !260
  tail call void asm sideeffect "str ${0:w}, [$1]", "r,r"(i32 42, ptr %0) #8, !dbg !261, !srcloc !262
  ret void, !dbg !263
}

; Function Attrs: mustprogress nofree nounwind willreturn
define noalias noundef ptr @_Z20test_noinline_mallocv() local_unnamed_addr #0 !dbg !264 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #7, !dbg !268
  ret ptr %1, !dbg !269
}

; Function Attrs: argmemonly mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define void @_Z19test_noinline_storePcc(ptr nocapture noundef writeonly %0, i8 noundef %1) local_unnamed_addr #4 !dbg !270 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !274, metadata !DIExpression()), !dbg !276
  call void @llvm.dbg.value(metadata i8 %1, metadata !275, metadata !DIExpression()), !dbg !276
  store i8 %1, ptr %0, align 1, !dbg !277, !tbaa !227
  ret void, !dbg !278
}

; Function Attrs: mustprogress norecurse
define noundef i32 @main() local_unnamed_addr #5 !dbg !279 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #7, !dbg !289
  call void @llvm.dbg.value(metadata ptr %1, metadata !281, metadata !DIExpression()), !dbg !291
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #7, !dbg !292
  call void @llvm.dbg.value(metadata ptr %2, metadata !282, metadata !DIExpression()), !dbg !291
  store i8 1, ptr %1, align 1, !dbg !294, !tbaa !227
  %3 = getelementptr inbounds i8, ptr %1, i64 1, !dbg !295
  store i8 2, ptr %3, align 1, !dbg !296, !tbaa !227
  %4 = getelementptr inbounds i8, ptr %1, i64 2, !dbg !297
  store i8 3, ptr %4, align 1, !dbg !298, !tbaa !227
  %5 = getelementptr inbounds i8, ptr %1, i64 3, !dbg !299
  store i8 4, ptr %5, align 1, !dbg !300, !tbaa !227
  call void @llvm.dbg.value(metadata i8 1, metadata !283, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 8)), !dbg !291
  call void @llvm.dbg.value(metadata i8 2, metadata !283, metadata !DIExpression(DW_OP_LLVM_fragment, 8, 8)), !dbg !291
  call void @llvm.dbg.value(metadata i8 3, metadata !283, metadata !DIExpression(DW_OP_LLVM_fragment, 16, 8)), !dbg !291
  call void @llvm.dbg.value(metadata i8 4, metadata !283, metadata !DIExpression(DW_OP_LLVM_fragment, 24, 8)), !dbg !291
  call void @llvm.dbg.value(metadata i32 134678021, metadata !283, metadata !DIExpression(DW_OP_LLVM_fragment, 32, 32)), !dbg !291
  call void @llvm.dbg.value(metadata ptr undef, metadata !217, metadata !DIExpression()), !dbg !301
  call void @llvm.dbg.value(metadata i32 5, metadata !218, metadata !DIExpression()), !dbg !301
  %6 = tail call dereferenceable_or_null(8) ptr @malloc(i64 noundef 8) #7, !dbg !303, !memtracer !223
  call void @llvm.dbg.value(metadata ptr %6, metadata !219, metadata !DIExpression()), !dbg !301
  call void @llvm.dbg.value(metadata ptr undef, metadata !220, metadata !DIExpression()), !dbg !301
  store i8 1, ptr %6, align 1, !dbg !304, !tbaa !227
  %7 = getelementptr inbounds i8, ptr %6, i64 1, !dbg !305
  store i8 2, ptr %7, align 1, !dbg !306, !tbaa !227
  store ptr %6, ptr @g_global_ptr, align 8, !dbg !307, !tbaa !240, !memtracer !242
  call void @llvm.dbg.value(metadata ptr %6, metadata !287, metadata !DIExpression()), !dbg !291
  call void @llvm.dbg.value(metadata ptr %2, metadata !288, metadata !DIExpression()), !dbg !291
  call void @llvm.dbg.value(metadata ptr %2, metadata !250, metadata !DIExpression()), !dbg !308
  call void @llvm.dbg.value(metadata i8 100, metadata !251, metadata !DIExpression()), !dbg !308
  store atomic volatile i8 100, ptr %2 seq_cst, align 1, !dbg !310
  call void @llvm.dbg.value(metadata ptr %1, metadata !259, metadata !DIExpression()), !dbg !311
  tail call void asm sideeffect "str ${0:w}, [$1]", "r,r"(i32 42, ptr nonnull %1) #8, !dbg !313, !srcloc !262
  %8 = load i8, ptr %6, align 1, !dbg !314, !tbaa !227
  %9 = zext i8 %8 to i32, !dbg !314
  store i32 %9, ptr @g_global_val, align 4, !dbg !315, !tbaa !316
  ret i32 0, !dbg !318
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #6

attributes #0 = { mustprogress nofree nounwind willreturn "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #2 = { argmemonly mustprogress nofree norecurse nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #3 = { mustprogress nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #4 = { argmemonly mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #5 = { mustprogress norecurse "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #6 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #7 = { allocsize(0) "reference-tracking"="true" }
attributes #8 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!206, !207, !208, !209, !210, !211}
!llvm.ident = !{!212}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_global_ptr", scope: !2, file: !3, line: 3, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 15.0.4", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !9, imports: !13, splitDebugInlining: false, nameTableKind: None)
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
!40 = !DISubprogram(name: "atof", scope: !23, file: !23, line: 26, type: !41, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!41 = !DISubroutineType(types: !42)
!42 = !{!43, !44}
!43 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !45, size: 64)
!45 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!46 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !47, file: !20, line: 99)
!47 = !DISubprogram(name: "atoi", scope: !23, file: !23, line: 23, type: !48, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!48 = !DISubroutineType(types: !49)
!49 = !{!12, !44}
!50 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !51, file: !20, line: 100)
!51 = !DISubprogram(name: "atol", scope: !23, file: !23, line: 24, type: !52, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!52 = !DISubroutineType(types: !53)
!53 = !{!30, !44}
!54 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !55, file: !20, line: 101)
!55 = !DISubprogram(name: "atoll", scope: !23, file: !23, line: 25, type: !56, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!56 = !DISubroutineType(types: !57)
!57 = !{!37, !44}
!58 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !59, file: !20, line: 102)
!59 = !DISubprogram(name: "strtod", scope: !23, file: !23, line: 29, type: !60, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!60 = !DISubroutineType(types: !61)
!61 = !{!43, !62, !63}
!62 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !44)
!63 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !64)
!64 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!65 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !66, file: !20, line: 103)
!66 = !DISubprogram(name: "strtof", scope: !23, file: !23, line: 28, type: !67, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!67 = !DISubroutineType(types: !68)
!68 = !{!69, !62, !63}
!69 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!70 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !71, file: !20, line: 104)
!71 = !DISubprogram(name: "strtold", scope: !23, file: !23, line: 30, type: !72, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!72 = !DISubroutineType(types: !73)
!73 = !{!74, !62, !63}
!74 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!75 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !76, file: !20, line: 105)
!76 = !DISubprogram(name: "strtol", scope: !23, file: !23, line: 32, type: !77, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!77 = !DISubroutineType(types: !78)
!78 = !{!30, !62, !63, !12}
!79 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !80, file: !20, line: 106)
!80 = !DISubprogram(name: "strtoll", scope: !23, file: !23, line: 34, type: !81, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!81 = !DISubroutineType(types: !82)
!82 = !{!37, !62, !63, !12}
!83 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !84, file: !20, line: 107)
!84 = !DISubprogram(name: "strtoul", scope: !23, file: !23, line: 33, type: !85, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!85 = !DISubroutineType(types: !86)
!86 = !{!19, !62, !63, !12}
!87 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !88, file: !20, line: 108)
!88 = !DISubprogram(name: "strtoull", scope: !23, file: !23, line: 35, type: !89, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!89 = !DISubroutineType(types: !90)
!90 = !{!91, !62, !63, !12}
!91 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!92 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !93, file: !20, line: 109)
!93 = !DISubprogram(name: "rand", scope: !23, file: !23, line: 37, type: !94, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!94 = !DISubroutineType(types: !95)
!95 = !{!12}
!96 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !97, file: !20, line: 110)
!97 = !DISubprogram(name: "srand", scope: !23, file: !23, line: 38, type: !98, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!98 = !DISubroutineType(types: !99)
!99 = !{null, !100}
!100 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!101 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !102, file: !20, line: 111)
!102 = !DISubprogram(name: "calloc", scope: !23, file: !23, line: 41, type: !103, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!103 = !DISubroutineType(types: !104)
!104 = !{!105, !17, !17}
!105 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!106 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !107, file: !20, line: 112)
!107 = !DISubprogram(name: "free", scope: !23, file: !23, line: 43, type: !108, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !105}
!110 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !111, file: !20, line: 113)
!111 = !DISubprogram(name: "malloc", scope: !23, file: !23, line: 40, type: !112, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!112 = !DISubroutineType(types: !113)
!113 = !{!105, !17}
!114 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !115, file: !20, line: 114)
!115 = !DISubprogram(name: "realloc", scope: !23, file: !23, line: 42, type: !116, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!116 = !DISubroutineType(types: !117)
!117 = !{!105, !105, !17}
!118 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !119, file: !20, line: 115)
!119 = !DISubprogram(name: "abort", scope: !23, file: !23, line: 46, type: !120, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!120 = !DISubroutineType(types: !121)
!121 = !{null}
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !123, file: !20, line: 116)
!123 = !DISubprogram(name: "atexit", scope: !23, file: !23, line: 48, type: !124, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!124 = !DISubroutineType(types: !125)
!125 = !{!12, !126}
!126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !120, size: 64)
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !128, file: !20, line: 117)
!128 = !DISubprogram(name: "exit", scope: !23, file: !23, line: 49, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!129 = !DISubroutineType(types: !130)
!130 = !{null, !12}
!131 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !132, file: !20, line: 118)
!132 = !DISubprogram(name: "_Exit", scope: !23, file: !23, line: 50, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!133 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !134, file: !20, line: 119)
!134 = !DISubprogram(name: "getenv", scope: !23, file: !23, line: 54, type: !135, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!135 = !DISubroutineType(types: !136)
!136 = !{!5, !44}
!137 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !138, file: !20, line: 120)
!138 = !DISubprogram(name: "system", scope: !23, file: !23, line: 56, type: !48, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!139 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !140, file: !20, line: 121)
!140 = !DISubprogram(name: "bsearch", scope: !23, file: !23, line: 58, type: !141, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!141 = !DISubroutineType(types: !142)
!142 = !{!105, !143, !143, !17, !17, !145}
!143 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !144, size: 64)
!144 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!145 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !146, size: 64)
!146 = !DISubroutineType(types: !147)
!147 = !{!12, !143, !143}
!148 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !149, file: !20, line: 122)
!149 = !DISubprogram(name: "qsort", scope: !23, file: !23, line: 59, type: !150, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!150 = !DISubroutineType(types: !151)
!151 = !{null, !105, !17, !17, !145}
!152 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !153, file: !20, line: 123)
!153 = !DISubprogram(name: "abs", linkageName: "_Z3absB6v15004e", scope: !154, file: !154, line: 129, type: !155, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!154 = !DIFile(filename: "stdlib.h", directory: "")
!155 = !DISubroutineType(types: !156)
!156 = !{!74, !74}
!157 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !158, file: !20, line: 124)
!158 = !DISubprogram(name: "labs", scope: !23, file: !23, line: 62, type: !159, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!159 = !DISubroutineType(types: !160)
!160 = !{!30, !30}
!161 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !162, file: !20, line: 125)
!162 = !DISubprogram(name: "llabs", scope: !23, file: !23, line: 63, type: !163, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!163 = !DISubroutineType(types: !164)
!164 = !{!37, !37}
!165 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !166, file: !20, line: 126)
!166 = !DISubprogram(name: "div", linkageName: "_Z3divB6v15004xx", scope: !154, file: !154, line: 152, type: !167, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!167 = !DISubroutineType(types: !168)
!168 = !{!33, !37, !37}
!169 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !170, file: !20, line: 127)
!170 = !DISubprogram(name: "ldiv", scope: !23, file: !23, line: 70, type: !171, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!171 = !DISubroutineType(types: !172)
!172 = !{!26, !30, !30}
!173 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !174, file: !20, line: 128)
!174 = !DISubprogram(name: "lldiv", scope: !23, file: !23, line: 71, type: !167, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!175 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !176, file: !20, line: 129)
!176 = !DISubprogram(name: "mblen", scope: !23, file: !23, line: 73, type: !177, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!177 = !DISubroutineType(types: !178)
!178 = !{!12, !44, !17}
!179 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !180, file: !20, line: 130)
!180 = !DISubprogram(name: "mbtowc", scope: !23, file: !23, line: 74, type: !181, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!181 = !DISubroutineType(types: !182)
!182 = !{!12, !183, !62, !17}
!183 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !184)
!184 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !185, size: 64)
!185 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!186 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !187, file: !20, line: 131)
!187 = !DISubprogram(name: "wctomb", scope: !23, file: !23, line: 75, type: !188, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!188 = !DISubroutineType(types: !189)
!189 = !{!12, !5, !185}
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !191, file: !20, line: 132)
!191 = !DISubprogram(name: "mbstowcs", scope: !23, file: !23, line: 76, type: !192, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!192 = !DISubroutineType(types: !193)
!193 = !{!17, !183, !62, !17}
!194 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !195, file: !20, line: 133)
!195 = !DISubprogram(name: "wcstombs", scope: !23, file: !23, line: 77, type: !196, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!196 = !DISubroutineType(types: !197)
!197 = !{!17, !198, !199, !17}
!198 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !5)
!199 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !200)
!200 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !201, size: 64)
!201 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !185)
!202 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !203, file: !20, line: 135)
!203 = !DISubprogram(name: "at_quick_exit", scope: !23, file: !23, line: 51, type: !124, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!204 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !15, entity: !205, file: !20, line: 136)
!205 = !DISubprogram(name: "quick_exit", scope: !23, file: !23, line: 52, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!206 = !{i32 7, !"Dwarf Version", i32 5}
!207 = !{i32 7, !"ReferenceTracking", i32 1}
!208 = !{i32 2, !"Debug Info Version", i32 3}
!209 = !{i32 1, !"wchar_size", i32 4}
!210 = !{i32 7, !"PIC Level", i32 2}
!211 = !{i32 7, !"frame-pointer", i32 1}
!212 = !{!"clang version 15.0.4"}
!213 = distinct !DISubprogram(name: "test_complex_flow", linkageName: "_Z17test_complex_flowPci", scope: !3, file: !3, line: 6, type: !214, scopeLine: 6, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !216)
!214 = !DISubroutineType(types: !215)
!215 = !{!5, !5, !12}
!216 = !{!217, !218, !219, !220}
!217 = !DILocalVariable(name: "input", arg: 1, scope: !213, file: !3, line: 6, type: !5)
!218 = !DILocalVariable(name: "n", arg: 2, scope: !213, file: !3, line: 6, type: !12)
!219 = !DILocalVariable(name: "p1", scope: !213, file: !3, line: 7, type: !5)
!220 = !DILocalVariable(name: "p2", scope: !213, file: !3, line: 8, type: !5)
!221 = !DILocation(line: 0, scope: !213)
!222 = !DILocation(line: 7, column: 23, scope: !213)
!223 = !{!"p1", !"char*"}
!224 = !DILocation(line: 8, column: 23, scope: !213)
!225 = !{!"p2", !"char*"}
!226 = !DILocation(line: 10, column: 13, scope: !213)
!227 = !{!228, !228, i64 0}
!228 = !{!"omnipotent char", !229, i64 0}
!229 = !{!"Simple C++ TBAA"}
!230 = !DILocation(line: 10, column: 11, scope: !213)
!231 = !DILocation(line: 11, column: 13, scope: !213)
!232 = !DILocation(line: 11, column: 5, scope: !213)
!233 = !DILocation(line: 11, column: 11, scope: !213)
!234 = !DILocation(line: 12, column: 13, scope: !213)
!235 = !DILocation(line: 12, column: 11, scope: !213)
!236 = !DILocation(line: 13, column: 13, scope: !213)
!237 = !DILocation(line: 13, column: 5, scope: !213)
!238 = !DILocation(line: 13, column: 11, scope: !213)
!239 = !DILocation(line: 15, column: 18, scope: !213)
!240 = !{!241, !241, i64 0}
!241 = !{!"any pointer", !228, i64 0}
!242 = !{!"g_global_ptr", !"char*"}
!243 = !DILocation(line: 17, column: 15, scope: !213)
!244 = !DILocation(line: 17, column: 12, scope: !213)
!245 = !DILocation(line: 17, column: 5, scope: !213)
!246 = distinct !DISubprogram(name: "test_atomic_store", linkageName: "_Z17test_atomic_storePVcc", scope: !3, file: !3, line: 20, type: !247, scopeLine: 20, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !249)
!247 = !DISubroutineType(types: !248)
!248 = !{null, !7, !6}
!249 = !{!250, !251}
!250 = !DILocalVariable(name: "ptr", arg: 1, scope: !246, file: !3, line: 20, type: !7)
!251 = !DILocalVariable(name: "val", arg: 2, scope: !246, file: !3, line: 20, type: !6)
!252 = !DILocation(line: 0, scope: !246)
!253 = !DILocation(line: 21, column: 5, scope: !246)
!254 = !DILocation(line: 22, column: 1, scope: !246)
!255 = distinct !DISubprogram(name: "test_inline_asm", linkageName: "_Z15test_inline_asmPc", scope: !3, file: !3, line: 24, type: !256, scopeLine: 24, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !258)
!256 = !DISubroutineType(types: !257)
!257 = !{null, !5}
!258 = !{!259}
!259 = !DILocalVariable(name: "ptr", arg: 1, scope: !255, file: !3, line: 24, type: !5)
!260 = !DILocation(line: 0, scope: !255)
!261 = !DILocation(line: 25, column: 5, scope: !255)
!262 = !{i64 536}
!263 = !DILocation(line: 26, column: 1, scope: !255)
!264 = distinct !DISubprogram(name: "test_noinline_malloc", linkageName: "_Z20test_noinline_mallocv", scope: !3, file: !3, line: 28, type: !265, scopeLine: 28, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !267)
!265 = !DISubroutineType(types: !266)
!266 = !{!5}
!267 = !{}
!268 = !DILocation(line: 29, column: 19, scope: !264)
!269 = !DILocation(line: 29, column: 5, scope: !264)
!270 = distinct !DISubprogram(name: "test_noinline_store", linkageName: "_Z19test_noinline_storePcc", scope: !3, file: !3, line: 33, type: !271, scopeLine: 33, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !273)
!271 = !DISubroutineType(types: !272)
!272 = !{null, !5, !6}
!273 = !{!274, !275}
!274 = !DILocalVariable(name: "ptr", arg: 1, scope: !270, file: !3, line: 33, type: !5)
!275 = !DILocalVariable(name: "val", arg: 2, scope: !270, file: !3, line: 33, type: !6)
!276 = !DILocation(line: 0, scope: !270)
!277 = !DILocation(line: 34, column: 10, scope: !270)
!278 = !DILocation(line: 35, column: 1, scope: !270)
!279 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 37, type: !94, scopeLine: 37, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !280)
!280 = !{!281, !282, !283, !287, !288}
!281 = !DILocalVariable(name: "p1", scope: !279, file: !3, line: 39, type: !5)
!282 = !DILocalVariable(name: "p2", scope: !279, file: !3, line: 40, type: !5)
!283 = !DILocalVariable(name: "data", scope: !279, file: !3, line: 52, type: !284)
!284 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64, elements: !285)
!285 = !{!286}
!286 = !DISubrange(count: 8)
!287 = !DILocalVariable(name: "result", scope: !279, file: !3, line: 53, type: !5)
!288 = !DILocalVariable(name: "atomic_ptr", scope: !279, file: !3, line: 56, type: !7)
!289 = !DILocation(line: 29, column: 19, scope: !264, inlinedAt: !290)
!290 = distinct !DILocation(line: 39, column: 16, scope: !279)
!291 = !DILocation(line: 0, scope: !279)
!292 = !DILocation(line: 29, column: 19, scope: !264, inlinedAt: !293)
!293 = distinct !DILocation(line: 40, column: 16, scope: !279)
!294 = !DILocation(line: 43, column: 11, scope: !279)
!295 = !DILocation(line: 44, column: 5, scope: !279)
!296 = !DILocation(line: 44, column: 11, scope: !279)
!297 = !DILocation(line: 45, column: 5, scope: !279)
!298 = !DILocation(line: 45, column: 11, scope: !279)
!299 = !DILocation(line: 46, column: 5, scope: !279)
!300 = !DILocation(line: 46, column: 11, scope: !279)
!301 = !DILocation(line: 0, scope: !213, inlinedAt: !302)
!302 = distinct !DILocation(line: 53, column: 20, scope: !279)
!303 = !DILocation(line: 7, column: 23, scope: !213, inlinedAt: !302)
!304 = !DILocation(line: 10, column: 11, scope: !213, inlinedAt: !302)
!305 = !DILocation(line: 11, column: 5, scope: !213, inlinedAt: !302)
!306 = !DILocation(line: 11, column: 11, scope: !213, inlinedAt: !302)
!307 = !DILocation(line: 15, column: 18, scope: !213, inlinedAt: !302)
!308 = !DILocation(line: 0, scope: !246, inlinedAt: !309)
!309 = distinct !DILocation(line: 57, column: 5, scope: !279)
!310 = !DILocation(line: 21, column: 5, scope: !246, inlinedAt: !309)
!311 = !DILocation(line: 0, scope: !255, inlinedAt: !312)
!312 = distinct !DILocation(line: 60, column: 5, scope: !279)
!313 = !DILocation(line: 25, column: 5, scope: !255, inlinedAt: !312)
!314 = !DILocation(line: 63, column: 29, scope: !279)
!315 = !DILocation(line: 63, column: 18, scope: !279)
!316 = !{!317, !317, i64 0}
!317 = !{!"int", !228, i64 0}
!318 = !DILocation(line: 65, column: 5, scope: !279)