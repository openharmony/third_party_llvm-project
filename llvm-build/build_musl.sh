#!/bin/bash
# Copyright (C) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set -e

# This script is used to create musl's libc.so.

#default variables
CLANG_BIN_ROOT="${PWD}/../../out/install/linux-x86_64/clang-dev/bin/"
TARGET_TRIPLE=""
TOPDIR="${PWD}/../.."
OUT="${PWD}/../../out"
make_libs=0

#argument parser
while getopts "c:t:T:o:lh" arg
do
    case "${arg}" in
        "c")
            CLANG_BIN_ROOT=${OPTARG}
            ;;
        "t")
            TARGET_TRIPLE=${OPTARG}
            ;;
        "T")
            TOPDIR=${OPTARG}
            ;;
        "o")
            OUT=${OPTARG}
            ;;
        "l")
            make_libs=1
            ;;
        "h")
            echo "Usage: ./build_musl.sh [OPTION]"
            echo " Build C/C++ dynamic libs and runtime object"
            echo " Options are:"
            echo "  -c <path>       Specify clang bin path"
            echo "  -t <target>     Specify target tripple"
            echo "  -T <dir>        Specify top of repo tree"
            echo "  -o <dir>        Specify the build output directory"
            echo "  -l              Install libs"
            exit 0
            ;;
        ?)
            echo "unkown argument"
            exit 1
            ;;
    esac
done

if [ -z "${TARGET_TRIPLE}" ]
then
      echo "[ERROR] Empty target triple in build_musl.sh"
      exit 1
fi

# Canonicalize paths
CLANG_BIN_ROOT=$(readlink -f ${CLANG_BIN_ROOT})

CFLAGS_FOR_TARGET=("-mfloat-abi=soft" "-mfloat-abi=soft -mcpu=cortex-a7"
"-mfloat-abi=softfp -mfpu=neon-vfpv4 -mcpu=cortex-a7" "-mfloat-abi=hard -mfpu=neon-vfpv4 -mcpu=cortex-a7")

if [ $TARGET_TRIPLE == "arm-liteos-ohos" ]; then
    TARGET_USER="liteos_a_user"
    TARGETS_PREFIX="arm"
elif [ $TARGET_TRIPLE == "arm-linux-ohos" ]; then
    TARGET_USER="linux_user"
    TARGETS_PREFIX="arm"
elif [ $TARGET_TRIPLE == "mipsel-linux-ohos" ]; then
    TARGET_USER="linux_user"
    TARGETS_PREFIX="mips"
    CFLAGS_FOR_TARGET=("-march=mips32r2")
elif [ $TARGET_TRIPLE == "riscv64-linux-ohos" ]; then
    TARGET_USER="linux_user"
    TARGETS_PREFIX="riscv64"
elif [ $TARGET_TRIPLE == "x86_64-linux-ohos" ]; then
    TARGET_USER="linux_user"
    TARGETS_PREFIX="x86_64"
else
    TARGET_USER="linux_user"
    TARGETS_PREFIX="aarch64"
fi

echo "CLANG_BIN_ROOT=${CLANG_BIN_ROOT}"
echo "ARCH=${TARGETS_PREFIX}"
echo "TARGET=${TARGET_TRIPLE}"
echo "TOPDIR=${TOPDIR}"
echo "TARGETS=${TARGET_USER}"
echo "OUT=${OUT}"
echo "SYSROOTDIR=${OUT}/sysroot"

# build musl_headers
make musl_header_install_for_${TARGET_USER} CLANG="${CLANG_BIN_ROOT}/clang" TOPDIR=${TOPDIR} \
     SYSROOTDIR=${OUT}/sysroot TARGETS=${TARGET_USER} TARGET=${TARGET_TRIPLE} ARCH=${TARGETS_PREFIX} -f Makefile

# build musl_libs
if ((make_libs == 1)); then
    if [ $TARGET_TRIPLE == "aarch64-linux-ohos" ] || [ $TARGET_TRIPLE == "riscv64-linux-ohos" ] || \
       [ $TARGET_TRIPLE == "mipsel-linux-ohos" ] || [ $TARGET_TRIPLE == "x86_64-linux-ohos" ]; then
        make CLANG="${CLANG_BIN_ROOT}/clang" TOPDIR=${TOPDIR} SYSROOTDIR=${OUT}/sysroot TARGETS=${TARGET_USER}\
            TARGET=${TARGET_TRIPLE} ARCH=${TARGETS_PREFIX} -f Makefile
    else
        for ARCH_CFLAG in "${CFLAGS_FOR_TARGET[@]}"
        do
            make CLANG="${CLANG_BIN_ROOT}/clang" TOPDIR=${TOPDIR} SYSROOTDIR=${OUT}/sysroot TARGETS=${TARGET_USER}\
            TARGET=${TARGET_TRIPLE} ARCH=${TARGETS_PREFIX} ARCH_CFLAGS="${ARCH_CFLAG}" -f Makefile
        done
    fi
fi
make distclean -f Makefile
