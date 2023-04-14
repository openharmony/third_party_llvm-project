# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t
# RUN: not ld.lld --eh-frame-hdr %t -o /dev/null 2>&1 | FileCheck %s

# OHOS_LOCAL begin
# CHECK: error: malformed CIE in .eh_frame: unexpected character in CIE augmentation string:
# OHOS_LOCAL end

.section .eh_frame,"a",@unwind
  .byte 0x0E
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x00
  .byte 0x03
  .byte 0x01
  .byte 0x00
  .byte 0x01
  
  .byte 0x01 # LEB128
  .byte 0x01 # LEB128

  .byte 0x01
  .byte 0x01
  .byte 0x01
  .byte 0x01
