#! /bin/bash

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SANITIZER="asan"
if [ "$#" -gt 0 ]; then
    SANITIZER=$1
    shift
fi

ENABLE_FILTER=0
if [ $# -eq 2 ]; then
  ENABLE_FILTER=$2
fi

export PYTHONUNBUFFERED=1

WORK_DIR=${PWD}
echo "globle config..."

LLVM_PROJECT_DIR=~/llvm/toolchain/llvm-project
LLVM_CMAKE_DIR=${WORK_DIR}/cmake/Modules
TOOLCHAIN_BIN_PATH=~/llvm/prebuilts/clang/ohos/linux-x86_64/clang-15.0.4/bin

#sed the llvm cmake 
CMAKE_TOOLCHAIN_PATH=~/llvm/prebuilts/clang/ohos/linux-x86_64/clang-15.0.4/lib/cmake/llvm/LLVMExports.cmake
sed -i '1159,1176 s/FATAL_ERROR/WARNING/g' ${CMAKE_TOOLCHAIN_PATH}

HOS_TMPDIR="/data/local/tmp"
SYSROOT=~/llvm/out/sysroot
LLVM_RT_DIR=${WORK_DIR}/llvm-install/lib/clang/15.0.4
LLVM_CONFIG_PATH="${TOOLCHAIN_BIN_PATH}/llvm-config"

TARGET_TRIPLE="aarch64-linux-ohos"
EXTRA_FLAGS="-fno-emulated-tls"
DEVICE_COMPILE_FLAGS="--target=${TARGET_TRIPLE} --sysroot=${SYSROOT} -v ${EXTRA_FLAGS} -O0 -fuse-ld=lld -stdlib=libc++"
DEVICE_LINKER_FLAGS="-Wl,-rpath,${HOS_TMPDIR},--allow-shlib-undefined,--unresolved-symbols=ignore-all"
BUILD_TEST="ON"

export HDC=~/bin/hdc
export HDC_UTID=3ZF0124617000063
export HDC_SERVER_IP_PORT=100.103.89.xxx:8710
export HDC_CMD="$HDC -s $HDC_SERVER_IP_PORT -t $HDC_UTID"
export PHONE_SYMBOLIZER_PATH="/system/bin/llvm-symbolizer"
export HOS_RUN_VERBOSE=1

echo "cmake config..."
~/llvm/prebuilts/cmake/linux-x86/bin/cmake \
    ${LLVM_PROJECT_DIR}/compiler-rt \
    -G Ninja \
    -DCMAKE_SYSTEM_NAME=OHOS \
    -DOHOS=ON \
    -DCMAKE_AR="${TOOLCHAIN_BIN_PATH}/llvm-ar" \
    -DLLVM_CONFIG_PATH="${LLVM_CONFIG_PATH}" \
    -DCMAKE_ASM_COMPILER_TARGET="${TARGET_TRIPLE}" \
    -DCMAKE_ASM_FLAGS="${DEVICE_COMPILE_FLAGS}" \
    -DCMAKE_C_COMPILER="${TOOLCHAIN_BIN_PATH}/clang" \
    -DCMAKE_C_COMPILER_TARGET="${TARGET_TRIPLE}" \
    -DCMAKE_C_FLAGS="${DEVICE_COMPILE_FLAGS}" \
    -DCMAKE_CXX_COMPILER="${TOOLCHAIN_BIN_PATH}/clang++" \
    -DCMAKE_CXX_COMPILER_TARGET="${TARGET_TRIPLE}" \
    -DCMAKE_CXX_FLAGS="${DEVICE_COMPILE_FLAGS}" \
    -DCMAKE_EXE_LINKER_FLAGS="${DEVICE_COMPILE_FLAGS} ${DEVICE_LINKER_FLAGS}" \
    -DCOMPILER_RT_BUILD_BUILTINS=OFF \
    -DCOMPILER_RT_BUILD_LIBFUZZER=ON \
    -DCOMPILER_RT_BUILD_MEMPROF=OFF \
    -DCOMPILER_RT_BUILD_PROFILE=ON \
    -DCOMPILER_RT_BUILD_SANITIZERS=ON \
    -DCOMPILER_RT_BUILD_XRAY=OFF \
    -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
    -DCOMPILER_RT_TEST_STANDALONE_BUILD_LIBS=OFF \
    -DCOMPILER_RT_INCLUDE_TESTS="${BUILD_TEST}" \
    -DCOMPILER_RT_CAN_EXECUTE_TESTS="${BUILD_TEST}" \
    -DCOMPILER_RT_TEST_COMPILER="${TOOLCHAIN_BIN_PATH}/clang" \
    -DCOMPILER_RT_TEST_COMPILER_CFLAGS="${DEVICE_COMPILE_FLAGS}" \
    -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMPILER_RT_OUTPUT_DIR="${LLVM_RT_DIR}"

echo "cmake compile..."
ninja compiler-rt -j16

SAN_FILTER="${SANITIZER}" 
if [ "$SANITIZER" = "all" ]; then
    SAN_FILTER="*"
fi

echo "copy to llvm-install..."
CLANG_PATH="llvm-install/lib/clang/15.0.4/lib/aarch64-linux-ohos/"
for file in $(find ./${CLANG_PATH} -type f -name "*${SAN_FILTER}*.a*"); do
    echo "$file"
    cp "$file" ~/llvm/out/${CLANG_PATH}
done

echo "copy to phone..."
${HDC_CMD} target mount

for file in $(find ./${CLANG_PATH} -type f -name "*${SAN_FILTER}*.so"); do
    md5sum "$file"
    ${HDC_CMD} file send "$file" /system/lib64
    ${HDC_CMD} shell md5sum /system/lib64/"*${SAN_FILTER}*.so"
done

${HDC_CMD} file send ./llvm-symbolizer /system/bin
${HDC_CMD} shell chmod +x /system/bin/llvm-symbolizer
export LLVM_SYMBOLIZER_PATH="/system/bin/llvm-symbolizer"

# filter list for hwasan fails
if [ "$ENABLE_FILTER" = 1 ] && [ "$SANITIZER" = "hwasan" ]; then
    export LIT_FILTER_OUT="TestCases/Linux/reuse-threads.cpp\
|TestCases/double-free.c\
|TestCases/stack-uas.c\
|TestCases/deep-recursion.c\
|TestCases/halt-on-error.cpp\
|TestCases/heap-buffer-overflow-into.c\
|TestCases/hwasan_symbolize.cpp\
|TestCases/malloc_bisect.c\
|TestCases/mem-intrinsics.c\
|TestCases/print-module-map.c\
|TestCases/realloc-after-free.c\
|TestCases/tag_in_free.c\
|TestCases/uaf_with_rb_distance.c\
|TestCases/use-after-free.c"
fi

# change lit timeout
echo "change lit timeout..."
find ${LLVM_PROJECT_DIR} -name hdc.py -exec sed -i 's/timeout=[0-9]\+/timeout=60/g' {} \;  -print

echo "run $SANITIZER tests..."

ninja check-${SANITIZER} -v -j1
