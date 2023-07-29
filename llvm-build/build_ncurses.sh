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

ncurses_package=${NCURSES_SRC_DIR}/ncurses-${NCURSES_VERSION}.tar.gz
if [ -e ${ncurses_package} ]; then
    tar -xvzf ${ncurses_package} --strip-components 1 -C ${NCURSES_SRC_DIR}
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
        patch -Np1 < $patch
    done

    if [ ! -b ${NCURSES_BUILD_PATH} ]; then
        mkdir -p ${NCURSES_BUILD_PATH}
    fi
    cd ${NCURSES_BUILD_PATH}
    # build ncurses
    if [ "${host_platform}" == "darwin" ]; then
        export LDFLAGS="-Wl,-rpath,@loader_path/../lib"
        SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
        flags="-Wl,-syslibroot,${SDKROOT}"
        export CPPFLAGS="$CPPFALGS -I${SDKROOT}/usr/include -I${SDKROOT}/usr/include/i368"
        export CFLAGS="$CFLAGS -isysroot${SDKROOT} $flags"

        ${NCURSES_SRC_DIR}/configure \
            --with-shared \
            --with-default-terminfo-dir=/usr/lib/terminfo:/lib/terminfo:/usr/share/terminfo \
            --disable-mixed-case \
            --prefix=${NCURSES_INSTALL_PATH} \
            CC=${CC_PATH} \
            CXX=${CXX_PATH}
        make -j$(nproc --all) install | tee build_ncurses.log
    fi
    if [ "${host_platform}" == "linux" ]; then
        export LDFLAGS="-Wl,-rpath,\$$ORIGIN/../lib"
        ${NCURSES_SRC_DIR}/configure \
            --with-shared \
            --with-default-terminfo-dir=/usr/lib/terminfo:/lib/terminfo:/usr/share/terminfo \
            --prefix=${NCURSES_INSTALL_PATH} \
            CC=${CC_PATH} \
            CXX=${CXX_PATH}
        make -j$(nproc --all) install | tee build_ncurses.log
    fi
fi

