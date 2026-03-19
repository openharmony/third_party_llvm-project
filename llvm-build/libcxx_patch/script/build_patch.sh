#!/bin/sh
# Copyright (C) 2025 Huawei Device Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# usage build_patch.sh <LLVM_OUT_PATH>
# check the args
if [ $# -ne 1 ]; then
    echo "Usage:    $0 <LLVM_OUT_PATH>"
    echo "Hint:     <LLVM_OUT_PATH> is the directory where the directory llvm-project is built."
    echo "Example:  $0 /home/ohos/workplace/ohos/llvm-toolchain/out"
    exit 1
fi

# check the install path
# there should be a directory named llvm in the install path
# and the llvm directory should contain the bin/clang file
LLVM_OUT_PATH=$1
if [ ! -d $LLVM_OUT_PATH/llvm-install ]; then
    echo "Error:    The LLVM_OUT_PATH: $LLVM_OUT_PATH does not a legal path."
    echo "Hint:     <LLVM_OUT_PATH> is the directory where the directory llvm-project is built."
    exit 1
fi
echo "LLVM_OUT_PATH: $LLVM_OUT_PATH"

BUILD_PATH=/tmp/libcxx_patch_build_$(($RANDOM + $(date +%s)))
echo "Building the patch..."
echo "BUILD_PATH: $BUILD_PATH"

# libc++.so
echo "Copying libc++.so..."
mkdir -p $BUILD_PATH/patch/llvm/system_libcxx/lib/aarch64-linux-ohos
cp $LLVM_OUT_PATH/llvm-install/lib/aarch64-linux-ohos/libc++.so $BUILD_PATH/patch/llvm/system_libcxx/lib/aarch64-linux-ohos/

# __config_site
echo "Copying __config_site..."
mkdir -p $BUILD_PATH/patch/llvm/system_libcxx/include
cp $LLVM_OUT_PATH/llvm-install/include/libcxx-ohos/include/c++/v1/__config_site $BUILD_PATH/patch/llvm/system_libcxx/include/

# ohos.toolchain.cmake
echo "Copying ohos.toolchain.cmake..."
mkdir -p $BUILD_PATH/patch/build/cmake
cp $LLVM_OUT_PATH/../build/ohos/ndk/cmake/ohos.toolchain.cmake $BUILD_PATH/patch/build/cmake/

echo "Patching cmake toolchain file..."
sed -i '/elseif(OHOS_STL STREQUAL c++_shared)/a\
elseif(OHOS_STL STREQUAL system)\
  # system libcxx is only supported for aarch64-linux-ohos\
  if(OHOS_TOOLCHAIN_NAME STREQUAL aarch64-linux-ohos)\
    include_directories("${OHOS_SDK_NATIVE}/llvm/system_libcxx/include")\
    link_directories("${OHOS_SDK_NATIVE}/llvm/system_libcxx/lib/${OHOS_TOOLCHAIN_NAME}")\
  else()\
    message(FATAL_ERROR "Unsupported STL configuration: ${OHOS_STL} which is only available for aarch64-linux-ohos.")\
  endif()' $BUILD_PATH/patch/build/cmake/ohos.toolchain.cmake

# script.sh
echo "Copying install.sh..."
mkdir -p $BUILD_PATH/script
cp -r $(dirname "$(realpath "$0")")/install.sh $BUILD_PATH/script

# test
echo "Copying test..."
mkdir -p $BUILD_PATH/test
cp -r $(dirname "$(realpath "$0")")/../test/* $BUILD_PATH/test

# tar the patch
echo "Taring the patch..."
tar -cf $LLVM_OUT_PATH/../packages/libcxx_patch.tar.xz -C $BUILD_PATH/.. $(basename $BUILD_PATH)

echo "Patch built successfully."
echo "The patch is saved in $LLVM_OUT_PATH/../packages/libcxx_patch.tar.xz"
