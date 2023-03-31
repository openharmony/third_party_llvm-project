; OHOS_LOCAL
; RUN: opt -inline %s -S | FileCheck %s
; RUN: opt -passes='cgscc(inline)' %s -S | FileCheck %s
; Ensure SSPRET attributes are propagated correctly when inlining.

@.str = private unnamed_addr constant [11 x i8] c"fun_nossp\0A\00", align 1
@.str1 = private unnamed_addr constant [9 x i8] c"fun_ssp\0A\00", align 1
@.str2 = private unnamed_addr constant [18 x i8] c"fun_sspretstrong\0A\00", align 1
@.str3 = private unnamed_addr constant [15 x i8] c"fun_sspretreq\0A\00", align 1

; These first four functions (@fun_sspretreq, @fun_sspretstrong, @fun_ssp, @fun_nossp)
; are used by the remaining functions to ensure that the SSP attributes are
; propagated correctly.  If the caller had an SSP attribute before inlining, it
; should have its new SSP attribute set as:
; strictest(caller-ssp-attr, callee-ssp-attr), where strictness is ordered as:
;  sspretreq > sspretstrong > ssp

define internal void @fun_sspretreq() sspretreq {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str3, i32 0, i32 0))
  ret void
}

define internal void @fun_sspretreq_alwaysinline() sspretreq alwaysinline {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str3, i32 0, i32 0))
  ret void
}

define internal void @fun_sspretstrong() sspretstrong {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str2, i32 0, i32 0))
  ret void
}

define internal void @fun_ssp() ssp {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str1, i32 0, i32 0))
  ret void
}

define internal void @fun_nossp() {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0))
  ret void
}

; Tests start below.

define void @inline_retreq_retreq() sspretreq {
entry:
; CHECK: @inline_retreq_retreq() #[[SSPRETREQ:[0-9]]]
  call void @fun_sspretreq()
  ret void
}

define void @inline_retreq_retstrong() sspretstrong {
entry:
; CHECK: @inline_retreq_retstrong() #[[SSPRETREQ]]
  call void @fun_sspretreq()
  ret void
}

define void @inline_retreq_ssp() ssp {
entry:
; CHECK: @inline_retreq_ssp() #[[SSPRETREQ]]
  call void @fun_sspretreq()
  ret void
}

define void @inline_retreq_nossp() {
entry:
; CHECK: @inline_retreq_nossp() {
  call void @fun_sspretreq()
  ret void
}

define void @alwaysinline_retreq_nossp() {
entry:
; CHECK: @alwaysinline_retreq_nossp() {
  call void @fun_sspretreq_alwaysinline()
  ret void
}

define void @inline_retstrong_retreq() sspretreq {
entry:
; CHECK: @inline_retstrong_retreq() #[[SSPRETREQ]]
  call void @fun_sspretstrong()
  ret void
}


define void @inline_retstrong_retstrong() sspretstrong {
entry:
; CHECK: @inline_retstrong_retstrong() #[[SSPRETSTRONG:[0-9]]]
  call void @fun_sspretstrong()
  ret void
}

define void @inline_retstrong_ssp() ssp {
entry:
; CHECK: @inline_retstrong_ssp() #[[SSPRETSTRONG]]
  call void @fun_sspretstrong()
  ret void
}

define void @inline_retstrong_nossp() {
entry:
; CHECK: @inline_retstrong_nossp() {
  call void @fun_sspretstrong()
  ret void
}

define void @inline_ssp_retreq() sspretreq {
entry:
; CHECK: @inline_ssp_retreq() #[[SSPRETREQ]]
  call void @fun_ssp()
  ret void
}


define void @inline_ssp_retstrong() sspretstrong {
entry:
; CHECK: @inline_ssp_retstrong() #[[SSPRETSTRONG]]
  call void @fun_ssp()
  ret void
}

define void @inline_ssp_ssp() ssp {
entry:
; CHECK: @inline_ssp_ssp() #[[SSP:[0-9]]]
  call void @fun_ssp()
  ret void
}

define void @inline_ssp_nossp() {
entry:
; CHECK: @inline_ssp_nossp() {
  call void @fun_ssp()
  ret void
}

define void @inline_nossp_retreq() sspretreq {
entry:
; CHECK: @inline_nossp_retreq() #[[SSPRETREQ]]
  call void @fun_nossp()
  ret void
}


define void @inline_nossp_retstrong() sspretstrong {
entry:
; CHECK: @inline_nossp_retstrong() #[[SSPRETSTRONG]]
  call void @fun_nossp()
  ret void
}

define void @inline_nossp_ssp() ssp {
entry:
; CHECK: @inline_nossp_ssp() #[[SSP]]
  call void @fun_nossp()
  ret void
}

define void @inline_nossp_nossp() {
entry:
; CHECK: @inline_nossp_nossp() {
  call void @fun_nossp()
  ret void
}

declare i32 @printf(i8*, ...)

; CHECK: attributes #[[SSPRETREQ]] = { sspretreq }
; CHECK: attributes #[[SSPRETSTRONG]] = { sspretstrong }
; CHECK: attributes #[[SSP]] = { ssp }
