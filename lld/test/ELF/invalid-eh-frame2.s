# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t
# RUN: not ld.lld --eh-frame-hdr %t -o /dev/null 2>&1 | FileCheck %s

# OHOS_LOCAL begin
# CHECK:      error: malformed CIE in .eh_frame: corrupted augmentation string
# OHOS_LOCAL end
# CHECK-NEXT: >>> defined in {{.*}}:(.eh_frame+0x9)

.section .eh_frame,"a",@unwind
.align 1
  .byte 0x08
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x01
  .byte 0x01
  .byte 0x01
  .byte 0x01
