#!/usr/bin/env bash
# NOTE: it is expected, that OHOS SDK is present on the build machine and git is installed

set -eu

function usage() {
cat << EOF
    ${0} --build-dir=<path_to_build_dir> --sdk-native=<path_to_sdk_native> [other]
    other opts might be:
    --runtime-core-branch=<branch for arkcompiler_runtime_core>
    --runtime-core-repo=<runtime core remote repo>
    --llvm-project-branch=<branch for third_party_llvm-project>
    --llvm-project-repo=<llvm project remote repo>
EOF
}

build_dir=
ohos_sdk_native=
runtime_core_branch="OpenHarmony_feature_20241108"
runtime_core_repo="https://gitee.com/openharmony/arkcompiler_runtime_core"
llvm_project_branch="2024_1127_llvm_ark_aot"
llvm_repo="https://gitee.com/openharmony/third_party_llvm-project"

while (( ${#} > 0 )); do
    opt="${1}"

    case "${opt}" in
        --build-dir=*)
            build_dir="${opt#"--build-dir="}";;
        --sdk-native=*)
            ohos_sdk_native="${opt#"--sdk-native="}";;
        --runtime-core-branch=*)
            runtime_core_branch="${opt#"--runtime-core-branch="}";;
        --runtime-core-repo=*)
            runtime_core_repo="${opt#"--runtime-core-repo="}";;
        --llvm-project-branch=*)
            llvm_project_branch="${opt#"--llvm-project-branch="}";;
        --llvm-project-repo=*)
            llvm_repo="${opt#"--llvm-project-repo="}";;
        *)
            usage
            exit 1
    esac
    shift
done

if [[ -z "${build_dir}" || -z "${ohos_sdk_native}" ]]; then
    usage
    exit 1
fi

# download arkcompiler sources
# for prebuilts script and build_llvm.sh
ARKCOMPILER_DIR="${build_dir}/arkcompiler_runtime_core"
git clone --depth 1 -b "${runtime_core_branch}" "${runtime_core_repo}" "${ARKCOMPILER_DIR}"

# Install prebuilts
# requires root !!!
bash ${ARKCOMPILER_DIR}/static_core/scripts/install-deps-ubuntu --install=dev --install=arm-all

# download llvm sources
LLVM_DIR="${build_dir}/llvm-project"
git clone --depth 1 -b "${llvm_project_branch}" "${llvm_repo}" "${LLVM_DIR}"

### Required variables
export BUILD_DIR=${build_dir}/build
export LLVM_SOURCES="${LLVM_DIR}/llvm"
export VERSION="15.0.4-ark18"
export PACKAGE_VERSION="${VERSION}" # must match REQUIRED_LLVM_VERSION in libllvmbackend/CMakeLists.txt

### Select targets to build, at least one must be set to "true"
export BUILD_X86_DEBUG=true
export BUILD_X86_RELEASE=true
export BUILD_AARCH64_DEBUG=true
export BUILD_AARCH64_RELEASE=true
export BUILD_OHOS_RELEASE=true
export BUILD_OHOS_RELEASE_GN=true

### Optional variables
export INSTALL_DIR="${build_dir}/install"

export DO_STRIPPING=true
export DO_TAR=true

# <sdk_root>/linux/native-linux-x64-5.0.2.52-Canary1/native
export OHOS_SDK="${ohos_sdk_native}"

export OHOS_PREBUILTS="${build_dir}"

# Hack: avoid dependency on OHOS prebuilts
# paths used in build_llvm.sh
ohos_prebuilts_bin="${OHOS_PREBUILTS}/clang/ohos/linux-x86_64/llvm"
mkdir -p "${ohos_prebuilts_bin}"
ln -s "${OHOS_SDK}/llvm/bin" "${ohos_prebuilts_bin}/bin"
ln -s "${OHOS_SDK}/llvm/lib" "${ohos_prebuilts_bin}/lib"

### Build tools
export CC="${OHOS_SDK}/llvm/bin/clang"
export CXX="${OHOS_SDK}/llvm/bin/clang++"
export STRIP="${OHOS_SDK}/llvm/bin/llvm-strip"
export OPTIMIZE_DEBUG=false

bash -x "${ARKCOMPILER_DIR}/static_core/scripts/llvm/build_llvm.sh"
