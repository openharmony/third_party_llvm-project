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

if [ -d ${LIBEDIT_SRC_DIR} ]; then
    cd ${LIBEDIT_SRC_DIR}
    cp -p config.sub config.sub.bak

    if [ "${host_platform}" = "linux" ]; then
        autoreconf -vfi
        mv -f config.sub.bak config.sub
    fi

    if [ ! -d ${LIBEDIT_BUILD_PATH} ]; then
        mkdir -p ${LIBEDIT_BUILD_PATH}
    fi

    # build libedit
    cd ${LIBEDIT_BUILD_PATH}
    ohos_suffix='-ohos'
    stack_flags="-fstack-protector-strong"
    got_ldflags="-Wl,-z,relro,-z,now -Wl,-z,noexecstack"
    if [[  ${TARGET} != *${ohos_suffix} ]]; then
        ldflags="-L${NCURSES_PATH}/lib"
        ncuses_flags="-I${NCURSES_PATH}/include"
        if [ "${host_platform}" = "darwin" ]; then
            ncurses_libs="-Wl,-rpath,@loader_path/../lib:${NCURSES_PATH}/lib"
            SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
            sdk_flags="-I${SDKROOT}/usr/include"
            export LDFLAGS="$LDFLAGS $sdk_flags $ldflags $ncurses_libs"
            export CFLAGS="$CFLAGS -isysroot$SDKROOT $ncuses_flags $stack_flags"
        fi

        if [ "${host_platform}" = "linux" ]; then
            ncurses_libs="-Wl,-rpath,\$$ORIGIN/../lib:${NCURSES_PATH}/lib"
            export LDFLAGS="$LDFLAGS $ldflags $ncuses_flags $ncurses_libs $got_ldflags"
            export CFLAGS="$CFLAGS $ncuses_flags $stack_flags"
        fi

        ${LIBEDIT_SRC_DIR}/configure \
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
        ${LIBEDIT_SRC_DIR}/configure \
            --prefix=${LIBEDIT_INSTALL_PATH} \
            --host="${TARGET}" \
            CC="${PREBUILT_PATH}/../out/llvm-install/bin/clang --target=${TARGET}" \
            CFLAGS="${C_FLAGS}" \
            LDFLAGS="-L${NCURSES_PATH}/lib $got_ldflags"

        make -j$(nproc --all) install | tee build_libedit_${TARGET}.log
    fi
    cd ${LIBEDIT_SRC_DIR}
    git reset --hard HEAD
    git clean -df
fi
