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
repo init -u https://gitcode.com/OpenHarmony/manifest.git -b master -m llvm-toolchain.xml
repo sync -c 
repo forall -c 'git lfs pull'
```
</br>

### Toolchain build process

Here is an example of starting build process on Linux or MacOS:
```
# update prebuilts, no need to run each time
bash -x toolchain/llvm-project/llvm-build/env_prepare.sh
#change llvm-project
cd ./toolchain/llvm-project
git remote update
git checkout gitcode/kotlin/llvm-19-apple
# build
bash ./toolchain/llvm-project/llvm-build/build.sh
```

</br>

**Note**
* The OHOS runtime library can only be built using build.py on Linux x86. If you are building on Linux x86, the libraries in package/clang-dev-linux-x86_64.tar.gz are the most complete.
* Other platforms, whether cross-built or host-side built, can not build the OHOS runtime library.
* The default header file library for clang++ is include/libcxx-ohos/include/c++/v1, which is added along with the OHOS runtime library.
* If you wish to use the full functionality of OHOS, such as the runtime library and libcxx-ohos header library, on platforms other than Linux-x86, please execute llvm-build/platform_package.sh. Please explore the specific usage of this script on your own.
* OHOS LLVM only provides the compiler, i.e., the output of the package. For SDK issues, please contact the SDK-related personnel.

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
