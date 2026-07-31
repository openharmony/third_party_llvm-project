; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'static-value.cpp'
source_filename = "static-value.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@_ZZ7get_ptrvE3ptr = internal global ptr null, align 8, !dbg !0

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef ptr @_Z7get_ptrv() #0 !dbg !2 {
  %1 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !19
  %2 = icmp eq ptr %1, null, !dbg !21
  br i1 %2, label %3, label %6, !dbg !21

3:                                                ; preds = %0
  %4 = call ptr @malloc(i64 noundef 4) #2, !dbg !22
  store ptr %4, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !24
  %5 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !25
  store i32 1, ptr %5, align 4, !dbg !26
  br label %6, !dbg !27

6:                                                ; preds = %3, %0
  %7 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !28
  ret ptr %7, !dbg !29
}

; CHECK: %4 = call ptr @malloc(i64 noundef 4) #2, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %4, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { allocsize(0) }

!llvm.dbg.cu = !{!8}
!llvm.module.flags = !{!11, !12, !13, !14, !15, !16, !17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ptr", scope: !2, file: !3, line: 3, type: !6, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "get_ptr", linkageName: "_Z7get_ptrv", scope: !3, file: !3, line: 2, type: !4, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !8)
!3 = !DIFile(filename: "static-value.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "70aab1cd334bb8bcb73572ce11bb94b0")
!4 = !DISubroutineType(types: !5)
!5 = !{!6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !9, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !5, globals: !10, splitDebugInlining: false, nameTableKind: None)
!9 = !DIFile(filename: "/root/mem_map/llvm_test/static-value.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "70aab1cd334bb8bcb73572ce11bb94b0")
!10 = !{!0}
!11 = !{i32 7, !"Dwarf Version", i32 5}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 8, !"PIC Level", i32 2}
!15 = !{i32 7, !"PIE Level", i32 2}
!16 = !{i32 7, !"uwtable", i32 2}
!17 = !{i32 7, !"frame-pointer", i32 1}
!18 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!19 = !DILocation(line: 4, column: 7, scope: !20)
!20 = distinct !DILexicalBlock(scope: !2, file: !3, line: 4, column: 7)
!21 = !DILocation(line: 4, column: 11, scope: !20)
!22 = !DILocation(line: 5, column: 18, scope: !23)
!23 = distinct !DILexicalBlock(scope: !20, file: !3, line: 4, column: 23)
!24 = !DILocation(line: 5, column: 9, scope: !23)
!25 = !DILocation(line: 6, column: 6, scope: !23)
!26 = !DILocation(line: 6, column: 10, scope: !23)
!27 = !DILocation(line: 7, column: 3, scope: !23)
!28 = !DILocation(line: 8, column: 10, scope: !2)
!29 = !DILocation(line: 8, column: 3, scope: !2)

; CHECK: !{{[0-9]+}} = !{!"ptr", !"int*"}
