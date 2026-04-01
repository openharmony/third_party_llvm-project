; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'malloc.cpp'
source_filename = "malloc.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

@g_ptr1 = dso_local global ptr null, align 8, !dbg !0
@g_ptr2 = dso_local global ptr null, align 8, !dbg !8

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z11init_memoryv() #0 !dbg !18 {
  %1 = call noalias ptr @malloc(i64 noundef 4) #4, !dbg !22
  store ptr %1, ptr @g_ptr1, align 8, !dbg !23
  %2 = call noalias ptr @malloc(i64 noundef 4) #4, !dbg !24
  store ptr %2, ptr @g_ptr2, align 8, !dbg !25
  %3 = load ptr, ptr @g_ptr1, align 8, !dbg !26
  store i32 1, ptr %3, align 4, !dbg !27
  %4 = load ptr, ptr @g_ptr2, align 8, !dbg !28
  store i32 2, ptr %4, align 4, !dbg !29
  ret void, !dbg !30
}

; CHECK: %1 = call noalias ptr @malloc(i64 noundef 4) #4, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %1, ptr @g_ptr1, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: %2 = call noalias ptr @malloc(i64 noundef 4) #4, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %2, ptr @g_ptr2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}

; Function Attrs: nounwind allocsize(0)
declare noalias ptr @malloc(i64 noundef) #1

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z11free_memoryv() #0 !dbg !31 {
  %1 = load ptr, ptr @g_ptr1, align 8, !dbg !32
  call void @free(ptr noundef %1) #5, !dbg !33
  %2 = load ptr, ptr @g_ptr2, align 8, !dbg !34
  call void @free(ptr noundef %2) #5, !dbg !35
  store ptr null, ptr @g_ptr1, align 8, !dbg !36
  store ptr null, ptr @g_ptr2, align 8, !dbg !37
  ret void, !dbg !38
}

; Function Attrs: nounwind
declare void @free(ptr noundef) #2

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() #3 !dbg !39 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @_Z11init_memoryv(), !dbg !42
  ret i32 0, !dbg !43
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { nounwind "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { mustprogress noinline norecurse nounwind uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #4 = { nounwind allocsize(0) }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13, !14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_ptr1", scope: !2, file: !3, line: 3, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !7, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "malloc.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "71cac070d2634af1d2284288c27b3127")
!4 = !{!5}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!0, !8}
!8 = !DIGlobalVariableExpression(var: !9, expr: !DIExpression())
!9 = distinct !DIGlobalVariable(name: "g_ptr2", scope: !2, file: !3, line: 4, type: !5, isLocal: false, isDefinition: true)
!10 = !{i32 7, !"Dwarf Version", i32 5}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 8, !"PIC Level", i32 2}
!14 = !{i32 7, !"PIE Level", i32 2}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{i32 7, !"frame-pointer", i32 1}
!17 = !{!"clang"}
!18 = distinct !DISubprogram(name: "init_memory", linkageName: "_Z11init_memoryv", scope: !3, file: !3, line: 6, type: !19, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !21)
!19 = !DISubroutineType(types: !20)
!20 = !{null}
!21 = !{}
!22 = !DILocation(line: 7, column: 21, scope: !18)
!23 = !DILocation(line: 7, column: 12, scope: !18)
!24 = !DILocation(line: 8, column: 21, scope: !18)
!25 = !DILocation(line: 8, column: 12, scope: !18)
!26 = !DILocation(line: 9, column: 6, scope: !18)
!27 = !DILocation(line: 9, column: 13, scope: !18)
!28 = !DILocation(line: 10, column: 6, scope: !18)
!29 = !DILocation(line: 10, column: 13, scope: !18)
!30 = !DILocation(line: 11, column: 1, scope: !18)
!31 = distinct !DISubprogram(name: "free_memory", linkageName: "_Z11free_memoryv", scope: !3, file: !3, line: 13, type: !19, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !21)
!32 = !DILocation(line: 14, column: 10, scope: !31)
!33 = !DILocation(line: 14, column: 5, scope: !31)
!34 = !DILocation(line: 15, column: 10, scope: !31)
!35 = !DILocation(line: 15, column: 5, scope: !31)
!36 = !DILocation(line: 16, column: 12, scope: !31)
!37 = !DILocation(line: 17, column: 12, scope: !31)
!38 = !DILocation(line: 18, column: 1, scope: !31)
!39 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 20, type: !40, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !21)
!40 = !DISubroutineType(types: !41)
!41 = !{!6}
!42 = !DILocation(line: 21, column: 5, scope: !39)
!43 = !DILocation(line: 22, column: 5, scope: !39)

; CHECK: !{{[0-9]+}} = !{!"g_ptr1", !"int*"}
; CHECK: !{{[0-9]+}} = !{!"g_ptr2", !"int*"}