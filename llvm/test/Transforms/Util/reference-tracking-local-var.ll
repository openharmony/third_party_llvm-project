; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; Check that AddReferenceTrackingInfo attaches !memtracer to a pointer store when
; the destination alloca is described by llvm.dbg.declare.

; ModuleID = 'local_var.cpp'
source_filename = "local_var.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !13 {
  %1 = alloca i32, align 4
  %2 = alloca i64, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !18, !DIExpression(), !20)
  store i64 4, ptr %2, align 8, !dbg !20
    #dbg_declare(ptr %3, !21, !DIExpression(), !23)
  %5 = call ptr @malloc(i64 noundef 4) #3, !dbg !24
  store ptr %5, ptr %3, align 8, !dbg !23
    #dbg_declare(ptr %4, !25, !DIExpression(), !26)
  %6 = load ptr, ptr %3, align 8, !dbg !27
  store ptr %6, ptr %4, align 8, !dbg !26
  %7 = load ptr, ptr %4, align 8, !dbg !28
  store i32 1, ptr %7, align 4, !dbg !29
  %8 = load ptr, ptr %4, align 8, !dbg !30
  call void @free(ptr noundef %8), !dbg !31
  ret i32 0, !dbg !32
}

; CHECK: %5 = call ptr @malloc(i64 noundef 4) #3, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %5, ptr %3, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %6, ptr %4, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

declare void @free(ptr noundef) #2

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { allocsize(0) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!5, !6, !7, !8, !9, !10, !11}
!llvm.ident = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/root/mem_map/llvm_test/local_var.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "36149757dfd1487fca5208170964cec2")
!2 = !{!3}
!3 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4, size: 64)
!4 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!5 = !{i32 7, !"Dwarf Version", i32 5}
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{i32 8, !"PIC Level", i32 2}
!9 = !{i32 7, !"PIE Level", i32 2}
!10 = !{i32 7, !"uwtable", i32 2}
!11 = !{i32 7, !"frame-pointer", i32 1}
!12 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!13 = distinct !DISubprogram(name: "main", scope: !14, file: !14, line: 3, type: !15, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!14 = !DIFile(filename: "local_var.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "36149757dfd1487fca5208170964cec2")
!15 = !DISubroutineType(types: !16)
!16 = !{!4}
!17 = !{}
!18 = !DILocalVariable(name: "p1", scope: !13, file: !14, line: 4, type: !19)
!19 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!20 = !DILocation(line: 4, column: 8, scope: !13)
!21 = !DILocalVariable(name: "p", scope: !13, file: !14, line: 5, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!23 = !DILocation(line: 5, column: 9, scope: !13)
!24 = !DILocation(line: 5, column: 13, scope: !13)
!25 = !DILocalVariable(name: "local_ptr", scope: !13, file: !14, line: 6, type: !3)
!26 = !DILocation(line: 6, column: 8, scope: !13)
!27 = !DILocation(line: 6, column: 27, scope: !13)
!28 = !DILocation(line: 7, column: 4, scope: !13)
!29 = !DILocation(line: 7, column: 14, scope: !13)
!30 = !DILocation(line: 8, column: 8, scope: !13)
!31 = !DILocation(line: 8, column: 3, scope: !13)
!32 = !DILocation(line: 9, column: 3, scope: !13)

; CHECK: !{{[0-9]+}} = !{!"p", !"void*"}
; CHECK: !{{[0-9]+}} = !{!"local_ptr", !"int*"}
