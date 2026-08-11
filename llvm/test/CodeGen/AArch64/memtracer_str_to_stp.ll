; REQUIRES: aarch64-registered-target

; RUN: llc -mtriple=aarch64-linux-ohos -relocation-model=pic -filetype=obj %s -O2 -o %t.o
; RUN: ld.lld -shared %t.o -o %t.o
; RUN: llvm-dwarfdump --mem_tracer %t.o | FileCheck %s


; CHECK: .mem_tracer contents:
; CHECK: var="g_sink[]" type="void*"
; CHECK: var="g_args" type="void*[]"
; CHECK: var="p1" type="void*"
; CHECK: var="p2" type="void*"
; CHECK: var="p" type="void*"


; ModuleID = 'memtracer_str_to_stp.cpp'
source_filename = "memtracer_str_to_stp.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64-unknown-linux-ohos"

@_ZZ11process_ptrPvS_E6g_args = internal unnamed_addr global [2 x ptr] zeroinitializer, align 8, !dbg !0
@_ZL6g_sink = internal unnamed_addr global [64 x ptr] zeroinitializer, align 8, !dbg !16
@_ZL10g_sink_idx = internal unnamed_addr global i32 0, align 4, !dbg !21
@switch.table._Z21test_switch_ptr_storei = private unnamed_addr constant [3 x i64] [i64 64, i64 128, i64 256], align 8

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z21test_continuous_storev() local_unnamed_addr #0 !dbg !295 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !301, !memtracer !302
    #dbg_value(ptr %1, !297, !DIExpression(), !303)
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !304, !memtracer !302
    #dbg_value(ptr %2, !298, !DIExpression(), !303)
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !305, !memtracer !302
    #dbg_value(ptr %3, !299, !DIExpression(), !303)
  %4 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !306, !memtracer !302
    #dbg_value(ptr %4, !300, !DIExpression(), !303)
    #dbg_value(ptr %1, !307, !DIExpression(), !310)
  %5 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !312, !tbaa !313
  %6 = add nsw i32 %5, 1, !dbg !312
  %7 = sext i32 %5 to i64, !dbg !317
  %8 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %7, !dbg !317
  store ptr %1, ptr %8, align 8, !dbg !318, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !321)
  %9 = add nsw i32 %5, 2, !dbg !323
  %10 = sext i32 %6 to i64, !dbg !324
  %11 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %10, !dbg !324
  store ptr %2, ptr %11, align 8, !dbg !325, !tbaa !319, !memtracer !302
    #dbg_value(ptr %3, !307, !DIExpression(), !326)
  %12 = add nsw i32 %5, 3, !dbg !328
  %13 = sext i32 %9 to i64, !dbg !329
  %14 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %13, !dbg !329
  store ptr %3, ptr %14, align 8, !dbg !330, !tbaa !319, !memtracer !302
    #dbg_value(ptr %4, !307, !DIExpression(), !331)
  %15 = add nsw i32 %5, 4, !dbg !333
  store i32 %15, ptr @_ZL10g_sink_idx, align 4, !dbg !333, !tbaa !313
  %16 = sext i32 %12 to i64, !dbg !334
  %17 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %16, !dbg !334
  store ptr %4, ptr %17, align 8, !dbg !335, !tbaa !319, !memtracer !302
  ret void, !dbg !336
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !dbg !119 noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z17test_struct_storev() local_unnamed_addr #0 !dbg !337 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !351, !memtracer !302
    #dbg_value(ptr %1, !339, !DIExpression(DW_OP_LLVM_fragment, 0, 64), !352)
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !353, !memtracer !302
    #dbg_value(ptr %2, !339, !DIExpression(DW_OP_LLVM_fragment, 64, 64), !352)
    #dbg_value(ptr %1, !307, !DIExpression(), !354)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !356, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !356
  %5 = sext i32 %3 to i64, !dbg !357
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !357
  store ptr %1, ptr %6, align 8, !dbg !358, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !359)
  %7 = add nsw i32 %3, 2, !dbg !361
  %8 = sext i32 %4 to i64, !dbg !362
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !362
  store ptr %2, ptr %9, align 8, !dbg !363, !tbaa !319, !memtracer !302
  %10 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !364, !memtracer !302
    #dbg_value(ptr %10, !344, !DIExpression(DW_OP_LLVM_fragment, 0, 64), !352)
  %11 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !365, !memtracer !302
    #dbg_value(ptr %11, !344, !DIExpression(DW_OP_LLVM_fragment, 64, 64), !352)
  %12 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !366, !memtracer !302
    #dbg_value(ptr %12, !344, !DIExpression(DW_OP_LLVM_fragment, 128, 64), !352)
  %13 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !367, !memtracer !302
    #dbg_value(ptr %13, !344, !DIExpression(DW_OP_LLVM_fragment, 192, 64), !352)
    #dbg_value(ptr %10, !307, !DIExpression(), !368)
  %14 = add nsw i32 %3, 3, !dbg !370
  %15 = sext i32 %7 to i64, !dbg !371
  %16 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %15, !dbg !371
  store ptr %10, ptr %16, align 8, !dbg !372, !tbaa !319, !memtracer !302
    #dbg_value(ptr %11, !307, !DIExpression(), !373)
  %17 = add nsw i32 %3, 4, !dbg !375
  %18 = sext i32 %14 to i64, !dbg !376
  %19 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %18, !dbg !376
  store ptr %11, ptr %19, align 8, !dbg !377, !tbaa !319, !memtracer !302
    #dbg_value(ptr %12, !307, !DIExpression(), !378)
  %20 = add nsw i32 %3, 5, !dbg !380
  %21 = sext i32 %17 to i64, !dbg !381
  %22 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %21, !dbg !381
  store ptr %12, ptr %22, align 8, !dbg !382, !tbaa !319, !memtracer !302
    #dbg_value(ptr %13, !307, !DIExpression(), !383)
  %23 = add nsw i32 %3, 6, !dbg !385
  store i32 %23, ptr @_ZL10g_sink_idx, align 4, !dbg !385, !tbaa !313
  %24 = sext i32 %20 to i64, !dbg !386
  %25 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %24, !dbg !386
  store ptr %13, ptr %25, align 8, !dbg !387, !tbaa !319, !memtracer !302
  ret void, !dbg !388
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z16test_array_storev() local_unnamed_addr #0 !dbg !389 {
    #dbg_assign(i1 poison, !391, !DIExpression(), !397, ptr poison, !DIExpression(), !398)
    #dbg_value(i32 0, !395, !DIExpression(), !399)
  %1 = load i32, ptr @_ZL10g_sink_idx, align 4, !tbaa !313
    #dbg_value(i32 0, !395, !DIExpression(), !399)
  %2 = sext i32 %1 to i64, !dbg !400
    #dbg_value(i32 0, !395, !DIExpression(), !399)
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %3, !307, !DIExpression(), !404)
  %4 = add nsw i64 %2, 1, !dbg !406
  %5 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %2, !dbg !407
  store ptr %3, ptr %5, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 1, !395, !DIExpression(), !399)
  %6 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %6, !307, !DIExpression(), !404)
  %7 = add nsw i64 %2, 2, !dbg !406
  %8 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %4, !dbg !407
  store ptr %6, ptr %8, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 2, !395, !DIExpression(), !399)
  %9 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %9, !307, !DIExpression(), !404)
  %10 = add nsw i64 %2, 3, !dbg !406
  %11 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %7, !dbg !407
  store ptr %9, ptr %11, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 3, !395, !DIExpression(), !399)
  %12 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %12, !307, !DIExpression(), !404)
  %13 = add nsw i64 %2, 4, !dbg !406
  %14 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %10, !dbg !407
  store ptr %12, ptr %14, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 4, !395, !DIExpression(), !399)
  %15 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %15, !307, !DIExpression(), !404)
  %16 = add nsw i64 %2, 5, !dbg !406
  %17 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %13, !dbg !407
  store ptr %15, ptr %17, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 5, !395, !DIExpression(), !399)
  %18 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %18, !307, !DIExpression(), !404)
  %19 = add nsw i64 %2, 6, !dbg !406
  %20 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %16, !dbg !407
  store ptr %18, ptr %20, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 6, !395, !DIExpression(), !399)
  %21 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %21, !307, !DIExpression(), !404)
  %22 = add nsw i64 %2, 7, !dbg !406
  %23 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %19, !dbg !407
  store ptr %21, ptr %23, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 7, !395, !DIExpression(), !399)
  %24 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !401, !memtracer !302
    #dbg_value(ptr %24, !307, !DIExpression(), !404)
  %25 = add i32 %1, 8, !dbg !406
  %26 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %22, !dbg !407
  store ptr %24, ptr %26, align 8, !dbg !408, !tbaa !319, !memtracer !302
    #dbg_value(i32 8, !395, !DIExpression(), !399)
  store i32 %25, ptr @_ZL10g_sink_idx, align 4, !dbg !406, !tbaa !313
  ret void, !dbg !409
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable
define void @_Z11process_ptrPvS_(ptr noundef %0, ptr noundef %1) local_unnamed_addr #2 !dbg !2 {
    #dbg_value(ptr %0, !281, !DIExpression(), !410)
    #dbg_value(ptr %1, !282, !DIExpression(), !410)
  store ptr %0, ptr @_ZZ11process_ptrPvS_E6g_args, align 8, !dbg !411, !tbaa !319, !memtracer !412
  store ptr %1, ptr getelementptr inbounds nuw (i8, ptr @_ZZ11process_ptrPvS_E6g_args, i64 8), align 8, !dbg !413, !tbaa !319, !memtracer !414
  ret void, !dbg !415
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: write) uwtable
define void @_Z16test_param_storev() local_unnamed_addr #3 !dbg !416 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !420, !memtracer !302
    #dbg_value(ptr %1, !418, !DIExpression(), !421)
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !422, !memtracer !302
    #dbg_value(ptr %2, !419, !DIExpression(), !421)
  tail call void @_Z11process_ptrPvS_(ptr noundef %1, ptr noundef %2) #15, !dbg !423
    #dbg_value(ptr %1, !307, !DIExpression(), !424)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !426, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !426
  %5 = sext i32 %3 to i64, !dbg !427
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !427
  store ptr %1, ptr %6, align 8, !dbg !428, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !429)
  %7 = add nsw i32 %3, 2, !dbg !431
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !431, !tbaa !313
  %8 = sext i32 %4 to i64, !dbg !432
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !432
  store ptr %2, ptr %9, align 8, !dbg !433, !tbaa !319, !memtracer !302
  ret void, !dbg !434
}

