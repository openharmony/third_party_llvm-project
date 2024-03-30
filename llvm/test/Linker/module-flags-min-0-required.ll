; RUN: rm -rf %t && split-file %s %t && cd %t
; RUN: not llvm-link a.ll b.ll -S -o - 2>&1 | FileCheck %s

; CHECK: error: linking module flags 'required_in_b': does not have the required value

;--- a.ll
!0 = !{ i32 8, !"foo", i16 2 }
!1 = !{ i32 8, !"bar", i64 4 }
!2 = !{ i32 8, !"only_in_a", i32 4 }

!llvm.module.flags = !{ !0, !1, !2 }

;--- b.ll
!0 = !{ i32 8, !"foo", i16 3 }
!1 = !{ i32 8, !"bar", i64 3 }
!2 = !{ i32 8, !"only_in_b", i32 3 }
!3 = !{ i32 8, !"required_in_b", i32 3 }
!4 = !{ i32 3, !"require", !{!"required_in_b", i32 3} }

!llvm.module.flags = !{ !0, !1, !2, !3, !4 }
