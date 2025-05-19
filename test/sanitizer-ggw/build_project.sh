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

echo "llvm-sanitizer：job start..."
source ${ROOT_DIR}/jenkins-ci/scripts/llvm_helpers.sh

#Environmental preparation
echo "llvm-sanitizer：Environmental preparation..."
sudo apt-get update
sudo apt-get -y install build-essential swig python3-dev libedit-dev libncurses5-dev binutils-dev gcc-multilib abigail-tools

echo "llvm-sanitizer：download_third_party_llvm..."
check_deps
prepare_git
download_third_party_llvm

# update llvm prebuilts
cd ${ROOT_DIR}
echo "llvm-sanitizer：env_prepare..."
./toolchain/llvm-project/llvm-build/env_prepare.sh

# add pr_number
cd "${ROOT_DIR}/toolchain/llvm-project"
if [[ -z "${PR_NUMBER}" || "${PR_NUMBER}" =~ ^\s*$ ]]; then
    echo "PR_NUMBER is empty or whitespace, skipping git operations."
else
    echo "PR_NUMBER = ${PR_NUMBER}"
    wget https://gitee.com/openharmony/third_party_llvm-project/pulls/${PR_NUMBER}.diff
    git apply ${PR_NUMBER}.diff
fi

# build llvm-project
echo "llvm-sanitizer：build..."
python3 ${ROOT_DIR}/toolchain/llvm-project/llvm-build/build.py --no-lto --no-build windows --enable-assertions --no-build-x86_64 --no-build-mipsel --no-build-arm --no-build-riscv64 --no-build windows,lldb-server,check-api 

# build compiler-rt
WORK_DIR=${PWD}
LLVM_PROJECT_DIR=${ROOT_DIR}/toolchain/llvm-project
TOOLCHAIN_BIN_PATH=${ROOT_DIR}/out/llvm-install/bin

HOS_TMPDIR="/data/local/tmp"
SYSROOT=${ROOT_DIR}/out/sysroot
LLVM_RT_DIR=${WORK_DIR}/llvm-install/lib/clang/15.0.4
LLVM_CONFIG_PATH="${TOOLCHAIN_BIN_PATH}/llvm-config"

TARGET_TRIPLE="aarch64-linux-ohos"
EXTRA_FLAGS="-fno-emulated-tls"
DEVICE_COMPILE_FLAGS="--target=${TARGET_TRIPLE} --sysroot=${SYSROOT} -v ${EXTRA_FLAGS} -O0 -fuse-ld=lld -stdlib=libc++"
DEVICE_LINKER_FLAGS="-Wl,-rpath,${HOS_TMPDIR},--allow-shlib-undefined -Wl,--unresolved-symbols=ignore-all"

mkdir -p $ROOT_DIR/test-build
cd $ROOT_DIR/test-build

echo "llvm-sanitizer：cmake config compiler-rt..."
${ROOT_DIR}/prebuilts/cmake/linux-x86/bin/cmake \
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
    -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
    -DCOMPILER_RT_BUILD_MEMPROF=OFF \
    -DCOMPILER_RT_BUILD_PROFILE=ON \
    -DCOMPILER_RT_BUILD_SANITIZERS=ON \
    -DCOMPILER_RT_BUILD_XRAY=OFF \
    -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
    -DCOMPILER_RT_TEST_STANDALONE_BUILD_LIBS=OFF \
    -DCOMPILER_RT_INCLUDE_TESTS="ON" \
    -DCOMPILER_RT_CAN_EXECUTE_TESTS="ON" \
    -DCOMPILER_RT_TEST_COMPILER="${TOOLCHAIN_BIN_PATH}/clang" \
    -DCOMPILER_RT_TEST_COMPILER_CFLAGS="${DEVICE_COMPILE_FLAGS}" \
    -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMPILER_RT_OUTPUT_DIR="${LLVM_RT_DIR}"

echo "llvm-sanitizer：cmake compile compiler-rt..."
ninja compiler-rt
