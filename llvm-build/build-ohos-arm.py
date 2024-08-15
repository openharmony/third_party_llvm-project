#!/usr/bin/env python3
# Copyright (C) 2024 Huawei Device Co., Ltd.
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


class OHOSArmToolchainBuilder(CrossToolchainBuilder):
    def __init__(self) -> None:
        super().__init__("arm-linux-ohos")
        self._llvm_prebuilt_path = self._build_utils.merge_out_path("llvm_make")

    def _update_build_args(self):
        self._cflags.extend(
            [
                "-march=armv7-a -mfloat-abi=soft",
            ]
        )

        self._llvm_defines.update(
            {
                "CMAKE_CXX_FLAGS": " ".join(self._cflags),
                "CMAKE_ASM_FLAGS": " ".join(self._cflags),
                "CMAKE_C_FLAGS": " ".join(self._cflags),
                "CMAKE_SHARED_LINKER_FLAGS": " ".join(self._ldflags),
                "CMAKE_MODULE_LINKER_FLAGS": " ".join(self._ldflags),
                "CMAKE_EXE_LINKER_FLAGS": " ".join(self._ldflags),
                "LLVM_ENABLE_ASSERTIONS": "OFF",
                "LLVM_USE_NEWPM": "ON",
                "LLVM_ENABLE_BINDINGS": "OFF",
                "CLANG_REPOSITORY_STRING": "llvm-project",
                "COMPILER_RT_BUILD_XRAY": "OFF",
                "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
                "LLVM_ENABLE_PER_TARGET_RUNTIME_DIR": "ON",
                "COMPILER_RT_USE_BUILTINS_LIBRARY": "ON",
                "LLVM_ENABLE_LIBCXX": "ON",
                "LLVM_ENABLE_PROJECTS": "clang;lldb",
                "CLANG_TABLEGEN": os.path.join(
                    self._llvm_root,
                    "..",
                    self._llvm_prebuilt_path,
                    "bin",
                    "clang-tblgen",
                ),
                "LLDB_TABLEGEN": os.path.join(
                    self._llvm_root,
                    "..",
                    self._llvm_prebuilt_path,
                    "bin",
                    "lldb-tblgen",
                ),
            }
        )


def main():
    print("Start building LLDB tools for OHOS ARM host")
    OHOSArmToolchainBuilder().build(build_target=["lldb", "lldb-server"])


if __name__ == "__main__":
    main()
