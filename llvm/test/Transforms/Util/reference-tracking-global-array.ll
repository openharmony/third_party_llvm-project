; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'global_array.cpp'
source_filename = "global_array.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@arr = dso_local global [4 x ptr] zeroinitializer, align 8, !dbg !0

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !20 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !24, !DIExpression(), !26)
  %3 = call ptr @malloc(i64 noundef 8) #3, !dbg !27
  store ptr %3, ptr %2, align 8, !dbg !26
  %4 = load ptr, ptr %2, align 8, !dbg !28
  store ptr %4, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !29
  %5 = load ptr, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !30
  store i32 4, ptr %5, align 4, !dbg !31
  %6 = load ptr, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !32
  call void @free(ptr noundef %6), !dbg !33
  store ptr null, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !34
  ret i32 0, !dbg !35
}

; CHECK: %3 = call ptr @malloc(i64 noundef 8) #3, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %3, ptr %2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %4, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr null, ptr getelementptr inbounds ([4 x ptr], ptr @arr, i64 0, i64 1), align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

declare void @free(ptr noundef) #2

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { allocsize(0) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!12, !13, !14, !15, !16, !17, !18}
!llvm.ident = !{!19}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "arr", scope: !2, file: !8, line: 3, type: !9, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !7, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/root/mem_map/llvm_test/global_array.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "9b7b244df720a84117c8fbe17d0dc240")
!4 = !{!5}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!0}
!8 = !DIFile(filename: "global_array.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "9b7b244df720a84117c8fbe17d0dc240")
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !5, size: 256, elements: !10)
!10 = !{!11}
!11 = !DISubrange(count: 4)
!12 = !{i32 7, !"Dwarf Version", i32 5}
!13 = !{i32 2, !"Debug Info Version", i32 3}
!14 = !{i32 1, !"wchar_size", i32 4}
!15 = !{i32 8, !"PIC Level", i32 2}
!16 = !{i32 7, !"PIE Level", i32 2}
!17 = !{i32 7, !"uwtable", i32 2}
!18 = !{i32 7, !"frame-pointer", i32 1}
!19 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!20 = distinct !DISubprogram(name: "main", scope: !8, file: !8, line: 4, type: !21, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !23)
!21 = !DISubroutineType(types: !22)
!22 = !{!6}
!23 = !{}
!24 = !DILocalVariable(name: "p", scope: !20, file: !8, line: 5, type: !25)
!25 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!26 = !DILocation(line: 5, column: 9, scope: !20)
!27 = !DILocation(line: 5, column: 13, scope: !20)
!28 = !DILocation(line: 6, column: 19, scope: !20)
!29 = !DILocation(line: 6, column: 10, scope: !20)
!30 = !DILocation(line: 7, column: 4, scope: !20)
!31 = !DILocation(line: 7, column: 11, scope: !20)
!32 = !DILocation(line: 8, column: 8, scope: !20)
!33 = !DILocation(line: 8, column: 3, scope: !20)
!34 = !DILocation(line: 9, column: 10, scope: !20)
!35 = !DILocation(line: 10, column: 3, scope: !20)

; CHECK: !{{[0-9]+}} = !{!"p", !"void*"}
; CHECK: !{{[0-9]+}} = !{!"arr[]", !"int*"}
