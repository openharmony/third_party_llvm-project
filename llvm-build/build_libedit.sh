#!/bin/bash
set -xe

# This script is used to create libedit

SCRIPT_PATH="${PWD}"
LIBEDIT_SRC_DIR=$1
LIBEDIT_BUILD_PATH=$2
LIBEDIT_INSTALL_PATH=$3
NCURSES_PATH=$4
PREBUILT_PATH=$5
CLANG_VERSION=$6
TARGET=$7
LIBEDIT_UNTAR_PATH=$8

# Get version and date form libedit.spec (Compatible with Linux and Mac)
# The format in the libedit.spec is as follows:
# Version:	3.1
# %global _date 20210910
SPECFILE="${LIBEDIT_SRC_DIR}/libedit.spec"
LIBEDIT_VERSION=$(grep -E '^Version:*' ${SPECFILE} | awk '{print $2}')
DATE=$(grep -E '^%global _date' ${SPECFILE} | sed 's/^.*\(20[0-9]\{6\}\).*$/\1/')

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

CC_PATH=${PREBUILT_PATH}/clang/ohos/${host_platform}-${host_cpu}/clang-${CLANG_VERSION}/bin/clang
CXX_PATH=${PREBUILT_PATH}/clang/ohos/${host_platform}-${host_cpu}/clang-${CLANG_VERSION}/bin/clang++

libedit_package=${LIBEDIT_SRC_DIR}/libedit-${DATE}-${LIBEDIT_VERSION}.tar.gz
if [ -e ${libedit_package} ]; then
    if [ ! -b ${LIBEDIT_UNTAR_PATH} ]; then
        mkdir -p ${LIBEDIT_UNTAR_PATH}
    fi
    tar -xzvf ${libedit_package} --strip-components 1 -C ${LIBEDIT_UNTAR_PATH}
    cd ${LIBEDIT_UNTAR_PATH}

    if [ ! -b ${LIBEDIT_BUILD_PATH} ]; then
        mkdir -p ${LIBEDIT_BUILD_PATH}
    fi
    patches=($(grep -E '^Patch[0-9]+:' "${SPECFILE}" | sed 's/^[^:]*: *//'))
    # Apply patches in order
    for patch in "${patches[@]}"
    do
        patch -Np1 < ${LIBEDIT_SRC_DIR}/$patch
    done

    # build libedit
    cd ${LIBEDIT_BUILD_PATH}
    ohos_suffix='-ohos'
    stack_flags="-fstack-protector-strong"
    got_ldflags="-Wl,-z,relro,-z,now"
    if [[  ${TARGET} != *${ohos_suffix} ]]; then
        ldflags="-L${NCURSES_PATH}/lib"
        ncuses_flags="-I${NCURSES_PATH}/include"
        if [ "${host_platform}" = "darwin" ]; then
            ncurses_libs="-Wl,-rpath,@loader_path/../lib:${NCURSES_PATH}/lib"
            SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
            sdk_flags="-I${SDKROOT}/usr/include"
            export LDFLAGS="$LDFLAGS $sdk_flags $ldflags $ncurses_libs $got_ldflags"
            export CFLAGS="$CFLAGS -isysroot$SDKROOT $ncuses_flags $stack_flags"
        fi

        if [ "${host_platform}" = "linux" ]; then
            ncurses_libs="-Wl,-rpath,\$$ORIGIN/../lib:${NCURSES_PATH}/lib"
            export LDFLAGS="$LDFLAGS $ldflags $ncuses_flags $ncurses_libs $got_ldflags"
            export CFLAGS="$CFLAGS $ncuses_flags $stack_flags"
        fi

        ${LIBEDIT_UNTAR_PATH}/configure \
            --prefix=${LIBEDIT_INSTALL_PATH} \
            CC=${CC_PATH} \
            CXX=${CXX_PATH}
        make -j$(nproc --all) install | tee build_libedit.log
    else
        C_FLAGS="-I${NCURSES_PATH}/include/ -I${NCURSES_PATH}/include/ncurses -D__STDC_ISO_10646__=201103L -fPIC"
        if [[ ${TARGET} =~ 'arm' ]]; then
            C_FLAGS="$C_FLAGS -march=armv7-a -mfloat-abi=soft"
        fi
        C_FLAGS="$C_FLAGS $stack_flags"
        export LDFLAGS="$LDFLAGS $got_ldflags"
        ${LIBEDIT_UNTAR_PATH}/configure \
            --prefix=${LIBEDIT_INSTALL_PATH} \
            --host="${TARGET}" \
            CC="${PREBUILT_PATH}/../out/llvm-install/bin/clang --target=${TARGET}" \
            CFLAGS="${C_FLAGS}" \
            LDFLAGS="-L${NCURSES_PATH}/lib"

        make -j$(nproc --all) install | tee build_libedit_${TARGET}.log
    fi
fi


