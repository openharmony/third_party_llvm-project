# Copyright (C) 2023 Huawei Device Co., Ltd.
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

set -e
set +x

while [[ $# -ge 1 ]]; do
    case $1 in
    -llvm-root)
        LLVM_PROJECT_DIR=$2
        shift 2
        ;;
    -use-module-cache)
        TEST_WITH_CACHE=$2
        shift 2
        ;;
    -hdc-path)
        HDC=$2
        shift 2
        ;;
    *)
        echo "Unknown option: $1"
        exit 1
        ;;
    esac
done

if [ -z $HDC ] || [ -z $LLVM_PROJECT_DIR ] || [ -z $TEST_WITH_CACHE ]; then
    echo "$0: Require options: [-hdc-path VALUE] [-llvm-path VALUE] [-use-module-cache VALUE]."
    exit 1
fi

LLVM_BUILD_PREFIX="${LLVM_PROJECT_DIR}/out"
LLVM_BUILD_LINUX_X86_PREFIX="${LLVM_BUILD_PREFIX}/llvm_make"
LLVM_BUILD_LINUX_X86_BIN="${LLVM_BUILD_LINUX_X86_PREFIX}/bin"

LLDB_TEST_PATH="${LLVM_PROJECT_DIR}/toolchain/llvm-project/lldb/test/API/lldb_time_test/"
LLDB_API_DOTEST="${LLVM_PROJECT_DIR}/toolchain/llvm-project/lldb/test/API/dotest.py"
LLDB_TEST_ROOT=${PWD}

SYSROOT="${LLVM_BUILD_PREFIX}/sysroot"

LLDB_API_DOTEST_ARCH="arm"
CONFIG="${LLDB_TEST_ROOT}/arm-linux-ohos.cfg"
TARGET="arm-linux-ohos"
CONFIG_OPT="--config ${CONFIG}"

REMOTE_PLATFORM_WORKDIR="/data/local/tmp/lldb_time_test"
LLDB_SERVER_BINARY_RELATIVE_TO_TOOLCHAIN_PATH="${LLVM_BUILD_PREFIX}/lib/lldb-server-${TARGET}/bin/lldb-server"
LLDB_SERVER_PORT_OHOS=12345
LLDB_API_REMOTE_PLATFORM_NAME_OHOS="remote-ohos"
LLDB_API_REMOTE_PLATFORM_URL_STRING_OHOS="connect://:${LLDB_SERVER_PORT_OHOS}"

if [ ! -f ${CONFIG} ]; then
    echo -e "--target=${LLDB_API_DOTEST_ARCH}-linux-ohos" >>${CONFIG}
    echo -e "--sysroot ${SYSROOT}" >>${CONFIG}
    echo -e "--static" >>${CONFIG}
fi

OHOS_DEVICE_SERIAL="$(${HDC} list targets)"
if [[ ! "${OHOS_DEVICE_SERIAL}" ]]; then
    # lldb will fail if phone serial will be with colon character in platform_url parameter.
    # It will fail because it expect tcp port number of lldb-server(for example 1234) on remote side,
    # and url like "main-provider.devops-spb.rnd.huawei.com:15121:1234" does not parse correctly.
    # The only way to workaround this is to set serial as environment variable and set only lldb-server
    # port without address in platform_url argument
    exit 1
fi

# restart lldb-server
function restart_lldb_server() {
    ${HDC} shell killall -9 lldb-server || true
    ${HDC} shell ps -A | grep lldb_server && (
        echo "lldb server wasn't killed"
        exit 1
    ) || true
    ${HDC} shell "rm -rf ${REMOTE_PLATFORM_WORKDIR} && mkdir ${REMOTE_PLATFORM_WORKDIR}"
    ${HDC} file send ${LLDB_SERVER_BINARY_RELATIVE_TO_TOOLCHAIN_PATH} ${REMOTE_PLATFORM_WORKDIR}
    ${HDC} shell chmod 777 ${REMOTE_PLATFORM_WORKDIR}/lldb-server
}

restart_lldb_server

${HDC} shell \
    ${REMOTE_PLATFORM_WORKDIR}/lldb-server \
    platform \
    --listen "*:${LLDB_SERVER_PORT_OHOS}" \
    --server >lldb_server.log 2>&1 &
hdc_lldb_server_pid=$!

export LD_LIBRARY_PATH="${LLVM_BUILD_LINUX_X86_PREFIX}/lib"

python_exe=${LLVM_PROJECT_DIR}/prebuilts/python3/linux-x86/3.10.2/bin/python3
if [ ! -f "${python_exe}" ]; then
    python_exe=python3
fi

set +e
${python_exe} \
    ${LLDB_API_DOTEST} \
    --arch ${LLDB_API_DOTEST_ARCH} \
    -u CXXFLAGS \
    -u CFLAGS \
    --env ARCHIVER=${LLVM_BUILD_LINUX_X86_BIN}/llvm-ar \
    --env OBJCOPY=${LLVM_BUILD_LINUX_X86_BIN}/llvm-objcopy \
    -v \
    --env ARFLAGS=rvU \
    -E "${CONFIG_OPT}" \
    --platform-name ${LLDB_API_REMOTE_PLATFORM_NAME_OHOS} \
    --platform-url ${LLDB_API_REMOTE_PLATFORM_URL_STRING_OHOS} \
    --platform-working-dir ${REMOTE_PLATFORM_WORKDIR} \
    --build-dir ${LLDB_TEST_ROOT}/build \
    --executable ${LLVM_BUILD_LINUX_X86_BIN}/lldb \
    --compiler ${LLVM_BUILD_LINUX_X86_BIN}/clang++ \
    --dsymutil ${LLVM_BUILD_LINUX_X86_BIN}/dsymutil \
    --lldb-libs-dir ${LD_LIBRARY_PATH} \
    --llvm-tools-dir ${LLVM_BUILD_LINUX_X86_BIN} \
    --env RUN_WITH_CACHE=${TEST_WITH_CACHE} \
    ${LLDB_TEST_PATH}

kill ${hdc_lldb_server_pid}
