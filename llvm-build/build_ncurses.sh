#!/bin/bash
set -xe

# This script is used to create ncurses

SCRIPT_PATH="${PWD}"
NCURSES_SRC_DIR=$1
NCURSES_BUILD_PATH=$2
NCURSES_INSTALL_PATH=$3
PREBUILT_PATH=$4
CLANG_VERSION=$5
NCURSES_VERSION=$6
TARGET=$7
IS_STATIC=$8

SPECFILE="${NCURSES_SRC_DIR}/ncurses.spec"

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

if [ -d ${NCURSES_SRC_DIR} ]; then
    cd ${NCURSES_SRC_DIR}

    # Get the list of patch files for ncurses.spec
    # The format in the ncurses.spec is as follows:
    # Patch8:        ncurses-config.patch
    # Patch9:        ncurses-libs.patch
    # Patch11:       ncurses-urxvt.patch
    patches=($(grep -E '^Patch[0-9]+:' "${SPECFILE}" | sed 's/^[^:]*: *//'))
    # Apply patches in order
    for patch in "${patches[@]}"
    do
        patch -Np1 < ${NCURSES_SRC_DIR}/$patch
    done

    if [ ! -d ${NCURSES_BUILD_PATH} ]; then
        mkdir -p ${NCURSES_BUILD_PATH}
    fi
    cd ${NCURSES_BUILD_PATH}
    # build ncurses
    ohos_suffix='-ohos'
    stack_flags="-fstack-protector-strong"
    got_ldflags="-Wl,-z,relro,-z,now -Wl,-z,noexecstack"
    if [[ ${7} != *${ohos_suffix} ]]; then
        if [ "${host_platform}" == "darwin" ]; then
            export LDFLAGS="-Wl,-rpath,@loader_path/../lib"
            SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
            flags="-Wl,-syslibroot,${SDKROOT}"
            export CPPFLAGS="$CPPFALGS -I${SDKROOT}/usr/include -I${SDKROOT}/usr/include/i368"
            export CFLAGS="$CFLAGS -isysroot${SDKROOT} $flags $stack_flags"

            ${NCURSES_SRC_DIR}/configure \
                --with-shared \
                --with-terminfo-dirs=${NCURSES_INSTALL_PATH}/share/terminfo:/usr/lib/terminfo:/lib/terminfo:/usr/share/terminfo \
                --disable-mixed-case \
                --disable-widec \
                --prefix=${NCURSES_INSTALL_PATH} \
                CC=${CC_PATH} \
                CXX=${CXX_PATH}
            make -j$(nproc --all) install | tee build_ncurses.log
        fi
        if [ "${host_platform}" == "linux" ]; then
            export LDFLAGS="-Wl,-rpath,\$$ORIGIN/../lib $got_ldflags"
            export CFLAGS="$CFLAGS $stack_flags"
            ${NCURSES_SRC_DIR}/configure \
                --with-shared \
                --disable-widec \
                --prefix=${NCURSES_INSTALL_PATH} \
                --datadir=${NCURSES_INSTALL_PATH}/share \
                --with-terminfo-dirs=${NCURSES_INSTALL_PATH}/share/terminfo:/usr/lib/terminfo:/lib/terminfo:/usr/share/terminfo \
                CC=${CC_PATH} \
                CXX=${CXX_PATH}
            make -j$(nproc --all) install | tee build_ncurses.log
        fi
    else
        C_FLAGS="--target=${TARGET} -fPIC"
        if [[ ${TARGET} =~ 'arm' ]]; then
            C_FLAGS="$C_FLAGS -march=armv7-a -mfloat-abi=soft"
        fi
        EXTRA_ARGS=""
        C_FLAGS="$C_FLAGS $stack_flags"
        export LDFLAGS="$LDFLAGS $got_ldflags"
        if [[ ${IS_STATIC} == "static" ]]; then
            NCURSES_HOST_INSTALL_PATH=${9}
            export LD_LIBRARY_PATH="${NCURSES_HOST_INSTALL_PATH}/lib:$LD_LIBRARY_PATH"
            EXTRA_ARGS="--with-fallbacks=linux,vt100,xterm \
                        --with-tic-path=${NCURSES_HOST_INSTALL_PATH}/bin/tic \
                        --with-infocmp-path=${NCURSES_HOST_INSTALL_PATH}/bin/infocmp"
        fi
        ${NCURSES_SRC_DIR}/configure \
            --host="${TARGET}" \
            --with-shared \
            --prefix=${NCURSES_INSTALL_PATH} \
            --with-termlib \
            --without-manpages \
            --with-strip-program="${PREBUILT_PATH}/../out/llvm-install/bin/llvm-strip" \
            --disable-widec \
            ${EXTRA_ARGS} \
            CC=${PREBUILT_PATH}/../out/llvm-install/bin/clang \
            CXX=${PREBUILT_PATH}/../out/llvm-install/bin/clang++ \
            CFLAGS="${C_FLAGS}"
        make -j$(nproc --all) install | tee build_ncurses_${TARGET}.log
    fi
    cd ${NCURSES_SRC_DIR}
    git reset --hard HEAD
    git clean -df
fi

