## Overview

This readme briefly describes the functionality of our LLVM toolchain and what we have included specially to the LLVM toolchain.

## Functionality

The LLVM toolchain is built based on LLVM 12.0.1. It is used to provide capability of building ohos image. For detailed information about LLVM 12.0.1, please refer to [LLVM 12.0.1](https://lists.llvm.org/pipermail/llvm-announce/2021-July/000093.html).
</br>


## Specifically Included Triplets

Despite all the components provided by LLVM community, we included several triplets for different types of ohos devices to our LLVM toochain, listed as below.  For specification, liteos is a newly included OS name which indicate the simplified linux kernel.

| Triplet Name           | Architecture | System Kernel | System          |
| ---------------------- | ------------ | ------------- | --------------- |
| arm-liteos-ohos        | ARM 32bits   | LiteOS        | Small system    |
| arm-linux-ohos         | ARM 32bits   | Linux         | Small system    |
| arm-linux-ohos         | ARM 32bits   | Linux         | Standard system |
| aarch64-linux-ohos     | ARM 64bits   | Linux         | Standard system |

For detailed definition of Small System and Standard System, please refer to [System Types](https://gitee.com/openharmony/docs/blob/master/en/device-dev/Readme-EN.md).

### Specify the triplet

To build images for different types of platform, you can configure the triplet in the build scripts by setting "--target=xxx" using cflags, xxx should be replaced with a specific triplet name.
