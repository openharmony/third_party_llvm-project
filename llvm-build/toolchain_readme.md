## Overview

This readme briefly describes the functionality of our LLVM toolchain and what we have included specially to the LLVM toolchain.

## Functionality

The LLVM toolchain is built based on LLVM 15.0.4. It is used to provide capability of building ohos image. For detailed information about LLVM 15.0.4, please refer to [LLVM 15.0.4](https://discourse.llvm.org/t/llvm-15-0-4-released/66337).
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
