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
sudo apt-get install build-essential swig python3-dev libedit-dev libncurses5-dev binutils-dev gcc-multilib abigail-tools elfutils pkg-config autoconf autoconf-archive libxml2
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
#change llvm-project
cd ./toolchain/llvm-project
git remote update
git checkout origin/llvm-18.1.8
# build
bash ./toolchain/llvm-project/llvm-build/build.sh
```

</br>

### Functionality

The LLVM toolchain is built based on LLVM 18.1.8. It is used to provide capability of building ohos image. For detailed information about LLVM 18.1.8, please refer to [LLVM 18.1.8](https://discourse.llvm.org/t/18-1-8-released/79725).
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
