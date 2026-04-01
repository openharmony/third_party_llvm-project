; REQUIRES: aarch64-registered-target

; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj %s -O2 -o %t.o
; RUN: ld.lld -shared %t.o -o %t.o
; RUN: llvm-dwarfdump --mem_tracer %t.o | FileCheck %s

; CHECK: .mem_tracer contents:
; CHECK: var="p1" type="void*"
; CHECK: var="p2" type="void*"
; CHECK: var="pair.ptr1" type="void*"
; CHECK: var="pair.ptr2" type="void*"
; CHECK: var="quad.p1" type="void*"
; CHECK: var="quad.p4" type="void*"
; CHECK: var="ptrs[]" type="void*"
; CHECK: var="g_args" type="void*[]"
; CHECK: var="buffers" type="void**"
; CHECK: var="result" type="void*"
; CHECK: var="obj1" type="MyClass*"
; CHECK: var="obj2" type="MyClass*"

; ModuleID = 'memtracer_str_to_stp.cpp'
source_filename = "memtracer_str_to_stp.cpp"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-ohos"

@_ZZ11process_ptrPvS_E6g_args = internal unnamed_addr global [2 x ptr] zeroinitializer, align 8, !dbg !0
@_ZL6g_sink = internal unnamed_addr global [64 x ptr] zeroinitializer, align 8, !dbg !16
@_ZL10g_sink_idx = internal unnamed_addr global i32 0, align 4, !dbg !21

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z21test_continuous_storev() local_unnamed_addr #0 !dbg !306 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !312, !memtracer !313
  call void @llvm.dbg.value(metadata ptr %1, metadata !308, metadata !DIExpression()), !dbg !314
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !315, !memtracer !316
  call void @llvm.dbg.value(metadata ptr %2, metadata !309, metadata !DIExpression()), !dbg !314
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !317, !memtracer !318
  call void @llvm.dbg.value(metadata ptr %3, metadata !310, metadata !DIExpression()), !dbg !314
  %4 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !319, !memtracer !320
  call void @llvm.dbg.value(metadata ptr %4, metadata !311, metadata !DIExpression()), !dbg !314
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !324
  %5 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !326, !tbaa !327
  %6 = add nsw i32 %5, 1, !dbg !326
  %7 = sext i32 %5 to i64, !dbg !331
  %8 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %7, !dbg !331
  store ptr %1, ptr %8, align 8, !dbg !332, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !336
  %9 = add nsw i32 %5, 2, !dbg !338
  %10 = sext i32 %6 to i64, !dbg !339
  %11 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %10, !dbg !339
  store ptr %2, ptr %11, align 8, !dbg !340, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %3, metadata !321, metadata !DIExpression()), !dbg !341
  %12 = add nsw i32 %5, 3, !dbg !343
  %13 = sext i32 %9 to i64, !dbg !344
  %14 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %13, !dbg !344
  store ptr %3, ptr %14, align 8, !dbg !345, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %4, metadata !321, metadata !DIExpression()), !dbg !346
  %15 = add nsw i32 %5, 4, !dbg !348
  store i32 %15, ptr @_ZL10g_sink_idx, align 4, !dbg !348, !tbaa !327
  %16 = sext i32 %12 to i64, !dbg !349
  %17 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %16, !dbg !349
  store ptr %4, ptr %17, align 8, !dbg !350, !tbaa !333, !memtracer !335
  ret void, !dbg !351
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z17test_struct_storev() local_unnamed_addr #0 !dbg !352 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !366, !memtracer !367
  call void @llvm.dbg.value(metadata ptr %1, metadata !354, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !368
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !369, !memtracer !370
  call void @llvm.dbg.value(metadata ptr %2, metadata !354, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !368
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !371
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !373, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !373
  %5 = sext i32 %3 to i64, !dbg !374
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !374
  store ptr %1, ptr %6, align 8, !dbg !375, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !376
  %7 = add nsw i32 %3, 2, !dbg !378
  %8 = sext i32 %4 to i64, !dbg !379
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !379
  store ptr %2, ptr %9, align 8, !dbg !380, !tbaa !333, !memtracer !335
  %10 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !381, !memtracer !382
  call void @llvm.dbg.value(metadata ptr %10, metadata !359, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 64)), !dbg !368
  %11 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !383, !memtracer !384
  call void @llvm.dbg.value(metadata ptr %11, metadata !359, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 64)), !dbg !368
  %12 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !385, !memtracer !386
  call void @llvm.dbg.value(metadata ptr %12, metadata !359, metadata !DIExpression(DW_OP_LLVM_fragment, 128, 64)), !dbg !368
  %13 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !387, !memtracer !388
  call void @llvm.dbg.value(metadata ptr %13, metadata !359, metadata !DIExpression(DW_OP_LLVM_fragment, 192, 64)), !dbg !368
  call void @llvm.dbg.value(metadata ptr %10, metadata !321, metadata !DIExpression()), !dbg !389
  %14 = add nsw i32 %3, 3, !dbg !391
  %15 = sext i32 %7 to i64, !dbg !392
  %16 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %15, !dbg !392
  store ptr %10, ptr %16, align 8, !dbg !393, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %11, metadata !321, metadata !DIExpression()), !dbg !394
  %17 = add nsw i32 %3, 4, !dbg !396
  %18 = sext i32 %14 to i64, !dbg !397
  %19 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !397
  store ptr %11, ptr %19, align 8, !dbg !398, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %12, metadata !321, metadata !DIExpression()), !dbg !399
  %20 = add nsw i32 %3, 5, !dbg !401
  %21 = sext i32 %17 to i64, !dbg !402
  %22 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %21, !dbg !402
  store ptr %12, ptr %22, align 8, !dbg !403, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %13, metadata !321, metadata !DIExpression()), !dbg !404
  %23 = add nsw i32 %3, 6, !dbg !406
  store i32 %23, ptr @_ZL10g_sink_idx, align 4, !dbg !406, !tbaa !327
  %24 = sext i32 %20 to i64, !dbg !407
  %25 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %24, !dbg !407
  store ptr %13, ptr %25, align 8, !dbg !408, !tbaa !333, !memtracer !335
  ret void, !dbg !409
}

; Function Attrs: mustprogress nofree noinline nounwind
define void @_Z16test_array_storev() local_unnamed_addr #3 !dbg !410 {
  call void @llvm.dbg.value(metadata i32 0, metadata !416, metadata !DIExpression()), !dbg !418
  %1 = load i32, ptr @_ZL10g_sink_idx, align 4, !tbaa !327
  call void @llvm.dbg.value(metadata i32 0, metadata !416, metadata !DIExpression()), !dbg !418
  %2 = sext i32 %1 to i64, !dbg !419
  call void @llvm.dbg.value(metadata i32 0, metadata !416, metadata !DIExpression()), !dbg !418
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %3, metadata !321, metadata !DIExpression()), !dbg !425
  %4 = add nsw i64 %2, 1, !dbg !427
  %5 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %2, !dbg !428
  store ptr %3, ptr %5, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 1, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 1, metadata !416, metadata !DIExpression()), !dbg !418
  %6 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %6, metadata !321, metadata !DIExpression()), !dbg !425
  %7 = add nsw i64 %2, 2, !dbg !427
  %8 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %4, !dbg !428
  store ptr %6, ptr %8, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 2, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 2, metadata !416, metadata !DIExpression()), !dbg !418
  %9 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %9, metadata !321, metadata !DIExpression()), !dbg !425
  %10 = add nsw i64 %2, 3, !dbg !427
  %11 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %7, !dbg !428
  store ptr %9, ptr %11, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 3, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 3, metadata !416, metadata !DIExpression()), !dbg !418
  %12 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %12, metadata !321, metadata !DIExpression()), !dbg !425
  %13 = add nsw i64 %2, 4, !dbg !427
  %14 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %10, !dbg !428
  store ptr %12, ptr %14, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 4, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 4, metadata !416, metadata !DIExpression()), !dbg !418
  %15 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %15, metadata !321, metadata !DIExpression()), !dbg !425
  %16 = add nsw i64 %2, 5, !dbg !427
  %17 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %13, !dbg !428
  store ptr %15, ptr %17, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 5, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 5, metadata !416, metadata !DIExpression()), !dbg !418
  %18 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %18, metadata !321, metadata !DIExpression()), !dbg !425
  %19 = add nsw i64 %2, 6, !dbg !427
  %20 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %16, !dbg !428
  store ptr %18, ptr %20, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 6, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 6, metadata !416, metadata !DIExpression()), !dbg !418
  %21 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %21, metadata !321, metadata !DIExpression()), !dbg !425
  %22 = add nsw i64 %2, 7, !dbg !427
  %23 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %19, !dbg !428
  store ptr %21, ptr %23, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 7, metadata !416, metadata !DIExpression()), !dbg !418
  call void @llvm.dbg.value(metadata i32 7, metadata !416, metadata !DIExpression()), !dbg !418
  %24 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !420, !memtracer !423
  call void @llvm.dbg.value(metadata ptr undef, metadata !412, metadata !DIExpression()), !dbg !424
  call void @llvm.dbg.value(metadata ptr %24, metadata !321, metadata !DIExpression()), !dbg !425
  %25 = add i32 %1, 8, !dbg !427
  %26 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %22, !dbg !428
  store ptr %24, ptr %26, align 8, !dbg !429, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata i32 8, metadata !416, metadata !DIExpression()), !dbg !418
  store i32 %25, ptr @_ZL10g_sink_idx, align 4, !dbg !427, !tbaa !327
  ret void, !dbg !430
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define void @_Z11process_ptrPvS_(ptr noundef %0, ptr noundef %1) local_unnamed_addr #4 !dbg !2 {
  call void @llvm.dbg.value(metadata ptr %0, metadata !294, metadata !DIExpression()), !dbg !431
  call void @llvm.dbg.value(metadata ptr %1, metadata !295, metadata !DIExpression()), !dbg !431
  store ptr %0, ptr @_ZZ11process_ptrPvS_E6g_args, align 8, !dbg !432, !tbaa !333, !memtracer !433
  store ptr %1, ptr getelementptr inbounds ([2 x ptr], ptr @_ZZ11process_ptrPvS_E6g_args, i64 0, i64 1), align 8, !dbg !434, !tbaa !333, !memtracer !435
  ret void, !dbg !436
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z16test_param_storev() local_unnamed_addr #0 !dbg !437 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !441, !memtracer !313
  call void @llvm.dbg.value(metadata ptr %1, metadata !439, metadata !DIExpression()), !dbg !442
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !443, !memtracer !316
  call void @llvm.dbg.value(metadata ptr %2, metadata !440, metadata !DIExpression()), !dbg !442
  tail call void @_Z11process_ptrPvS_(ptr noundef %1, ptr noundef %2) #14, !dbg !444
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !445
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !447, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !447
  %5 = sext i32 %3 to i64, !dbg !448
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !448
  store ptr %1, ptr %6, align 8, !dbg !449, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !450
  %7 = add nsw i32 %3, 2, !dbg !452
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !452, !tbaa !327
  %8 = sext i32 %4 to i64, !dbg !453
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !453
  store ptr %2, ptr %9, align 8, !dbg !454, !tbaa !333, !memtracer !335
  ret void, !dbg !455
}

