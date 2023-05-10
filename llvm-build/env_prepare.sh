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

DOWNLOADER="wget -t3 -T10 -O "

function download_and_archive() {
    archive_dir=$1
    download_source_url=$2
    bin_file=$(basename ${download_source_url})
    ${DOWNLOADER} "${bin_dir}/${bin_file}" "${download_source_url}"
    if (( $? )) ; then
        echo "Failed to download ${download_source_url}" 2>& 1
        exit 1
    fi
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
    if (( $? )) ; then
        echo "Failed to unpack ${bin_dir}/${bin_file}, corrupted?" 2>& 1
        exit 1
    fi
}


copy_config="""
"""

CLANG_LINUX_BUILD=clang_linux-x86_64-025192-20230330
copy_config_linux_x86_64="""
prebuilts/cmake,https://mirrors.huaweicloud.com/harmonyos/compiler/cmake/3.16.5/${host_platform}/cmake-${host_platform}-x86-3.16.5.tar.gz
prebuilts/clang/ohos/${host_platform}-${host_cpu},https://mirrors.huaweicloud.com/openharmony/compiler/clang/15.0.4-025192/linux/${CLANG_LINUX_BUILD}.tar.bz2
prebuilts/python3,https://mirrors.huaweicloud.com/harmonyos/compiler/python/3.10.2/${host_platform}/python-${host_platform}-x86-3.10.2_20230604.tar.gz
"""


CLANG_DARWIN_BUILD=clang_darwin-x86_64-8e906c-20230415
copy_config_darwin_x86_64="""
prebuilts/cmake,https://mirrors.huaweicloud.com/harmonyos/compiler/cmake/3.16.5/${host_platform}/cmake-${host_platform}-x86-3.16.5.tar.gz
prebuilts/clang/ohos/${host_platform}-${host_cpu},http://mirrors.huaweicloud.com/harmonyos/compiler/clang/15.0.4-8e906c/darwin/${CLANG_DARWIN_BUILD}.tar.bz2
prebuilts/python3,https://mirrors.huaweicloud.com/harmonyos/compiler/python/3.10.2/${host_platform}/python-${host_platform}-x86-3.10.2_20230604.tar.gz
"""

copy_config_darwin_arm64="""
prebuilts/python3,https://mirrors.huaweicloud.com/harmonyos/compiler/python/3.10.2/${host_platform}/python-${host_platform}-x86-3.10.2_20230604.tar.gz
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
        copy_config+=${copy_config_darwin_arm64}
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


if [ -d "${code_dir}/prebuilts/clang/ohos/linux-x86_64/${CLANG_LINUX_BUILD}" ]; then
    SET_CLANG_VERSION='15.0.4'
    mv "${code_dir}/prebuilts/clang/ohos/linux-x86_64/${CLANG_LINUX_BUILD}" "${code_dir}/prebuilts/clang/ohos/linux-x86_64/clang-${SET_CLANG_VERSION}"
fi

if [ -d "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/${CLANG_DARWIN_BUILD}" ]; then
    SET_CLANG_VERSION='15.0.4'
    mv "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/${CLANG_DARWIN_BUILD}" "${code_dir}/prebuilts/clang/ohos/darwin-x86_64/clang-${SET_CLANG_VERSION}"
fi

# try to detect version ...
BASE_CLANG_DIR="${code_dir}/prebuilts/clang/ohos/${host_platform}-x86_64"
CLANG_FOUND_VERSION=$(cd ${BASE_CLANG_DIR}; basename $(ls -d clang*/ | head -1) | sed s/clang-//)

# check that pipe above didn't fail and that we have (any) clang version
if [ ! $? == 0 ] || [ x == x${CLANG_FOUND_VERSION} ] ; then 
    echo "env_prepare.sh: could not detect clang version" 2>&1
    exit 1
fi
# ... and compare it with one in python file
echo "prebuilts_clang_version='${CLANG_FOUND_VERSION}'" | diff -q - $(dirname $0)/prebuilts_clang_version.py || ( echo Clang versions mismatch ; exit 1 )
exit 0
