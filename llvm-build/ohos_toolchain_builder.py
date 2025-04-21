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
from typing import List
from build import BuildConfig, BuildUtils, LlvmLibs, SysrootComposer, LlvmPackage
from python_builder import OHOSPythonBuilder


class OHOSToolchainBuilder:
    def __init__(self, llvm_triple) -> None:
        self._llvm_triple = llvm_triple
        self._platform = llvm_triple.split("-")[0]
        self._build_config = BuildConfig()
        self._build_utils = BuildUtils(self._build_config)
        self._sysroot_composer = SysrootComposer(self._build_config)
        self._llvm_package = LlvmPackage(self._build_config)
        self._llvm_libs = LlvmLibs(
            self._build_config, self._sysroot_composer, self._llvm_package
        )
        self._python_builder = OHOSPythonBuilder(self._build_utils, self._llvm_triple)
        self._llvm_project_path = os.path.abspath(
            os.path.join(self._build_config.LLVM_PROJECT_DIR, "llvm")
        )
        self._llvm_path = self._build_utils.merge_out_path(f"ohos-{self._platform}")
        self._llvm_install = self._build_utils.merge_out_path(
            f"ohos-{self._platform}-install"
        )
        self._llvm_root = self._build_utils.merge_out_path("llvm-install")
        self._sysroot = self._build_utils.merge_out_path("sysroot")

        self._cflags = self._init_cflags()
        self._ldflags = self._init_ldflags()
        self._llvm_defines = self._init_llvm_defines()

    def _init_cflags(self) -> List[str]:
        cflags = [
            "-fstack-protector-strong",
            "--target=%s" % self._llvm_triple,
            "-ffunction-sections",
            "-fdata-sections",
        ]
        return cflags

    def _init_ldflags(self) -> List[str]:
        ldflags = [
            "-fuse-ld=lld",
            "-Wl,--gc-sections",
            "-Wl,--build-id=sha1",
            "--rtlib=compiler-rt",
            "-stdlib=libc++",
            "-Wl,-z,relro,-z,now",
            "-pie",
            "-lunwind",
            "-Wl,-rpath,'$ORIGIN/../lib'",
        ]
        return ldflags

    def _init_llvm_defines(self):
        llvm_defines = {}
        llvm_defines["LLVM_TARGET_ARCH"] = self._platform
        llvm_defines["OHOS"] = "1"
        llvm_defines["CMAKE_SYSTEM_NAME"] = "OHOS"
        llvm_defines["CMAKE_CROSSCOMPILING"] = "True"
        llvm_defines["CMAKE_INSTALL_PREFIX"] = self._llvm_install
        llvm_defines["CMAKE_SYSROOT"] = self._sysroot
        llvm_defines["LLVM_HOST_TRIPLE"] = self._llvm_triple
        llvm_defines["LLVM_TARGETS_TO_BUILD"] = self._build_config.TARGETS
        llvm_defines["LLVM_DEFAULT_TARGET_TRIPLE"] = self._llvm_triple
        llvm_defines["LLVM_ENABLE_TERMINFO"] = "OFF"
        llvm_defines["LLVM_CONFIG_PATH"] = os.path.join(
            self._llvm_root, "bin", "llvm-config"
        )
        llvm_defines["LLVM_TABLEGEN"] = os.path.join(
            self._llvm_root, "bin", "llvm-tblgen"
        )
        llvm_defines["CMAKE_C_COMPILER"] = os.path.join(self._llvm_root, "bin", "clang")
        llvm_defines["CMAKE_CXX_COMPILER"] = os.path.join(
            self._llvm_root, "bin", "clang++"
        )
        llvm_defines["CMAKE_AR"] = os.path.join(self._llvm_root, "bin", "llvm-ar")
        llvm_defines["CMAKE_FIND_ROOT_PATH_MODE_INCLUDE"] = "ONLY"
        llvm_defines["CMAKE_FIND_ROOT_PATH_MODE_LIBRARY"] = "ONLY"
        llvm_defines["CMAKE_FIND_ROOT_PATH_MODE_PACKAGE"] = "ONLY"
        llvm_defines["CMAKE_FIND_ROOT_PATH_MODE_PROGRAM"] = "NEVER"
        llvm_defines["Python3_EXECUTABLE"] = os.path.join(
            self._build_utils.get_python_dir(), "bin", self._build_config.LLDB_PYTHON
        )

        if self._build_config.debug:
            llvm_defines["CMAKE_BUILD_TYPE"] = "Debug"
        else:
            llvm_defines["CMAKE_BUILD_TYPE"] = "Release"

        llvm_defines.update(self._init_lldb_defines())

        return llvm_defines

    def _init_lldb_defines(self):
        lldb_defines = {}
        lldb_defines["LLDB_INCLUDE_TESTS"] = "OFF"
        if self._build_config.lldb_timeout:
            lldb_defines["LLDB_ENABLE_TIMEOUT"] = "True"
        # Optional Dependencies
        if self._build_config.build_ncurses:
            lldb_defines["LLDB_ENABLE_CURSES"] = "ON"
            lldb_defines["CURSES_INCLUDE_DIRS"] = ";".join(
                [
                    self._build_utils.merge_ncurses_install_dir(
                        self._llvm_triple, "include"
                    ),
                    self._build_utils.merge_ncurses_install_dir(
                        self._llvm_triple, "include", "ncurses"
                    ),
                ]
            )
            ncurses_libs = []
            for library in self._build_utils.get_ncurses_dependence_libs(self._llvm_triple):
                library_path = self._build_utils.merge_ncurses_install_dir(
                    self._llvm_triple,
                    "lib",
                    f"{library}.so.%s" % self._build_utils.get_ncurses_version(),
                )
                ncurses_libs.append(library_path)
            lldb_defines["CURSES_LIBRARIES"] = ";".join(ncurses_libs)
            lldb_defines["PANEL_LIBRARIES"] = ";".join(ncurses_libs)

        if self._build_config.build_libxml2:
            self._build_config.LIBXML2_VERSION = self._build_utils.get_libxml2_version()

            lldb_defines["LLDB_ENABLE_LIBXML2"] = "ON"
            lldb_defines["LIBXML2_INCLUDE_DIR"] = (
                self._build_utils.merge_libxml2_install_dir(
                    self._llvm_triple, "include", "libxml2"
                )
            )
            lldb_defines["LIBXML2_LIBRARY"] = (
                self._build_utils.merge_libxml2_install_dir(
                    self._llvm_triple,
                    "lib",
                    "libxml2.so.%s" % self._build_utils.get_libxml2_version(),
                )
            )

        if self._build_config.build_libedit:
            lldb_defines["LLDB_ENABLE_LIBEDIT"] = "ON"
            lldb_defines["LibEdit_INCLUDE_DIRS"] = (
                self._build_utils.merge_libedit_install_dir(
                    self._llvm_triple, "include"
                )
            )
            lldb_defines["LibEdit_LIBRARIES"] = (
                self._build_utils.merge_libedit_install_dir(
                    self._llvm_triple, "lib", "libedit.so.0.0.68"
                )
            )

        if self._build_config.build_python:
            lldb_defines["LLDB_ENABLE_PYTHON"] = "ON"
            lldb_defines["LLDB_EMBED_PYTHON_HOME"] = "ON"
            lldb_defines["LLDB_PYTHON_HOME"] = f"../{self._build_config.LLDB_PYTHON}"
            lldb_defines["LLDB_PYTHON_RELATIVE_PATH"] = "bin/python/lib/python%s" % (
                self._build_config.LLDB_PY_VERSION
            )
            lldb_defines["LLDB_PYTHON_EXE_RELATIVE_PATH"] = "bin/python"
            lldb_defines["LLDB_PYTHON_EXT_SUFFIX"] = ".so"
            lldb_defines["Python3_INCLUDE_DIRS"] = (
                self._build_utils.merge_python_install_dir(
                    self._llvm_triple,
                    "include",
                    f"python{self._build_config.LLDB_PY_VERSION}",
                )
            )
            lldb_defines["Python3_LIBRARIES"] = self._build_utils.merge_python_install_dir(
                self._llvm_triple,
                "lib",
                "libpython%s.so" % self._build_config.LLDB_PY_VERSION,
            )

        lldb_defines["LLDB_ENABLE_LZMA"] = "OFF"
        # Debug & Tuning
        if self._build_config.enable_monitoring:
            lldb_defines["LLDB_ENABLE_PERFORMANCE"] = "ON"

        return lldb_defines

    def _build_and_install(self, build_target):
        if self._build_config.build_ncurses:
            self._llvm_libs.build_ncurses(self._llvm_path, self._llvm_install, self._llvm_triple)
        if self._build_config.build_libxml2:
            self._llvm_libs.build_libxml2(self._llvm_triple, self._llvm_path, self._llvm_install)
        if self._build_config.build_libedit:
            self._llvm_libs.build_libedit(self._llvm_path, self._llvm_install, self._llvm_triple)
        if self._build_config.build_python:
            self._python_builder.build()
            self._python_builder.prepare_for_package()

        self._build_utils.invoke_cmake(
            self._llvm_project_path,
            self._llvm_path,
            self._llvm_defines,
            env=dict(self._build_config.ORIG_ENV),
        )

        self._build_utils.invoke_ninja(
            out_path=self._llvm_path,
            env=dict(self._build_config.ORIG_ENV),
            target=build_target,
            install=True,
        )

        if self._build_config.build_python:
            self._python_builder.copy_python_to_host(self._llvm_install)

        # Copy required arm-linux-ohos libs from main toolchain build.
        arch_list = [
            self._build_utils.liteos_triple("arm"),
            self._build_utils.open_ohos_triple("arm"),
            self._build_utils.open_ohos_triple("aarch64"),
            self._build_utils.open_ohos_triple("riscv64"),
            self._build_utils.open_ohos_triple("loongarch64"),
            self._build_utils.open_ohos_triple("mipsel"),
            self._build_utils.open_ohos_triple("x86_64"),
        ]
        for arch in arch_list:
            self._build_utils.check_copy_tree(
                os.path.join(self._llvm_root, "lib", arch),
                os.path.join(self._llvm_install, "lib", arch),
            )
            self._build_utils.check_copy_tree(
                os.path.join(self._llvm_root, "lib", "clang", "15.0.4", "lib", arch),
                os.path.join(self._llvm_install, "lib", "clang", "15.0.4", "lib", arch),
            )

        # Copy required c++ headerfiles from main toolchain build.
        self._build_utils.check_copy_tree(
            os.path.join(self._llvm_root, "include", "c++"),
            os.path.join(self._llvm_install, "include", "c++"),
        )
        self._build_utils.check_copy_tree(
            os.path.join(self._llvm_root, "include", "libcxx-ohos"),
            os.path.join(self._llvm_install, "include", "libcxx-ohos"),
        )

    def _package_if_need(self):
        if self._build_config.do_package:
            tarball_name = (
                f"clang-{self._build_config.build_name}-ohos-{self._platform}"
            )
            package_path = "%s%s" % (
                self._build_utils.merge_packages_path(tarball_name),
                self._build_config.ARCHIVE_EXTENSION,
            )
            bin_dir = os.path.join(self._llvm_path, "bin")
            sript_bins = ['git-clang-format', 'scan-build','scan-view', 'scan-build-py', 'llvm-locstats', 'llvm-lit', 'intercept-build', 'analyze-build', 'lldb-python']
            for binfile_name in os.listdir(bin_dir):
                binary = os.path.join(bin_dir, binfile_name)
                if not os.path.isfile(binary):
                    continue
                if binfile_name not in sript_bins and self._build_config.strip:
                    self._build_utils.check_call(['eu-strip', binary])
            self._build_utils.logger().info("Packaging %s", package_path)
            args = [
                "tar",
                self._build_config.ARCHIVE_OPTION,
                "-h",
                "-C",
                self._build_config.OUT_PATH,
                "-f",
                package_path,
                f"ohos-{self._platform}-install",
            ]
            self._build_utils.check_create_dir(self._build_config.PACKAGES_PATH)
            self._build_utils.check_call(args)

    # virtual function
    def _update_build_args(self):
        return

    def build(self, build_target=None):
        self._update_build_args()

        self._build_and_install(build_target)

        self._package_if_need()