; Function Attrs: mustprogress nofree noinline nounwind
define void @_Z15test_loop_storei(i32 noundef %0) local_unnamed_addr #3 !dbg !456 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !458, metadata !DIExpression()), !dbg !462
  %2 = sext i32 %0 to i64, !dbg !463
  %3 = shl nsw i64 %2, 3, !dbg !464
  %4 = tail call ptr @malloc(i64 noundef %3) #13, !dbg !465, !memtracer !466
  call void @llvm.dbg.value(metadata ptr %4, metadata !459, metadata !DIExpression()), !dbg !462
  call void @llvm.dbg.value(metadata i32 0, metadata !460, metadata !DIExpression()), !dbg !467
  %5 = load i32, ptr @_ZL10g_sink_idx, align 4, !tbaa !327
  call void @llvm.dbg.value(metadata i32 0, metadata !460, metadata !DIExpression()), !dbg !467
  %6 = icmp sgt i32 %0, 0, !dbg !468
  br i1 %6, label %7, label %12, !dbg !470

7:                                                ; preds = %1
  %8 = sext i32 %5 to i64, !dbg !470
  %9 = zext i32 %0 to i64, !dbg !468
  br label %17, !dbg !470

10:                                               ; preds = %17
  %11 = trunc i64 %22 to i32, !dbg !471
  br label %12, !dbg !470

12:                                               ; preds = %10, %1
  %13 = phi i32 [ %11, %10 ], [ %5, %1 ], !dbg !474
  call void @llvm.dbg.value(metadata ptr %4, metadata !321, metadata !DIExpression()), !dbg !476
  %14 = add nsw i32 %13, 1, !dbg !474
  store i32 %14, ptr @_ZL10g_sink_idx, align 4, !dbg !474, !tbaa !327
  %15 = sext i32 %13 to i64, !dbg !477
  %16 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %15, !dbg !477
  store ptr %4, ptr %16, align 8, !dbg !478, !tbaa !333, !memtracer !335
  ret void, !dbg !479

17:                                               ; preds = %7, %17
  %18 = phi i64 [ %8, %7 ], [ %22, %17 ]
  %19 = phi i64 [ 0, %7 ], [ %24, %17 ]
  call void @llvm.dbg.value(metadata i64 %19, metadata !460, metadata !DIExpression()), !dbg !467
  %20 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !480, !memtracer !481
  %21 = getelementptr inbounds ptr, ptr %4, i64 %19, !dbg !482
  store ptr %20, ptr %21, align 8, !dbg !483, !tbaa !333, !memtracer !481
  call void @llvm.dbg.value(metadata ptr %20, metadata !321, metadata !DIExpression()), !dbg !484
  %22 = add nsw i64 %18, 1, !dbg !471
  %23 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !485
  store ptr %20, ptr %23, align 8, !dbg !486, !tbaa !333, !memtracer !335
  %24 = add nuw nsw i64 %19, 1, !dbg !487
  call void @llvm.dbg.value(metadata i64 %24, metadata !460, metadata !DIExpression()), !dbg !467
  %25 = icmp eq i64 %24, %9, !dbg !468
  br i1 %25, label %10, label %17, !dbg !470, !llvm.loop !488
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z22test_conditional_storei(i32 noundef %0) local_unnamed_addr #0 !dbg !491 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !493, metadata !DIExpression()), !dbg !497
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !498, !memtracer !313
  call void @llvm.dbg.value(metadata ptr %2, metadata !494, metadata !DIExpression()), !dbg !497
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !499, !memtracer !316
  call void @llvm.dbg.value(metadata ptr %3, metadata !495, metadata !DIExpression()), !dbg !497
  %4 = icmp sgt i32 %0, 0, !dbg !500
  %5 = select i1 %4, ptr %2, ptr %3, !dbg !501
  call void @llvm.dbg.value(metadata ptr %5, metadata !496, metadata !DIExpression()), !dbg !497
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !502
  %6 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !504, !tbaa !327
  %7 = add nsw i32 %6, 1, !dbg !504
  %8 = sext i32 %6 to i64, !dbg !505
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !505
  store ptr %2, ptr %9, align 8, !dbg !506, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %3, metadata !321, metadata !DIExpression()), !dbg !507
  %10 = add nsw i32 %6, 2, !dbg !509
  %11 = sext i32 %7 to i64, !dbg !510
  %12 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %11, !dbg !510
  store ptr %3, ptr %12, align 8, !dbg !511, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %5, metadata !321, metadata !DIExpression()), !dbg !512
  %13 = add nsw i32 %6, 3, !dbg !514
  store i32 %13, ptr @_ZL10g_sink_idx, align 4, !dbg !514, !tbaa !327
  %14 = sext i32 %10 to i64, !dbg !515
  %15 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %14, !dbg !515
  store ptr %5, ptr %15, align 8, !dbg !516, !tbaa !333, !memtracer !335
  ret void, !dbg !517
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define noundef ptr @_Z21test_switch_ptr_storei(i32 noundef %0) local_unnamed_addr #0 !dbg !518 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !522, metadata !DIExpression()), !dbg !524
  call void @llvm.dbg.value(metadata ptr null, metadata !523, metadata !DIExpression()), !dbg !524
  switch i32 %0, label %8 [
    i32 0, label %2
    i32 1, label %4
    i32 2, label %6
  ], !dbg !525

2:                                                ; preds = %1
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !526, !memtracer !528
  call void @llvm.dbg.value(metadata ptr %3, metadata !523, metadata !DIExpression()), !dbg !524
  br label %10, !dbg !529

4:                                                ; preds = %1
  %5 = tail call dereferenceable_or_null(128) ptr @malloc(i64 noundef 128) #13, !dbg !530, !memtracer !528
  call void @llvm.dbg.value(metadata ptr %5, metadata !523, metadata !DIExpression()), !dbg !524
  br label %10, !dbg !531

6:                                                ; preds = %1
  %7 = tail call dereferenceable_or_null(256) ptr @malloc(i64 noundef 256) #13, !dbg !532, !memtracer !528
  call void @llvm.dbg.value(metadata ptr %7, metadata !523, metadata !DIExpression()), !dbg !524
  br label %10, !dbg !533

8:                                                ; preds = %1
  %9 = tail call dereferenceable_or_null(512) ptr @malloc(i64 noundef 512) #13, !dbg !534, !memtracer !528
  call void @llvm.dbg.value(metadata ptr %9, metadata !523, metadata !DIExpression()), !dbg !524
  br label %10, !dbg !535

