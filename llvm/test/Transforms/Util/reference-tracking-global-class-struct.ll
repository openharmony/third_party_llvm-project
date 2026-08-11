; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; AsmWriter prints attachments sorted by metadata kind ID; opt output order of
; !heapallocsite vs !memtracer after !dbg is not guaranteed.

; ModuleID = 'global_class_struct.cpp'
source_filename = "global_class_struct.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

%struct.Node = type { i32, ptr }
%class.A = type { i32, ptr }

@node = dso_local global ptr null, align 8, !dbg !0
@a = dso_local global ptr null, align 8, !dbg !5

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !27 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !31, !DIExpression(), !32)
  %3 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !33, !heapallocsite !15
  call void @llvm.memset.p0.i64(ptr align 16 %3, i8 0, i64 16, i1 false), !dbg !34
  store ptr %3, ptr %2, align 8, !dbg !32
  %4 = load ptr, ptr %2, align 8, !dbg !35
  %5 = getelementptr inbounds nuw %struct.Node, ptr %4, i32 0, i32 0, !dbg !36
  store i32 1, ptr %5, align 8, !dbg !37
  %6 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !38, !heapallocsite !15
  call void @llvm.memset.p0.i64(ptr align 16 %6, i8 0, i64 16, i1 false), !dbg !39
  %7 = load ptr, ptr %2, align 8, !dbg !40
  %8 = getelementptr inbounds nuw %struct.Node, ptr %7, i32 0, i32 1, !dbg !41
  store ptr %6, ptr %8, align 8, !dbg !42
  %9 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !43, !heapallocsite !15
  call void @llvm.memset.p0.i64(ptr align 16 %9, i8 0, i64 16, i1 false), !dbg !44
  %10 = load ptr, ptr %2, align 8, !dbg !45
  %11 = getelementptr inbounds nuw %struct.Node, ptr %10, i32 0, i32 1, !dbg !46
  %12 = load ptr, ptr %11, align 8, !dbg !46
  %13 = getelementptr inbounds nuw %struct.Node, ptr %12, i32 0, i32 1, !dbg !47
  store ptr %9, ptr %13, align 8, !dbg !48
  %14 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !49, !heapallocsite !9
  call void @llvm.memset.p0.i64(ptr align 16 %14, i8 0, i64 16, i1 false), !dbg !50
  store ptr %14, ptr @a, align 8, !dbg !51
  %15 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !52, !heapallocsite !15
  call void @llvm.memset.p0.i64(ptr align 16 %15, i8 0, i64 16, i1 false), !dbg !53
  %16 = load ptr, ptr @a, align 8, !dbg !54
  %17 = getelementptr inbounds nuw %class.A, ptr %16, i32 0, i32 1, !dbg !55
  store ptr %15, ptr %17, align 8, !dbg !56
  ret i32 0, !dbg !57
}

; CHECK: %3 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %3, ptr %2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %6 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %6, ptr %8, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %9 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %9, ptr %13, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %14 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %14, ptr @a, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %15 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %15, ptr %17, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #2

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { nobuiltin allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #3 = { builtin allocsize(0) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!19, !20, !21, !22, !23, !24, !25}
!llvm.ident = !{!26}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "node", scope: !2, file: !7, line: 3, type: !14, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/root/mem_map/llvm_test/global_class_struct.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "3fa33536bf73b5d2905c83abcee5554e")
!4 = !{!0, !5}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !7, line: 4, type: !8, isLocal: false, isDefinition: true)
!7 = !DIFile(filename: "global_class_struct.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "3fa33536bf73b5d2905c83abcee5554e")
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!9 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !7, line: 2, size: 128, flags: DIFlagTypePassByValue, elements: !10, identifier: "_ZTS1A")
!10 = !{!11, !13}
!11 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !9, file: !7, line: 2, baseType: !12, size: 32, flags: DIFlagPublic)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !9, file: !7, line: 2, baseType: !14, size: 64, offset: 64, flags: DIFlagPublic)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!15 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Node", file: !7, line: 1, size: 128, flags: DIFlagTypePassByValue, elements: !16, identifier: "_ZTS4Node")
!16 = !{!17, !18}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !15, file: !7, line: 1, baseType: !12, size: 32)
!18 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !15, file: !7, line: 1, baseType: !14, size: 64, offset: 64)
!19 = !{i32 7, !"Dwarf Version", i32 5}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"wchar_size", i32 4}
!22 = !{i32 8, !"PIC Level", i32 2}
!23 = !{i32 7, !"PIE Level", i32 2}
!24 = !{i32 7, !"uwtable", i32 2}
!25 = !{i32 7, !"frame-pointer", i32 1}
!26 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!27 = distinct !DISubprogram(name: "main", scope: !7, file: !7, line: 5, type: !28, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !30)
!28 = !DISubroutineType(types: !29)
!29 = !{!12}
!30 = !{}
!31 = !DILocalVariable(name: "head", scope: !27, file: !7, line: 6, type: !14)
!32 = !DILocation(line: 6, column: 9, scope: !27)
!33 = !DILocation(line: 6, column: 16, scope: !27)
!34 = !DILocation(line: 6, column: 20, scope: !27)
!35 = !DILocation(line: 7, column: 3, scope: !27)
!36 = !DILocation(line: 7, column: 9, scope: !27)
!37 = !DILocation(line: 7, column: 13, scope: !27)
!38 = !DILocation(line: 8, column: 16, scope: !27)
!39 = !DILocation(line: 8, column: 20, scope: !27)
!40 = !DILocation(line: 8, column: 3, scope: !27)
!41 = !DILocation(line: 8, column: 9, scope: !27)
!42 = !DILocation(line: 8, column: 14, scope: !27)
!43 = !DILocation(line: 9, column: 22, scope: !27)
!44 = !DILocation(line: 9, column: 26, scope: !27)
!45 = !DILocation(line: 9, column: 3, scope: !27)
!46 = !DILocation(line: 9, column: 9, scope: !27)
!47 = !DILocation(line: 9, column: 15, scope: !27)
!48 = !DILocation(line: 9, column: 20, scope: !27)
!49 = !DILocation(line: 10, column: 7, scope: !27)
!50 = !DILocation(line: 10, column: 11, scope: !27)
!51 = !DILocation(line: 10, column: 5, scope: !27)
!52 = !DILocation(line: 11, column: 13, scope: !27)
!53 = !DILocation(line: 11, column: 17, scope: !27)
!54 = !DILocation(line: 11, column: 3, scope: !27)
!55 = !DILocation(line: 11, column: 6, scope: !27)
!56 = !DILocation(line: 11, column: 11, scope: !27)
!57 = !DILocation(line: 12, column: 3, scope: !27)

; CHECK: !{{[0-9]+}} = !{!"head", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"head->next", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"head->next->next", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"a", !"A*"}
; CHECK: !{{[0-9]+}} = !{!"a->node", !"Node*"}
