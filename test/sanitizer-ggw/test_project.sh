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

cd $ROOT_DIR/test-build

# replace unstripped hdc for coredump to debug segv issue
rm -f /opt/ohos-sdk/toolchains/hdc
curl https://nexus-cn-yz.devops-spb.rnd.x.com/repository/devops-binaries/hdc.unstripped.x86_64 -o /opt/ohos-sdk/toolchains/hdc
chmod a+x /opt/ohos-sdk/toolchains/hdc
ulimit -c 100000000

# prepare hdc env
HDC_SERVER_IP_PORT="${HDC_OPTS#*-s }"
export HDC="hdc"
export HDC_SERVER_IP_PORT="${HDC_SERVER_IP_PORT%% -t*}"
export HDC_UTID="${HDC_OPTS#*-t }"
echo $HDC_SERVER_IP_PORT $HDC_UTID $HDC_OPTS

hdc ${HDC_OPTS} file send ${ROOT_DIR}/llvm-symbolizer /system/bin
hdc ${HDC_OPTS} shell chmod +x /system/bin/llvm-symbolizer

export PYTHONUNBUFFERED=1
export PHONE_SYMBOLIZER_PATH="/system/bin/llvm-symbolizer"
export LLVM_SYMBOLIZER_PATH="/system/bin/llvm-symbolizer"
export HWASAN_SYMBOLIZER_PATH="/system/bin/llvm-symbolizer"

echo "llvm-sanitizer：copy to llvm-install..."
CLANG_PATH="llvm-install/lib/clang/15.0.4/lib/aarch64-linux-ohos/"
for file in $(find $ROOT_DIR/test-build/${CLANG_PATH} -type f -name "*.a*"); do
    echo "$file"
    cp "$file" ${ROOT_DIR}/out/${CLANG_PATH}
done    

echo "llvm-sanitizer：copy to phone..."
hdc ${HDC_OPTS} target mount

for file in $(find $ROOT_DIR/test-build/out/${CLANG_PATH} -type f -name "*.so"); do
    echo "$file"
    hdc ${HDC_OPTS} file send "$file" /system/lib64
done

# change lit timeout
echo "llvm-sanitizer：change lit timeout..."
find ${ROOT_DIR}/toolchain/llvm-project/compiler-rt/test -name hdc.py -exec sed -i 's/timeout=[0-9]\+/timeout=60/g' {} \;  -print

set +e
# run TESTS
for TEST in "cfi" "gwp_asan" "hwasan" "asan" "ubsan" "tsan"; do    
    ninja check-$TEST -v || true
    [[ -f core ]] && mv core core.$TEST
done
set -e

# # libomp prepare
# DEVICE_COMPILE_FLAGS="--target=aarch64-linux-ohos  --sysroot=${ROOT_DIR}/out/sysroot -v -fstack-protector-strong -ffunction-sections -fdata-sections  -D__MUSL__"
# TARGET_TRIPLE="aarch64-linux-ohos"
# TOOLCHAIN_BIN_PATH="${ROOT_DIR}/out/llvm-install/bin"
# CMAKE_BIN_PATH="${ROOT_DIR}/prebuilts/cmake/linux-x86/bin"
# CMAKE_SOURCE_PATH="${ROOT_DIR}/toolchain/llvm-project/openmp"
# CMAKE_TARGET_PATH="${ROOT_DIR}/toolchain/omp_test/test"
# OMP_LIB_PATH="${ROOT_DIR}/out/llvm-install/lib/aarch64-linux-ohos/libomp.so"

# mkdir -p "$CMAKE_TARGET_PATH"

# ${CMAKE_BIN_PATH}/cmake -G Ninja ${CMAKE_SOURCE_PATH}/   \
#             -B ${CMAKE_TARGET_PATH} \
#             -DCMAKE_SYSTEM_NAME=OHOS \
#             -DOHOS=ON \
#             -DCMAKE_AR="${TOOLCHAIN_BIN_PATH}/llvm-ar" \
#             -DCMAKE_C_COMPILER="${TOOLCHAIN_BIN_PATH}/clang" \
#             -DOPENMP_ENABLE_LIBOMPTARGET="FALSE"  \
#             -DCMAKE_CXX_COMPILER="${TOOLCHAIN_BIN_PATH}/clang++" \
#             -DLLVM_TARGETS_TO_BUILD=AArch64 \
#             -DOPENMP_LLVM_TOOLS_DIR="${ROOT_DIR}/out/llvm_make/bin" \
#             -DOPENMP_LLVM_LIT_EXECUTABLE="${ROOT_DIR}/out/llvm_make/bin/llvm-lit" \
#             -DCMAKE_C_COMPILER_TARGET="aarch64-linux-ohos" \
#             -DCMAKE_CXX_COMPILER_TARGET="aarch64-linux-ohos" \
#             -DCMAKE_CXX_FLAGS="${DEVICE_COMPILE_FLAGS}" \
#             -DCMAKE_C_FLAGS="${DEVICE_COMPILE_FLAGS}" \
#             -DCMAKE_EXE_LINKER_FLAGS="${DEVICE_LINKER_FLAGS}" \
#             -DCMAKE_ASM_FLAGS="${DEVICE_COMPILE_FLAGS}" \
#             -DCMAKE_ASM_COMPILER_TARGET="${TARGET_TRIPLE}" \
#             -DCMAKE_BUILD_TYPE=Release
        
# export LIB_PATH_ON_TARGET="/data/local/temp/lib"
# export OPENMP_STATIC_TEST="True"

# export LIT_FILTER_OUT="libarcher/*"

# # Push libomp.so to device
# hdc ${HDC_OPTS} shell mkdir $LIB_PATH_ON_TARGET/
# hdc ${HDC_OPTS} file send ${OMP_LIB_PATH} $LIB_PATH_ON_TARGET/

# # Run libomp test
# LOG_DATE=$(date +%Y%m%d_%H%M%S)
# cd ${CMAKE_TARGET_PATH}
# ninja check-openmp | tee ${CMAKE_TARGET_PATH}/../ohos-check-openmp${LOG_DATE}.log