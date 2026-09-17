# REQUIRES: ohos_llvm
# REQUIRES: mips
# Having an R_MIPS_64 relocation in eh_frame would previously crash LLD.
# OHOS rewrites DW_EH_PE_absptr to DW_EH_PE_pcrel in .eh_frame, so a shared
# object / PIE links without -z notext and emits no dynamic relocation for the
# FDE pointer.
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %s -o %t.o
# RUN: llvm-readobj -r %t.o | FileCheck %s -check-prefix OBJ
# RUN: ld.lld --eh-frame-hdr -shared -o %t.so %t.o
# RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix PIC-RELOCS

# Linking this as a PIE executable would also previously crash
# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd %S/../../ELF/Inputs/archive2.s -o %t-foo.o
# RUN: ld.lld --eh-frame-hdr -Bdynamic -pie -o %t-pie-dynamic.exe %t.o %t-foo.o
# RUN: llvm-readobj -r %t-pie-dynamic.exe | FileCheck %s -check-prefix PIC-RELOCS


# OBJ:       Section ({{.*}}) .rela.text {
# OBJ-NEXT:    0x0 R_MIPS_GPREL16/R_MIPS_SUB/R_MIPS_HI16 foo 0x0
# OBJ-NEXT:  }
# OBJ-NEXT:  Section ({{.*}}) .rela.eh_frame {
# OBJ-NEXT:    0x1C R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .text 0x0
# OBJ-NEXT:  }

# OHOS_LOCAL begin
# PIC-RELOCS: Relocations [
# PIC-RELOCS-NEXT:]
# OHOS_LOCAL end


.globl foo

bar:
.cfi_startproc
lui	$11, %hi(%neg(%gp_rel(foo)))
.cfi_endproc

.globl __start
__start:
b bar
