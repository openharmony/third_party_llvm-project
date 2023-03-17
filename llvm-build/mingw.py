#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""donwload and build mingw64-7.0.0 sysroot"""

import logging
import os
import shutil
import subprocess
import sys


class BuildConfig():

    def __init__(self, clang_version):
        self.LLVM_BUILD_DIR = os.path.realpath(os.path.dirname(__file__))

        parent_of_llvm_build = os.path.basename(os.path.dirname(self.LLVM_BUILD_DIR))
        if parent_of_llvm_build == 'toolchain':
            self.REPOROOT_DIR = os.path.realpath(os.path.join(self.LLVM_BUILD_DIR, '../..'))
        else:
            assert parent_of_llvm_build == 'llvm-project'
            self.REPOROOT_DIR = os.path.realpath(os.path.join(self.LLVM_BUILD_DIR, '../../..'))

        self.OUT_DIR = os.environ.get('OUT_DIR', self.repo_root('out'))
        self.CLANG_VERSION = clang_version
        self.LLVM_TRIPLE = 'x86_64-windows-gnu'

    def repo_root(self, *args):
        return os.path.join(self.REPOROOT_DIR, *args)

    def out_root(self, *args):
        return os.path.join(self.OUT_DIR, *args)

    def mingw64_dir(self):
        return self.out_root('mingw', self.LLVM_TRIPLE)

    def llvm_path(self, *args):
        return os.path.join(self.REPOROOT_DIR, 'toolchain/llvm-project', *args)


