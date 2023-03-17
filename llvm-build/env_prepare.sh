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

# downloaded files will be automatically unzipped under this path
bin_dir=${code_dir}/download_packages

# download and initialize prebuild files
if [ ! -d "${bin_dir}" ];then
    mkdir -p "${bin_dir}"
fi


function download_and_archive() {
    archive_dir=$1
    download_source_url=$2
    bin_file=$(basename ${download_source_url})
    wget -t3 -T10 -O "${bin_dir}/${bin_file}" "${download_source_url}"
    if [ ! -d "${code_dir}/${archive_dir}" ];then
        mkdir -p "${code_dir}/${archive_dir}"
    fi
    if [ "X${bin_file:0-3}" = "Xzip" ];then
        unzip -o "${bin_dir}/${bin_file}" -d "${code_dir}/${archive_dir}/"
    elif [ "X${bin_file:0-6}" = "Xtar.gz" ];then
        tar -xvzf "${bin_dir}/${bin_file}" -C "${code_dir}/${archive_dir}"
    else
        tar -xvf "${bin_dir}/${bin_file}" -C "${code_dir}/${archive_dir}"
    fi
}


copy_config="""
"""

copy_config_linux_x86_64="""
prebuilts/cmake,https://mirrors.huaweicloud.com/harmonyos/compiler/cmake/3.16.5/${host_platform}/cmake-${host_platform}-x86-3.16.5.tar.gz
prebuilts/clang/ohos/${host_platform}-${host_cpu},https://mirrors.huaweicloud.com/openharmony/compiler/clang/10.0.1-62608/${host_platform}/llvm.tar.gz
prebuilts/clang/ohos/${host_platform}-${host_cpu},https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.1/clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
"""


copy_config_darwin_x86_64="""
prebuilts/cmake,https://mirrors.huaweicloud.com/harmonyos/compiler/cmake/3.16.5/${host_platform}/cmake-${host_platform}-x86-3.16.5.tar.gz
prebuilts/clang/ohos/${host_platform}-${host_cpu},https://github.com/llvm/llvm-project/releases/download/llvmorg-12.0.0/clang+llvm-12.0.0-x86_64-apple-darwin.tar.xz
"""


if [[ "${host_platform}" == "linux" ]]; then
    if [[ "${host_cpu}" == "x86_64" ]]; then
        copy_config+=${copy_config_linux_x86_64}
        echo "add ubuntu here"
    else
        echo "unknwon host_cpu - ${host_cpu} for linux"
    fi
elif [[ "${host_platform}" == "darwin" ]]; then
    if [[ "${host_cpu}" == "x86_64" ]]; then
        copy_config+=${copy_config_darwin_x86_64}
        echo "add x86-64 mac here"
    elif [[ "${host_cpu}" == "arm64" ]]; then
        echo "add m1 config here"
    else
        echo "unknwon host_cpu - ${host_cpu} for darwin"
    fi
else
    echo "unknown ${host_platform}"
fi


for i in $(echo ${copy_config})
do
    unzip_dir=$(echo $i|awk -F ',' '{print $1}')
    remote_url=$(echo $i|awk -F ',' '{print $2}')
    download_and_archive "${unzip_dir}" "${remote_url}"
done


if [ -d "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-62608" ];then
    rm -rf "${code_dir}/prebuilts/clang/ohos/linux-x86_64/llvm"
    mv "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-62608" "${code_dir}/prebuilts/clang/ohos/linux-x86_64/llvm"
    ln -snf 10.0.1 "${code_dir}/prebuilts/clang/ohos/linux-x86_64/llvm/lib/clang/current"
fi


if [ -d "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04" ];then
    rm -rf "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-10.0.1"
    mv "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang+llvm-10.0.1-x86_64-linux-gnu-ubuntu-16.04" "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-10.0.1"
fi

if [ -d "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang+llvm-12.0.0-x86_64-apple-darwin" ];then
    rm -rf "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang-10.0.1"
    mv "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang+llvm-12.0.0-x86_64-apple-darwin" "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang-10.0.1"
fi
