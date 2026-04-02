; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'static-value.cpp'
source_filename = "static-value.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

@_ZZ7get_ptrvE3ptr = internal global ptr null, align 8, !dbg !0

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef ptr @_Z7get_ptrv() #0 !dbg !2 {
  %1 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !19
  %2 = icmp eq ptr %1, null, !dbg !21
  br i1 %2, label %3, label %6, !dbg !22

3:                                                ; preds = %0
  %4 = call noalias ptr @malloc(i64 noundef 4) #2, !dbg !23
  store ptr %4, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !25
  %5 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !26
  store i32 1, ptr %5, align 4, !dbg !27
  br label %6, !dbg !28

6:                                                ; preds = %3, %0
  %7 = load ptr, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !29
  ret ptr %7, !dbg !30
}

; CHECK: %4 = call noalias ptr @malloc(i64 noundef 4) #2, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %4, ptr @_ZZ7get_ptrvE3ptr, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}

; Function Attrs: nounwind allocsize(0)
declare noalias ptr @malloc(i64 noundef) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { nounwind allocsize(0) }

!llvm.dbg.cu = !{!8}
!llvm.module.flags = !{!11, !12, !13, !14, !15, !16, !17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ptr", scope: !2, file: !3, line: 4, type: !6, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "get_ptr", linkageName: "_Z7get_ptrv", scope: !3, file: !3, line: 3, type: !4, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !8, retainedNodes: !10)
!3 = !DIFile(filename: "static-value.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "db5b63a8854a8f045d35ef6fee725429")
!4 = !DISubroutineType(types: !5)
!5 = !{!6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !5, globals: !9, splitDebugInlining: false, nameTableKind: None)
!9 = !{!0}
!10 = !{}
!11 = !{i32 7, !"Dwarf Version", i32 5}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 8, !"PIC Level", i32 2}
!15 = !{i32 7, !"PIE Level", i32 2}
!16 = !{i32 7, !"uwtable", i32 2}
!17 = !{i32 7, !"frame-pointer", i32 1}
!18 = !{!"clang"}
!19 = !DILocation(line: 5, column: 9, scope: !20)
!20 = distinct !DILexicalBlock(scope: !2, file: !3, line: 5, column: 9)
!21 = !DILocation(line: 5, column: 13, scope: !20)
!22 = !DILocation(line: 5, column: 9, scope: !2)
!23 = !DILocation(line: 6, column: 22, scope: !24)
!24 = distinct !DILexicalBlock(scope: !20, file: !3, line: 5, column: 22)
!25 = !DILocation(line: 6, column: 13, scope: !24)
!26 = !DILocation(line: 7, column: 10, scope: !24)
!27 = !DILocation(line: 7, column: 14, scope: !24)
!28 = !DILocation(line: 8, column: 5, scope: !24)
!29 = !DILocation(line: 9, column: 12, scope: !2)
!30 = !DILocation(line: 9, column: 5, scope: !2)

; CHECK: !{{[0-9]+}} = !{!"ptr", !"int*"}