; Function Attrs: mustprogress nofree noinline nounwind memory(readwrite, argmem: none) uwtable
define void @_Z15test_loop_storei(i32 noundef %0) local_unnamed_addr #4 !dbg !435 {
    #dbg_value(i32 %0, !437, !DIExpression(), !441)
    #dbg_value(ptr poison, !438, !DIExpression(), !441)
    #dbg_value(i32 0, !439, !DIExpression(), !442)
    #dbg_value(i32 0, !439, !DIExpression(), !442)
  %2 = icmp sgt i32 %0, 0, !dbg !443
  br i1 %2, label %3, label %8, !dbg !445

3:                                                ; preds = %1
  %4 = load i32, ptr @_ZL10g_sink_idx, align 4
  %5 = sext i32 %4 to i64, !dbg !445
  br label %9, !dbg !445

6:                                                ; preds = %9
  %7 = trunc nsw i64 %13 to i32, !dbg !446
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !446, !tbaa !313
  br label %8, !dbg !445

8:                                                ; preds = %6, %1
  ret void, !dbg !449

9:                                                ; preds = %9, %3
  %10 = phi i64 [ %5, %3 ], [ %13, %9 ]
  %11 = phi i32 [ 0, %3 ], [ %15, %9 ]
    #dbg_value(i32 %11, !439, !DIExpression(), !442)
  %12 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !450, !memtracer !302
    #dbg_value(ptr %12, !307, !DIExpression(), !451)
  %13 = add nsw i64 %10, 1, !dbg !446
  %14 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %10, !dbg !452
  store ptr %12, ptr %14, align 8, !dbg !453, !tbaa !319, !memtracer !302
  %15 = add nuw nsw i32 %11, 1, !dbg !454
    #dbg_value(i32 %15, !439, !DIExpression(), !442)
  %16 = icmp eq i32 %15, %0, !dbg !443
  br i1 %16, label %6, label %9, !dbg !445, !llvm.loop !455
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z22test_conditional_storei(i32 noundef %0) local_unnamed_addr #0 !dbg !458 {
    #dbg_value(i32 %0, !460, !DIExpression(), !464)
  %2 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !465, !memtracer !466
    #dbg_value(ptr %2, !461, !DIExpression(), !464)
  %3 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !467, !memtracer !468
    #dbg_value(ptr %3, !462, !DIExpression(), !464)
  %4 = icmp sgt i32 %0, 0, !dbg !469
  %5 = select i1 %4, ptr %2, ptr %3, !dbg !470
    #dbg_value(ptr %5, !463, !DIExpression(), !464)
    #dbg_value(ptr %5, !307, !DIExpression(), !471)
  %6 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !473, !tbaa !313
  %7 = add nsw i32 %6, 1, !dbg !473
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !473, !tbaa !313
  %8 = sext i32 %6 to i64, !dbg !474
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !474
  store ptr %5, ptr %9, align 8, !dbg !475, !tbaa !319, !memtracer !302
  ret void, !dbg !476
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define noundef ptr @_Z21test_switch_ptr_storei(i32 noundef %0) local_unnamed_addr #0 !dbg !477 {
    #dbg_value(i32 %0, !481, !DIExpression(), !483)
    #dbg_value(ptr null, !482, !DIExpression(), !483)
  %2 = icmp ult i32 %0, 3, !dbg !484
  br i1 %2, label %3, label %8, !dbg !484

3:                                                ; preds = %1
  %4 = zext nneg i32 %0 to i64, !dbg !484
  %5 = getelementptr inbounds nuw [3 x i64], ptr @switch.table._Z21test_switch_ptr_storei, i64 0, i64 %4, !dbg !484
  %6 = load i64, ptr %5, align 8, !dbg !484
  %7 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef %6) #14, !dbg !485
  br label %8, !dbg !487

8:                                                ; preds = %3, %1
  %9 = phi ptr [ null, %1 ], [ %7, %3 ], !dbg !485
    #dbg_value(ptr %9, !482, !DIExpression(), !483)
    #dbg_value(ptr %9, !307, !DIExpression(), !489)
  %10 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !487, !tbaa !313
  %11 = add nsw i32 %10, 1, !dbg !487
  store i32 %11, ptr @_ZL10g_sink_idx, align 4, !dbg !487, !tbaa !313
  %12 = sext i32 %10 to i64, !dbg !490
  %13 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %12, !dbg !490
  store ptr %9, ptr %13, align 8, !dbg !491, !tbaa !319, !memtracer !302
  ret ptr %9, !dbg !492
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z19test_template_storev() local_unnamed_addr #0 !dbg !493 {
    #dbg_value(i64 64, !497, !DIExpression(), !504)
  %1 = tail call noundef dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !506, !memtracer !302
    #dbg_value(ptr %1, !495, !DIExpression(), !507)
    #dbg_value(i64 32, !508, !DIExpression(), !515)
  %2 = tail call noundef dereferenceable_or_null(128) ptr @malloc(i64 noundef 128) #14, !dbg !517, !memtracer !302
    #dbg_value(ptr %2, !496, !DIExpression(), !507)
    #dbg_value(ptr %1, !307, !DIExpression(), !518)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !520, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !520
  %5 = sext i32 %3 to i64, !dbg !521
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !521
  store ptr %1, ptr %6, align 8, !dbg !522, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !523)
  %7 = add nsw i32 %3, 2, !dbg !525
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !525, !tbaa !313
  %8 = sext i32 %4 to i64, !dbg !526
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !526
  store ptr %2, ptr %9, align 8, !dbg !527, !tbaa !319, !memtracer !302
  ret void, !dbg !528
}

; Function Attrs: mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z17test_lambda_storev() local_unnamed_addr #0 !dbg !529 {
    #dbg_value(ptr poison, !541, !DIExpression(DW_OP_LLVM_fragment, 0, 8), !546)
    #dbg_value(ptr poison, !541, !DIExpression(DW_OP_LLVM_fragment, 0, 8), !548)
    #dbg_value(ptr undef, !541, !DIExpression(), !548)
    #dbg_value(i64 64, !544, !DIExpression(), !548)
  %1 = tail call noalias noundef dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !550, !memtracer !302
    #dbg_value(ptr %1, !539, !DIExpression(), !551)
    #dbg_value(ptr undef, !541, !DIExpression(), !546)
    #dbg_value(i64 128, !544, !DIExpression(), !546)
  %2 = tail call noalias noundef dereferenceable_or_null(128) ptr @malloc(i64 noundef 128) #14, !dbg !552, !memtracer !302
    #dbg_value(ptr %2, !540, !DIExpression(), !551)
    #dbg_value(ptr %1, !307, !DIExpression(), !553)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !555, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !555
  %5 = sext i32 %3 to i64, !dbg !556
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !556
  store ptr %1, ptr %6, align 8, !dbg !557, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !558)
  %7 = add nsw i32 %3, 2, !dbg !560
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !560, !tbaa !313
  %8 = sext i32 %4 to i64, !dbg !561
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !561
  store ptr %2, ptr %9, align 8, !dbg !562, !tbaa !319, !memtracer !302
  ret void, !dbg !563
}

