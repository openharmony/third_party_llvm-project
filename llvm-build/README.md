## Overview

This readme briefly describes the functionality of our LLVM toolchain and how to build it

1. [Build WIKI](#build_wiki)
2. [Function Introduction](#function_introduction)

<a name="build_wiki"></a>
## Build WIKI
</br>

### System Requirements for Toolchain BUild

Ubuntu >= 16.04  
MacOS X >= 10.15.4  

</br>

### Environmental preparation 

ubuntu 
```
sudo apt-get install build-essential swig python3-dev libedit-dev libncurses5-dev binutils-dev gcc-multilib abigail-tools elfutils
```
mac 
```
brew install swig  git-lfs java coreutils wget 
```

</br>

### Get Code
```
repo init -u https://gitee.com/OpenHarmony/manifest.git -b master -m llvm-toolchain.xml
repo sync -c 
repo forall -c 'git lfs pull'
```
</br>

### Toolchain build process

Here is an example of starting build process on Linux or MacOS:
```
# update prebuilts, no need to run each time
./toolchain/llvm-project/llvm-build/env_prepare.sh
# build
python3 ./toolchain/llvm-project/llvm-build/build.py
```

1. env_prepare (one time only)
![输入图片说明](data/one_time_setup.png)

2. build
![输入图片说明](data/llvm_build.png)

</br>

### Options

build.py options：

```
--skip-build			# skip compile and goto package step
--skip-package			# do compile without package step
--enable-assertions		# enable assertion when compiling
--build-name 			# specify release package name
--debug					# build debug version llvm toolchain
--target-debug          # build debug version of target llvm libraries, runtimes, and lldb-server
--strip					# strip llvm toolchain binaries
--no-strip-libs         # used with -- strip to preserve the symbols of the lib library
--no-build-arm			# skip triplet arm
--no-build-aarch64  	# skip triplet arm64
--no-build-x86_64 		# skip triplet x86_64
--no-lto  				# disable LTO optimization when build toolchain
--build-instrumented	# enable instrument pgo when build toolchain
--xunit-xml-output 		# specify LLVM unit test XML report path
--build-lldb-static     # build statically lldb tool for ARM and AARCH64
--build-python          # build python (not using prebuilt one, currently effective for Windows and OHOS)
--build-ncurses         # build ncurses tool for linux, Mac x86-64 or M1
--build-libedit         # build libedit tool for linux, Mac x86-64 or M1
--build-libxml2         # build libxml2 tool for linux, windows, Mac x86_64 or M1
--lldb-timeout          # automatically exit when timeout (currently effective for lldb-server)
--no-build              # optional, skip some targets
    windows
    libs
    lldb-server
    linux 
    check-api
--build-clean           # delete out folder after build packages
--build_libs_flags      # which kind of flags for build_crts and build_runtimes
    OH
    LLVM
    BOTH
--build-only            # build only some targets, skip building windows, lldb-server and package step
    lld
    llvm-readelf
    llvm-objdump
    musl
    compiler-rt
    libcxx
--enable-check-abi      # Check libc++_shared.so ABI. If the ABI was changed then interrupt a build process and report an error.
--separate-libs         # Separate the cross compilation library (compiler-rt and libcxx) in Linux.
```
</br>

### Output Layout

When build successfully completed. following artifacts will be available in `out` directory

`sysroot` -> sysroots for OHOS targets  
`install` -> toolchain build  
`*.tar.bz2` -> archived versions of toolchain and sysroots  
</br>

### OHOS Archive

1. llvm
```
contains: 
1. toolchain which provides clang compiler, lldb(-mi), clang-tidy etc. tools
2. libc++/clang_rt/asan/fuzzer libs for target device

OHOS sync from: https://mirrors.huaweicloud.com/openharmony/compiler/clang/
Which is the same as: out/clang-dev-${platform}-${arch}.tar.bz2
OHOS archive to: prebuilts/clang/ohos//${platform}/llvm

License: Apache License v2.0 with LLVM Exceptions
```

2. libcxx-ndk
```
contains: provide libc++ for ndk in target device

OHOS fetch prebuilts from: https://mirrors.huaweicloud.com/openharmony/compiler/clang/ and archive it to prebuilts/clang/ohos//${platform}/libcxx-ndk. This tar is 

License: Apache License v2.0 with LLVM Exceptions
```

### Build process of AArch64 toolchain

First build toolchain on Linux.
Here is an example of starting build process on Linux:
```
# build
python3 ./toolchain/llvm-project/llvm-build/build-ohos-aarch64.py
```

</br>

build-ohos-aarch64.py options：

```
--enable-assertions             # enable assertion when compiling
--debug                         # build debug version llvm toolchain
--strip                         # strip llvm toolchain binaries
--build-python                  # build python and enable script in debugger
--build-ncurses                 # build ncurses and enable ncurses in debugger
--build-libedit                 # build libedit and enable libedit in debugger
--build-libxml2                 # build libxml2 and enable libxml in debugger
```
</br>

When build successfully completed, artifacts will be available in `out/ohos-aarch64-install` directory, including clang, lld, runtimes, LLVM tools and libLLVM.so for aarch64.

<a name="function_introduction"></a>
## Function Introduction
</br>

### Build process of Arm debugger

First build toolchain on Linux.
Here is an example of starting build process on Linux:
```
# build
python3 ./toolchain/llvm-project/llvm-build/build-ohos-arm.py
```

</br>

build-ohos-arm.py options：

```
--build-python                  # build python and enable script in debugger
--build-ncurses                 # build ncurses and enable ncurses in debugger
--build-libedit                 # build libedit and enable libedit in debugger
--build-libxml2                 # build libxml2 and enable libxml in debugger
```
</br>

When build successfully completed, artifacts will be available in `out/ohos-arm-install` directory, including lldb for arm.

<a name="function_introduction"></a>
## Function Introduction
</br>

### Build process of AArch64 toolchain for Ubuntu (aarch64-linux-gnu triple)

First build toolchain on Linux.
Install additional package libstdc++-11-dev-arm64-cross.

Here is an example of starting build process on Linux:
```
# build
python3 ./toolchain/llvm-project/llvm-build/build-linux-aarch64.py
```

</br>

build-linux-aarch64.py options：

```
--enable-assertions             # enable assertion when compiling
--debug                         # build debug version llvm toolchain
--strip                         # strip llvm toolchain binaries
--build-python                  # build python and enable script in debugger
--build-ncurses                 # build ncurses and enable ncurses in debugger
--build-libedit                 # build libedit and enable libedit in debugger
--build-libxml2                 # build libxml2 and enable libxml in debugger
```
</br>

When build successfully completed, artifacts will be available in `out/linux-aarch64-install` directory, including clang, lld, runtimes, LLVM tools and libLLVM.so for aarch64.

</br>

### Functionality

The LLVM toolchain is built based on LLVM 15.0.4. It is used to provide capability of building ohos image. For detailed information about LLVM 15.0.4, please refer to [LLVM 15.0.4](https://discourse.llvm.org/t/llvm-15-0-4-released/66337).
</br>

### Specifically Included Triplets

Despite all the components provided by LLVM community, we included several triplets for different types of ohos devices to our LLVM toochain, listed as below. For specification, liteos is a newly included OS name which indicate the simplified linux kernel.

| Triplet Name           | Architecture | System Kernel | System          |
| ---------------------- | ------------ | ------------- | --------------- |
| arm-liteos-ohos        | ARM 32bits   | LiteOS        | Small system    |
| arm-linux-ohos         | ARM 32bits   | Linux         | Small system    |
| arm-linux-ohos         | ARM 32bits   | Linux         | Standard system |
| aarch64-linux-ohos     | ARM 64bits   | Linux         | Standard system |

For detailed definition of Small System and Standard System, please refer to [System Types](https://gitee.com/openharmony/docs/blob/master/en/device-dev/Readme-EN.md).

### Testing musl libc

Toolchain build process includes musl libc build. libc.so is available in sysroot.
Sometimes it's needed to build libc tests.

Here is an example of starting build process on Linux:
```
# build
python3 ./toolchain/llvm-project/llvm-build/build-libc-test.py
```

When build successfully completed, artifacts will be available in `out/llvm_build/musl` directory, including test libraries, libc tests and musl_unittest.
Scripts to execute libc tests could be found in `third_party/musl/scripts` directory.
For detailed information about musl, please refer to [third_party_musl](https://gitee.com/openharmony/third_party_musl).
