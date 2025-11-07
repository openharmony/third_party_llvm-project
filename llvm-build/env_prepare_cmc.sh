set -x
case $(uname -s) in
    Linux)

        host_platform=linux
        ;;
    Darwin)
        host_platform=darwin
        ;;
    *)
        echo "Unsupported host platform: $(uname -s)"
        exit 1
esac

case $(uname -m) in
    arm64)

        host_cpu=arm64
        ;;
    *)
        host_cpu=x86_64
esac

# sync code directory
code_dir=$(pwd)
if [ ! -d "${code_dir}/prebuilts/clang/ohos" ]; then
  mkdir -p "${code_dir}/prebuilts/clang/ohos"
fi

if [ -d "${code_dir}/prebuilts/clang/ohos-sdk/linux-x86_64" ] && [ ${host_platform} == "linux" ]; then
    SET_CLANG_VERSION='15.0.4'
    mv "${code_dir}/prebuilts/clang/ohos-sdk/linux-x86_64" "${code_dir}/prebuilts/clang/ohos/linux-x86_64"
    ln -s "${code_dir}/prebuilts/clang/ohos/linux-x86_64/llvm" "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-${SET_CLANG_VERSION}"
fi

if [ ${host_platform} == "darwin" ]; then
    if [ -d "${code_dir}/prebuilts/clang/ohos-sdk/darwin-x86_64" ] && [ ${host_cpu} == "x86_64" ]; then
        SET_CLANG_VERSION='15.0.4'
        mv "${code_dir}/prebuilts/clang/ohos-sdk/darwin-x86_64" "${code_dir}/prebuilts/clang/ohos/darwin-x86_64"
        ln -s "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/llvm" "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang-${SET_CLANG_VERSION}"
    fi

    if [ -d "${code_dir}/prebuilts/clang/ohos-sdk/darwin-arm64" ] && [ ${host_cpu} == "arm64" ]; then
        SET_CLANG_VERSION='15.0.4'
        mv "${code_dir}/prebuilts/clang/ohos-sdk/darwin-arm64" "${code_dir}/prebuilts/clang/ohos/darwin-arm64"
        mv "${code_dir}/prebuilts/build-tools/darwin-arm" "${code_dir}/prebuilts/build-tools/darwin-arm64"
        ln -s "${code_dir}/prebuilts/clang/ohos/darwin-arm64/llvm" "${code_dir}/prebuilts/clang/ohos/darwin-arm64/clang-${SET_CLANG_VERSION}" 
    fi
fi 

if [ -d "${code_dir}/prebuilts/python/python_llvm" ]; then
    mv "${code_dir}/prebuilts/python/python_llvm" "${code_dir}/prebuilts/python3"
fi

# try to detect version ...
BASE_CLANG_DIR="${code_dir}/prebuilts/clang/ohos/${host_platform}-${host_cpu}"
CLANG_FOUND_VERSION=$(cd ${BASE_CLANG_DIR}; basename $(ls -d clang*/ | head -1) | sed s/clang-//)

# check that pipe above didn't fail and that we have (any) clang version
if [ ! $? == 0 ] || [ x == x${CLANG_FOUND_VERSION} ] ; then
    echo "env_prepare.sh: could not detect clang version" 2>&1
    exit 1
fi
# ... and compare it with one in python file
echo "prebuilts_clang_version='${CLANG_FOUND_VERSION}'" | diff -q - $(dirname $0)/prebuilts_clang_version.py || ( echo Clang versions mismatch ; exit 1 )
exit 0
