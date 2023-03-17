#!/bin/bash

PROJECT_ROOT=$(pwd)
CPYTHON_MINGW_SRC=${PROJECT_ROOT}/third_party/python
MINGW_W64_SRC=${PROJECT_ROOT}/third_party/mingw-w64

# prepare SYSROOT for cpython-mingw build
CLANG_MINGW_BUILD=${PROJECT_ROOT}/out/clang_mingw
python3 ${PROJECT_ROOT}/toolchain/llvm-build/mingw.py

# refreash cpython-mingw config
cd ${CPYTHON_MINGW_SRC}
cat ./patchs/cpython_mingw_v3.10.2.patch | patch -d ./ -p1 -E
rm -rf ${CPYTHON_MINGW_SRC}/configure
autoconf ${CPYTHON_MINGW_SRC}/configure.ac > ${CPYTHON_MINGW_SRC}/configure
chmod 777 ${CPYTHON_MINGW_SRC}/configure
./configure

# # prepare build config and build libpython for cpython-mingw
CPYTHON_MINGW_BUILD=${PROJECT_ROOT}/out/cpython-mingw-build
mkdir -p ${CPYTHON_MINGW_BUILD}
cd ${CPYTHON_MINGW_BUILD}
MINGW_PREFIX=${CPYTHON_MINGW_BUILD}/mingw64
MINGW_CHOST=x86_64-w64-mingw32
MINGW_BUILD=x86_64-unknown-linux-gnu
CLANG_VERSION=clang-10.0.1
TOOLCHAIN_ROOT=${CLANG_MINGW_BUILD}/${CLANG_VERSION}/bin
SYSROOT=${CLANG_MINGW_BUILD}/${CLANG_VERSION}/x86_64-w64-mingw32
mkdir $MINGW_BUILD

export CC=$TOOLCHAIN_ROOT/clang
export CXX=$TOOLCHAIN_ROOT/clang++
export CFLAGS="-target ${MINGW_CHOST} --sysroot=$SYSROOT"
export LDFLAGS="--sysroot=$SYSROOT -rtlib=compiler-rt -target ${MINGW_CHOST} -lucrt -lucrtbase -fuse-ld=lld"

${CPYTHON_MINGW_SRC}/configure \
    --prefix=${MINGW_PREFIX} \
    --host=${MINGW_CHOST} \
    --build=${MINGW_BUILD} \
    --enable-shared \
    --enable-static \
    --without-ensurepip \
    --without-c-locale-coercion \
    --enable-loadable-sqlite-extensions

make -j4

# copy build result to mingw-w64
cp ${CPYTHON_MINGW_BUILD}/libpython3.10.dll.a ${MINGW_W64_SRC}/mingw-w64-crt/lib64
cp ${CPYTHON_MINGW_BUILD}/libpython3.10.dll ${MINGW_W64_SRC}/mingw-w64-python/3.10
cd ../..

# example steps to install autoconf 2.71
# wget http://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.gz
# export PATH=/mnt/disk2/liwentao/workspace/tools/autoconf/autoconf-2.71/bin:$PATH
# sudo cp -r /mnt/disk2/liwentao/workspace/tools/autoconf/autoconf-2.71/lib /usr/local/share/autoconf

# refreash configure and Makefile for mingw-w64
# you need autoconf (2.69 for Ubuntu 20.04 LTS apt install, to support mingw-w64 9.x and later version, please install autoconf 2.71 http://ftp.gnu.org/gnu/autoconf/)
# and automake (1.16.1 for Ubuntu 20.04 LTS apt install)
cd ${MINGW_W64_SRC}/mingw-w64-crt
rm -rf configure
autoconf configure.ac > configure
chmod 777 configure

aclocal
automake

cd ${MINGW_W64_SRC}/mingw-w64-headers
rm -rf configure
autoconf configure.ac > configure
chmod 777 configure

aclocal
automake

# move cpython-mingw build result and python header file to mingw-w64
cp -r ${CPYTHON_MINGW_SRC}/Include ${MINGW_W64_SRC}/mingw-w64-headers/include/python3.10
cp ${CPYTHON_MINGW_SRC}/pyconfig.h ${MINGW_W64_SRC}/mingw-w64-headers/include/python3.10
mkdir -p ${MINGW_W64_SRC}/mingw-w64-python/3.10
cp -r ${CPYTHON_MINGW_SRC}/Lib ${MINGW_W64_SRC}/mingw-w64-python/3.10/lib