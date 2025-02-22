# LLVM编译器基础架构

此目录及其子目录包含LLVM的源代码，LLVM是一个用于构建高度优化的编译器、优化器和运行时环境的工具包。

自述文件简要介绍了如何开始构建LLVM。有关如何为LLVM项目做出贡献的更多信息，请查看 [为LLVM做出贡献](https://llvm.org/docs/Contributing.html) 指南。

## LLVM系统入门

取自 [https://llvm.org/docs/GettingStarted.html](https://gitee.com/link?target=https%3A%2F%2Fllvm.org%2Fdocs%2FGettingStarted.html).

### 概述

欢迎来到LLVM项目！

LLVM项目有多个组件。该项目的核心本身被称为“LLVM”。这包含处理中间表示并将其转换为对象文件所需的所有工具、库和头文件。工具包括汇编器、反汇编器、位码分析器和位码优化器。它还包含基本的回归测试。

类C语言使用 [Clang](https://clang.llvm.org/) 前端。此组件将C、C++、目标-C和目标-C++代码编译为LLVM位码，并使用LLVM将其编译为目标文件。

其他组件包括：[libc++ C++ 标准库](https://libcxx.llvm.org/)，[LLD链接器](https://lld.llvm.org/) 等。

### 获取源代码并构建LLVM

LLVM入门文档可能已过期。[Clang入门](https://clang.llvm.org/get_started.html) 页面可能有更准确的信息。

这是获取和构建LLVM源的示例工作流和配置：

1. Checkout LLVM (包括Clang等相关子项目):

   - `git clone https://github.com/llvm/llvm-project.git`
   - 或 在 Windows上, `git clone --config core.autocrlf=false https://github.com/llvm/llvm-project.git`

2. 配置和构建LLVM和Clang：

   - `cd llvm-project`

   - `mkdir build`

   - `cd build`

   - `cmake -G <generator> [options] ../llvm`

     一些常见的构建系统生成器有：

     - `Ninja`---用于生成 [Ninja](https://ninja-build.org/) 构建文件。大多数llvm开发人员使用Ninja。
     - `Unix Makefiles` --- 用于生成与make兼容的并行makefile。
     - `Visual Studio` --- 用于生成Visual Studio项目和解决方案。
     - `Xcode` --- 用于生成Xcode项目。

     一些常见选项：

     - `-DLLVM_ENABLE_PROJECTS='...'` --- 要额外构建的LLVM子项目的分号分隔列表。可以包括：clang、clang-tools-extra、libcxx、libcxxabi、libunwind、lldb、编译器-rt、lld、波莉或调试信息测试中的任何一个。

       例如，要构建LLVM、Clang、libcxx和libcxxabi，请使用 `-DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi"`.

     - `-DCMAKE_INSTALL_PREFIX=directory` --- 为*目录* 指定要安装LLVM工具和库的完整路径名 (默认 `/usr/local`).

     - `-DCMAKE_BUILD_TYPE=type` --- *类型* 的有效选项为 调试、发布、带调试信息发布 和最小尺寸发布。默认值为调试。

     - `-DLLVM_ENABLE_ASSERTIONS=On` --- 在启用断言检查的情况下编译（调试编译的默认值为Yes，所有其他编译类型的默认值为No）。

   - `cmake --build . [-- [options] <target>]` 或直接在上面指定的构建系统。

     - 默认目标（即 `ninja` or `make`）将构建所有LLVM。
     - `check-all`目标（即`ninja check-all`）将运行回归测试，以确保一切都正常工作。
     - CMake将为每个工具和库生成目标，大多数LLVM子项目生成自己的`check-<project>`目标。
     - 运行串行构建将是 **缓慢** 。要提高速度，请尝试运行并行构建。在忍者中，默认情况下，这是在“make”中完成的；对于`make`，请使用选项`-j NNN`，其中`NNN`是并行作业的数量，例如您拥有的CPU数量。

   - 有关详细信息，请参阅 [CMake](https://llvm.org/docs/CMake.html)

请参阅 [LLVM入门](https://llvm.org/docs/GettingStarted.html#getting-started-with-llvm) 页面，了解有关配置和编译LLVM的详细信息。您可以访问 [目录布局](https://llvm.org/docs/GettingStarted.html#directory-layout) 以了解源代码树的布局。