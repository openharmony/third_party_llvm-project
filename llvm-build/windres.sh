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
# This script is used to deal version.rc file.
set -e

#default variables

LLVM_BUILD_DIR=$(dirname $0)
PARENT_OF_LLVM_BUILD=$(basename "$(realpath "$LLVM_BUILD_DIR/..")")
case $PARENT_OF_LLVM_BUILD in
  toolchain)    TOPDIR="$(realpath "$LLVM_BUILD_DIR/../..")" ;;
  llvm-project) TOPDIR="$(realpath "$LLVM_BUILD_DIR/../../..")" ;;
  *)            echo "Cannot detect TOPDIR" 1>&2 ; exit 1 ;;
esac

CLANG_BIN_ROOT="$TOPDIR/out/clang_mingw/clang-10.0.1/bin/"
CLANG_FLAGS="clang -target x86_64-w64-mingw32 -rtlib=compiler-rt -stdlib=libc++ -fuse-ld=lld -Qunused-arguments -E -xc -DRC_INVOKED=1"
LLVM_RC_FLAGS="llvm-rc -I ../src src/.libs/version.o.preproc.rc -c 1252 -fo src/.libs/version.o.out.res"
LLVM_CVTRES_FLAGS="llvm-cvtres src/.libs/version.o.out.res -machine:X64 -out:"

while getopts "i:o:h" arg
do
    case "${arg}" in
        "i")
            IN=${OPTARG}
            ;;
        "o")
            OUT=${OPTARG}
            ;;
        "h")
            echo "Usage: ./build_libs.sh [OPTION]"
            echo " Build C/C++ dynamic libs and runtime object"
            echo " Options are:"
            echo "  -i <arg>        Name of the input file."
            echo "  -o <arg>        Name of the output file."
            echo "  -h              Display this message and exit."
            exit 0
            ;;
        ?)
            echo "unkown argument"
            exit 1
            ;;
    esac
done

CLANG_FLAGS="${CLANG_BIN_ROOT}${CLANG_FLAGS} -v $IN -o src/.libs/version.o.preproc.rc"
LLVM_RC_FLAGS="${CLANG_BIN_ROOT}${LLVM_RC_FLAGS}"
LLVM_CVTRES_FLAGS="${CLANG_BIN_ROOT}${LLVM_CVTRES_FLAGS}${OUT}"

mkdir src/.libs
${CLANG_FLAGS}
${LLVM_RC_FLAGS}
${LLVM_CVTRES_FLAGS}
