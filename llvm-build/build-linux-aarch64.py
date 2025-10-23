#!/usr/bin/env python3
# Copyright (C) 2023 Huawei Device Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from cross_toolchain_builder import CrossToolchainBuilder


class UbuntuAarch64ToolchainBuilder(CrossToolchainBuilder):
    def __init__(self) -> None:
        super().__init__("aarch64-linux-gnu", True)

    def _update_build_args(self):
        self._cflags.extend(
            [
                "-v",
                "-funwind-tables",
            ]
        )

        cflags_debug = "-O0 -g -fno-limit-debug-info"
        cflags_release = "-O2 -DNDEBUG"

        self._ldflags.extend(["-static-libstdc++"])

        env = {**self._build_config.ORIG_ENV, 'LD_LIBRARY_PATH': os.path.join(self._llvm_root, "lib")}

        # We do not build runtimes, since they will be copied from main toolchain build
        self._llvm_defines.update(
            {
                "CMAKE_SYSTEM_NAME": "Linux",
                "CMAKE_SYSROOT": "",
                "OHOS": "0",
                "CMAKE_C_FLAGS_DEBUG": cflags_debug,
                "CMAKE_CXX_FLAGS_DEBUG": cflags_debug,
                "CMAKE_ASM_FLAGS_DEBUG": cflags_debug,
                "CMAKE_C_FLAGS_RELEASE": cflags_release,
                "CMAKE_CXX_FLAGS_RELEASE": cflags_release,
                "CMAKE_ASM_FLAGS_RELEASE": cflags_release,
                "OPENMP_STANDALONE_BUILD": "ON",
                "LLVM_DIR": os.path.join(self._llvm_root, "lib", "cmake", "llvm"),
                "LLVM_ENABLE_FFI": "OFF",
                "LLVM_BUILD_LLVM_DYLIB": "ON",
                "CMAKE_LIBRARY_ARCHITECTURE": self._llvm_triple,
                "LLVM_INCLUDE_BENCHMARKS": "OFF",
                "LLVM_INCLUDE_EXAMPLES": "OFF",
                "LLVM_INCLUDE_TESTS": "OFF",
                "LLVM_BUILD_TOOLS": "ON",
                "LLVM_INSTALL_UTILS": "ON",
                "LLVM_ENABLE_ZLIB": "OFF",
                "LLVM_ENABLE_PROJECTS": ";".join(self._build_config.host_projects),
                "CMAKE_NM": os.path.join(self._llvm_root, "bin", "llvm-nm"),
                "CMAKE_RANLIB": os.path.join(self._llvm_root, "bin", "llvm-ranlib"),
                "CMAKE_OBJCOPY": os.path.join(self._llvm_root, "bin", "llvm-objcopy"),
                "CMAKE_OBJDUMP": os.path.join(self._llvm_root, "bin", "llvm-objdump"),
                "CMAKE_READELF": os.path.join(self._llvm_root, "bin", "llvm-readelf"),
                "CMAKE_STRIP": os.path.join(self._llvm_root, "bin", "llvm-strip"),
                "CMAKE_LINKER": os.path.join(self._llvm_root, "bin", "ld.lld"),
                "CMAKE_POSITION_INDEPENDENT_CODE": "True",
                "CMAKE_CXX_FLAGS": " ".join(self._cflags),
                "CMAKE_ASM_FLAGS": " ".join(self._cflags),
                "CMAKE_C_FLAGS": " ".join(self._cflags),
                "CMAKE_SHARED_LINKER_FLAGS": " ".join(self._ldflags),
                "CMAKE_MODULE_LINKER_FLAGS": " ".join(self._ldflags),
                "CMAKE_EXE_LINKER_FLAGS": " ".join(self._ldflags)
                + " -Wl,--gc-sections",
            }
        )

        if self._build_config.enable_assertions:
            self._llvm_defines["LLVM_ENABLE_ASSERTIONS"] = "ON"

        if not self._build_config.build_libxml2:
            self._llvm_defines["LLDB_ENABLE_LIBXML2"] = "OFF"

        self._build_config.build_python = False


def main():
    print("Start cross-compiling LLVM toolchain for Ubuntu AArch64 host on linux")
    UbuntuAarch64ToolchainBuilder().build()


if __name__ == "__main__":
    main()
