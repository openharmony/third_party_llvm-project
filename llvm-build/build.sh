#!/bin/bash
WORK_DIR=$(pwd)
LLVM_DIR=${WORK_DIR}/toolchain/llvm-project

# Update PATCH_MAP to only contain the desired patch
PATCH_MAP=(
    "${WORK_DIR}/toolchain/llvm-project/llvm-build/0001-adapt-build-for-llvm16.patch:${WORK_DIR}/build"
)

apply_patch() {
    local patch_file="$1"
    local target_dir="$2"
    
    # check patch
    if [ ! -f "$patch_file" ]; then
        echo "error: $patch_file not exit!!!!" >&2
        return 1
    fi
    if [ ! -d "$target_dir" ]; then
        echo "error: $target_dir not exit!!!" >&2
        return 1
    fi

    # check patch apply status
    if (cd "$target_dir" && git apply --check "$patch_file" &>/dev/null); then
        echo "$patch_file can be applied  $target_dir"
        echo "begin apply patch"
        if (cd "$target_dir" && git apply "$patch_file"); then
            echo "$patch_file apply success"
            return 0
        else
            echo "error: $patch_file apply failed" >&2
            return 1
        fi
    else
        echo "skip: $patch_file may have already been applied or there may be conflicts"
        return 1
    fi
}

build_llvm() {
    export PATH=${WORK_DIR}/prebuilts/python3/linux-x86/3.11.4/bin:$PATH
    rm -rf ${WORK_DIR}/out
    /usr/bin/python3 ${WORK_DIR}/toolchain/llvm-project/llvm-build/build.py --no-build-riscv64 --no-build-loongarch64 --no-build-mipsel --no-build lldb-server
}

main() {
    for entry in "${PATCH_MAP[@]}"; do
        IFS=':' read -r patch_file target_dir <<< "$entry"
        echo "------------------------------"
        apply_patch "$patch_file" "$target_dir"
    done
    build_llvm
}

main
