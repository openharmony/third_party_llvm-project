# REQUIRES: mips
## Check that we can link a shared library containing an eh_frame section without
## -z notext. This was not possible LLVM started emitting values using the
## DW_EH_PE_pcrel | DW_EH_PE_sdata4 encoding.

## OHOS_LOCAL begin

## MIPS64

## Without -fPIC, the object files produced by the compiler will have
## DW_EH_PE_absptr encoded pointers in .eh_frame section. The linker must
## convert them to DW_EH_PE_pcrel when linking a .so library since an absolute
## pointer would require emitting a dynamic relocation which is not desireable
## for a read-only .eh_frame section.
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-linux %s -o %t-nopic.o
# RUN: llvm-dwarfdump --eh-frame %t-nopic.o | FileCheck %s --check-prefix=ABS64-EH-FRAME
# RUN: llvm-readobj -r %t-nopic.o | FileCheck %s --check-prefixes=RELOCS,ABS64-RELOCS
# RUN: ld.lld -shared %t-nopic.o -o %t-nopic.so
# RUN: llvm-dwarfdump --eh-frame %t-nopic.so | FileCheck %s --check-prefix=PCREL64-EH-FRAME

## For -fPIC, .eh_frame should contain DW_EH_PE_pcrel | DW_EH_PE_sdata4 values:
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-linux --position-independent %s -o %t-pic.o
# RUN: llvm-readobj -r %t-pic.o | FileCheck %s --check-prefixes=RELOCS,PIC64-RELOCS
# RUN: ld.lld -shared %t-pic.o -o %t-pic.so
# RUN: llvm-dwarfdump --eh-frame %t-pic.so | FileCheck %s --check-prefix=PCREL32-EH-FRAME

## MIPS32

## Without -fPIC (see detailed comments in MIPS64 section).
## Note: ld.bfd converts the R_MIPS_32 relocs to DW_EH_PE_pcrel | DW_EH_PE_sdata4
## for MIPS32, we mimic this behavior.
# RUN: llvm-mc -filetype=obj -triple=mips-unknown-linux %s -o %t-nopic32.o
# RUN: llvm-dwarfdump --eh-frame %t-nopic32.o | FileCheck %s --check-prefix=ABS32-EH-FRAME
# RUN: llvm-readobj -r %t-nopic32.o | FileCheck %s --check-prefixes=RELOCS,ABS32-RELOCS
# RUN: ld.lld -shared %t-nopic32.o -o %t-nopic32.so
# RUN: llvm-dwarfdump --eh-frame %t-nopic32.so | FileCheck %s --check-prefix=PCREL32-EH-FRAME

## For -fPIC, .eh_frame should contain DW_EH_PE_pcrel | DW_EH_PE_sdata4 values:
# RUN: llvm-mc -filetype=obj -triple=mips-unknown-linux --position-independent %s -o %t-pic32.o
# RUN: llvm-readobj -r %t-pic32.o | FileCheck %s --check-prefixes=RELOCS,PIC32-RELOCS
# RUN: ld.lld -shared %t-pic32.o -o %t-pic32.so
# RUN: llvm-dwarfdump --eh-frame %t-pic32.so | FileCheck %s --check-prefix=PCREL32-EH-FRAME

# RELOCS:            .rel{{a?}}.eh_frame {
# ABS32-RELOCS-NEXT:   0x1C R_MIPS_32 .text
# ABS64-RELOCS-NEXT:   0x1C R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .text
# PIC32-RELOCS-NEXT:   0x1C R_MIPS_PC32 -
# PIC64-RELOCS-NEXT:   0x1C R_MIPS_PC32/R_MIPS_NONE/R_MIPS_NONE -
# RELOCS-NEXT:       }

# ABS64-EH-FRAME: Augmentation data: 0C
##                                   ^^ fde pointer encoding: DW_EH_PE_absptr | DW_EH_PE_sdata8
# ABS32-EH-FRAME: Augmentation data: 0B
##                                   ^^ fde pointer encoding: DW_EH_PE_absptr | DW_EH_PE_sdata4
# PCREL64-EH-FRAME: Augmentation data: 1C
##                                     ^^ fde pointer encoding: DW_EH_PE_pcrel | DW_EH_PE_sdata8
# PCREL32-EH-FRAME: Augmentation data: 1B
##                                     ^^ fde pointer encoding: DW_EH_PE_pcrel | DW_EH_PE_sdata4

## OHOS_LOCAL end

.ent func
.global func
func:
	.cfi_startproc
	nop
	.cfi_endproc
.end func
