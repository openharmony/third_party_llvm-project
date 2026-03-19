#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2020-2030. All rights reserved.
set -e

while getopts "o:i:l:t:h" arg
do
    case "${arg}" in
        "o")
            OUT_DIR=${OPTARG}
            ;;
        "i")
            SOURCE_DIR=${OPTARG}
            ;;
        "l")
            TOOL_DIR=${OPTARG}
            ;;
        "h")
            echo "help"
            ;;
        ?)
            echo "unkonw argument"
            exit 1
            ;;
    esac
done


ndk_dir=$(ls ${SOURCE_DIR}/lib/|more |grep ohos |grep -v mipsel |grep -v riscv |grep -v arm|grep -v loongarch64|awk '{print $NF}')

for i in $ndk_dir
do
	find ${OUT_DIR}/lib/$i -name 'libc++.a' -type f  -exec bash -c 'echo "INPUT(-lc++_static -lc++abi)" > $1' _ {}  \;
	find ${OUT_DIR}/lib/$i -name 'libc++.so' -type f -exec bash -c 'echo "INPUT(-lc++_shared)" > $1'  _ {}  \;
done

cp -rfp ${SOURCE_DIR}/lib/*   ${OUT_DIR}
cp -rfp ${SOURCE_DIR}/include/*  ${OUT_DIR}

function strip_dir() {
    for file in `ls $1`
    do
        if [ -f $1"/"$file ] && [[ $(file -b $1"/"$file) =~ "shared object" ]]
        then
            echo $1"/"$file
            ${TOOL_DIR}/llvm-strip $1"/"$file
        elif [ -d $1"/"$file ]
        then
            strip_dir $1"/"$file
        fi
    done
}
function remove_unnecessary_file {
    file_list=("bin/llvm-ifs" "bin/llvm-ifs.exe" "lib/clang/current" "lib/arm-liteos-ohos" "lib/loongarch64-linux-ohos" "lib/mipsel-linux-ohos" "lib/riscv64-linux-ohos" "lib/clang/15.0.4/bin/loongarch64-linux-ohos" "lib/clang/15.0.4/lib/arm-liteos-ohos" "lib/clang/15.0.4/lib/i386-unknown-linux-gnu" "lib/clang/15.0.4/lib/loongarch64-linux-ohos" "lib/clang/15.0.4/lib/mipsel-linux-ohos" "lib/clang/15.0.4/lib/riscv64-linux-ohos" "lib/clang/15.0.4/lib/windows")

    for i in "${file_list[@]}"; do
        if [ -e "${OUT_DIR}/${i}" ]; then
            rm -rf "${OUT_DIR}/${i}"
        fi
    done
}
remove_unnecessary_file
strip_dir ${OUT_DIR}/lib
