# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t
# RUN: not ld.lld --eh-frame-hdr %t -o /dev/null 2>&1 | FileCheck %s

# OHOS_LOCAL begin
# CHECK: error: malformed CIE in .eh_frame: cannot get personality pointer for personality encoding 5
# CHECK-NEXT: >>> defined in {{.*}}:(.eh_frame+0xF)
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
  .byte 0x01
  
  .byte 0x50 # Augmentation string: 'P','\0'
  .byte 0x00
  
  .byte 0x01
  
  .byte 0x01 # LEB128
  .byte 0x01 # LEB128

  .byte 0x05 # OHOS_LOCAL (now we support 0x01 encoding, so use 0x05 as invalid one for test)
  .byte 0x01
  .byte 0x01
  .byte 0x01
