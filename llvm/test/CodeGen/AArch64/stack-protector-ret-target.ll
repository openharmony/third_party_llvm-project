; OHOS_LOCAL
; Test target-specific stack cookie location.
; RUN: llc -mtriple=aarch64-linux-ohos < %s -o - | FileCheck --check-prefix=OHOS-AARCH64 %s

define void @_Z1fv() sspretreq {
entry:
  %x = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @_Z7CapturePi(i32* nonnull %x)
  ret void
}

declare void @_Z7CapturePi(i32*)

; OHOS-AARCH64: adrp x15, __sspret_cookie_
; OHOS-AARCH64: ldr x15, [x15, :lo12:__sspret_cookie_
; OHOS-AARCH64: eor x15, x15, x29
; OHOS-AARCH64: eor x15, x15, x30
; OHOS-AARCH64: sub sp, sp,
; OHOS-AARCH64: stp x30, x15, [sp,
; OHOS-AARCH64: ]
; OHOS-AARCH64: ldp x30, x15, [sp,
; OHOS-AARCH64: add sp,
; OHOS-AARCH64: adrp x9, __sspret_cookie_
; OHOS-AARCH64: ldr x9, [x9, :lo12:__sspret_cookie_
; OHOS-AARCH64: eor x9, x9, x29
; OHOS-AARCH64: eor x9, x9, x30
; OHOS-AARCH64: subs x15, x15, x9
; OHOS-AARCH64: cbz x15,
; OHOS-AARCH64: brk #0x1
; OHOS-AARCH64: ret