; Function Attrs: mustprogress nofree noinline nounwind memory(inaccessiblemem: readwrite) uwtable
define noalias noundef ptr @_Z15recursive_alloci(i32 noundef %0) local_unnamed_addr #5 !dbg !564 {
    #dbg_value(i32 %0, !566, !DIExpression(), !568)
  %2 = icmp slt i32 %0, 1, !dbg !569
  br i1 %2, label %8, label %3, !dbg !569

3:                                                ; preds = %3, %1
  %4 = phi i32 [ %7, %3 ], [ %0, %1 ]
    #dbg_value(i32 %4, !566, !DIExpression(), !568)
  %5 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !571, !memtracer !572
    #dbg_value(ptr %5, !567, !DIExpression(), !568)
  %6 = icmp eq i32 %4, 1, !dbg !573
  %7 = add nsw i32 %4, -1, !dbg !575
    #dbg_value(i32 %7, !566, !DIExpression(), !568)
  br i1 %6, label %8, label %3, !dbg !573

8:                                                ; preds = %3, %1
  %9 = phi ptr [ null, %1 ], [ %5, %3 ], !dbg !568
  ret ptr %9, !dbg !576
}

; Function Attrs: mustprogress nofree noinline nounwind memory(readwrite, argmem: none) uwtable
define void @_Z20test_recursive_storev() local_unnamed_addr #4 !dbg !577 {
  %1 = tail call noundef ptr @_Z15recursive_alloci(i32 noundef 2) #15, !dbg !581
    #dbg_value(ptr %1, !579, !DIExpression(), !582)
  %2 = tail call noundef ptr @_Z15recursive_alloci(i32 noundef 3) #15, !dbg !583
    #dbg_value(ptr %2, !580, !DIExpression(), !582)
    #dbg_value(ptr %1, !307, !DIExpression(), !584)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !586, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !586
  %5 = sext i32 %3 to i64, !dbg !587
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !587
  store ptr %1, ptr %6, align 8, !dbg !588, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !589)
  %7 = add nsw i32 %3, 2, !dbg !591
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !591, !tbaa !313
  %8 = sext i32 %4 to i64, !dbg !592
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !592
  store ptr %2, ptr %9, align 8, !dbg !593, !tbaa !319, !memtracer !302
  ret void, !dbg !594
}

; Function Attrs: mustprogress noinline nounwind willreturn memory(readwrite, argmem: none) uwtable
define void @_Z25test_realloc_calloc_storev() local_unnamed_addr #6 !dbg !595 {
  %1 = tail call dereferenceable_or_null(64) ptr @malloc(i64 noundef 64) #14, !dbg !600, !memtracer !466
    #dbg_value(ptr %1, !597, !DIExpression(), !601)
  %2 = tail call dereferenceable_or_null(128) ptr @realloc(ptr noundef %1, i64 noundef 128) #16, !dbg !602, !memtracer !302
    #dbg_value(ptr %2, !598, !DIExpression(), !601)
  %3 = tail call dereferenceable_or_null(128) ptr @calloc(i64 noundef 16, i64 noundef 8) #17, !dbg !603, !memtracer !302
    #dbg_value(ptr %3, !599, !DIExpression(), !601)
    #dbg_value(ptr %2, !307, !DIExpression(), !604)
  %4 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !606, !tbaa !313
  %5 = add nsw i32 %4, 1, !dbg !606
  %6 = sext i32 %4 to i64, !dbg !607
  %7 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %6, !dbg !607
  store ptr %2, ptr %7, align 8, !dbg !608, !tbaa !319, !memtracer !302
    #dbg_value(ptr %3, !307, !DIExpression(), !609)
  %8 = add nsw i32 %4, 2, !dbg !611
  store i32 %8, ptr @_ZL10g_sink_idx, align 4, !dbg !611, !tbaa !313
  %9 = sext i32 %5 to i64, !dbg !612
  %10 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %9, !dbg !612
  store ptr %3, ptr %10, align 8, !dbg !613, !tbaa !319, !memtracer !302
  ret void, !dbg !614
}

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare !dbg !123 noalias noundef ptr @realloc(ptr allocptr noundef captures(none), i64 noundef) local_unnamed_addr #7

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite)
declare !dbg !109 noalias noundef ptr @calloc(i64 noundef, i64 noundef) local_unnamed_addr #8

; Function Attrs: mustprogress noinline uwtable
define void @_Z14test_new_storev() local_unnamed_addr #9 !dbg !615 {
  %1 = tail call noalias noundef nonnull dereferenceable(64) ptr @_Znwm(i64 noundef 64) #18, !dbg !624, !memtracer !302, !heapallocsite !619
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(64) %1, i8 0, i64 64, i1 false), !dbg !625
    #dbg_value(ptr %1, !617, !DIExpression(), !626)
  %2 = tail call noalias noundef nonnull dereferenceable(64) ptr @_Znwm(i64 noundef 64) #18, !dbg !627, !memtracer !302, !heapallocsite !619
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(64) %2, i8 0, i64 64, i1 false), !dbg !628
    #dbg_value(ptr %2, !623, !DIExpression(), !626)
    #dbg_value(ptr %1, !307, !DIExpression(), !629)
  %3 = load i32, ptr @_ZL10g_sink_idx, align 4, !dbg !631, !tbaa !313
  %4 = add nsw i32 %3, 1, !dbg !631
  %5 = sext i32 %3 to i64, !dbg !632
  %6 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %5, !dbg !632
  store ptr %1, ptr %6, align 8, !dbg !633, !tbaa !319, !memtracer !302
    #dbg_value(ptr %2, !307, !DIExpression(), !634)
  %7 = add nsw i32 %3, 2, !dbg !636
  store i32 %7, ptr @_ZL10g_sink_idx, align 4, !dbg !636, !tbaa !313
  %8 = sext i32 %4 to i64, !dbg !637
  %9 = getelementptr inbounds [64 x ptr], ptr @_ZL6g_sink, i64 0, i64 %8, !dbg !637
  store ptr %2, ptr %9, align 8, !dbg !638, !tbaa !319, !memtracer !302
  tail call void @_ZdlPvm(ptr noundef nonnull %1, i64 noundef 64) #19, !dbg !639
  tail call void @_ZdlPvm(ptr noundef nonnull %2, i64 noundef 64) #19, !dbg !640
  ret void, !dbg !641
}

; Function Attrs: nobuiltin allocsize(0)
declare noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #10

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #11

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPvm(ptr noundef, i64 noundef) local_unnamed_addr #12

; Function Attrs: mustprogress norecurse uwtable
define noundef i32 @main() local_unnamed_addr #13 !dbg !642 {
  tail call void @_Z21test_continuous_storev() #15, !dbg !643
  tail call void @_Z17test_struct_storev() #15, !dbg !644
  tail call void @_Z16test_array_storev() #15, !dbg !645
  tail call void @_Z16test_param_storev() #15, !dbg !646
  tail call void @_Z15test_loop_storei(i32 noundef 8) #15, !dbg !647
  tail call void @_Z22test_conditional_storei(i32 noundef 1) #15, !dbg !648
  tail call void @_Z22test_conditional_storei(i32 noundef -1) #15, !dbg !649
  %1 = tail call noundef ptr @_Z21test_switch_ptr_storei(i32 noundef 0) #15, !dbg !650
  %2 = tail call noundef ptr @_Z21test_switch_ptr_storei(i32 noundef 2) #15, !dbg !651
  tail call void @_Z19test_template_storev() #15, !dbg !652
  tail call void @_Z17test_lambda_storev() #15, !dbg !653
  tail call void @_Z20test_recursive_storev() #15, !dbg !654
  tail call void @_Z25test_realloc_calloc_storev() #15, !dbg !655
  tail call void @_Z14test_new_storev() #15, !dbg !656
  ret i32 0, !dbg !657
}