class LlvmMingw():

    def __init__(self, build_config):
        self.build_config = build_config

        self.CMAKE_BIN_PATH = os.path.join(self.cmake_prebuilt_bin_dir(), 'cmake')
        self.NINJA_BIN_PATH = os.path.join(self.cmake_prebuilt_bin_dir(), 'ninja')

        self.LLVM_ROOT = self.build_config.out_root('llvm-install')
        self.LLVM_CONFIG = os.path.join(self.LLVM_ROOT, 'bin', 'llvm-config')
        self.SYSROOT = self.build_config.mingw64_dir()
        self.CRT_PATH = self.build_config.out_root('lib', 'mingw-clangrt-%s' % self.build_config.LLVM_TRIPLE)
        self.CRT_INSTALL = self.build_config.out_root('llvm-install', 'lib', 'clang', self.build_config.CLANG_VERSION)
        # prefix & env
        self.prefix = build_config.mingw64_dir()
        common_flags = "-target x86_64-w64-mingw32 -rtlib=compiler-rt -stdlib=libc++ \
            -fuse-ld=lld -Qunused-arguments -g -O2 --sysroot=" + self.build_config.mingw64_dir()
        self.env = {
            "CC": "%s/bin/clang" % self.LLVM_ROOT,
            "CXX": "%s/bin/clang++" % self.LLVM_ROOT,
            "AR": "%s/bin/llvm-ar" % self.LLVM_ROOT,
            "DLLTOOL": "%s/bin/llvm-dlltool" % self.LLVM_ROOT,
            "LD": "%s/bin/ld.lld" % self.LLVM_ROOT,
            "RANLIB": "%s/bin/llvm-ranlib" % self.LLVM_ROOT,
            "STRIP": "%s/bin/llvm-strip" % self.LLVM_ROOT,
            "LDFLAGS": common_flags,
            "CFLAGS": common_flags,
            "CXXFLAGS": common_flags,
            "PATH": os.environ["PATH"],
        }

    def cmake_prebuilt_bin_dir(self):
        return self.build_config.repo_root('prebuilts/cmake', 'linux-x86', 'bin')

    def base_cmake_defines(self):
        self.cmake_defines = {}
        self.cmake_defines['CMAKE_BUILD_TYPE'] = 'Release'
        self.cmake_defines['LLVM_ENABLE_ASSERTIONS'] = 'OFF'
        self.cmake_defines['LLVM_ENABLE_TERMINFO'] = 'OFF'
        self.cmake_defines['LLVM_ENABLE_THREADS'] = 'ON'
        self.cmake_defines['LLVM_USE_NEWPM'] = 'ON'
        self.cmake_defines['COMPILER_RT_BUILD_XRAY'] = 'OFF'
        return self.cmake_defines

    @staticmethod
    def check_create_path(path):
        if not os.path.exists(path):
            os.makedirs(path)

    @staticmethod
    def check_call(cmd, *args, **kwargs):
        subprocess.check_call(cmd, *args, **kwargs)

    @staticmethod
    def update_cmake_sysroot_flags(defines, sysroot):
        defines['CMAKE_SYSROOT'] = sysroot
        defines['CMAKE_FIND_ROOT_PATH_MODE_INCLUDE'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_LIBRARY'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_PACKAGE'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_PROGRAM'] = 'NEVER'

    def compiler_rt_defines(self):
        arch = 'x86_64'
        cc = os.path.join(self.LLVM_ROOT, 'bin', 'clang')
        cxx = os.path.join(self.LLVM_ROOT, 'bin', 'clang++')
        self.crt_defines = {}
        self.crt_defines['CMAKE_C_COMPILER'] = cc
        self.crt_defines['CMAKE_CXX_COMPILER'] = cxx
        self.crt_defines['LLVM_CONFIG_PATH'] = self.LLVM_CONFIG
        clang_libcxx_lib = self.build_config.out_root('clang_mingw', 'clang-%s' % self.build_config.CLANG_VERSION, 'lib')
        ldflags = [
            '-L%s' % clang_libcxx_lib,
            '-fuse-ld=lld',
            '-Wl,--gc-sections',
            '-pie',
            '--rtlib=compiler-rt',
            '-stdlib=libc++',
            '-v',
        ]
        self.crt_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        self.update_cmake_sysroot_flags(self.crt_defines, self.SYSROOT)
        cflags = [
            '--target=%s' % self.build_config.LLVM_TRIPLE,
            '-ffunction-sections',
            '-fdata-sections',
        ]
        cflags.append('-funwind-tables')
        cflags.append('-v')
        self.crt_defines['CMAKE_C_FLAGS'] = ' '.join(cflags)
        self.crt_defines['CMAKE_ASM_FLAGS'] = ' '.join(cflags)
        self.crt_defines['CMAKE_CXX_FLAGS'] = ' '.join(cflags)
        self.crt_defines['COMPILER_RT_TEST_COMPILER_CFLAGS'] = ' '.join(cflags)
        self.crt_defines['COMPILER_RT_TEST_TARGET_TRIPLE'] = self.build_config.LLVM_TRIPLE
        self.crt_defines['COMPILER_RT_DEFAULT_TARGET_TRIPLE'] = self.build_config.LLVM_TRIPLE
        self.crt_defines['COMPILER_RT_INCLUDE_TESTS'] = 'OFF'
        self.crt_defines['CMAKE_INSTALL_PREFIX'] = self.CRT_INSTALL
        self.crt_defines['COMPILER_RT_BUILD_LIBFUZZER'] = 'OFF'
        self.crt_defines['COMPILER_RT_USE_BUILTINS_LIBRARY'] = 'TRUE'
        self.crt_defines['CMAKE_SYSTEM_NAME'] = 'Windows'
        self.crt_defines['CMAKE_CROSSCOMPILING'] = 'True'
        self.crt_defines['CMAKE_C_COMPILER_WORKS'] = '1'
        self.crt_defines['CMAKE_CXX_COMPILER_WORKS'] = '1'
        self.crt_defines['SANITIZER_CXX_ABI'] = 'libcxxabi'
        self.crt_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'
        self.crt_defines['COMPILER_RT_BUILD_SANITIZERS'] = 'OFF'
        self.crt_defines['COMPILER_RT_EXCLUDE_ATOMIC_BUILTIN'] = 'OFF'
        #cmake config
        self.crt_defines.update(self.base_cmake_defines())
        return self.crt_defines

    def build_compiler_rt(self):
        crt_defines = self.compiler_rt_defines()
        #environ
        crt_env = dict(dict(os.environ))
        #compiler-rt path
        crt_cmake_path = self.build_config.llvm_path('compiler-rt')
        #cmake
        self.invoke_cmake(
            out_path=self.CRT_PATH,
            defines=crt_defines,
            env=crt_env,
            cmake_path=crt_cmake_path)

    def invoke_cmake(self, out_path, defines, env, cmake_path, target=None, install=True):
        flags = ['-G', 'Ninja']
        flags += ['-DCMAKE_PREFIX_PATH=%s' % self.cmake_prebuilt_bin_dir()]

        for key in defines:
            newdef = ''.join(['-D', key, '=', defines[key]])
            flags += [newdef]
        flags += [cmake_path]

        self.check_create_path(out_path)

        if target:
            ninja_target = [target]
        else:
            ninja_target = []

        self.check_call([self.CMAKE_BIN_PATH] + flags, cwd=out_path, env=env)
        self.check_call([self.NINJA_BIN_PATH] + ninja_target, cwd=out_path, env=env)
        if install:
            self.check_call([self.NINJA_BIN_PATH, 'install'], cwd=out_path, env=env)

    def build_mingw64_headers(self):
        headers_dir = self.build_config.repo_root('third_party', 'mingw-w64', 'mingw-w64-headers', 'build')
        if os.path.isdir(headers_dir):
            shutil.rmtree(headers_dir)
        os.makedirs(headers_dir)
        os.chdir(headers_dir)
        cmd = ['../configure', '--prefix=%s' % self.prefix,
            '--enable-idl', '--with-default-win32-winnt=0x600',
            '--with-default-msvcrt=ucrt']
        self.env['INSTALL'] = "install -C"
        self.check_call(cmd, env=self.env)
        self.check_call(['make', 'install'], env=self.env)

    def build_mingw64_crtlibs(self):
        crtlibs_dir = self.build_config.repo_root('third_party', 'mingw-w64', 'mingw-w64-crt', 'build')
        if os.path.isdir(crtlibs_dir):
            shutil.rmtree(crtlibs_dir)
        os.makedirs(crtlibs_dir)
        os.chdir(crtlibs_dir)
        cmd = ['../configure', '--prefix=%s' % self.prefix,
            '--host=x86_64-w64-mingw32', '--disable-lib32',
            '--enable-lib64', '--with-default-msvcrt=ucrt']
        self.check_call(cmd, env=self.env)
        self.check_call(['make', '-j4'], env=self.env)
        self.check_call(['make', 'install'], env=self.env)


def main(clang_version):
    build_config = BuildConfig(clang_version)
    llvm_mingw = LlvmMingw(build_config)

    llvm_mingw.build_mingw64_headers()
    llvm_mingw.build_mingw64_crtlibs()
    llvm_mingw.build_compiler_rt()
    return 0

if __name__ == '__main__':
    main()
