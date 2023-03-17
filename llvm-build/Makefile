# Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ARCH = arm
ifneq ($(GCC),)
TARGET =
CROSS_COMPILE = $(GCC:%gcc=%)
CC = $(GCC) $(ARCH_CFLAGS)
MULTILIB = $(patsubst %.,%,$(shell $(CC) -print-multi-directory))
else
TARGET = $(ARCH)-liteos-ohos
CLANG ?= clang
CROSS_COMPILE = $(CLANG:%clang=%llvm-)
CC = $(CLANG) --target=$(TARGET) $(ARCH_CFLAGS)
BUILD=x86_64-linux-gnu
MULTILIB = $(patsubst $(dir $(shell $(filter-out $(ARCH_CFLAGS),$(CC)) -print-libgcc-file-name))%,/%,$(dir $(shell $(CC) -print-libgcc-file-name)))
endif
MUSLBUILDDIR = build_$(or $(TARGET),$(ARCH))$(subst /,_,$(MULTILIB:%/=%))
HIDE = @
BUILD_DEBUG = false
SED_ARGS = -e '/install-libs:/s/if/and/g'

TOPDIR = $(shell pwd)/../../../..
MUSLDIR = $(TOPDIR)/third_party/musl
LINUXKERNELDIR = $(TOPDIR)/third_party/Linux_Kernel
OPTRTDIR = $(TOPDIR)/third_party/optimized-routines
NUTTXDIR = $(TOPDIR)/third_party/NuttX
SYSROOTDIR = $(TOPDIR)/prebuilts/lite/sysroot
LITEOSADIR = $(TOPDIR)/kernel/liteos_a
LINUXDIR = $(TOPDIR)/kernel/linux/linux-5.10

TARGETS = $(if $(wildcard $(LITEOSADIR)),liteos_a_user,)
TARGETS += $(if $(wildcard $(LINUXDIR)),linux_user,)

define LINUX_TYPES_H
#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H
#include <stdint.h>
typedef uint32_t __u32, __le32;
#endif
endef
export LINUX_TYPES_H

ifeq ($(ARCH),arm)
ARCH_CFLAGS = -mfloat-abi=softfp -mcpu=cortex-a7 -mfpu=neon-vfpv4
else
ifeq ($(ARCH),aarch64)
ARCH_CFLAGS =
else
ifeq ($(ARCH),riscv64)
ARCH_CFLAGS =
else
ifeq ($(ARCH),mips)
ARCH_CFLAGS =
else
ifeq ($(ARCH),x86_64)
ARCH_CFLAGS =
else
$(warning *** warning: ARCH $(ARCH) has not been tested yet, use with cautions!)
ARCH_CFLAGS =
endif
endif
endif
endif
endif

ifeq ($(ARCH),aarch64)
CFLAGS = -march=armv8 -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack
else
ifeq ($(ARCH),riscv64)
CFLAGS = -march=rv64gc -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack
else
ifeq ($(ARCH),mips)
CFLAGS = -march=mips32r2 -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack
else
ifeq ($(ARCH),x86_64)
CFLAGS = -march=x86-64 -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack
else
CFLAGS = -march=armv7-a -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack
endif
endif
endif
endif

CFLAGS += -Wno-int-conversion

.PHONY: $(TARGETS:%=musl_copy_for_%)
.PHONY: $(TARGETS:%=musl_patch_for_%)
.PHONY: $(TARGETS:%=musl_install_for_%)
.PHONY: $(TARGETS:%=musl_header_install_for_%)
.PHONY: $(TARGETS:%=linux_header_install_for_%)
.PHONY: $(TARGETS:%=nuttx_header_install_for_%)
.PHONY: $(TARGETS:%=optimized_routines_install_for_%)
.PHONY: all clean distclean

all: $(TARGETS:%=musl_install_for_%)

$(TARGETS:%=musl_copy_for_%):
		$(HIDE) mkdir -p $@
		$(HIDE) cp -rfu $(MUSLDIR)/[!p]* $@

