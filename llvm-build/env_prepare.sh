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

linux_platform=linux-x86
darwin_platform=darwin-x86
download_url=https://mirrors.huaweicloud.com

# sync code directory
code_dir=$(pwd)

# downloaded files will be automatically unzipped under this path
bin_dir=${code_dir}/download_packages

# download and initialize prebuild files
if [ ! -d "${bin_dir}" ];then
    mkdir -p "${bin_dir}"
fi

DOWNLOADER="wget -t3 -T10 -O"

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
copy_config_linux_x86_64="""
prebuilts/cmake,cmake-${linux_platform}
prebuilts/clang/ohos/${host_platform}-${host_cpu},linux/clang_${linux_platform}
prebuilts/python3,python-${linux_platform}
prebuilts/build-tools/${host_platform}-x86/bin,gn-${linux_platform}
prebuilts/build-tools/${host_platform}-x86/bin,ninja-${linux_platform}
"""

copy_config_darwin_x86_64="""
prebuilts/cmake,cmake-${darwin_platform}
prebuilts/clang/ohos/${host_platform}-${host_cpu},darwin/clang_${darwin_platform}
prebuilts/python3,python-${darwin_platform}
prebuilts/build-tools/${host_platform}-x86/bin,gn-${darwin_platform}
prebuilts/build-tools/${host_platform}-x86/bin,ninja-${darwin_platform}
"""

copy_config_darwin_arm64="""
prebuilts/python3,python-${darwin_platform}
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
  keyword=$(echo $i|awk -F ',' '{print $2}')
  url_part=$(cat ./build/prebuilts_download_config.json | sort | uniq | grep "$keyword" | grep -oE '"/[^"]+"' | sed 's/^"//;s/"$//' |head -1)
  echo $url_part
  if [ -n "$url_part" ]; then
    full_url="${download_url}${url_part}"
  else
    echo "URL not found"
  fi
  download_and_archive "${unzip_dir}" "${full_url}"

done

linux_filename=$(cat ./build/prebuilts_download_config.json | sort | uniq | grep "clang_linux-x86" | grep -oE '"/[^"]+"' | sed 's/^"//;s/"$//')
CLANG_LINUX_BUILD=$(basename "$linux_filename" .tar.bz2)

darwin_filename=$(cat ./build/prebuilts_download_config.json | sort | uniq | grep "clang_darwin-x86" | grep -oE '"/[^"]+"' | sed 's/^"//;s/"$//')
CLANG_DARWIN_BUILD=$(basename "$darwin_filename" .tar.bz2)

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