10:                                               ; preds = %8, %6, %4, %2
  %11 = phi ptr [ %9, %8 ], [ %7, %6 ], [ %5, %4 ], [ %3, %2 ], !dbg !536
  call void @llvm.dbg.value(metadata ptr %11, metadata !523, metadata !DIExpression()), !dbg !524
  call void @llvm.dbg.value(metadata ptr %11, metadata !321, metadata !DIExpression()), !dbg !537
  %12 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !539, !tbaa !327
  %13 = add nsw i32 %12, 1, !dbg !539
  store i32 %13, ptr @_ZL10g_sink_idx, align 4, !dbg !539, !tbaa !327
  %14 = sext i32 %12 to i64, !dbg !540
  %15 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %14, !dbg !540
  store ptr %11, ptr %15, align 8, !dbg !541, !tbaa !333, !memtracer !335
  ret ptr %11, !dbg !542
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z19test_template_storev() local_unnamed_addr #0 !dbg !543 {
  call void @llvm.dbg.value(metadata i64 64, metadata !547, metadata !DIExpression()), !dbg !554
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !556
  call void @llvm.dbg.value(metadata ptr %1, metadata !545, metadata !DIExpression()), !dbg !557
  call void @llvm.dbg.value(metadata i64 32, metadata !558, metadata !DIExpression()), !dbg !565
  %2 = tail call dereferenceable_or_null(128) ptr @malloc(i64 noundef 128) #13, !dbg !567
  call void @llvm.dbg.value(metadata ptr %2, metadata !546, metadata !DIExpression()), !dbg !557
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !568
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !570, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !570
  %5 = sext i32 %3 to i64, !dbg !571
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !571
  store ptr %1, ptr %6, align 8, !dbg !572, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !573
  %7 = add nsw i32 %3, 2, !dbg !575
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !575, !tbaa !327
  %8 = sext i32 %4 to i64, !dbg !576
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !576
  store ptr %2, ptr %9, align 8, !dbg !577, !tbaa !333, !memtracer !335
  ret void, !dbg !578
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn
define void @_Z17test_lambda_storev() local_unnamed_addr #0 !dbg !579 {
  call void @llvm.dbg.declare(metadata ptr undef, metadata !581, metadata !DIExpression()), !dbg !591
  call void @llvm.dbg.value(metadata ptr poison, metadata !592, metadata !DIExpression()), !dbg !599
  call void @llvm.dbg.value(metadata i64 64, metadata !596, metadata !DIExpression()), !dbg !599
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !601
  call void @llvm.dbg.value(metadata ptr %1, metadata !589, metadata !DIExpression()), !dbg !602
  call void @llvm.dbg.value(metadata ptr poison, metadata !592, metadata !DIExpression()), !dbg !603
  call void @llvm.dbg.value(metadata i64 128, metadata !596, metadata !DIExpression()), !dbg !603
  %2 = tail call dereferenceable_or_null(128) ptr @malloc(i64 noundef 128) #13, !dbg !605
  call void @llvm.dbg.value(metadata ptr %2, metadata !590, metadata !DIExpression()), !dbg !602
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !606
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !608, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !608
  %5 = sext i32 %3 to i64, !dbg !609
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !609
  store ptr %1, ptr %6, align 8, !dbg !610, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !611
  %7 = add nsw i32 %3, 2, !dbg !613
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !613, !tbaa !327
  %8 = sext i32 %4 to i64, !dbg !614
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !614
  store ptr %2, ptr %9, align 8, !dbg !615, !tbaa !333, !memtracer !335
  ret void, !dbg !616
}

; Function Attrs: mustprogress nofree noinline nounwind
define noalias noundef ptr @_Z15recursive_alloci(i32 noundef %0) local_unnamed_addr #3 !dbg !617 {
  call void @llvm.dbg.value(metadata i32 undef, metadata !619, metadata !DIExpression()), !dbg !620
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !621, !memtracer !623
  ret ptr %2, !dbg !624
}

; Function Attrs: mustprogress nofree noinline nounwind
define void @_Z20test_recursive_storev() local_unnamed_addr #3 !dbg !625 {
  %1 = tail call noundef ptr @_Z15recursive_alloci(i32 noundef 3) #14, !dbg !629
  call void @llvm.dbg.value(metadata ptr %1, metadata !627, metadata !DIExpression()), !dbg !630
  %2 = tail call noundef ptr @_Z15recursive_alloci(i32 noundef 5) #14, !dbg !631
  call void @llvm.dbg.value(metadata ptr %2, metadata !628, metadata !DIExpression()), !dbg !630
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !632
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !634, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !634
  %5 = sext i32 %3 to i64, !dbg !635
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !635
  store ptr %1, ptr %6, align 8, !dbg !636, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !637
  %7 = add nsw i32 %3, 2, !dbg !639
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !639, !tbaa !327
  %8 = sext i32 %4 to i64, !dbg !640
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !640
  store ptr %2, ptr %9, align 8, !dbg !641, !tbaa !333, !memtracer !335
  ret void, !dbg !642
}

; Function Attrs: mustprogress noinline nounwind willreturn
define void @_Z25test_realloc_calloc_storev() local_unnamed_addr #5 !dbg !643 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #13, !dbg !648, !memtracer !313
  call void @llvm.dbg.value(metadata ptr %1, metadata !645, metadata !DIExpression()), !dbg !649
  %2 = tail call dereferenceable_or_null(128) ptr @realloc(ptr noundef %1, i64 noundef 128) #15, !dbg !650, !memtracer !316
  call void @llvm.dbg.value(metadata ptr %2, metadata !646, metadata !DIExpression()), !dbg !649
  %3 = tail call dereferenceable_or_null(40) ptr @calloc(i64 noundef 10, i64 noundef 4) #16, !dbg !651, !memtracer !318
  call void @llvm.dbg.value(metadata ptr %3, metadata !647, metadata !DIExpression()), !dbg !649
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !652
  %4 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !654, !tbaa !327
  %5 = add nsw i32 %4, 1, !dbg !654
  %6 = sext i32 %4 to i64, !dbg !655
  %7 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %6, !dbg !655
  store ptr %2, ptr %7, align 8, !dbg !656, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %3, metadata !321, metadata !DIExpression()), !dbg !657
  %8 = add nsw i32 %4, 2, !dbg !659
  store i32 %8, ptr @_ZL10g_sink_idx, align 4, !dbg !659, !tbaa !327
  %9 = sext i32 %5 to i64, !dbg !660
  %10 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %9, !dbg !660
  store ptr %3, ptr %10, align 8, !dbg !661, !tbaa !333, !memtracer !335
  ret void, !dbg !662
}

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("realloc") allocsize(1)
declare noalias noundef ptr @realloc(ptr allocptr nocapture noundef, i64 noundef) local_unnamed_addr #6

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1)
declare noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress noinline
define void @_Z14test_new_storev() local_unnamed_addr #8 !dbg !663 {
  %1 = tail call noalias noundef nonnull dereferenceable(64) ptr @_Znwm(i64 noundef 64) #17, !dbg !674, !memtracer !675, !heapallocsite !667
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(64) %1, i8 0, i64 64, i1 false), !dbg !676
  call void @llvm.dbg.value(metadata ptr %1, metadata !665, metadata !DIExpression()), !dbg !677
  %2 = tail call noalias noundef nonnull dereferenceable(64) ptr @_Znwm(i64 noundef 64) #17, !dbg !678, !memtracer !679, !heapallocsite !667
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(64) %2, i8 0, i64 64, i1 false), !dbg !680
  call void @llvm.dbg.value(metadata ptr %2, metadata !673, metadata !DIExpression()), !dbg !677
  call void @llvm.dbg.value(metadata ptr %1, metadata !321, metadata !DIExpression()), !dbg !681
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !683, !tbaa !327
  %4 = add nsw i32 %3, 1, !dbg !683
  %5 = sext i32 %3 to i64, !dbg !684
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !684
  store ptr %1, ptr %6, align 8, !dbg !685, !tbaa !333, !memtracer !335
  call void @llvm.dbg.value(metadata ptr %2, metadata !321, metadata !DIExpression()), !dbg !686
  %7 = add nsw i32 %3, 2, !dbg !688
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !688, !tbaa !327
  %8 = sext i32 %4 to i64, !dbg !689
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !689
  store ptr %2, ptr %9, align 8, !dbg !690, !tbaa !333, !memtracer !335
  ret void, !dbg !691
}

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #9

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #10

; Function Attrs: mustprogress norecurse
define noundef i32 @main() local_unnamed_addr #11 !dbg !692 {
  tail call void @_Z21test_continuous_storev() #14, !dbg !694
  tail call void @_Z17test_struct_storev() #14, !dbg !695
  tail call void @_Z16test_array_storev() #14, !dbg !696
  tail call void @_Z16test_param_storev() #14, !dbg !697
  tail call void @_Z15test_loop_storei(i32 noundef 8) #14, !dbg !698
  tail call void @_Z22test_conditional_storei(i32 noundef 1) #14, !dbg !699
  tail call void @_Z22test_conditional_storei(i32 noundef -1) #14, !dbg !700
  %1 = tail call noundef ptr @_Z21test_switch_ptr_storei(i32 noundef 0) #14, !dbg !701
  %2 = tail call noundef ptr @_Z21test_switch_ptr_storei(i32 noundef 2) #14, !dbg !702
  tail call void @_Z19test_template_storev() #14, !dbg !703
  tail call void @_Z17test_lambda_storev() #14, !dbg !704
  tail call void @_Z20test_recursive_storev() #14, !dbg !705
  tail call void @_Z25test_realloc_calloc_storev() #14, !dbg !706
  tail call void @_Z14test_new_storev() #14, !dbg !707
  ret i32 0, !dbg !708
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #12

