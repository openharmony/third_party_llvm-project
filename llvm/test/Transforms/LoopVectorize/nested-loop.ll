; RUN: opt -loop-vectorize -S %s -o - | FileCheck %s
; CHECK:       vector.memcheck:
; CHECK-NEXT:    %bound0 = icmp ult ptr
; CHECK-NEXT:    %bound1 = icmp ult ptr
; CHECK-NEXT:    %found.conflict = and i1 %bound0, %bound1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nofree nounwind uwtable
define dso_local void @array_magick(ptr nocapture noundef %a, ptr nocapture noundef readonly %b, i32 noundef %len) local_unnamed_addr #0 {
entry:
  %cmp24 = icmp sgt i32 %len, 0
  br i1 %cmp24, label %for.cond1.preheader.us.preheader, label %for.end12

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %wide.trip.count30 = zext i32 %len to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.preheader.us.preheader, %for.cond1.for.end_crit_edge.us
  %indvars.iv27 = phi i64 [ 0, %for.cond1.preheader.us.preheader ], [ %indvars.iv.next28, %for.cond1.for.end_crit_edge.us ]
  %arrayidx.us = getelementptr inbounds i32, ptr %a, i64 %indvars.iv27
  %.pre = load i32, ptr %arrayidx.us, align 4, !tbaa !5
  br label %for.body3.us

for.body3.us:                                     ; preds = %for.cond1.preheader.us, %for.body3.us
  %0 = phi i32 [ %.pre, %for.cond1.preheader.us ], [ %sub.us, %for.body3.us ]
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next, %for.body3.us ]
  %arrayidx5.us = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx5.us, align 4, !tbaa !5
  %sub.us = sub i32 %0, %1
  store i32 %sub.us, ptr %arrayidx.us, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count30
  br i1 %exitcond.not, label %for.cond1.for.end_crit_edge.us, label %for.body3.us, !llvm.loop !9

for.cond1.for.end_crit_edge.us:                   ; preds = %for.body3.us
  %sub.us.lcssa = phi i32 [ %sub.us, %for.body3.us ]
  %call.us = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str, i32 noundef %sub.us.lcssa)
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond31.not = icmp eq i64 %indvars.iv.next28, %wide.trip.count30
  br i1 %exitcond31.not, label %for.end12.loopexit, label %for.cond1.preheader.us, !llvm.loop !12

for.end12.loopexit:                               ; preds = %for.cond1.for.end_crit_edge.us
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 16.0.0 (https://github.com/llvm/llvm-project.git 09f608fda51ca9dd2d88c2985bad1cfc1e36251e)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!12, !10, !11}