optimized_routines_install_for_liteos_a_user: musl_copy_for_liteos_a_user
ifneq ($(ARCH),)
		$(HIDE) cp -rfp $(OPTRTDIR)/string/$(ARCH)/* $</src/string/$(ARCH)/
		$(HIDE) cp -rfp $(OPTRTDIR)/string/asmdefs.h $</src/string/asmdefs.h
ifeq ($(ARCH),arm)
		$(HIDE) rm -f $</src/string/arm/memcpy.c
		$(HIDE) rm -f $</src/string/arm/memcpy_le.S
		$(HIDE) rm -f $</src/string/arm/memset.S
		$(HIDE) rm -f $</src/string/arm/strcpy.c
		$(HIDE) true >> $</src/string/arm/strlen.S

CFLAGS += \
		-D__strlen_armv6t2=strlen \
		-D__strcpy_arm=strcpy \
		-D__strcmp_arm=strcmp \
		-D__memcpy_arm=memcpy \
		-D__memchr_arm=memchr

endif
endif

nuttx_header_install_for_liteos_a_user: musl_copy_for_liteos_a_user
	$(HIDE) sed '/^#include/d' $(NUTTXDIR)/include/nuttx/video/fb.h | sed 's,FAR ,,g' | sed 's,LosVmMapRegion,void,g' > $</include/fb.h

linux_header_install_for_liteos_a_user: musl_copy_for_liteos_a_user
	$(HIDE) make -sj headers_install ARCH=$(ARCH) O=$(shell pwd)/$@ -C $(LINUXKERNELDIR)
	$(HIDE) install -p -m 644 -D $@/usr/include/linux/capability.h $</include/linux/capability.h
	$(HIDE) echo "$$LINUX_TYPES_H" > $</include/linux/types.h

musl_patch_for_liteos_a_user: nuttx_header_install_for_liteos_a_user
musl_patch_for_liteos_a_user: linux_header_install_for_liteos_a_user
musl_patch_for_liteos_a_user: optimized_routines_install_for_liteos_a_user
musl_patch_for_liteos_a_user: musl_copy_for_liteos_a_user
	$(HIDE) cp -rfp $(MUSLDIR)/porting/liteos_a/user/* $</

musl_patch_for_liteos_a_user_debug: musl_patch_for_liteos_a_user
musl_patch_for_liteos_a_user_debug: musl_copy_for_liteos_a_user
	$(HIDE) cp -rfp $(MUSLDIR)/porting/liteos_a/user_debug/* $</

ifeq ($(BUILD_DEBUG),true)
ifeq ($(GCC),)
musl_install_for_liteos_a_user: LDFLAGS = $(shell $(CC) -print-file-name=libunwind.a) -Wl,--no-dependent-libraries
musl_install_for_liteos_a_user: SED_ARGS += -e 's,$$(AR) rc $$@ $$(AOBJS)$$,cp $$(shell $$(CC) -print-file-name=libunwind.a) $$@ \&\& &,g'
endif
musl_install_for_liteos_a_user: SED_ARGS += -e 's,CFLAGS_ALL += -DCRT$$,& -fno-unwind-tables -fno-asynchronous-unwind-tables,g'
musl_install_for_liteos_a_user: CFLAGS += -funwind-tables -fasynchronous-unwind-tables -rdynamic -I $(shell $(CC) "-print-file-name=include")
musl_install_for_liteos_a_user: musl_patch_for_liteos_a_user_debug
endif

musl_header_install_for_liteos_a_user: musl_patch_for_liteos_a_user
		$(HIDE) cd musl_copy_for_liteos_a_user && mkdir -p $(MUSLBUILDDIR) && cd $(MUSLBUILDDIR) && \
		../configure --prefix=$(SYSROOTDIR)/$(TARGET)/usr --target=$(TARGET) \
				--includedir=$(SYSROOTDIR)/$(TARGET)/usr/include \
				--libdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--syslibdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--build=$(BUILD) \
				$(if $(LDFLAGS),LDFLAGS="$(LDFLAGS)",) \
				CC="$(CC)" CROSS_COMPILE="$(CROSS_COMPILE)" CFLAGS="$(CFLAGS)" >/dev/null && \
		make -sj install-headers

musl_install_for_liteos_a_user: musl_patch_for_liteos_a_user
		$(HIDE) cd musl_copy_for_liteos_a_user && mkdir -p $(MUSLBUILDDIR) && cd $(MUSLBUILDDIR) && \
		../configure --prefix=$(SYSROOTDIR)/$(TARGET)/usr --target=$(TARGET) \
				--includedir=$(SYSROOTDIR)/$(TARGET)/usr/include \
				--libdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--syslibdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--build=$(BUILD) \
				$(if $(LDFLAGS),LDFLAGS="$(LDFLAGS)",) \
				CC="$(CC)" CROSS_COMPILE="$(CROSS_COMPILE)" CFLAGS="$(CFLAGS)" >/dev/null && \
		make -sj install

musl_patch_for_linux_user: musl_copy_for_linux_user
		$(HIDE) cp -rfp $(MUSLDIR)/porting/linux/user/* $</

ifeq ($(GCC),)
musl_install_for_linux_user: TARGET = $(ARCH)-linux-ohos
endif

musl_header_install_for_linux_user: musl_patch_for_linux_user
		$(HIDE) cd musl_copy_for_linux_user && mkdir -p $(MUSLBUILDDIR) && cd $(MUSLBUILDDIR) && \
		../configure --prefix=$(SYSROOTDIR)/$(TARGET)/usr --target=$(TARGET) \
				--includedir=$(SYSROOTDIR)/$(TARGET)/usr/include \
				--libdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--syslibdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--build=$(BUILD) \
				CC="$(CC)" CROSS_COMPILE="$(CROSS_COMPILE)" CFLAGS="$(CFLAGS)" >/dev/null && \
		make -sj install-headers

musl_install_for_linux_user: musl_patch_for_linux_user
		$(HIDE) cd musl_copy_for_linux_user && mkdir -p $(MUSLBUILDDIR) && cd $(MUSLBUILDDIR) && \
		../configure --prefix=$(SYSROOTDIR)/$(TARGET)/usr --target=$(TARGET) \
				--includedir=$(SYSROOTDIR)/$(TARGET)/usr/include \
				--libdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--syslibdir=$(SYSROOTDIR)/$(TARGET)/usr/lib/$(MULTILIB) \
				--build=$(BUILD) \
				CC="$(CC)" CROSS_COMPILE="$(CROSS_COMPILE)" CFLAGS="$(CFLAGS)" >/dev/null && \
		make -sj install

clean:
		$(HIDE) rm -rf musl_copy_for_* linux_header_install_for_*

distclean: clean
		$(HIDE) rm -rf $(SYSROOTDIR)/lib $(SYSROOTDIR)/usr