attributes #0 = { mustprogress nofree noinline nounwind willreturn "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #3 = { mustprogress nofree noinline nounwind "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #4 = { mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #5 = { mustprogress noinline nounwind willreturn "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #6 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("realloc") allocsize(1) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #7 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #8 = { mustprogress noinline "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #9 = { nobuiltin allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #10 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }
attributes #11 = { mustprogress norecurse "frame-pointer"="non-leaf" "min-legal-vector-width"="0" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+neon,+v8a" }
attributes #12 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #13 = { allocsize(0) "reference-tracking"="true" }
attributes #14 = { "reference-tracking"="true" }
attributes #15 = { allocsize(1) "reference-tracking"="true" }
attributes #16 = { allocsize(0,1) "reference-tracking"="true" }
attributes #17 = { builtin allocsize(0) "reference-tracking"="true" }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!299, !300, !301, !302, !303, !304}
!llvm.ident = !{!305}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_args", scope: !2, file: !3, line: 54, type: !296, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "process_ptr", linkageName: "_Z11process_ptrPvS_", scope: !3, file: !3, line: 53, type: !4, scopeLine: 53, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !293)
!3 = !DIFile(filename: "memtracer_str_to_stp.cpp", directory: "", checksumkind: CSK_MD5, checksum: "4b82ba3ede9c4121086d403546e90f19")
!4 = !DISubroutineType(types: !5)
!5 = !{null, !6, !6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!7 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !8, producer: "clang version 15.0.4", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !9, globals: !15, imports: !23, splitDebugInlining: false, nameTableKind: None)
!8 = !DIFile(filename: "memtracer_str_to_stp.cpp", directory: "", checksumkind: CSK_MD5, checksum: "4b82ba3ede9c4121086d403546e90f19")
!9 = !{!10, !11, !13}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_unsigned_char)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{!0, !16, !21}
!16 = !DIGlobalVariableExpression(var: !17, expr: !DIExpression())
!17 = distinct !DIGlobalVariable(name: "g_sink", linkageName: "_ZL6g_sink", scope: !7, file: !3, line: 13, type: !18, isLocal: true, isDefinition: true)
!18 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 4096, elements: !19)
!19 = !{!20}
!20 = !DISubrange(count: 64)
!21 = !DIGlobalVariableExpression(var: !22, expr: !DIExpression())
!22 = distinct !DIGlobalVariable(name: "g_sink_idx", linkageName: "_ZL10g_sink_idx", scope: !7, file: !3, line: 14, type: !14, isLocal: true, isDefinition: true)
!23 = !{!24, !31, !35, !42, !49, !56, !60, !64, !68, !75, !80, !85, !89, !93, !97, !102, !106, !111, !115, !119, !123, !127, !131, !136, !140, !142, !146, !148, !157, !161, !166, !170, !174, !178, !182, !184, !188, !195, !199, !203, !211, !213, !215, !217, !224, !228, !232, !236, !238, !240, !244, !248, !252, !254, !258, !263, !267, !271, !275, !277, !279, !281, !283, !285, !289}
!24 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !27, file: !30, line: 94)
!25 = !DINamespace(name: "__h", scope: !26, exportSymbols: true)
!26 = !DINamespace(name: "std", scope: null)
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !28, line: 58, baseType: !29)
!28 = !DIFile(filename: "alltypes.h", directory: "", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!29 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!30 = !DIFile(filename: "cstdlib", directory: "")
!31 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !32, file: !30, line: 95)
!32 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !33, line: 65, baseType: !34)
!33 = !DIFile(filename: "stdlib.h", directory: "", checksumkind: CSK_MD5, checksum: "4ae56b2feb06fe30283b2148e55e1d18")
!34 = !DICompositeType(tag: DW_TAG_structure_type, file: !33, line: 65, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!35 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !36, file: !30, line: 96)
!36 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !33, line: 66, baseType: !37)
!37 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !33, line: 66, size: 128, flags: DIFlagTypePassByValue, elements: !38, identifier: "_ZTS6ldiv_t")
!38 = !{!39, !41}
!39 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !37, file: !33, line: 66, baseType: !40, size: 64)
!40 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!41 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !37, file: !33, line: 66, baseType: !40, size: 64, offset: 64)
!42 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !43, file: !30, line: 97)
!43 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !33, line: 67, baseType: !44)
!44 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !33, line: 67, size: 128, flags: DIFlagTypePassByValue, elements: !45, identifier: "_ZTS7lldiv_t")
!45 = !{!46, !48}
!46 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !44, file: !33, line: 67, baseType: !47, size: 64)
!47 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!48 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !44, file: !33, line: 67, baseType: !47, size: 64, offset: 64)
!49 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !50, file: !30, line: 98)
!50 = !DISubprogram(name: "atof", scope: !33, file: !33, line: 26, type: !51, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!51 = !DISubroutineType(types: !52)
!52 = !{!53, !54}
!53 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!54 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !55, size: 64)
!55 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !12)
!56 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !57, file: !30, line: 99)
!57 = !DISubprogram(name: "atoi", scope: !33, file: !33, line: 23, type: !58, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!58 = !DISubroutineType(types: !59)
!59 = !{!14, !54}
!60 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !61, file: !30, line: 100)
!61 = !DISubprogram(name: "atol", scope: !33, file: !33, line: 24, type: !62, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!62 = !DISubroutineType(types: !63)
!63 = !{!40, !54}
!64 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !65, file: !30, line: 101)
!65 = !DISubprogram(name: "atoll", scope: !33, file: !33, line: 25, type: !66, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!66 = !DISubroutineType(types: !67)
!67 = !{!47, !54}
!68 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !69, file: !30, line: 102)
!69 = !DISubprogram(name: "strtod", scope: !33, file: !33, line: 29, type: !70, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!70 = !DISubroutineType(types: !71)
!71 = !{!53, !72, !73}
!72 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !54)
!73 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !74)
!74 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!75 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !76, file: !30, line: 103)
!76 = !DISubprogram(name: "strtof", scope: !33, file: !33, line: 28, type: !77, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!77 = !DISubroutineType(types: !78)
!78 = !{!79, !72, !73}
!79 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!80 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !81, file: !30, line: 104)
!81 = !DISubprogram(name: "strtold", scope: !33, file: !33, line: 30, type: !82, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!82 = !DISubroutineType(types: !83)
!83 = !{!84, !72, !73}
!84 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!85 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !86, file: !30, line: 105)
!86 = !DISubprogram(name: "strtol", scope: !33, file: !33, line: 32, type: !87, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!87 = !DISubroutineType(types: !88)
!88 = !{!40, !72, !73, !14}
!89 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !90, file: !30, line: 106)
!90 = !DISubprogram(name: "strtoll", scope: !33, file: !33, line: 34, type: !91, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!91 = !DISubroutineType(types: !92)
!92 = !{!47, !72, !73, !14}
!93 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !94, file: !30, line: 107)
!94 = !DISubprogram(name: "strtoul", scope: !33, file: !33, line: 33, type: !95, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!95 = !DISubroutineType(types: !96)
!96 = !{!29, !72, !73, !14}
!97 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !98, file: !30, line: 108)
!98 = !DISubprogram(name: "strtoull", scope: !33, file: !33, line: 35, type: !99, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!99 = !DISubroutineType(types: !100)
!100 = !{!101, !72, !73, !14}
!101 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!102 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !103, file: !30, line: 109)
!103 = !DISubprogram(name: "rand", scope: !33, file: !33, line: 37, type: !104, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!104 = !DISubroutineType(types: !105)
!105 = !{!14}
!106 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !107, file: !30, line: 110)
!107 = !DISubprogram(name: "srand", scope: !33, file: !33, line: 38, type: !108, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!108 = !DISubroutineType(types: !109)
!109 = !{null, !110}
!110 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!111 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !112, file: !30, line: 111)
!112 = !DISubprogram(name: "calloc", scope: !33, file: !33, line: 41, type: !113, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!113 = !DISubroutineType(types: !114)
!114 = !{!6, !27, !27}
!115 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !116, file: !30, line: 112)
!116 = !DISubprogram(name: "free", scope: !33, file: !33, line: 43, type: !117, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!117 = !DISubroutineType(types: !118)
!118 = !{null, !6}
!119 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !120, file: !30, line: 113)
!120 = !DISubprogram(name: "malloc", scope: !33, file: !33, line: 40, type: !121, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!121 = !DISubroutineType(types: !122)
!122 = !{!6, !27}
!123 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !124, file: !30, line: 114)
!124 = !DISubprogram(name: "realloc", scope: !33, file: !33, line: 42, type: !125, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!125 = !DISubroutineType(types: !126)
!126 = !{!6, !6, !27}
!127 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !128, file: !30, line: 115)
!128 = !DISubprogram(name: "abort", scope: !33, file: !33, line: 46, type: !129, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!129 = !DISubroutineType(types: !130)
!130 = !{null}
!131 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !132, file: !30, line: 116)
!132 = !DISubprogram(name: "atexit", scope: !33, file: !33, line: 48, type: !133, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!133 = !DISubroutineType(types: !134)
!134 = !{!14, !135}
!135 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !129, size: 64)
!136 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !137, file: !30, line: 117)
!137 = !DISubprogram(name: "exit", scope: !33, file: !33, line: 49, type: !138, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!138 = !DISubroutineType(types: !139)
!139 = !{null, !14}
!140 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !141, file: !30, line: 118)
!141 = !DISubprogram(name: "_Exit", scope: !33, file: !33, line: 50, type: !138, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!142 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !143, file: !30, line: 119)
!143 = !DISubprogram(name: "getenv", scope: !33, file: !33, line: 54, type: !144, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!144 = !DISubroutineType(types: !145)
!145 = !{!11, !54}
!146 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !147, file: !30, line: 120)
!147 = !DISubprogram(name: "system", scope: !33, file: !33, line: 56, type: !58, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!148 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !149, file: !30, line: 121)
!149 = !DISubprogram(name: "bsearch", scope: !33, file: !33, line: 58, type: !150, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!150 = !DISubroutineType(types: !151)
!151 = !{!6, !152, !152, !27, !27, !154}
!152 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !153, size: 64)
!153 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!154 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !155, size: 64)
!155 = !DISubroutineType(types: !156)
!156 = !{!14, !152, !152}
!157 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !158, file: !30, line: 122)
!158 = !DISubprogram(name: "qsort", scope: !33, file: !33, line: 59, type: !159, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!159 = !DISubroutineType(types: !160)
!160 = !{null, !6, !27, !27, !154}
!161 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !162, file: !30, line: 123)
!162 = !DISubprogram(name: "abs", linkageName: "_Z3absB6v15004e", scope: !163, file: !163, line: 129, type: !164, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!163 = !DIFile(filename: "stdlib.h", directory: "")
!164 = !DISubroutineType(types: !165)
!165 = !{!84, !84}
!166 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !167, file: !30, line: 124)
!167 = !DISubprogram(name: "labs", scope: !33, file: !33, line: 62, type: !168, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!168 = !DISubroutineType(types: !169)
!169 = !{!40, !40}
!170 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !171, file: !30, line: 125)
!171 = !DISubprogram(name: "llabs", scope: !33, file: !33, line: 63, type: !172, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!172 = !DISubroutineType(types: !173)
!173 = !{!47, !47}
!174 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !175, file: !30, line: 126)
!175 = !DISubprogram(name: "div", linkageName: "_Z3divB6v15004xx", scope: !163, file: !163, line: 152, type: !176, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!176 = !DISubroutineType(types: !177)
!177 = !{!43, !47, !47}
!178 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !179, file: !30, line: 127)
!179 = !DISubprogram(name: "ldiv", scope: !33, file: !33, line: 70, type: !180, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!180 = !DISubroutineType(types: !181)
!181 = !{!36, !40, !40}
!182 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !183, file: !30, line: 128)
!183 = !DISubprogram(name: "lldiv", scope: !33, file: !33, line: 71, type: !176, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!184 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !185, file: !30, line: 129)
!185 = !DISubprogram(name: "mblen", scope: !33, file: !33, line: 73, type: !186, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!186 = !DISubroutineType(types: !187)
!187 = !{!14, !54, !27}
!188 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !189, file: !30, line: 130)
!189 = !DISubprogram(name: "mbtowc", scope: !33, file: !33, line: 74, type: !190, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!190 = !DISubroutineType(types: !191)
!191 = !{!14, !192, !72, !27}
!192 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !193)
!193 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !194, size: 64)
!194 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!195 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !196, file: !30, line: 131)
!196 = !DISubprogram(name: "wctomb", scope: !33, file: !33, line: 75, type: !197, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!197 = !DISubroutineType(types: !198)
!198 = !{!14, !11, !194}
!199 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !200, file: !30, line: 132)
!200 = !DISubprogram(name: "mbstowcs", scope: !33, file: !33, line: 76, type: !201, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!201 = !DISubroutineType(types: !202)
!202 = !{!27, !192, !72, !27}
!203 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !204, file: !30, line: 133)
!204 = !DISubprogram(name: "wcstombs", scope: !33, file: !33, line: 77, type: !205, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!205 = !DISubroutineType(types: !206)
!206 = !{!27, !207, !208, !27}
!207 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !11)
!208 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !209)
!209 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !210, size: 64)
!210 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !194)
!211 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !212, file: !30, line: 135)
!212 = !DISubprogram(name: "at_quick_exit", scope: !33, file: !33, line: 51, type: !133, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!213 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !214, file: !30, line: 136)
!214 = !DISubprogram(name: "quick_exit", scope: !33, file: !33, line: 52, type: !138, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!215 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !27, file: !216, line: 69)
!216 = !DIFile(filename: "cstring", directory: "")
!217 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !218, file: !216, line: 70)
!218 = !DISubprogram(name: "memcpy", scope: !219, file: !219, line: 27, type: !220, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!219 = !DIFile(filename: "string.h", directory: "", checksumkind: CSK_MD5, checksum: "eb1bf98d1059ccc3c197a450734602f7")
!220 = !DISubroutineType(types: !221)
!221 = !{!6, !222, !223, !27}
!222 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !6)
!223 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !152)
!224 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !225, file: !216, line: 71)
!225 = !DISubprogram(name: "memmove", scope: !219, file: !219, line: 28, type: !226, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!226 = !DISubroutineType(types: !227)
!227 = !{!6, !6, !152, !27}
!228 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !229, file: !216, line: 72)
!229 = !DISubprogram(name: "strcpy", scope: !219, file: !219, line: 33, type: !230, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!230 = !DISubroutineType(types: !231)
!231 = !{!11, !207, !72}
!232 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !233, file: !216, line: 73)
!233 = !DISubprogram(name: "strncpy", scope: !219, file: !219, line: 34, type: !234, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!234 = !DISubroutineType(types: !235)
!235 = !{!11, !207, !72, !27}
!236 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !237, file: !216, line: 74)
!237 = !DISubprogram(name: "strcat", scope: !219, file: !219, line: 36, type: !230, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!238 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !239, file: !216, line: 75)
!239 = !DISubprogram(name: "strncat", scope: !219, file: !219, line: 37, type: !234, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!240 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !241, file: !216, line: 76)
!241 = !DISubprogram(name: "memcmp", scope: !219, file: !219, line: 30, type: !242, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!242 = !DISubroutineType(types: !243)
!243 = !{!14, !152, !152, !27}
!244 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !245, file: !216, line: 77)
!245 = !DISubprogram(name: "strcmp", scope: !219, file: !219, line: 39, type: !246, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!246 = !DISubroutineType(types: !247)
!247 = !{!14, !54, !54}
!248 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !249, file: !216, line: 78)
!249 = !DISubprogram(name: "strncmp", scope: !219, file: !219, line: 40, type: !250, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!250 = !DISubroutineType(types: !251)
!251 = !{!14, !54, !54, !27}
!252 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !253, file: !216, line: 79)
!253 = !DISubprogram(name: "strcoll", scope: !219, file: !219, line: 42, type: !246, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!254 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !255, file: !216, line: 80)
!255 = !DISubprogram(name: "strxfrm", scope: !219, file: !219, line: 43, type: !256, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!256 = !DISubroutineType(types: !257)
!257 = !{!27, !207, !72, !27}
!258 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !259, file: !216, line: 81)
!259 = !DISubprogram(name: "memchr", linkageName: "_Z6memchrB6v15004Ua9enable_ifILb1EEPvim", scope: !260, file: !260, line: 98, type: !261, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!260 = !DIFile(filename: "string.h", directory: "")
!261 = !DISubroutineType(types: !262)
!262 = !{!6, !6, !14, !27}
!263 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !264, file: !216, line: 82)
!264 = !DISubprogram(name: "strchr", linkageName: "_Z6strchrB6v15004Ua9enable_ifILb1EEPci", scope: !260, file: !260, line: 77, type: !265, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!265 = !DISubroutineType(types: !266)
!266 = !{!11, !11, !14}
!267 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !268, file: !216, line: 83)
!268 = !DISubprogram(name: "strcspn", scope: !219, file: !219, line: 48, type: !269, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!269 = !DISubroutineType(types: !270)
!270 = !{!27, !54, !54}
!271 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !272, file: !216, line: 84)
!272 = !DISubprogram(name: "strpbrk", linkageName: "_Z7strpbrkB6v15004Ua9enable_ifILb1EEPcPKc", scope: !260, file: !260, line: 84, type: !273, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!273 = !DISubroutineType(types: !274)
!274 = !{!11, !11, !54}
!275 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !276, file: !216, line: 85)
!276 = !DISubprogram(name: "strrchr", linkageName: "_Z7strrchrB6v15004Ua9enable_ifILb1EEPci", scope: !260, file: !260, line: 91, type: !265, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!277 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !278, file: !216, line: 86)
!278 = !DISubprogram(name: "strspn", scope: !219, file: !219, line: 49, type: !269, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!279 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !280, file: !216, line: 87)
!280 = !DISubprogram(name: "strstr", linkageName: "_Z6strstrB6v15004Ua9enable_ifILb1EEPcPKc", scope: !260, file: !260, line: 105, type: !273, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!281 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !282, file: !216, line: 88)
!282 = !DISubprogram(name: "strtok", scope: !219, file: !219, line: 52, type: !230, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!283 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !284, file: !216, line: 89)
!284 = !DISubprogram(name: "memset", scope: !219, file: !219, line: 29, type: !261, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!285 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !286, file: !216, line: 90)
!286 = !DISubprogram(name: "strerror", scope: !219, file: !219, line: 56, type: !287, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!287 = !DISubroutineType(types: !288)
!288 = !{!11, !14}
!289 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !290, file: !216, line: 91)
!290 = !DISubprogram(name: "strlen", scope: !219, file: !219, line: 54, type: !291, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!291 = !DISubroutineType(types: !292)
!292 = !{!27, !54}
!293 = !{!294, !295}
!294 = !DILocalVariable(name: "p1", arg: 1, scope: !2, file: !3, line: 53, type: !6)
!295 = !DILocalVariable(name: "p2", arg: 2, scope: !2, file: !3, line: 53, type: !6)
!296 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 128, elements: !297)
!297 = !{!298}
!298 = !DISubrange(count: 2)
!299 = !{i32 7, !"Dwarf Version", i32 5}
!300 = !{i32 7, !"ReferenceTracking", i32 1}
!301 = !{i32 2, !"Debug Info Version", i32 3}
!302 = !{i32 1, !"wchar_size", i32 4}
!303 = !{i32 7, !"PIC Level", i32 2}
!304 = !{i32 7, !"frame-pointer", i32 1}
!305 = !{!"clang version 15.0.4"}
!306 = distinct !DISubprogram(name: "test_continuous_store", linkageName: "_Z21test_continuous_storev", scope: !3, file: !3, line: 19, type: !129, scopeLine: 19, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !307)
!307 = !{!308, !309, !310, !311}
!308 = !DILocalVariable(name: "p1", scope: !306, file: !3, line: 20, type: !6)
!309 = !DILocalVariable(name: "p2", scope: !306, file: !3, line: 21, type: !6)
!310 = !DILocalVariable(name: "p3", scope: !306, file: !3, line: 22, type: !6)
!311 = !DILocalVariable(name: "p4", scope: !306, file: !3, line: 23, type: !6)
!312 = !DILocation(line: 20, column: 16, scope: !306)
!313 = !{!"p1", !"void*"}
!314 = !DILocation(line: 0, scope: !306)
!315 = !DILocation(line: 21, column: 16, scope: !306)
!316 = !{!"p2", !"void*"}
!317 = !DILocation(line: 22, column: 16, scope: !306)
!318 = !{!"p3", !"void*"}
!319 = !DILocation(line: 23, column: 16, scope: !306)
!320 = !{!"p4", !"void*"}
!321 = !DILocalVariable(name: "p", arg: 1, scope: !322, file: !3, line: 16, type: !6)
!322 = distinct !DISubprogram(name: "save_ptr", linkageName: "_ZL8save_ptrPv", scope: !3, file: !3, line: 16, type: !117, scopeLine: 16, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !323)
!323 = !{!321}
!324 = !DILocation(line: 0, scope: !322, inlinedAt: !325)
!325 = distinct !DILocation(line: 24, column: 5, scope: !306)
!326 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !325)
!327 = !{!328, !328, i64 0}
!328 = !{!"int", !329, i64 0}
!329 = !{!"omnipotent char", !330, i64 0}
!330 = !{!"Simple C++ TBAA"}
!331 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !325)
!332 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !325)
!333 = !{!334, !334, i64 0}
!334 = !{!"any pointer", !329, i64 0}
!335 = !{!"g_sink[]", !"void*"}
!336 = !DILocation(line: 0, scope: !322, inlinedAt: !337)
!337 = distinct !DILocation(line: 24, column: 19, scope: !306)
!338 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !337)
!339 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !337)
!340 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !337)
!341 = !DILocation(line: 0, scope: !322, inlinedAt: !342)
!342 = distinct !DILocation(line: 24, column: 33, scope: !306)
!343 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !342)
!344 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !342)
!345 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !342)
!346 = !DILocation(line: 0, scope: !322, inlinedAt: !347)
!347 = distinct !DILocation(line: 24, column: 47, scope: !306)
!348 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !347)
!349 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !347)
!350 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !347)
!351 = !DILocation(line: 25, column: 1, scope: !306)
!352 = distinct !DISubprogram(name: "test_struct_store", linkageName: "_Z17test_struct_storev", scope: !3, file: !3, line: 31, type: !129, scopeLine: 31, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !353)
!353 = !{!354, !359}
!354 = !DILocalVariable(name: "pair", scope: !352, file: !3, line: 32, type: !355)
!355 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "PointerPair", file: !3, line: 28, size: 128, flags: DIFlagTypePassByValue, elements: !356, identifier: "_ZTS11PointerPair")
!356 = !{!357, !358}
!357 = !DIDerivedType(tag: DW_TAG_member, name: "ptr1", scope: !355, file: !3, line: 28, baseType: !6, size: 64)
!358 = !DIDerivedType(tag: DW_TAG_member, name: "ptr2", scope: !355, file: !3, line: 28, baseType: !6, size: 64, offset: 64)
!359 = !DILocalVariable(name: "quad", scope: !352, file: !3, line: 37, type: !360)
!360 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "PointerQuad", file: !3, line: 29, size: 256, flags: DIFlagTypePassByValue, elements: !361, identifier: "_ZTS11PointerQuad")
!361 = !{!362, !363, !364, !365}
!362 = !DIDerivedType(tag: DW_TAG_member, name: "p1", scope: !360, file: !3, line: 29, baseType: !6, size: 64)
!363 = !DIDerivedType(tag: DW_TAG_member, name: "p2", scope: !360, file: !3, line: 29, baseType: !6, size: 64, offset: 64)
!364 = !DIDerivedType(tag: DW_TAG_member, name: "p3", scope: !360, file: !3, line: 29, baseType: !6, size: 64, offset: 128)
!365 = !DIDerivedType(tag: DW_TAG_member, name: "p4", scope: !360, file: !3, line: 29, baseType: !6, size: 64, offset: 192)
!366 = !DILocation(line: 33, column: 17, scope: !352)
!367 = !{!"pair.ptr1", !"void*"}
!368 = !DILocation(line: 0, scope: !352)
!369 = !DILocation(line: 34, column: 17, scope: !352)
!370 = !{!"pair.ptr2", !"void*"}
!371 = !DILocation(line: 0, scope: !322, inlinedAt: !372)
!372 = distinct !DILocation(line: 35, column: 5, scope: !352)
!373 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !372)
!374 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !372)
!375 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !372)
!376 = !DILocation(line: 0, scope: !322, inlinedAt: !377)
!377 = distinct !DILocation(line: 35, column: 26, scope: !352)
!378 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !377)
!379 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !377)
!380 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !377)
!381 = !DILocation(line: 38, column: 15, scope: !352)
!382 = !{!"quad.p1", !"void*"}
!383 = !DILocation(line: 38, column: 37, scope: !352)
!384 = !{!"quad.p2", !"void*"}
!385 = !DILocation(line: 39, column: 15, scope: !352)
!386 = !{!"quad.p3", !"void*"}
!387 = !DILocation(line: 39, column: 37, scope: !352)
!388 = !{!"quad.p4", !"void*"}
!389 = !DILocation(line: 0, scope: !322, inlinedAt: !390)
!390 = distinct !DILocation(line: 40, column: 5, scope: !352)
!391 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !390)
!392 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !390)
!393 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !390)
!394 = !DILocation(line: 0, scope: !322, inlinedAt: !395)
!395 = distinct !DILocation(line: 40, column: 24, scope: !352)
!396 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !395)
!397 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !395)
!398 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !395)
!399 = !DILocation(line: 0, scope: !322, inlinedAt: !400)
!400 = distinct !DILocation(line: 40, column: 43, scope: !352)
!401 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !400)
!402 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !400)
!403 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !400)
!404 = !DILocation(line: 0, scope: !322, inlinedAt: !405)
!405 = distinct !DILocation(line: 40, column: 62, scope: !352)
!406 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !405)
!407 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !405)
!408 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !405)
!409 = !DILocation(line: 41, column: 1, scope: !352)
!410 = distinct !DISubprogram(name: "test_array_store", linkageName: "_Z16test_array_storev", scope: !3, file: !3, line: 44, type: !129, scopeLine: 44, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !411)
!411 = !{!412, !416}
!412 = !DILocalVariable(name: "ptrs", scope: !410, file: !3, line: 45, type: !413)
!413 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 512, elements: !414)
!414 = !{!415}
!415 = !DISubrange(count: 8)
!416 = !DILocalVariable(name: "i", scope: !417, file: !3, line: 46, type: !14)
!417 = distinct !DILexicalBlock(scope: !410, file: !3, line: 46, column: 5)
!418 = !DILocation(line: 0, scope: !417)
!419 = !DILocation(line: 46, column: 5, scope: !417)
!420 = !DILocation(line: 47, column: 19, scope: !421)
!421 = distinct !DILexicalBlock(scope: !422, file: !3, line: 46, column: 33)
!422 = distinct !DILexicalBlock(scope: !417, file: !3, line: 46, column: 5)
!423 = !{!"ptrs[]", !"void*"}
!424 = !DILocation(line: 0, scope: !410)
!425 = !DILocation(line: 0, scope: !322, inlinedAt: !426)
!426 = distinct !DILocation(line: 48, column: 9, scope: !421)
!427 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !426)
!428 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !426)
!429 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !426)
!430 = !DILocation(line: 50, column: 1, scope: !410)
!431 = !DILocation(line: 0, scope: !2)
!432 = !DILocation(line: 55, column: 15, scope: !2)
!433 = !{!"g_args", !"void*[]"}
!434 = !DILocation(line: 55, column: 31, scope: !2)
!435 = !{!"g_args[]", !"void*"}
!436 = !DILocation(line: 56, column: 1, scope: !2)
!437 = distinct !DISubprogram(name: "test_param_store", linkageName: "_Z16test_param_storev", scope: !3, file: !3, line: 58, type: !129, scopeLine: 58, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !438)
!438 = !{!439, !440}
!439 = !DILocalVariable(name: "p1", scope: !437, file: !3, line: 59, type: !6)
!440 = !DILocalVariable(name: "p2", scope: !437, file: !3, line: 60, type: !6)
!441 = !DILocation(line: 59, column: 16, scope: !437)
!442 = !DILocation(line: 0, scope: !437)
!443 = !DILocation(line: 60, column: 16, scope: !437)
!444 = !DILocation(line: 61, column: 5, scope: !437)
!445 = !DILocation(line: 0, scope: !322, inlinedAt: !446)
!446 = distinct !DILocation(line: 62, column: 5, scope: !437)
!447 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !446)
!448 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !446)
!449 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !446)
!450 = !DILocation(line: 0, scope: !322, inlinedAt: !451)
!451 = distinct !DILocation(line: 62, column: 19, scope: !437)
!452 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !451)
!453 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !451)
!454 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !451)
!455 = !DILocation(line: 63, column: 1, scope: !437)
!456 = distinct !DISubprogram(name: "test_loop_store", linkageName: "_Z15test_loop_storei", scope: !3, file: !3, line: 66, type: !138, scopeLine: 66, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !457)
!457 = !{!458, !459, !460}
!458 = !DILocalVariable(name: "count", arg: 1, scope: !456, file: !3, line: 66, type: !14)
!459 = !DILocalVariable(name: "buffers", scope: !456, file: !3, line: 67, type: !10)
!460 = !DILocalVariable(name: "i", scope: !461, file: !3, line: 68, type: !14)
!461 = distinct !DILexicalBlock(scope: !456, file: !3, line: 68, column: 5)
!462 = !DILocation(line: 0, scope: !456)
!463 = !DILocation(line: 67, column: 37, scope: !456)
!464 = !DILocation(line: 67, column: 43, scope: !456)
!465 = !DILocation(line: 67, column: 30, scope: !456)
!466 = !{!"buffers", !"void**"}
!467 = !DILocation(line: 0, scope: !461)
!468 = !DILocation(line: 68, column: 23, scope: !469)
!469 = distinct !DILexicalBlock(scope: !461, file: !3, line: 68, column: 5)
!470 = !DILocation(line: 68, column: 5, scope: !461)
!471 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !472)
!472 = distinct !DILocation(line: 70, column: 9, scope: !473)
!473 = distinct !DILexicalBlock(scope: !469, file: !3, line: 68, column: 37)
!474 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !475)
!475 = distinct !DILocation(line: 72, column: 5, scope: !456)
!476 = !DILocation(line: 0, scope: !322, inlinedAt: !475)
!477 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !475)
!478 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !475)
!479 = !DILocation(line: 73, column: 1, scope: !456)
!480 = !DILocation(line: 69, column: 22, scope: !473)
!481 = !{!"buffers", !"void*"}
!482 = !DILocation(line: 69, column: 9, scope: !473)
!483 = !DILocation(line: 69, column: 20, scope: !473)
!484 = !DILocation(line: 0, scope: !322, inlinedAt: !472)
!485 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !472)
!486 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !472)
!487 = !DILocation(line: 68, column: 33, scope: !469)
!488 = distinct !{!488, !470, !489, !490}
!489 = !DILocation(line: 71, column: 5, scope: !461)
!490 = !{!"llvm.loop.mustprogress"}
!491 = distinct !DISubprogram(name: "test_conditional_store", linkageName: "_Z22test_conditional_storei", scope: !3, file: !3, line: 76, type: !138, scopeLine: 76, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !492)
!492 = !{!493, !494, !495, !496}
!493 = !DILocalVariable(name: "cond", arg: 1, scope: !491, file: !3, line: 76, type: !14)
!494 = !DILocalVariable(name: "p1", scope: !491, file: !3, line: 77, type: !6)
!495 = !DILocalVariable(name: "p2", scope: !491, file: !3, line: 78, type: !6)
!496 = !DILocalVariable(name: "result", scope: !491, file: !3, line: 79, type: !6)
!497 = !DILocation(line: 0, scope: !491)
!498 = !DILocation(line: 77, column: 16, scope: !491)
!499 = !DILocation(line: 78, column: 16, scope: !491)
!500 = !DILocation(line: 79, column: 25, scope: !491)
!501 = !DILocation(line: 79, column: 20, scope: !491)
!502 = !DILocation(line: 0, scope: !322, inlinedAt: !503)
!503 = distinct !DILocation(line: 80, column: 5, scope: !491)
!504 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !503)
!505 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !503)
!506 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !503)
!507 = !DILocation(line: 0, scope: !322, inlinedAt: !508)
!508 = distinct !DILocation(line: 80, column: 19, scope: !491)
!509 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !508)
!510 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !508)
!511 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !508)
!512 = !DILocation(line: 0, scope: !322, inlinedAt: !513)
!513 = distinct !DILocation(line: 80, column: 33, scope: !491)
!514 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !513)
!515 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !513)
!516 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !513)
!517 = !DILocation(line: 81, column: 1, scope: !491)
!518 = distinct !DISubprogram(name: "test_switch_ptr_store", linkageName: "_Z21test_switch_ptr_storei", scope: !3, file: !3, line: 84, type: !519, scopeLine: 84, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !521)
!519 = !DISubroutineType(types: !520)
!520 = !{!6, !14}
!521 = !{!522, !523}
!522 = !DILocalVariable(name: "condition", arg: 1, scope: !518, file: !3, line: 84, type: !14)
!523 = !DILocalVariable(name: "result", scope: !518, file: !3, line: 85, type: !6)
!524 = !DILocation(line: 0, scope: !518)
!525 = !DILocation(line: 86, column: 5, scope: !518)
!526 = !DILocation(line: 87, column: 26, scope: !527)
!527 = distinct !DILexicalBlock(scope: !518, file: !3, line: 86, column: 24)
!528 = !{!"result", !"void*"}
!529 = !DILocation(line: 87, column: 38, scope: !527)
!530 = !DILocation(line: 88, column: 26, scope: !527)
!531 = !DILocation(line: 88, column: 39, scope: !527)
!532 = !DILocation(line: 89, column: 26, scope: !527)
!533 = !DILocation(line: 89, column: 39, scope: !527)
!534 = !DILocation(line: 90, column: 27, scope: !527)
!535 = !DILocation(line: 90, column: 40, scope: !527)
!536 = !DILocation(line: 0, scope: !527)
!537 = !DILocation(line: 0, scope: !322, inlinedAt: !538)
!538 = distinct !DILocation(line: 92, column: 5, scope: !518)
!539 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !538)
!540 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !538)
!541 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !538)
!542 = !DILocation(line: 93, column: 5, scope: !518)
!543 = distinct !DISubprogram(name: "test_template_store", linkageName: "_Z19test_template_storev", scope: !3, file: !3, line: 102, type: !129, scopeLine: 102, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !544)
!544 = !{!545, !546}
!545 = !DILocalVariable(name: "p1", scope: !543, file: !3, line: 103, type: !11)
!546 = !DILocalVariable(name: "p2", scope: !543, file: !3, line: 104, type: !13)
!547 = !DILocalVariable(name: "count", arg: 1, scope: !548, file: !3, line: 98, type: !27)
!548 = distinct !DISubprogram(name: "allocate_array<char>", linkageName: "_Z14allocate_arrayIcEPT_m", scope: !3, file: !3, line: 98, type: !549, scopeLine: 98, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, templateParams: !552, retainedNodes: !551)
!549 = !DISubroutineType(types: !550)
!550 = !{!11, !27}
!551 = !{!547}
!552 = !{!553}
!553 = !DITemplateTypeParameter(name: "T", type: !12)
!554 = !DILocation(line: 0, scope: !548, inlinedAt: !555)
!555 = distinct !DILocation(line: 103, column: 16, scope: !543)
!556 = !DILocation(line: 99, column: 16, scope: !548, inlinedAt: !555)
!557 = !DILocation(line: 0, scope: !543)
!558 = !DILocalVariable(name: "count", arg: 1, scope: !559, file: !3, line: 98, type: !27)
!559 = distinct !DISubprogram(name: "allocate_array<int>", linkageName: "_Z14allocate_arrayIiEPT_m", scope: !3, file: !3, line: 98, type: !560, scopeLine: 98, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, templateParams: !563, retainedNodes: !562)
!560 = !DISubroutineType(types: !561)
!561 = !{!13, !27}
!562 = !{!558}
!563 = !{!564}
!564 = !DITemplateTypeParameter(name: "T", type: !14)
!565 = !DILocation(line: 0, scope: !559, inlinedAt: !566)
!566 = distinct !DILocation(line: 104, column: 15, scope: !543)
!567 = !DILocation(line: 99, column: 16, scope: !559, inlinedAt: !566)
!568 = !DILocation(line: 0, scope: !322, inlinedAt: !569)
!569 = distinct !DILocation(line: 105, column: 5, scope: !543)
!570 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !569)
!571 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !569)
!572 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !569)
!573 = !DILocation(line: 0, scope: !322, inlinedAt: !574)
!574 = distinct !DILocation(line: 105, column: 19, scope: !543)
!575 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !574)
!576 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !574)
!577 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !574)
!578 = !DILocation(line: 106, column: 1, scope: !543)
!579 = distinct !DISubprogram(name: "test_lambda_store", linkageName: "_Z17test_lambda_storev", scope: !3, file: !3, line: 109, type: !129, scopeLine: 109, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !580)
!580 = !{!581, !589, !590}
!581 = !DILocalVariable(name: "alloc", scope: !579, file: !3, line: 110, type: !582)
!582 = distinct !DICompositeType(tag: DW_TAG_class_type, scope: !579, file: !3, line: 110, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !583)
!583 = !{!584}
!584 = !DISubprogram(name: "operator()", scope: !582, file: !3, line: 110, type: !585, scopeLine: 110, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagOptimized)
!585 = !DISubroutineType(types: !586)
!586 = !{!6, !587, !27}
!587 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !588, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!588 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !582)
!589 = !DILocalVariable(name: "p1", scope: !579, file: !3, line: 111, type: !6)
!590 = !DILocalVariable(name: "p2", scope: !579, file: !3, line: 112, type: !6)
!591 = !DILocation(line: 110, column: 10, scope: !579)
!592 = !DILocalVariable(name: "this", arg: 1, scope: !593, type: !597, flags: DIFlagArtificial | DIFlagObjectPointer)
!593 = distinct !DISubprogram(name: "operator()", linkageName: "_ZZ17test_lambda_storevENK3$_0clEm", scope: !582, file: !3, line: 110, type: !594, scopeLine: 110, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !7, declaration: !584, retainedNodes: !595)
!594 = !DISubroutineType(cc: DW_CC_nocall, types: !586)
!595 = !{!592, !596}
!596 = !DILocalVariable(name: "s", arg: 2, scope: !593, file: !3, line: 110, type: !27)
!597 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !598)
!598 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !588, size: 64)
!599 = !DILocation(line: 0, scope: !593, inlinedAt: !600)
!600 = distinct !DILocation(line: 111, column: 16, scope: !579)
!601 = !DILocation(line: 110, column: 49, scope: !593, inlinedAt: !600)
!602 = !DILocation(line: 0, scope: !579)
!603 = !DILocation(line: 0, scope: !593, inlinedAt: !604)
!604 = distinct !DILocation(line: 112, column: 16, scope: !579)
!605 = !DILocation(line: 110, column: 49, scope: !593, inlinedAt: !604)
!606 = !DILocation(line: 0, scope: !322, inlinedAt: !607)
!607 = distinct !DILocation(line: 113, column: 5, scope: !579)
!608 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !607)
!609 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !607)
!610 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !607)
!611 = !DILocation(line: 0, scope: !322, inlinedAt: !612)
!612 = distinct !DILocation(line: 113, column: 19, scope: !579)
!613 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !612)
!614 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !612)
!615 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !612)
!616 = !DILocation(line: 114, column: 1, scope: !579)
!617 = distinct !DISubprogram(name: "recursive_alloc", linkageName: "_Z15recursive_alloci", scope: !3, file: !3, line: 117, type: !519, scopeLine: 117, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !618)
!618 = !{!619}
!619 = !DILocalVariable(name: "depth", arg: 1, scope: !617, file: !3, line: 117, type: !14)
!620 = !DILocation(line: 0, scope: !617)
!621 = !DILocation(line: 118, column: 28, scope: !622)
!622 = distinct !DILexicalBlock(scope: !617, file: !3, line: 118, column: 9)
!623 = !{!"0_UNKNOWN_", !"0_UNKNOWN_"}
!624 = !DILocation(line: 120, column: 1, scope: !617)
!625 = distinct !DISubprogram(name: "test_recursive_store", linkageName: "_Z20test_recursive_storev", scope: !3, file: !3, line: 122, type: !129, scopeLine: 122, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !626)
!626 = !{!627, !628}
!627 = !DILocalVariable(name: "p1", scope: !625, file: !3, line: 123, type: !6)
!628 = !DILocalVariable(name: "p2", scope: !625, file: !3, line: 124, type: !6)
!629 = !DILocation(line: 123, column: 16, scope: !625)
!630 = !DILocation(line: 0, scope: !625)
!631 = !DILocation(line: 124, column: 16, scope: !625)
!632 = !DILocation(line: 0, scope: !322, inlinedAt: !633)
!633 = distinct !DILocation(line: 125, column: 5, scope: !625)
!634 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !633)
!635 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !633)
!636 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !633)
!637 = !DILocation(line: 0, scope: !322, inlinedAt: !638)
!638 = distinct !DILocation(line: 125, column: 19, scope: !625)
!639 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !638)
!640 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !638)
!641 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !638)
!642 = !DILocation(line: 126, column: 1, scope: !625)
!643 = distinct !DISubprogram(name: "test_realloc_calloc_store", linkageName: "_Z25test_realloc_calloc_storev", scope: !3, file: !3, line: 129, type: !129, scopeLine: 129, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !644)
!644 = !{!645, !646, !647}
!645 = !DILocalVariable(name: "p1", scope: !643, file: !3, line: 130, type: !6)
!646 = !DILocalVariable(name: "p2", scope: !643, file: !3, line: 131, type: !6)
!647 = !DILocalVariable(name: "p3", scope: !643, file: !3, line: 132, type: !6)
!648 = !DILocation(line: 130, column: 16, scope: !643)
!649 = !DILocation(line: 0, scope: !643)
!650 = !DILocation(line: 131, column: 16, scope: !643)
!651 = !DILocation(line: 132, column: 16, scope: !643)
!652 = !DILocation(line: 0, scope: !322, inlinedAt: !653)
!653 = distinct !DILocation(line: 133, column: 5, scope: !643)
!654 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !653)
!655 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !653)
!656 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !653)
!657 = !DILocation(line: 0, scope: !322, inlinedAt: !658)
!658 = distinct !DILocation(line: 133, column: 19, scope: !643)
!659 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !658)
!660 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !658)
!661 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !658)
!662 = !DILocation(line: 134, column: 1, scope: !643)
!663 = distinct !DISubprogram(name: "test_new_store", linkageName: "_Z14test_new_storev", scope: !3, file: !3, line: 142, type: !129, scopeLine: 142, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !664)
!664 = !{!665, !673}
!665 = !DILocalVariable(name: "obj1", scope: !663, file: !3, line: 143, type: !666)
!666 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !667, size: 64)
!667 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "MyClass", file: !3, line: 137, size: 512, flags: DIFlagTypePassByValue, elements: !668, identifier: "_ZTS7MyClass")
!668 = !{!669}
!669 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !667, file: !3, line: 139, baseType: !670, size: 512, flags: DIFlagPublic)
!670 = !DICompositeType(tag: DW_TAG_array_type, baseType: !14, size: 512, elements: !671)
!671 = !{!672}
!672 = !DISubrange(count: 16)
!673 = !DILocalVariable(name: "obj2", scope: !663, file: !3, line: 144, type: !666)
!674 = !DILocation(line: 143, column: 21, scope: !663)
!675 = !{!"obj1", !"MyClass*"}
!676 = !DILocation(line: 143, column: 25, scope: !663)
!677 = !DILocation(line: 0, scope: !663)
!678 = !DILocation(line: 144, column: 21, scope: !663)
!679 = !{!"obj2", !"MyClass*"}
!680 = !DILocation(line: 144, column: 25, scope: !663)
!681 = !DILocation(line: 0, scope: !322, inlinedAt: !682)
!682 = distinct !DILocation(line: 145, column: 5, scope: !663)
!683 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !682)
!684 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !682)
!685 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !682)
!686 = !DILocation(line: 0, scope: !322, inlinedAt: !687)
!687 = distinct !DILocation(line: 145, column: 21, scope: !663)
!688 = !DILocation(line: 16, column: 50, scope: !322, inlinedAt: !687)
!689 = !DILocation(line: 16, column: 33, scope: !322, inlinedAt: !687)
!690 = !DILocation(line: 16, column: 54, scope: !322, inlinedAt: !687)
!691 = !DILocation(line: 146, column: 1, scope: !663)
!692 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 148, type: !104, scopeLine: 148, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !693)
!693 = !{}
!694 = !DILocation(line: 149, column: 5, scope: !692)
!695 = !DILocation(line: 150, column: 5, scope: !692)
!696 = !DILocation(line: 151, column: 5, scope: !692)
!697 = !DILocation(line: 152, column: 5, scope: !692)
!698 = !DILocation(line: 153, column: 5, scope: !692)
!699 = !DILocation(line: 154, column: 5, scope: !692)
!700 = !DILocation(line: 155, column: 5, scope: !692)
!701 = !DILocation(line: 156, column: 5, scope: !692)
!702 = !DILocation(line: 157, column: 5, scope: !692)
!703 = !DILocation(line: 158, column: 5, scope: !692)
!704 = !DILocation(line: 159, column: 5, scope: !692)
!705 = !DILocation(line: 160, column: 5, scope: !692)
!706 = !DILocation(line: 161, column: 5, scope: !692)
!707 = !DILocation(line: 162, column: 5, scope: !692)
!708 = !DILocation(line: 163, column: 5, scope: !692)