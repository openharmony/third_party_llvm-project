; RUN: opt -S -passes=reference-tracking < %s 2>&1 | FileCheck %s

; ModuleID = 'global_var.cpp'
source_filename = "global_var.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@g_ptr = dso_local global ptr null, align 8, !dbg !0

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !17 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
    #dbg_declare(ptr %2, !21, !DIExpression(), !23)
  %3 = call ptr @malloc(i64 noundef 8) #3, !dbg !24
  store ptr %3, ptr %2, align 8, !dbg !23
  %4 = load ptr, ptr %2, align 8, !dbg !25
  store ptr %4, ptr @g_ptr, align 8, !dbg !26
  %5 = load ptr, ptr @g_ptr, align 8, !dbg !27
  store i32 4, ptr %5, align 4, !dbg !28
  %6 = load ptr, ptr @g_ptr, align 8, !dbg !29
  call void @free(ptr noundef %6), !dbg !30
  store ptr null, ptr @g_ptr, align 8, !dbg !31
  ret i32 0, !dbg !32
}

; CHECK: %3 = call ptr @malloc(i64 noundef 8) #3, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %3, ptr %2, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr %4, ptr @g_ptr, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}
; CHECK: store ptr null, ptr @g_ptr, align 8, !dbg !{{[0-9]+}}, !memtracer !{{[0-9]+}}


; Function Attrs: allocsize(0)
declare ptr @malloc(i64 noundef) #1

declare void @free(ptr noundef) #2

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { allocsize(0) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14, !15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_ptr", scope: !2, file: !8, line: 3, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !7, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/root/mem_map/llvm_test/global_var.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "af5b81345257e146da1913036a9877cb")
!4 = !{!5}
!5 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!0}
!8 = !DIFile(filename: "global_var.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "af5b81345257e146da1913036a9877cb")
!9 = !{i32 7, !"Dwarf Version", i32 5}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 8, !"PIC Level", i32 2}
!13 = !{i32 7, !"PIE Level", i32 2}
!14 = !{i32 7, !"uwtable", i32 2}
!15 = !{i32 7, !"frame-pointer", i32 1}
!16 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!17 = distinct !DISubprogram(name: "main", scope: !8, file: !8, line: 4, type: !18, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{!6}
!20 = !{}
!21 = !DILocalVariable(name: "p4", scope: !17, file: !8, line: 5, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!23 = !DILocation(line: 5, column: 9, scope: !17)
!24 = !DILocation(line: 5, column: 14, scope: !17)
!25 = !DILocation(line: 6, column: 18, scope: !17)
!26 = !DILocation(line: 6, column: 9, scope: !17)
!27 = !DILocation(line: 7, column: 4, scope: !17)
!28 = !DILocation(line: 7, column: 10, scope: !17)
!29 = !DILocation(line: 8, column: 8, scope: !17)
!30 = !DILocation(line: 8, column: 3, scope: !17)
!31 = !DILocation(line: 9, column: 9, scope: !17)
!32 = !DILocation(line: 10, column: 3, scope: !17)

; CHECK: !{{[0-9]+}} = !{!"p4", !"void*"}
; CHECK: !{{[0-9]+}} = !{!"g_ptr", !"int*"}
