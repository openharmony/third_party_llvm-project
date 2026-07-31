; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'local_class_struct.cpp'
source_filename = "local_class_struct.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

%struct.Node = type { i32, ptr }
%class.A = type { i32, ptr }

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !16, !DIExpression(), !22)
  store ptr null, ptr %2, align 8, !dbg !22
    #dbg_declare(ptr %3, !23, !DIExpression(), !29)
  store ptr null, ptr %3, align 8, !dbg !29
    #dbg_declare(ptr %4, !30, !DIExpression(), !31)
  %5 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !32, !heapallocsite !18
  call void @llvm.memset.p0.i64(ptr align 16 %5, i8 0, i64 16, i1 false), !dbg !33
  store ptr %5, ptr %4, align 8, !dbg !31
  %6 = load ptr, ptr %4, align 8, !dbg !34
  %7 = getelementptr inbounds nuw %struct.Node, ptr %6, i32 0, i32 0, !dbg !35
  store i32 1, ptr %7, align 8, !dbg !36
  %8 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !37, !heapallocsite !18
  call void @llvm.memset.p0.i64(ptr align 16 %8, i8 0, i64 16, i1 false), !dbg !38
  %9 = load ptr, ptr %4, align 8, !dbg !39
  %10 = getelementptr inbounds nuw %struct.Node, ptr %9, i32 0, i32 1, !dbg !40
  store ptr %8, ptr %10, align 8, !dbg !41
  %11 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !42, !heapallocsite !18
  call void @llvm.memset.p0.i64(ptr align 16 %11, i8 0, i64 16, i1 false), !dbg !43
  %12 = load ptr, ptr %4, align 8, !dbg !44
  %13 = getelementptr inbounds nuw %struct.Node, ptr %12, i32 0, i32 1, !dbg !45
  %14 = load ptr, ptr %13, align 8, !dbg !45
  %15 = getelementptr inbounds nuw %struct.Node, ptr %14, i32 0, i32 1, !dbg !46
  store ptr %11, ptr %15, align 8, !dbg !47
  %16 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !48, !heapallocsite !25
  call void @llvm.memset.p0.i64(ptr align 16 %16, i8 0, i64 16, i1 false), !dbg !49
  store ptr %16, ptr %3, align 8, !dbg !50
  %17 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !51, !heapallocsite !18
  call void @llvm.memset.p0.i64(ptr align 16 %17, i8 0, i64 16, i1 false), !dbg !52
  %18 = load ptr, ptr %3, align 8, !dbg !53
  %19 = getelementptr inbounds nuw %class.A, ptr %18, i32 0, i32 1, !dbg !54
  store ptr %17, ptr %19, align 8, !dbg !55
  %20 = load ptr, ptr %4, align 8, !dbg !56
  store ptr %20, ptr %2, align 8, !dbg !57
  ret i32 0, !dbg !58
}

; CHECK: store ptr null, ptr %2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr null, ptr %3, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %5 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %5, ptr %4, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %8 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %8, ptr %10, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %11 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %11, ptr %15, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %16 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %16, ptr %3, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %17 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 16) #3, !dbg !{{[0-9]+}}, {{(.*!heapallocsite ![0-9]+.*!memtracer ![0-9]+)|(.*!memtracer ![0-9]+.*!heapallocsite ![0-9]+)}}
; CHECK: store ptr %17, ptr %19, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %20, ptr %2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #2

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { nobuiltin allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #3 = { builtin allocsize(0) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/root/mem_map/llvm_test/local_class_struct.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "c03a9a9328ee44de8b45d7fe28c175c9")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 1}
!9 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!10 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 3, type: !12, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DIFile(filename: "local_class_struct.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "c03a9a9328ee44de8b45d7fe28c175c9")
!12 = !DISubroutineType(types: !13)
!13 = !{!14}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "head", scope: !10, file: !11, line: 4, type: !17)
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Node", file: !11, line: 1, size: 128, flags: DIFlagTypePassByValue, elements: !19, identifier: "_ZTS4Node")
!19 = !{!20, !21}
!20 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !18, file: !11, line: 1, baseType: !14, size: 32)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !18, file: !11, line: 1, baseType: !17, size: 64, offset: 64)
!22 = !DILocation(line: 4, column: 9, scope: !10)
!23 = !DILocalVariable(name: "a", scope: !10, file: !11, line: 5, type: !24)
!24 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !25, size: 64)
!25 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !11, line: 2, size: 128, flags: DIFlagTypePassByValue, elements: !26, identifier: "_ZTS1A")
!26 = !{!27, !28}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !25, file: !11, line: 2, baseType: !14, size: 32, flags: DIFlagPublic)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !25, file: !11, line: 2, baseType: !17, size: 64, offset: 64, flags: DIFlagPublic)
!29 = !DILocation(line: 5, column: 6, scope: !10)
!30 = !DILocalVariable(name: "local_head", scope: !10, file: !11, line: 6, type: !17)
!31 = !DILocation(line: 6, column: 9, scope: !10)
!32 = !DILocation(line: 6, column: 22, scope: !10)
!33 = !DILocation(line: 6, column: 26, scope: !10)
!34 = !DILocation(line: 7, column: 3, scope: !10)
!35 = !DILocation(line: 7, column: 15, scope: !10)
!36 = !DILocation(line: 7, column: 19, scope: !10)
!37 = !DILocation(line: 8, column: 22, scope: !10)
!38 = !DILocation(line: 8, column: 26, scope: !10)
!39 = !DILocation(line: 8, column: 3, scope: !10)
!40 = !DILocation(line: 8, column: 15, scope: !10)
!41 = !DILocation(line: 8, column: 20, scope: !10)
!42 = !DILocation(line: 9, column: 28, scope: !10)
!43 = !DILocation(line: 9, column: 32, scope: !10)
!44 = !DILocation(line: 9, column: 3, scope: !10)
!45 = !DILocation(line: 9, column: 15, scope: !10)
!46 = !DILocation(line: 9, column: 21, scope: !10)
!47 = !DILocation(line: 9, column: 26, scope: !10)
!48 = !DILocation(line: 10, column: 7, scope: !10)
!49 = !DILocation(line: 10, column: 11, scope: !10)
!50 = !DILocation(line: 10, column: 5, scope: !10)
!51 = !DILocation(line: 11, column: 13, scope: !10)
!52 = !DILocation(line: 11, column: 17, scope: !10)
!53 = !DILocation(line: 11, column: 3, scope: !10)
!54 = !DILocation(line: 11, column: 6, scope: !10)
!55 = !DILocation(line: 11, column: 11, scope: !10)
!56 = !DILocation(line: 12, column: 10, scope: !10)
!57 = !DILocation(line: 12, column: 8, scope: !10)
!58 = !DILocation(line: 13, column: 3, scope: !10)

; CHECK: !{{[0-9]+}} = !{!"head", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"a", !"A*"}
; CHECK: !{{[0-9]+}} = !{!"local_head", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"local_head->next", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"local_head->next->next", !"Node*"}
; CHECK: !{{[0-9]+}} = !{!"a->node", !"Node*"}