attributes #0 = { mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: none) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #3 = { mustprogress nofree noinline nounwind willreturn memory(readwrite, argmem: write) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #4 = { mustprogress nofree noinline nounwind memory(readwrite, argmem: none) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #5 = { mustprogress nofree noinline nounwind memory(inaccessiblemem: readwrite) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #6 = { mustprogress noinline nounwind willreturn memory(readwrite, argmem: none) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #7 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #8 = { mustprogress nofree nounwind willreturn allockind("alloc,zeroed") allocsize(0,1) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #9 = { mustprogress noinline uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #10 = { nobuiltin allocsize(0) "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #11 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #12 = { nobuiltin nounwind "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #13 = { mustprogress norecurse uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "reference-tracking"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fix-cortex-a53-835769,+fp-armv8,+neon,+v8a" }
attributes #14 = { allocsize(0) "reference-tracking"="true" }
attributes #15 = { "reference-tracking"="true" }
attributes #16 = { allocsize(1) "reference-tracking"="true" }
attributes #17 = { allocsize(0,1) "reference-tracking"="true" }
attributes #18 = { builtin allocsize(0) "reference-tracking"="true" }
attributes #19 = { builtin nounwind "reference-tracking"="true" }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!286, !287, !288, !289, !290, !291, !292, !293}
!llvm.ident = !{!294}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g_args", scope: !2, file: !3, line: 61, type: !283, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "process_ptr", linkageName: "_Z11process_ptrPvS_", scope: !3, file: !3, line: 60, type: !4, scopeLine: 60, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !280)
!3 = !DIFile(filename: "memtracer_str_to_stp.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "b6d4f3ebbcf376ca84d9427f2aaed688")
!4 = !DISubroutineType(types: !5)
!5 = !{null, !6, !6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!7 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !8, producer: "OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !9, globals: !15, imports: !23, splitDebugInlining: false, nameTableKind: None)
!8 = !DIFile(filename: "/root/mem_map/llvm_test/memtracer_str_to_stp.cpp", directory: "/root/mem_map/llvm_test", checksumkind: CSK_MD5, checksum: "b6d4f3ebbcf376ca84d9427f2aaed688")
!9 = !{!10, !11, !13}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_unsigned_char)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{!0, !16, !21}
!16 = !DIGlobalVariableExpression(var: !17, expr: !DIExpression())
!17 = distinct !DIGlobalVariable(name: "g_sink", linkageName: "_ZL6g_sink", scope: !7, file: !3, line: 4, type: !18, isLocal: true, isDefinition: true)
!18 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 4096, elements: !19)
!19 = !{!20}
!20 = !DISubrange(count: 64)
!21 = !DIGlobalVariableExpression(var: !22, expr: !DIExpression())
!22 = distinct !DIGlobalVariable(name: "g_sink_idx", linkageName: "_ZL10g_sink_idx", scope: !7, file: !3, line: 5, type: !14, isLocal: true, isDefinition: true)
!23 = !{!24, !31, !38, !45, !52, !56, !60, !64, !71, !76, !81, !85, !89, !94, !99, !103, !108, !114, !118, !122, !126, !130, !135, !139, !141, !145, !147, !156, !160, !164, !168, !173, !177, !179, !183, !190, !194, !198, !206, !208, !210, !212, !217, !221, !224, !226, !228, !231, !234, !236, !238, !241, !243, !245, !247, !249, !251, !253, !255, !257, !260, !262, !264, !266, !268, !270, !272, !274, !276, !278}
!24 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !27, file: !30, line: 106)
!25 = !DINamespace(name: "__h", scope: !26, exportSymbols: true)
!26 = !DINamespace(name: "std", scope: null)
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "div_t", file: !28, line: 67, baseType: !29)
!28 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/stdlib.h", directory: "/root", checksumkind: CSK_MD5, checksum: "5a72a9fe8603a6e9a660970416eead7b")
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !28, line: 67, size: 64, flags: DIFlagFwdDecl, identifier: "_ZTS5div_t")
!30 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/cstdlib", directory: "/root")
!31 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !32, file: !30, line: 107)
!32 = !DIDerivedType(tag: DW_TAG_typedef, name: "ldiv_t", file: !28, line: 68, baseType: !33)
!33 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !28, line: 68, size: 128, flags: DIFlagTypePassByValue, elements: !34, identifier: "_ZTS6ldiv_t")
!34 = !{!35, !37}
!35 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !33, file: !28, line: 68, baseType: !36, size: 64)
!36 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!37 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !33, file: !28, line: 68, baseType: !36, size: 64, offset: 64)
!38 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !39, file: !30, line: 108)
!39 = !DIDerivedType(tag: DW_TAG_typedef, name: "lldiv_t", file: !28, line: 69, baseType: !40)
!40 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !28, line: 69, size: 128, flags: DIFlagTypePassByValue, elements: !41, identifier: "_ZTS7lldiv_t")
!41 = !{!42, !44}
!42 = !DIDerivedType(tag: DW_TAG_member, name: "quot", scope: !40, file: !28, line: 69, baseType: !43, size: 64)
!43 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!44 = !DIDerivedType(tag: DW_TAG_member, name: "rem", scope: !40, file: !28, line: 69, baseType: !43, size: 64, offset: 64)
!45 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !46, file: !30, line: 109)
!46 = !DISubprogram(name: "atof", scope: !28, file: !28, line: 28, type: !47, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!47 = !DISubroutineType(types: !48)
!48 = !{!49, !50}
!49 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!50 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !51, size: 64)
!51 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !12)
!52 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !53, file: !30, line: 110)
!53 = !DISubprogram(name: "atoi", scope: !28, file: !28, line: 25, type: !54, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!54 = !DISubroutineType(types: !55)
!55 = !{!14, !50}
!56 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !57, file: !30, line: 111)
!57 = !DISubprogram(name: "atol", scope: !28, file: !28, line: 26, type: !58, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!58 = !DISubroutineType(types: !59)
!59 = !{!36, !50}
!60 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !61, file: !30, line: 112)
!61 = !DISubprogram(name: "atoll", scope: !28, file: !28, line: 27, type: !62, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!62 = !DISubroutineType(types: !63)
!63 = !{!43, !50}
!64 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !65, file: !30, line: 113)
!65 = !DISubprogram(name: "strtod", scope: !28, file: !28, line: 31, type: !66, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!66 = !DISubroutineType(types: !67)
!67 = !{!49, !68, !69}
!68 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !50)
!69 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !70)
!70 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!71 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !72, file: !30, line: 114)
!72 = !DISubprogram(name: "strtof", scope: !28, file: !28, line: 30, type: !73, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!73 = !DISubroutineType(types: !74)
!74 = !{!75, !68, !69}
!75 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!76 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !77, file: !30, line: 115)
!77 = !DISubprogram(name: "strtold", scope: !28, file: !28, line: 32, type: !78, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!78 = !DISubroutineType(types: !79)
!79 = !{!80, !68, !69}
!80 = !DIBasicType(name: "long double", size: 128, encoding: DW_ATE_float)
!81 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !82, file: !30, line: 116)
!82 = !DISubprogram(name: "strtol", scope: !28, file: !28, line: 34, type: !83, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!83 = !DISubroutineType(types: !84)
!84 = !{!36, !68, !69, !14}
!85 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !86, file: !30, line: 117)
!86 = !DISubprogram(name: "strtoll", scope: !28, file: !28, line: 36, type: !87, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!87 = !DISubroutineType(types: !88)
!88 = !{!43, !68, !69, !14}
!89 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !90, file: !30, line: 118)
!90 = !DISubprogram(name: "strtoul", scope: !28, file: !28, line: 35, type: !91, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!91 = !DISubroutineType(types: !92)
!92 = !{!93, !68, !69, !14}
!93 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!94 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !95, file: !30, line: 119)
!95 = !DISubprogram(name: "strtoull", scope: !28, file: !28, line: 37, type: !96, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!96 = !DISubroutineType(types: !97)
!97 = !{!98, !68, !69, !14}
!98 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!99 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !100, file: !30, line: 120)
!100 = !DISubprogram(name: "rand", scope: !28, file: !28, line: 39, type: !101, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!101 = !DISubroutineType(types: !102)
!102 = !{!14}
!103 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !104, file: !30, line: 121)
!104 = !DISubprogram(name: "srand", scope: !28, file: !28, line: 40, type: !105, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !107}
!107 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!108 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !109, file: !30, line: 122)
!109 = !DISubprogram(name: "calloc", scope: !28, file: !28, line: 43, type: !110, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!110 = !DISubroutineType(types: !111)
!111 = !{!6, !112, !112}
!112 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !113, line: 58, baseType: !93)
!113 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/bits/alltypes.h", directory: "/root", checksumkind: CSK_MD5, checksum: "1071e718a958c5a168e8e771d1f30b89")
!114 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !115, file: !30, line: 123)
!115 = !DISubprogram(name: "free", scope: !28, file: !28, line: 45, type: !116, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!116 = !DISubroutineType(types: !117)
!117 = !{null, !6}
!118 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !119, file: !30, line: 124)
!119 = !DISubprogram(name: "malloc", scope: !28, file: !28, line: 42, type: !120, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!120 = !DISubroutineType(types: !121)
!121 = !{!6, !112}
!122 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !123, file: !30, line: 125)
!123 = !DISubprogram(name: "realloc", scope: !28, file: !28, line: 44, type: !124, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!124 = !DISubroutineType(types: !125)
!125 = !{!6, !6, !112}
!126 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !127, file: !30, line: 126)
!127 = !DISubprogram(name: "abort", scope: !28, file: !28, line: 48, type: !128, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!128 = !DISubroutineType(types: !129)
!129 = !{null}
!130 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !131, file: !30, line: 127)
!131 = !DISubprogram(name: "atexit", scope: !28, file: !28, line: 50, type: !132, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!132 = !DISubroutineType(types: !133)
!133 = !{!14, !134}
!134 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !128, size: 64)
!135 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !136, file: !30, line: 128)
!136 = !DISubprogram(name: "exit", scope: !28, file: !28, line: 51, type: !137, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!137 = !DISubroutineType(types: !138)
!138 = !{null, !14}
!139 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !140, file: !30, line: 129)
!140 = !DISubprogram(name: "_Exit", scope: !28, file: !28, line: 52, type: !137, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!141 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !142, file: !30, line: 130)
!142 = !DISubprogram(name: "getenv", scope: !28, file: !28, line: 56, type: !143, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!143 = !DISubroutineType(types: !144)
!144 = !{!11, !50}
!145 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !146, file: !30, line: 131)
!146 = !DISubprogram(name: "system", scope: !28, file: !28, line: 58, type: !54, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!147 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !148, file: !30, line: 132)
!148 = !DISubprogram(name: "bsearch", scope: !28, file: !28, line: 60, type: !149, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!149 = !DISubroutineType(types: !150)
!150 = !{!6, !151, !151, !112, !112, !153}
!151 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !152, size: 64)
!152 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!153 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !154, size: 64)
!154 = !DISubroutineType(types: !155)
!155 = !{!14, !151, !151}
!156 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !157, file: !30, line: 133)
!157 = !DISubprogram(name: "qsort", scope: !28, file: !28, line: 61, type: !158, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!158 = !DISubroutineType(types: !159)
!159 = !{null, !6, !112, !112, !153}
!160 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !161, file: !30, line: 135)
!161 = !DISubprogram(name: "labs", scope: !28, file: !28, line: 64, type: !162, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!162 = !DISubroutineType(types: !163)
!163 = !{!36, !36}
!164 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !165, file: !30, line: 136)
!165 = !DISubprogram(name: "llabs", scope: !28, file: !28, line: 65, type: !166, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!166 = !DISubroutineType(types: !167)
!167 = !{!43, !43}
!168 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !169, file: !30, line: 137)
!169 = !DISubprogram(name: "div", linkageName: "_Z3divB8ne210108xx", scope: !170, file: !170, line: 128, type: !171, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!170 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/stdlib.h", directory: "/root")
!171 = !DISubroutineType(types: !172)
!172 = !{!39, !43, !43}
!173 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !174, file: !30, line: 138)
!174 = !DISubprogram(name: "ldiv", scope: !28, file: !28, line: 72, type: !175, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!175 = !DISubroutineType(types: !176)
!176 = !{!32, !36, !36}
!177 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !178, file: !30, line: 139)
!178 = !DISubprogram(name: "lldiv", scope: !28, file: !28, line: 73, type: !171, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!179 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !180, file: !30, line: 140)
!180 = !DISubprogram(name: "mblen", scope: !28, file: !28, line: 75, type: !181, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!181 = !DISubroutineType(types: !182)
!182 = !{!14, !50, !112}
!183 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !184, file: !30, line: 142)
!184 = !DISubprogram(name: "mbtowc", scope: !28, file: !28, line: 76, type: !185, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!185 = !DISubroutineType(types: !186)
!186 = !{!14, !187, !68, !112}
!187 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !188)
!188 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !189, size: 64)
!189 = !DIBasicType(name: "wchar_t", size: 32, encoding: DW_ATE_unsigned)
!190 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !191, file: !30, line: 143)
!191 = !DISubprogram(name: "wctomb", scope: !28, file: !28, line: 77, type: !192, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!192 = !DISubroutineType(types: !193)
!193 = !{!14, !11, !189}
!194 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !195, file: !30, line: 144)
!195 = !DISubprogram(name: "mbstowcs", scope: !28, file: !28, line: 78, type: !196, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!196 = !DISubroutineType(types: !197)
!197 = !{!112, !187, !68, !112}
!198 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !199, file: !30, line: 145)
!199 = !DISubprogram(name: "wcstombs", scope: !28, file: !28, line: 79, type: !200, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!200 = !DISubroutineType(types: !201)
!201 = !{!112, !202, !203, !112}
!202 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !11)
!203 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !204)
!204 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !205, size: 64)
!205 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !189)
!206 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !207, file: !30, line: 148)
!207 = !DISubprogram(name: "at_quick_exit", scope: !28, file: !28, line: 53, type: !132, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!208 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !209, file: !30, line: 149)
!209 = !DISubprogram(name: "quick_exit", scope: !28, file: !28, line: 54, type: !137, flags: DIFlagPrototyped | DIFlagNoReturn, spFlags: DISPFlagOptimized)
!210 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !211, file: !30, line: 152)
!211 = !DISubprogram(name: "aligned_alloc", scope: !28, file: !28, line: 46, type: !110, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!212 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !213, file: !216, line: 22)
!213 = !DIDerivedType(tag: DW_TAG_typedef, name: "max_align_t", file: !214, line: 24, baseType: !215)
!214 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/lib/clang/21/include/__stddef_max_align_t.h", directory: "/root", checksumkind: CSK_MD5, checksum: "3c0a2f19d136d39aa835c737c7105def")
!215 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !214, line: 19, size: 256, flags: DIFlagFwdDecl, identifier: "_ZTS11max_align_t")
!216 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/__cstddef/max_align_t.h", directory: "/root")
!217 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !218, file: !220, line: 158)
!218 = !DIDerivedType(tag: DW_TAG_typedef, name: "int8_t", file: !113, line: 104, baseType: !219)
!219 = !DIBasicType(name: "signed char", size: 8, encoding: DW_ATE_signed_char)
!220 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../include/libcxx-ohos/include/c++/v1/cstdint", directory: "/root")
!221 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !222, file: !220, line: 159)
!222 = !DIDerivedType(tag: DW_TAG_typedef, name: "int16_t", file: !113, line: 109, baseType: !223)
!223 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!224 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !225, file: !220, line: 160)
!225 = !DIDerivedType(tag: DW_TAG_typedef, name: "int32_t", file: !113, line: 114, baseType: !14)
!226 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !227, file: !220, line: 161)
!227 = !DIDerivedType(tag: DW_TAG_typedef, name: "int64_t", file: !113, line: 119, baseType: !36)
!228 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !229, file: !220, line: 163)
!229 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint8_t", file: !113, line: 129, baseType: !230)
!230 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!231 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !232, file: !220, line: 164)
!232 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint16_t", file: !113, line: 134, baseType: !233)
!233 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!234 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !235, file: !220, line: 165)
!235 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !113, line: 139, baseType: !107)
!236 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !237, file: !220, line: 166)
!237 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint64_t", file: !113, line: 144, baseType: !93)
!238 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !239, file: !220, line: 168)
!239 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least8_t", file: !240, line: 25, baseType: !218)
!240 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/stdint.h", directory: "/root", checksumkind: CSK_MD5, checksum: "19b17d487ee68139328911f286d556b7")
!241 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !242, file: !220, line: 169)
!242 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least16_t", file: !240, line: 26, baseType: !222)
!243 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !244, file: !220, line: 170)
!244 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least32_t", file: !240, line: 27, baseType: !225)
!245 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !246, file: !220, line: 171)
!246 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_least64_t", file: !240, line: 28, baseType: !227)
!247 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !248, file: !220, line: 173)
!248 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least8_t", file: !240, line: 33, baseType: !229)
!249 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !250, file: !220, line: 174)
!250 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least16_t", file: !240, line: 34, baseType: !232)
!251 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !252, file: !220, line: 175)
!252 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least32_t", file: !240, line: 35, baseType: !235)
!253 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !254, file: !220, line: 176)
!254 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_least64_t", file: !240, line: 36, baseType: !237)
!255 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !256, file: !220, line: 178)
!256 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast8_t", file: !240, line: 22, baseType: !218)
!257 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !258, file: !220, line: 179)
!258 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast16_t", file: !259, line: 1, baseType: !225)
!259 = !DIFile(filename: "ohos_llvm_21/out/llvm-install/bin/../../sysroot/aarch64-linux-ohos/usr/include/bits/stdint.h", directory: "/root", checksumkind: CSK_MD5, checksum: "45f4ec4fcef0a8b922beea40b4d26d92")
!260 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !261, file: !220, line: 180)
!261 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast32_t", file: !259, line: 2, baseType: !225)
!262 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !263, file: !220, line: 181)
!263 = !DIDerivedType(tag: DW_TAG_typedef, name: "int_fast64_t", file: !240, line: 23, baseType: !227)
!264 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !265, file: !220, line: 183)
!265 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast8_t", file: !240, line: 30, baseType: !229)
!266 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !267, file: !220, line: 184)
!267 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast16_t", file: !259, line: 3, baseType: !235)
!268 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !269, file: !220, line: 185)
!269 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast32_t", file: !259, line: 4, baseType: !235)
!270 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !271, file: !220, line: 186)
!271 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint_fast64_t", file: !240, line: 31, baseType: !237)
!272 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !273, file: !220, line: 188)
!273 = !DIDerivedType(tag: DW_TAG_typedef, name: "intptr_t", file: !113, line: 78, baseType: !36)
!274 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !275, file: !220, line: 189)
!275 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintptr_t", file: !113, line: 63, baseType: !93)
!276 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !277, file: !220, line: 191)
!277 = !DIDerivedType(tag: DW_TAG_typedef, name: "intmax_t", file: !113, line: 124, baseType: !36)
!278 = !DIImportedEntity(tag: DW_TAG_imported_declaration, scope: !25, entity: !279, file: !220, line: 192)
!279 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintmax_t", file: !113, line: 154, baseType: !93)
!280 = !{!281, !282}
!281 = !DILocalVariable(name: "p1", arg: 1, scope: !2, file: !3, line: 60, type: !6)
!282 = !DILocalVariable(name: "p2", arg: 2, scope: !2, file: !3, line: 60, type: !6)
!283 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 128, elements: !284)
!284 = !{!285}
!285 = !DISubrange(count: 2)
!286 = !{i32 7, !"Dwarf Version", i32 5}
!287 = !{i32 7, !"ReferenceTracking", i32 1}
!288 = !{i32 2, !"Debug Info Version", i32 3}
!289 = !{i32 1, !"wchar_size", i32 4}
!290 = !{i32 8, !"PIC Level", i32 2}
!291 = !{i32 7, !"uwtable", i32 2}
!292 = !{i32 7, !"frame-pointer", i32 1}
!293 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!294 = !{!"OHOS (dev) clang version 21.1.8 (llvm-project 9050642cf6411e79e64c53ff95311e35d04a1356)"}
!295 = distinct !DISubprogram(name: "test_continuous_store", linkageName: "_Z21test_continuous_storev", scope: !3, file: !3, line: 11, type: !128, scopeLine: 11, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !296)
!296 = !{!297, !298, !299, !300}
!297 = !DILocalVariable(name: "p1", scope: !295, file: !3, line: 12, type: !6)
!298 = !DILocalVariable(name: "p2", scope: !295, file: !3, line: 13, type: !6)
!299 = !DILocalVariable(name: "p3", scope: !295, file: !3, line: 14, type: !6)
!300 = !DILocalVariable(name: "p4", scope: !295, file: !3, line: 15, type: !6)
!301 = !DILocation(line: 12, column: 14, scope: !295)
!302 = !{!"g_sink[]", !"void*"}
!303 = !DILocation(line: 0, scope: !295)
!304 = !DILocation(line: 13, column: 14, scope: !295)
!305 = !DILocation(line: 14, column: 14, scope: !295)
!306 = !DILocation(line: 15, column: 14, scope: !295)
!307 = !DILocalVariable(name: "p", arg: 1, scope: !308, file: !3, line: 7, type: !6)
!308 = distinct !DISubprogram(name: "save_ptr", linkageName: "_ZL8save_ptrPv", scope: !3, file: !3, line: 7, type: !116, scopeLine: 7, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !309)
!309 = !{!307}
!310 = !DILocation(line: 0, scope: !308, inlinedAt: !311)
!311 = distinct !DILocation(line: 16, column: 3, scope: !295)
!312 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !311)
!313 = !{!314, !314, i64 0}
!314 = !{!"int", !315, i64 0}
!315 = !{!"omnipotent char", !316, i64 0}
!316 = !{!"Simple C++ TBAA"}
!317 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !311)
!318 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !311)
!319 = !{!320, !320, i64 0}
!320 = !{!"any pointer", !315, i64 0}
!321 = !DILocation(line: 0, scope: !308, inlinedAt: !322)
!322 = distinct !DILocation(line: 17, column: 3, scope: !295)
!323 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !322)
!324 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !322)
!325 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !322)
!326 = !DILocation(line: 0, scope: !308, inlinedAt: !327)
!327 = distinct !DILocation(line: 18, column: 3, scope: !295)
!328 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !327)
!329 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !327)
!330 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !327)
!331 = !DILocation(line: 0, scope: !308, inlinedAt: !332)
!332 = distinct !DILocation(line: 19, column: 3, scope: !295)
!333 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !332)
!334 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !332)
!335 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !332)
!336 = !DILocation(line: 20, column: 1, scope: !295)
!337 = distinct !DISubprogram(name: "test_struct_store", linkageName: "_Z17test_struct_storev", scope: !3, file: !3, line: 34, type: !128, scopeLine: 34, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !338)
!338 = !{!339, !344}
!339 = !DILocalVariable(name: "pair", scope: !337, file: !3, line: 35, type: !340)
!340 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "PointerPair", file: !3, line: 22, size: 128, flags: DIFlagTypePassByValue, elements: !341, identifier: "_ZTS11PointerPair")
!341 = !{!342, !343}
!342 = !DIDerivedType(tag: DW_TAG_member, name: "ptr1", scope: !340, file: !3, line: 23, baseType: !6, size: 64)
!343 = !DIDerivedType(tag: DW_TAG_member, name: "ptr2", scope: !340, file: !3, line: 24, baseType: !6, size: 64, offset: 64)
!344 = !DILocalVariable(name: "quad", scope: !337, file: !3, line: 41, type: !345)
!345 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "PointerQuad", file: !3, line: 27, size: 256, flags: DIFlagTypePassByValue, elements: !346, identifier: "_ZTS11PointerQuad")
!346 = !{!347, !348, !349, !350}
!347 = !DIDerivedType(tag: DW_TAG_member, name: "p1", scope: !345, file: !3, line: 28, baseType: !6, size: 64)
!348 = !DIDerivedType(tag: DW_TAG_member, name: "p2", scope: !345, file: !3, line: 29, baseType: !6, size: 64, offset: 64)
!349 = !DIDerivedType(tag: DW_TAG_member, name: "p3", scope: !345, file: !3, line: 30, baseType: !6, size: 64, offset: 128)
!350 = !DIDerivedType(tag: DW_TAG_member, name: "p4", scope: !345, file: !3, line: 31, baseType: !6, size: 64, offset: 192)
!351 = !DILocation(line: 36, column: 15, scope: !337)
!352 = !DILocation(line: 0, scope: !337)
!353 = !DILocation(line: 37, column: 15, scope: !337)
!354 = !DILocation(line: 0, scope: !308, inlinedAt: !355)
!355 = distinct !DILocation(line: 38, column: 3, scope: !337)
!356 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !355)
!357 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !355)
!358 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !355)
!359 = !DILocation(line: 0, scope: !308, inlinedAt: !360)
!360 = distinct !DILocation(line: 39, column: 3, scope: !337)
!361 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !360)
!362 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !360)
!363 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !360)
!364 = !DILocation(line: 42, column: 13, scope: !337)
!365 = !DILocation(line: 43, column: 13, scope: !337)
!366 = !DILocation(line: 44, column: 13, scope: !337)
!367 = !DILocation(line: 45, column: 13, scope: !337)
!368 = !DILocation(line: 0, scope: !308, inlinedAt: !369)
!369 = distinct !DILocation(line: 46, column: 3, scope: !337)
!370 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !369)
!371 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !369)
!372 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !369)
!373 = !DILocation(line: 0, scope: !308, inlinedAt: !374)
!374 = distinct !DILocation(line: 47, column: 3, scope: !337)
!375 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !374)
!376 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !374)
!377 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !374)
!378 = !DILocation(line: 0, scope: !308, inlinedAt: !379)
!379 = distinct !DILocation(line: 48, column: 3, scope: !337)
!380 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !379)
!381 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !379)
!382 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !379)
!383 = !DILocation(line: 0, scope: !308, inlinedAt: !384)
!384 = distinct !DILocation(line: 49, column: 3, scope: !337)
!385 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !384)
!386 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !384)
!387 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !384)
!388 = !DILocation(line: 50, column: 1, scope: !337)
!389 = distinct !DISubprogram(name: "test_array_store", linkageName: "_Z16test_array_storev", scope: !3, file: !3, line: 52, type: !128, scopeLine: 52, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !390)
!390 = !{!391, !395}
!391 = !DILocalVariable(name: "ptrs", scope: !389, file: !3, line: 53, type: !392)
!392 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 512, elements: !393)
!393 = !{!394}
!394 = !DISubrange(count: 8)
!395 = !DILocalVariable(name: "i", scope: !396, file: !3, line: 54, type: !14)
!396 = distinct !DILexicalBlock(scope: !389, file: !3, line: 54, column: 3)
!397 = distinct !DIAssignID()
!398 = !DILocation(line: 0, scope: !389)
!399 = !DILocation(line: 0, scope: !396)
!400 = !DILocation(line: 54, column: 3, scope: !396)
!401 = !DILocation(line: 55, column: 15, scope: !402)
!402 = distinct !DILexicalBlock(scope: !403, file: !3, line: 54, column: 31)
!403 = distinct !DILexicalBlock(scope: !396, file: !3, line: 54, column: 3)
!404 = !DILocation(line: 0, scope: !308, inlinedAt: !405)
!405 = distinct !DILocation(line: 56, column: 5, scope: !402)
!406 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !405)
!407 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !405)
!408 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !405)
!409 = !DILocation(line: 58, column: 1, scope: !389)
!410 = !DILocation(line: 0, scope: !2)
!411 = !DILocation(line: 62, column: 13, scope: !2)
!412 = !{!"g_args", !"void*[]"}
!413 = !DILocation(line: 63, column: 13, scope: !2)
!414 = !{!"g_args[]", !"void*"}
!415 = !DILocation(line: 64, column: 1, scope: !2)
!416 = distinct !DISubprogram(name: "test_param_store", linkageName: "_Z16test_param_storev", scope: !3, file: !3, line: 66, type: !128, scopeLine: 66, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !417)
!417 = !{!418, !419}
!418 = !DILocalVariable(name: "p1", scope: !416, file: !3, line: 67, type: !6)
!419 = !DILocalVariable(name: "p2", scope: !416, file: !3, line: 68, type: !6)
!420 = !DILocation(line: 67, column: 14, scope: !416)
!421 = !DILocation(line: 0, scope: !416)
!422 = !DILocation(line: 68, column: 14, scope: !416)
!423 = !DILocation(line: 69, column: 3, scope: !416)
!424 = !DILocation(line: 0, scope: !308, inlinedAt: !425)
!425 = distinct !DILocation(line: 70, column: 3, scope: !416)
!426 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !425)
!427 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !425)
!428 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !425)
!429 = !DILocation(line: 0, scope: !308, inlinedAt: !430)
!430 = distinct !DILocation(line: 71, column: 3, scope: !416)
!431 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !430)
!432 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !430)
!433 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !430)
!434 = !DILocation(line: 72, column: 1, scope: !416)
!435 = distinct !DISubprogram(name: "test_loop_store", linkageName: "_Z15test_loop_storei", scope: !3, file: !3, line: 74, type: !137, scopeLine: 74, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !436)
!436 = !{!437, !438, !439}
!437 = !DILocalVariable(name: "count", arg: 1, scope: !435, file: !3, line: 74, type: !14)
!438 = !DILocalVariable(name: "buffers", scope: !435, file: !3, line: 75, type: !10)
!439 = !DILocalVariable(name: "i", scope: !440, file: !3, line: 76, type: !14)
!440 = distinct !DILexicalBlock(scope: !435, file: !3, line: 76, column: 3)
!441 = !DILocation(line: 0, scope: !435)
!442 = !DILocation(line: 0, scope: !440)
!443 = !DILocation(line: 76, column: 21, scope: !444)
!444 = distinct !DILexicalBlock(scope: !440, file: !3, line: 76, column: 3)
!445 = !DILocation(line: 76, column: 3, scope: !440)
!446 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !447)
!447 = distinct !DILocation(line: 78, column: 5, scope: !448)
!448 = distinct !DILexicalBlock(scope: !444, file: !3, line: 76, column: 35)
!449 = !DILocation(line: 81, column: 1, scope: !435)
!450 = !DILocation(line: 77, column: 18, scope: !448)
!451 = !DILocation(line: 0, scope: !308, inlinedAt: !447)
!452 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !447)
!453 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !447)
!454 = !DILocation(line: 76, column: 30, scope: !444)
!455 = distinct !{!455, !445, !456, !457}
!456 = !DILocation(line: 79, column: 3, scope: !440)
!457 = !{!"llvm.loop.mustprogress"}
!458 = distinct !DISubprogram(name: "test_conditional_store", linkageName: "_Z22test_conditional_storei", scope: !3, file: !3, line: 83, type: !137, scopeLine: 83, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !459)
!459 = !{!460, !461, !462, !463}
!460 = !DILocalVariable(name: "cond", arg: 1, scope: !458, file: !3, line: 83, type: !14)
!461 = !DILocalVariable(name: "p1", scope: !458, file: !3, line: 84, type: !6)
!462 = !DILocalVariable(name: "p2", scope: !458, file: !3, line: 85, type: !6)
!463 = !DILocalVariable(name: "result", scope: !458, file: !3, line: 86, type: !6)
!464 = !DILocation(line: 0, scope: !458)
!465 = !DILocation(line: 84, column: 14, scope: !458)
!466 = !{!"p1", !"void*"}
!467 = !DILocation(line: 85, column: 14, scope: !458)
!468 = !{!"p2", !"void*"}
!469 = !DILocation(line: 86, column: 23, scope: !458)
!470 = !DILocation(line: 86, column: 18, scope: !458)
!471 = !DILocation(line: 0, scope: !308, inlinedAt: !472)
!472 = distinct !DILocation(line: 87, column: 3, scope: !458)
!473 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !472)
!474 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !472)
!475 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !472)
!476 = !DILocation(line: 88, column: 1, scope: !458)
!477 = distinct !DISubprogram(name: "test_switch_ptr_store", linkageName: "_Z21test_switch_ptr_storei", scope: !3, file: !3, line: 90, type: !478, scopeLine: 90, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !480)
!478 = !DISubroutineType(types: !479)
!479 = !{!6, !14}
!480 = !{!481, !482}
!481 = !DILocalVariable(name: "condition", arg: 1, scope: !477, file: !3, line: 90, type: !14)
!482 = !DILocalVariable(name: "result", scope: !477, file: !3, line: 91, type: !6)
!483 = !DILocation(line: 0, scope: !477)
!484 = !DILocation(line: 92, column: 3, scope: !477)
!485 = !DILocation(line: 0, scope: !486)
!486 = distinct !DILexicalBlock(scope: !477, file: !3, line: 92, column: 22)
!487 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !488)
!488 = distinct !DILocation(line: 106, column: 3, scope: !477)
!489 = !DILocation(line: 0, scope: !308, inlinedAt: !488)
!490 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !488)
!491 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !488)
!492 = !DILocation(line: 107, column: 3, scope: !477)
!493 = distinct !DISubprogram(name: "test_template_store", linkageName: "_Z19test_template_storev", scope: !3, file: !3, line: 115, type: !128, scopeLine: 115, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !494)
!494 = !{!495, !496}
!495 = !DILocalVariable(name: "p1", scope: !493, file: !3, line: 116, type: !11)
!496 = !DILocalVariable(name: "p2", scope: !493, file: !3, line: 117, type: !13)
!497 = !DILocalVariable(name: "count", arg: 1, scope: !498, file: !3, line: 111, type: !93)
!498 = distinct !DISubprogram(name: "allocate_array<char>", linkageName: "_Z14allocate_arrayIcEPT_m", scope: !3, file: !3, line: 111, type: !499, scopeLine: 111, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, templateParams: !502, retainedNodes: !501)
!499 = !DISubroutineType(types: !500)
!500 = !{!11, !93}
!501 = !{!497}
!502 = !{!503}
!503 = !DITemplateTypeParameter(name: "T", type: !12)
!504 = !DILocation(line: 0, scope: !498, inlinedAt: !505)
!505 = distinct !DILocation(line: 116, column: 14, scope: !493)
!506 = !DILocation(line: 112, column: 14, scope: !498, inlinedAt: !505)
!507 = !DILocation(line: 0, scope: !493)
!508 = !DILocalVariable(name: "count", arg: 1, scope: !509, file: !3, line: 111, type: !93)
!509 = distinct !DISubprogram(name: "allocate_array<int>", linkageName: "_Z14allocate_arrayIiEPT_m", scope: !3, file: !3, line: 111, type: !510, scopeLine: 111, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, templateParams: !513, retainedNodes: !512)
!510 = !DISubroutineType(types: !511)
!511 = !{!13, !93}
!512 = !{!508}
!513 = !{!514}
!514 = !DITemplateTypeParameter(name: "T", type: !14)
!515 = !DILocation(line: 0, scope: !509, inlinedAt: !516)
!516 = distinct !DILocation(line: 117, column: 13, scope: !493)
!517 = !DILocation(line: 112, column: 14, scope: !509, inlinedAt: !516)
!518 = !DILocation(line: 0, scope: !308, inlinedAt: !519)
!519 = distinct !DILocation(line: 118, column: 3, scope: !493)
!520 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !519)
!521 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !519)
!522 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !519)
!523 = !DILocation(line: 0, scope: !308, inlinedAt: !524)
!524 = distinct !DILocation(line: 119, column: 3, scope: !493)
!525 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !524)
!526 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !524)
!527 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !524)
!528 = !DILocation(line: 120, column: 1, scope: !493)
!529 = distinct !DISubprogram(name: "test_lambda_store", linkageName: "_Z17test_lambda_storev", scope: !3, file: !3, line: 122, type: !128, scopeLine: 122, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !530)
!530 = !{!531, !539, !540}
!531 = !DILocalVariable(name: "alloc", scope: !529, file: !3, line: 123, type: !532)
!532 = distinct !DICompositeType(tag: DW_TAG_class_type, scope: !529, file: !3, line: 123, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !533)
!533 = !{!534}
!534 = !DISubprogram(name: "operator()", scope: !532, file: !3, line: 123, type: !535, scopeLine: 123, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagOptimized)
!535 = !DISubroutineType(types: !536)
!536 = !{!6, !537, !93}
!537 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !538, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!538 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !532)
!539 = !DILocalVariable(name: "p1", scope: !529, file: !3, line: 124, type: !6)
!540 = !DILocalVariable(name: "p2", scope: !529, file: !3, line: 125, type: !6)
!541 = !DILocalVariable(name: "this", arg: 1, scope: !542, type: !545, flags: DIFlagArtificial | DIFlagObjectPointer)
!542 = distinct !DISubprogram(name: "operator()", linkageName: "_ZZ17test_lambda_storevENK3$_0clEm", scope: !532, file: !3, line: 123, type: !535, scopeLine: 123, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !7, declaration: !534, retainedNodes: !543)
!543 = !{!541, !544}
!544 = !DILocalVariable(name: "s", arg: 2, scope: !542, file: !3, line: 123, type: !93)
!545 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !538, size: 64)
!546 = !DILocation(line: 0, scope: !542, inlinedAt: !547)
!547 = distinct !DILocation(line: 125, column: 14, scope: !529)
!548 = !DILocation(line: 0, scope: !542, inlinedAt: !549)
!549 = distinct !DILocation(line: 124, column: 14, scope: !529)
!550 = !DILocation(line: 123, column: 54, scope: !542, inlinedAt: !549)
!551 = !DILocation(line: 0, scope: !529)
!552 = !DILocation(line: 123, column: 54, scope: !542, inlinedAt: !547)
!553 = !DILocation(line: 0, scope: !308, inlinedAt: !554)
!554 = distinct !DILocation(line: 126, column: 3, scope: !529)
!555 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !554)
!556 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !554)
!557 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !554)
!558 = !DILocation(line: 0, scope: !308, inlinedAt: !559)
!559 = distinct !DILocation(line: 127, column: 3, scope: !529)
!560 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !559)
!561 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !559)
!562 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !559)
!563 = !DILocation(line: 128, column: 1, scope: !529)
!564 = distinct !DISubprogram(name: "recursive_alloc", linkageName: "_Z15recursive_alloci", scope: !3, file: !3, line: 130, type: !478, scopeLine: 130, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !565)
!565 = !{!566, !567}
!566 = !DILocalVariable(name: "depth", arg: 1, scope: !564, file: !3, line: 130, type: !14)
!567 = !DILocalVariable(name: "p", scope: !564, file: !3, line: 133, type: !6)
!568 = !DILocation(line: 0, scope: !564)
!569 = !DILocation(line: 131, column: 13, scope: !570)
!570 = distinct !DILexicalBlock(scope: !564, file: !3, line: 131, column: 7)
!571 = !DILocation(line: 133, column: 13, scope: !564)
!572 = !{!"p", !"void*"}
!573 = !DILocation(line: 134, column: 13, scope: !574)
!574 = distinct !DILexicalBlock(scope: !564, file: !3, line: 134, column: 7)
!575 = !DILocation(line: 136, column: 32, scope: !564)
!576 = !DILocation(line: 137, column: 1, scope: !564)
!577 = distinct !DISubprogram(name: "test_recursive_store", linkageName: "_Z20test_recursive_storev", scope: !3, file: !3, line: 139, type: !128, scopeLine: 139, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !578)
!578 = !{!579, !580}
!579 = !DILocalVariable(name: "p1", scope: !577, file: !3, line: 140, type: !6)
!580 = !DILocalVariable(name: "p2", scope: !577, file: !3, line: 141, type: !6)
!581 = !DILocation(line: 140, column: 14, scope: !577)
!582 = !DILocation(line: 0, scope: !577)
!583 = !DILocation(line: 141, column: 14, scope: !577)
!584 = !DILocation(line: 0, scope: !308, inlinedAt: !585)
!585 = distinct !DILocation(line: 142, column: 3, scope: !577)
!586 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !585)
!587 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !585)
!588 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !585)
!589 = !DILocation(line: 0, scope: !308, inlinedAt: !590)
!590 = distinct !DILocation(line: 143, column: 3, scope: !577)
!591 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !590)
!592 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !590)
!593 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !590)
!594 = !DILocation(line: 144, column: 1, scope: !577)
!595 = distinct !DISubprogram(name: "test_realloc_calloc_store", linkageName: "_Z25test_realloc_calloc_storev", scope: !3, file: !3, line: 146, type: !128, scopeLine: 146, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !596)
!596 = !{!597, !598, !599}
!597 = !DILocalVariable(name: "p1", scope: !595, file: !3, line: 147, type: !6)
!598 = !DILocalVariable(name: "p2", scope: !595, file: !3, line: 148, type: !6)
!599 = !DILocalVariable(name: "p3", scope: !595, file: !3, line: 149, type: !6)
!600 = !DILocation(line: 147, column: 14, scope: !595)
!601 = !DILocation(line: 0, scope: !595)
!602 = !DILocation(line: 148, column: 14, scope: !595)
!603 = !DILocation(line: 149, column: 14, scope: !595)
!604 = !DILocation(line: 0, scope: !308, inlinedAt: !605)
!605 = distinct !DILocation(line: 150, column: 3, scope: !595)
!606 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !605)
!607 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !605)
!608 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !605)
!609 = !DILocation(line: 0, scope: !308, inlinedAt: !610)
!610 = distinct !DILocation(line: 151, column: 3, scope: !595)
!611 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !610)
!612 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !610)
!613 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !610)
!614 = !DILocation(line: 152, column: 1, scope: !595)
!615 = distinct !DISubprogram(name: "test_new_store", linkageName: "_Z14test_new_storev", scope: !3, file: !3, line: 159, type: !128, scopeLine: 159, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !616)
!616 = !{!617, !623}
!617 = !DILocalVariable(name: "obj1", scope: !615, file: !3, line: 160, type: !618)
!618 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !619, size: 64)
!619 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "MyClass", file: !3, line: 154, size: 512, flags: DIFlagTypePassByValue, elements: !620, identifier: "_ZTS7MyClass")
!620 = !{!621}
!621 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !619, file: !3, line: 156, baseType: !622, size: 512, flags: DIFlagPublic)
!622 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 512, elements: !19)
!623 = !DILocalVariable(name: "obj2", scope: !615, file: !3, line: 161, type: !618)
!624 = !DILocation(line: 160, column: 19, scope: !615)
!625 = !DILocation(line: 160, column: 23, scope: !615)
!626 = !DILocation(line: 0, scope: !615)
!627 = !DILocation(line: 161, column: 19, scope: !615)
!628 = !DILocation(line: 161, column: 23, scope: !615)
!629 = !DILocation(line: 0, scope: !308, inlinedAt: !630)
!630 = distinct !DILocation(line: 162, column: 3, scope: !615)
!631 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !630)
!632 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !630)
!633 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !630)
!634 = !DILocation(line: 0, scope: !308, inlinedAt: !635)
!635 = distinct !DILocation(line: 163, column: 3, scope: !615)
!636 = !DILocation(line: 8, column: 20, scope: !308, inlinedAt: !635)
!637 = !DILocation(line: 8, column: 3, scope: !308, inlinedAt: !635)
!638 = !DILocation(line: 8, column: 24, scope: !308, inlinedAt: !635)
!639 = !DILocation(line: 164, column: 3, scope: !615)
!640 = !DILocation(line: 165, column: 3, scope: !615)
!641 = !DILocation(line: 166, column: 1, scope: !615)
!642 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 168, type: !101, scopeLine: 168, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7)
!643 = !DILocation(line: 169, column: 3, scope: !642)
!644 = !DILocation(line: 170, column: 3, scope: !642)
!645 = !DILocation(line: 171, column: 3, scope: !642)
!646 = !DILocation(line: 172, column: 3, scope: !642)
!647 = !DILocation(line: 173, column: 3, scope: !642)
!648 = !DILocation(line: 174, column: 3, scope: !642)
!649 = !DILocation(line: 175, column: 3, scope: !642)
!650 = !DILocation(line: 176, column: 3, scope: !642)
!651 = !DILocation(line: 177, column: 3, scope: !642)
!652 = !DILocation(line: 178, column: 3, scope: !642)
!653 = !DILocation(line: 179, column: 3, scope: !642)
!654 = !DILocation(line: 180, column: 3, scope: !642)
!655 = !DILocation(line: 181, column: 3, scope: !642)
!656 = !DILocation(line: 182, column: 3, scope: !642)
!657 = !DILocation(line: 183, column: 3, scope: !642)
