# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t
# RUN: not ld.lld --eh-frame-hdr %t -o /dev/null 2>&1 | FileCheck %s

# OHOS_LOCAL begin
# CHECK:      error: malformed CIE in .eh_frame: corrupted version
# OHOS_LOCAL end
# CHECK-NEXT: >>> defined in {{.*}}:(.eh_frame+0x8)

.section .eh_frame,"a",@unwind
  .byte 0x04
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
