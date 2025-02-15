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

# usage install.sh <INSTALL_PATH>
# check the args
if [ $# -ne 1 ]; then
    echo "Usage:    $0 <INSTALL_PATH>"
    echo "Hint:     <INSTALL_PATH> is the directory where the directory llvm is located."
    echo "Example:  $0 /home/ohos/command-line-tools/sdk/default/openharmony/native"
    exit 1
fi

# check the install path
# there should be a directory named llvm in the install path
# and the llvm directory should contain the bin/clang file
INSTALL_PATH=$1
if [ ! -f $INSTALL_PATH/llvm/bin/clang ]; then
    echo "Error:    The INSTALL_PATH: $INSTALL_PATH does not a legal path."
    echo "Hint:     <INSTALL_PATH> is the directory where the directory llvm is located."
    exit 1
fi
echo "INSTALL_PATH: $INSTALL_PATH"

# apply the patch
echo "Applying the patch..."
SCRIPT_PATH=$(dirname "$(realpath "$0")")
cp -rf $SCRIPT_PATH/../patch/* $INSTALL_PATH
echo "Patch applied."

# testing the patch
echo "Testing the patch..."

# find cmake
CMAKE=$INSTALL_PATH/build-tools/cmake/bin/cmake
if [ ! -f $CMAKE ]; then
    echo "Error:    The cmake is not found in the path: $CMAKE"
    exit 1
else
    echo "CMAKE:     $CMAKE"
fi

# find llvm-readelf
READELF=$INSTALL_PATH/llvm/bin/llvm-readelf
if [ ! -f $READELF ]; then
    echo "Error:    The llvm-readelf is not found in the path: $READELF"
    exit 1
else
    echo "READELF:   $READELF"
fi

# == test c++_shared ==
# build the test
BUILD_PATH_SHARED=/tmp/libcxx_patch_c++_shared_$(($RANDOM + $(date +%s)))
echo "Testing OHOS_STL=c++_shared..."
$CMAKE -DOHOS_STL=c++_shared -DCMAKE_TOOLCHAIN_FILE=$INSTALL_PATH/build/cmake/ohos.toolchain.cmake -B $BUILD_PATH_SHARED -S $SCRIPT_PATH/../test > /dev/null
$CMAKE --build $BUILD_PATH_SHARED > /dev/null
# check the shared library using readelf
$READELF -d $BUILD_PATH_SHARED/test | grep libc++_shared.so
if [ $? -ne 0 ]; then
    echo "Error:    Test failed: The libc++_shared.so is not linked."
    exit 1
fi

# == test system ==
# build the test

BUILD_PATH_SYSTEM=/tmp/libcxx_patch_c++_system_$(($RANDOM + $(date +%s)))
echo "Testing OHOS_STL=system..."
$CMAKE -DOHOS_STL=system -DCMAKE_TOOLCHAIN_FILE=$INSTALL_PATH/build/cmake/ohos.toolchain.cmake -B $BUILD_PATH_SYSTEM -S $SCRIPT_PATH/../test > /dev/null
$CMAKE --build $BUILD_PATH_SYSTEM > /dev/null
# check the shared library using readelf
$READELF -d $BUILD_PATH_SYSTEM/test | grep libc++.so
if [ $? -ne 0 ]; then
    echo "Error:    Test failed: The libc++.so is not linked."
    exit 1
fi

echo "All tests passed."
echo "Patch installed successfully."

