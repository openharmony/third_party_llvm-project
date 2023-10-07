#!/usr/bin/env python3
# Copyright (C) 2021 Huawei Device Co., Ltd.
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
# 2021.3.15 build for OHOS LLVM.
#     Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

import os
import platform
import re
import datetime
import logging
import glob
import subprocess
import shutil
import argparse
import mingw
import stat

from python_builder import MinGWPythonBuilder
from prebuilts_clang_version import prebuilts_clang_version

class BuildConfig():
    # Defines public methods and functions and obtains script parameters.

    def __init__(self):
        args = self.parse_args()
        self.do_build = not args.skip_build
        self.do_package = not args.skip_package
        self.build_name = args.build_name
        self.debug = args.debug
        self.target_debug = args.target_debug
        self.strip = args.strip
        self.no_lto = args.no_lto
        self.build_instrumented = args.build_instrumented
        self.xunit_xml_output = args.xunit_xml_output
        self.enable_assertions = args.enable_assertions
        self.build_clean = args.build_clean
        self.need_libs = self.do_build and 'libs' not in args.no_build
        self.need_lldb_server = self.do_build and 'lldb-server' not in args.no_build
        self.build_python = args.build_python

        self.no_build_arm = args.skip_build or args.no_build_arm
        self.no_build_aarch64 = args.skip_build or args.no_build_aarch64
        self.no_build_riscv64 = args.skip_build or args.no_build_riscv64
        self.no_build_mipsel = args.skip_build or args.no_build_mipsel
        self.no_build_x86_64 = args.skip_build or args.no_build_x86_64
        self.build_ncurses = args.build_ncurses
        self.build_libedit = args.build_libedit
        self.build_lldb_static = args.build_lldb_static
        self.need_lldb_tools = self.need_lldb_server or self.build_lldb_static
        self.build_libxml2 = args.build_libxml2
        self.lldb_timeout = args.lldb_timeout

        self.discover_paths()

        self.TARGETS = 'AArch64;ARM;BPF;Mips;RISCV;X86'
        self.ORIG_ENV = dict(os.environ)
        self.VERSION = None # autodetected

        self.OPENHOS_SFX = '-linux-ohos'
        self.LITEOS_SFX = '-liteos-ohos'
        self.LLDB_PY_VERSION = '3.10'
        self.LLDB_PYTHON = 'python3'
        self.LLDB_PY_DETAILED_VERSION = self.LLDB_PY_VERSION + '.2'
        self.CLANG_VERSION = prebuilts_clang_version
        self.MINGW_TRIPLE = 'x86_64-windows-gnu'
        logging.basicConfig(level=logging.INFO)

    def discover_paths(self):
        # Location of llvm-build directory
        self.LLVM_BUILD_DIR = os.path.abspath(os.path.dirname(__file__))

        parent_of_llvm_build = os.path.basename(os.path.dirname(self.LLVM_BUILD_DIR))
        if parent_of_llvm_build == 'toolchain':
            self.REPOROOT_DIR = os.path.abspath(os.path.join(self.LLVM_BUILD_DIR, '../..'))
        else:
            assert parent_of_llvm_build == 'llvm-project'
            self.REPOROOT_DIR = os.path.abspath(os.path.join(self.LLVM_BUILD_DIR, '../../..'))

        self.LLVM_PROJECT_DIR = os.path.join(self.REPOROOT_DIR, 'toolchain', 'llvm-project')
        self.OUT_PATH = os.path.join(self.REPOROOT_DIR, 'out')
        self.PACKAGES_PATH = os.path.join(self.REPOROOT_DIR, 'packages')

    @staticmethod
    def parse_add_argument(parser):

        parser.add_argument(
            '--enable-assertions',
            action='store_true',
            default=False,
            help='Apply assertions, some parameters are affected.')

        parser.add_argument(
            '--build-name',
            default='dev',
            help='Release name for the package.')

        parser.add_argument(
            '--debug',
            action='store_true',
            default=False,
            help='Building Clang and LLVM Tools for Debugging (only affects stage2)')

        parser.add_argument(
            '--target-debug',
            action='store_true',
            default=False,
            help='Building libraries, runtimes and lldb for the target for debugging')

        parser.add_argument(
            '--strip',
            action='store_true',
            default=False,
            help='Strip final LLVM binaries.')

        parser.add_argument(
            '--no-build-arm',
            action='store_true',
            default=False,
            help='Omit build os target: arm.')

        parser.add_argument(
            '--no-build-aarch64',
            action='store_true',
            default=False,
            help='Omit build os target: aarch64.')

        parser.add_argument(
            '--no-build-riscv64',
            action='store_true',
            default=False,
            help='Omit build os target: 64-bit RISC-V.')

        parser.add_argument(
            '--no-build-mipsel',
            action='store_true',
            default=False,
            help='Omit build os target: mipsel.')

        parser.add_argument(
            '--no-build-x86_64',
            action='store_true',
            default=False,
            help='Omit build os target: x86_64.')

        parser.add_argument(
            '--no-lto',
            action='store_true',
            default=False,
            help='Accelerate builds by disabling LTO (only affects llvm product)')

        parser.add_argument(
            '--build-instrumented',
            action='store_true',
            default=False,
            help='Using the PGO instrumentation to build LLVM tool')

        parser.add_argument(
            '--xunit-xml-output',
            default=None,
            help='Output path for LLVM unit tests XML report')

        parser.add_argument(
            '--build-lldb-static',
            action='store_true',
            default=False,
            help='Build statically lldb tool for ARM and AARCH64')

        parser.add_argument(
            '--build-clean',
            action='store_true',
            default=False,
            help='Delete out folder after build packages')

        parser.add_argument(
            '--build-python',
            action='store_true',
            default=False,
            help='Build Python (not using prebuilt one, currently effective for Windows)')

        parser.add_argument(
            '--build-ncurses',
            action='store_true',
            default=False,
            help='Build ncurses tool for Linux, Mac x86-64 or M1')

        parser.add_argument(
            '--build-libedit',
            action='store_true',
            default=False,
            help='Build libedit tool for Linux, Mac x86-64 or M1')

        parser.add_argument(
            '--build-libxml2',
            action='store_true',
            default=False,
            help='Build libxml2 tool')

        parser.add_argument(
            '--lldb-timeout',
            action='store_true',
            default=False,
            help='Automatically exit when timeout (currently effective for lldb-server)')

    def parse_args(self):

        parser = argparse.ArgumentParser(description='Process some integers.')

        # Options to skip build or packaging, can't skip two
        build_package_group = parser.add_mutually_exclusive_group()
        build_package_group.add_argument(
            '--skip-build',
            '-sb',
            action='store_true',
            default=False,
            help='Omit the build, perform the packaging step directly.')

        build_package_group.add_argument(
            '--skip-package',
            '-sp',
            action='store_true',
            default=False,
            help='Omit the packaging, perform the packaging step directly.')

        self.parse_add_argument(parser)

        known_platforms = ('windows', 'libs', 'lldb-server', 'linux', 'check-api')
        known_platforms_str = ', '.join(known_platforms)

        class SeparatedListByCommaAction(argparse.Action):
            def __call__(self, parser, namespace, vals, option_string):
                for val in vals.split(','):
                    if val in known_platforms:
                        continue
                    else:
                        error = '\'{}\' invalid.  Choose from {}'.format(val, known_platforms)
                        raise argparse.ArgumentError(self, error)
                setattr(namespace, self.dest, vals.split(','))

        parser.add_argument(
            '--no-build',
            action=SeparatedListByCommaAction,
            default=list(),
            help='Don\'t build toolchain for specified platforms.  Choices: ' + known_platforms_str)

        return parser.parse_args()


class ClangVersion(object):
    """Parse and save clang version from version file."""

    def __init__(self, version_file):
        self._parse_version_file(version_file)

    @staticmethod
    def _parse(text, key):
        return re.findall(r'%s\s+(\d+)' % key, text)[0]

    def _parse_version_file(self, version_file):
        with open(version_file, 'r') as fp:
            text = fp.read()
        self.major = self._parse(text, 'CLANG_VERSION_MAJOR')
        self.minor = self._parse(text, 'CLANG_VERSION_MINOR')
        self.patch = self._parse(text, 'CLANG_VERSION_PATCHLEVEL')

    def long_version(self):
        return '.'.join([self.major, self.minor, self.patch])

    def short_version(self):
        return '.'.join([self.major, self.minor])

    def major_version(self):
        return self.major


class BuildUtils(object):

    def __init__(self, build_config):
        self.build_config = build_config

        self.CMAKE_BIN_DIR = os.path.abspath(
            os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts/cmake', self.platform_prefix(), 'bin'))
        self._mingw_python_dir = None

    def open_ohos_triple(self, arch):
        return arch + self.build_config.OPENHOS_SFX

    def liteos_triple(self, arch):
        return arch + self.build_config.LITEOS_SFX

    def set_clang_version(self, install_dir):
        self.build_config.VERSION = self.get_clang_version(install_dir).long_version()

    def invoke_ninja(self,
                     out_path,
                     env,
                     target=None, # target support list type
                     install=True,
                     build_threads=False):

        ninja_bin_path = os.path.join(self.CMAKE_BIN_DIR, 'ninja')

        ninja_list = ['-l{}'.format(build_threads)] if build_threads else []

        ninja_target = target if target else []

        self.check_call([ninja_bin_path] + ninja_list + ninja_target, cwd=out_path, env=env)
        if install:
            self.check_call([ninja_bin_path, 'install'], cwd=out_path, env=env)

    def invoke_cmake(self,
                     cmake_path,
                     out_path,
                     invoke_defines,
                     env):

        cmake_bin_path = os.path.join(self.CMAKE_BIN_DIR, 'cmake')
        flags = ['-G', 'Ninja']
        flags += ['-DCMAKE_PREFIX_PATH=%s' % self.CMAKE_BIN_DIR]

        for key in invoke_defines:
            newdef = ''.join(['-D', key, '=', invoke_defines[key]])
            flags += [newdef]

        flags += [cmake_path]
        self.check_create_dir(out_path)

        self.check_call([cmake_bin_path] + flags, cwd=out_path, env=env)

    @staticmethod
    def logger():
        """Returns the module level logger."""
        return logging.getLogger(__name__)

    @staticmethod
    def get_clang_version(llvm_install):
        version_file = os.path.join(llvm_install, 'include', 'clang', 'Basic',
                                'Version.inc')
        return ClangVersion(version_file)

    def check_create_dir(self, path):
        if not os.path.exists(path):
            """Proxy for os.makedirs with logging and dry-run support."""
            self.logger().info('makedirs %s', path)
            os.makedirs(path)

    def check_rm_tree(self, tree_dir):
        """Removes directory tree."""
        def chmod_and_retry(func, path, _):
            if not os.access(path, os.W_OK):
                os.chmod(path, stat.S_IWUSR)
                return func(path)
            raise IOError("rmtree on %s failed" % path)

        if os.path.exists(tree_dir):
            self.logger().info('shutil rmtree %s', tree_dir)
            shutil.rmtree(tree_dir, onerror=chmod_and_retry)

    def check_copy_tree(self, src_dir, dst_dir):
        self.check_rm_tree(dst_dir)
        """Proxy for shutil.copytree with logging and dry-run support."""
        self.logger().info('copytree %s %s', src_dir, dst_dir)
        shutil.copytree(src_dir, dst_dir, symlinks=True)

    def check_copy_file(self, src_file, dst_file):
        if os.path.exists(src_file):
            """Proxy for shutil.copy2 with logging and dry-run support."""
            self.logger().info('copy %s %s', src_file, dst_file)
            shutil.copy2(src_file, dst_file)

    def check_call(self, cmd, *args, **kwargs):
        """subprocess.check_call with logging."""
        self.logger().info('check_call:%s %s',
                           datetime.datetime.now().strftime("%H:%M:%S"), subprocess.list2cmdline(cmd))

        subprocess.check_call(cmd, *args, **kwargs)

    def merge_out_path(self, *args):
        return os.path.abspath(os.path.join(self.build_config.OUT_PATH, *args))

    def merge_packages_path(self, *args):
        return os.path.abspath(os.path.join(self.build_config.PACKAGES_PATH, *args))

    @staticmethod
    def use_platform():
        sysstr = platform.system().lower()
        arch = platform.machine()
        return "%s-%s" % (sysstr, arch)

    def platform_prefix(self):
        prefix = self.use_platform()
        if (prefix.endswith('x86_64')):
            return prefix[:-3]
        return prefix

    def host_is_linux(self):
        return self.use_platform().startswith('linux-')

    def host_is_darwin(self):
        return self.use_platform().startswith('darwin-')

    def rm_cmake_cache(self, cache_dir):
        for dirpath, dirs, files in os.walk(cache_dir):
            if 'CMakeCache.txt' in files:
                self.logger().info('rm CMakeCache.txt on %s', cache_dir)
                os.remove(os.path.join(dirpath, 'CMakeCache.txt'))
            if 'CMakeFiles' in dirs:
                self.logger().info('rm CMakeFiles on %s', cache_dir)
                self.check_rm_tree(os.path.join(dirpath, 'CMakeFiles'))

    @staticmethod
    def find_program(name):
        # FIXME: Do we need Windows support here?
        return os.popen('which ' + name).read().strip()

    # Base cmake options such as build type that are common across all invocations
    def base_cmake_defines(self):
        mac_min_version = '10.9'
        defines = {}

        defines['CMAKE_BUILD_TYPE'] = 'Release'
        defines['LLVM_ENABLE_ASSERTIONS'] = 'OFF'
        defines['LLVM_ENABLE_TERMINFO'] = 'OFF'
        defines['LLVM_ENABLE_THREADS'] = 'ON'
        defines['LLVM_USE_NEWPM'] = 'ON'
        defines['LLVM_ENABLE_BINDINGS'] = 'OFF'
        defines['CLANG_REPOSITORY_STRING'] = 'llvm-project'
        defines['Python3_EXECUTABLE'] = os.path.join(self.get_python_dir(), 'bin', self.build_config.LLDB_PYTHON)
        defines['Python3_INCLUDE_DIRS'] = os.path.join(self.get_python_dir(), 'include',
            'python%s' % self.build_config.LLDB_PY_VERSION)

        if self.host_is_darwin():
            defines['CMAKE_OSX_DEPLOYMENT_TARGET'] = mac_min_version
            defines['LLDB_INCLUDE_TESTS'] = 'OFF'
            defines['LIBCXX_INCLUDE_TESTS'] = 'OFF'
            defines['Python3_LIBRARIES'] = os.path.join(self.get_python_dir(),
                'lib', 'libpython%s.dylib' % self.build_config.LLDB_PY_VERSION)
        else:
            defines['Python3_LIBRARIES'] = os.path.join(self.get_python_dir(),
                'lib', 'libpython%s.so' % self.build_config.LLDB_PY_VERSION)

        defines['COMPILER_RT_BUILD_XRAY'] = 'OFF'
        return defines

    def get_python_dir(self):
        platform_path = self.platform_prefix()
        if (self.host_is_darwin()):
            platform_path = "darwin-x86"
        python_dir = os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts', self.build_config.LLDB_PYTHON,
            platform_path, self.build_config.LLDB_PY_DETAILED_VERSION)
        return python_dir

    def get_prebuilts_dir(self, name):
        prebuilts_dir = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts', name))
        return prebuilts_dir

    def rm_build_output(self):
        return self.check_rm_tree(self.build_config.OUT_PATH)

    def set_mingw_python_dir(self, mingw_python_dir):
        self._mingw_python_dir = mingw_python_dir

    def get_mingw_python_dir(self):
        return self._mingw_python_dir

    def get_ncurses_version(self):
        ncurses_spec = os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'ncurses', 'ncurses.spec')
        if os.path.exists(ncurses_spec):
            with open(ncurses_spec, 'r') as file:
                lines = file.readlines()

            prog = re.compile(r'Version:\s*(\S+)')
            for line in lines:
                version_match = prog.match(line)
                if version_match:
                    return version_match.group(1)

        return None

    def get_libxml2_version(self):
        version_file = os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'libxml2', 'configure.ac')
        if os.path.isfile(version_file):
            with open(version_file, 'r') as file:
                lines = file.readlines()
                MAJOR_VERSION = ''
                MINOR_VERSION = ''
                MICRO_VERSION = ''
                for line in lines:
                    if "m4_define([MAJOR_VERSION]" in line:
                        MAJOR_VERSION = re.findall(r'\d+', line)[1]
                    elif "m4_define([MINOR_VERSION]" in line:
                        MINOR_VERSION = re.findall(r'\d+', line)[1]
                    elif "m4_define([MICRO_VERSION]" in line:
                        MICRO_VERSION = re.findall(r'\d+', line)[1]

                    if  MAJOR_VERSION != '' and MINOR_VERSION != '' and MICRO_VERSION != '' :
                        return MAJOR_VERSION+'.'+MINOR_VERSION+'.'+MICRO_VERSION
                return None

        return None

class LlvmCore(BuildUtils):

    def __init__(self, build_config):
        super(LlvmCore, self).__init__(build_config)

    def build_llvm(self,
                   targets,
                   build_dir,
                   install_dir,
                   build_name,
                   extra_defines=None,
                   extra_env=None):

        common_defines = self.base_cmake_defines()
        common_defines['CMAKE_INSTALL_PREFIX'] = install_dir
        common_defines['LLVM_INSTALL_UTILS'] = 'ON'
        common_defines['LLVM_TARGETS_TO_BUILD'] = targets
        common_defines['LLVM_BUILD_LLVM_DYLIB'] = 'ON'

        build_number = ''
        if re.match(r'\d+-.\D*$', build_name):
            build_number, build_name = build_name.split('-', 1)
        elif re.match(r'^\d+$', build_name):
            build_number = build_name
            build_name = ''
        elif re.match(r'^\D+$', build_name):
            build_name = build_name
        else:
            raise Exception('Warning! Build name is invalid, because it must not contain digits. '
                            'If you want to pass digit version, please, use follow template: {NUMBER}-{SUFFIX} '
                            'or just pass number. Otherwise unit tests will fail with assertions')

        common_defines['CLANG_VENDOR'] = 'OHOS (%s) ' % build_name
        common_defines['CLANG_VENDOR_BUILD_VERSION'] = build_number
        common_defines.update(extra_defines)

        env = dict(self.build_config.ORIG_ENV)
        if extra_env is not None:
            env.update(extra_env)

        llvm_project_path = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'llvm'))

        self.invoke_cmake(llvm_project_path,
                          build_dir,
                          common_defines,
                          env=env)

        self.invoke_ninja(out_path=build_dir,
                          env=env,
                          target=None,
                          install=True)

    def llvm_compile_darwin_defines(self, llvm_defines):
        if self.host_is_darwin():
            llvm_defines['LIBUNWIND_ENABLE_SHARED'] = 'OFF'
            llvm_defines['LLDB_NO_DEBUGSERVER'] = 'ON'
            llvm_defines['COMPILER_RT_BUILD_LIBFUZZER'] = 'OFF'
            llvm_defines['LLVM_BUILD_EXTERNAL_COMPILER_RT'] = 'ON'
            llvm_defines['LLVM_ENABLE_ZSTD'] = 'OFF'
            llvm_defines['LLDB_PYTHON_EXT_SUFFIX'] = '.dylib'
            if self.build_config.build_ncurses:
                libncurse = os.path.join(self.get_prebuilts_dir('ncurses'), 'lib', 'libncurses.6.dylib')
                libpanel = os.path.join(self.get_prebuilts_dir('ncurses'), 'lib', 'libpanel.6.dylib')
                libform = os.path.join(self.get_prebuilts_dir('ncurses'), 'lib', 'libform.6.dylib')
                ncurses_libs = ';'.join([libncurse, libpanel, libform])
                llvm_defines['CURSES_LIBRARIES'] = ncurses_libs
                llvm_defines['PANEL_LIBRARIES'] = ncurses_libs

            if self.build_config.build_libedit:
                llvm_defines['LibEdit_LIBRARIES'] = os.path.join(self.get_prebuilts_dir('libedit'), 'lib', 'libedit.0.dylib')

            libxml2_version = self.get_libxml2_version()
            if self.build_config.build_libxml2 and libxml2_version is not None:
                llvm_defines['LIBXML2_LIBRARIES'] = os.path.join(self.get_prebuilts_dir('libxml2'), self.use_platform(), 'lib', f'libxml2.{libxml2_version}.dylib')


    def llvm_compile_linux_defines(self,
                                   llvm_defines,
                                   debug_build=False,
                                   no_lto=False,
                                   build_instrumented=False):
        if self.host_is_linux():
            llvm_defines['LLVM_ENABLE_LLD'] = 'ON'
            llvm_defines['COMPILER_RT_BUILD_LIBFUZZER'] = 'ON'
            llvm_defines['LIBCXX_ENABLE_STATIC_ABI_LIBRARY'] = 'ON'
            llvm_defines['LIBCXX_ENABLE_ABI_LINKER_SCRIPT'] = 'OFF'
            llvm_defines['LIBCXX_USE_COMPILER_RT'] = 'ON'
            llvm_defines['LIBCXXABI_USE_LLVM_UNWINDER'] = 'ON'
            llvm_defines['LIBCXXABI_ENABLE_STATIC_UNWINDER'] = 'ON'
            llvm_defines['LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY'] = 'YES'
            llvm_defines['LIBCXXABI_USE_COMPILER_RT'] = 'ON'
            llvm_defines['COMPILER_RT_USE_LLVM_UNWINDER'] = 'ON'
            llvm_defines['COMPILER_RT_ENABLE_STATIC_UNWINDER'] = 'ON'
            llvm_defines['COMPILER_RT_USE_BUILTINS_LIBRARY'] = 'ON'
            llvm_defines['COMPILER_RT_BUILD_ORC'] = 'OFF'
            llvm_defines['LIBUNWIND_USE_COMPILER_RT'] = 'ON'
            llvm_defines['LLVM_BINUTILS_INCDIR'] = '/usr/include'
            llvm_defines['LLDB_PYTHON_EXT_SUFFIX'] = '.so'
            ncurses_version = self.get_ncurses_version()
            if self.build_config.build_ncurses and ncurses_version is not None:
                ncurses_libs = []
                prebuilts_dir = self.get_prebuilts_dir('ncurses')
                for library in ['libncurses', 'libpanel', 'libform']:
                    library_path = os.path.join(prebuilts_dir, 'lib', f'{library}.so.%s' % ncurses_version)
                    ncurses_libs.append(library_path)
                ncurses_libs = ';'.join(ncurses_libs)
                llvm_defines['CURSES_LIBRARIES'] = ncurses_libs
                llvm_defines['PANEL_LIBRARIES'] = ncurses_libs

            if self.build_config.build_libedit:
                llvm_defines['LibEdit_LIBRARIES'] = os.path.join(self.get_prebuilts_dir('libedit'), 'lib', 'libedit.so.0.0.68')

            if not build_instrumented and not no_lto and not debug_build:
                llvm_defines['LLVM_ENABLE_LTO'] = 'Thin'

            libxml2_version = self.get_libxml2_version()
            if self.build_config.build_libxml2 and libxml2_version is not None:
                llvm_defines['LIBXML2_LIBRARY'] = os.path.join(self.get_prebuilts_dir('libxml2'), self.use_platform(), 'lib', f'libxml2.so.{libxml2_version}')

    def llvm_compile_llvm_defines(self, llvm_defines, llvm_root, cflags, ldflags):
        llvm_defines['LLVM_ENABLE_PROJECTS'] = 'clang;lld;clang-tools-extra;openmp;lldb'
        llvm_defines['LLVM_ENABLE_RUNTIMES'] = 'libunwind;libcxxabi;libcxx;compiler-rt'
        llvm_defines['LLVM_ENABLE_BINDINGS'] = 'OFF'
        llvm_defines['CMAKE_C_COMPILER'] = os.path.join(llvm_root, 'bin', 'clang')
        llvm_defines['CMAKE_CXX_COMPILER'] = os.path.join(llvm_root, 'bin', 'clang++')
        llvm_defines['CMAKE_AR'] = os.path.join(llvm_root, 'bin', 'llvm-ar')
        llvm_defines['CMAKE_RANLIB'] = os.path.join(llvm_root, 'bin', 'llvm-ranlib')
        llvm_defines['LLVM_ENABLE_LIBCXX'] = 'ON'
        llvm_defines['SANITIZER_ALLOW_CXXABI'] = 'OFF'
        llvm_defines['LIBOMP_ENABLE_SHARED'] = 'FALSE'
        llvm_defines['OPENMP_TEST_FLAGS'] = '-Wl,-ldl'
        llvm_defines['CLANG_BUILD_EXAMPLES'] = 'OFF'
        llvm_defines['LLDB_ENABLE_LIBEDIT'] = 'OFF'
        llvm_defines['COMPILER_RT_BUILD_SANITIZERS'] = 'OFF'
        llvm_defines['COMPILER_RT_BUILD_MEMPROF'] = 'OFF'
        llvm_defines['CMAKE_ASM_FLAGS'] = cflags
        llvm_defines['CMAKE_C_FLAGS'] = cflags
        llvm_defines['CMAKE_CXX_FLAGS'] = '%s -stdlib=libc++' % cflags
        llvm_defines['CMAKE_EXE_LINKER_FLAGS'] = ldflags
        llvm_defines['CMAKE_SHARED_LINKER_FLAGS'] = ldflags
        llvm_defines['CMAKE_MODULE_LINKER_FLAGS'] = ldflags
        llvm_defines['CMAKE_POSITION_INDEPENDENT_CODE'] = 'ON'
        llvm_defines['LLDB_ENABLE_PYTHON'] = 'ON'
        llvm_defines['LLDB_EMBED_PYTHON_HOME'] = 'ON'
        llvm_defines['LLDB_PYTHON_HOME'] = os.path.join('..', self.build_config.LLDB_PYTHON)
        llvm_defines['LLDB_PYTHON_RELATIVE_PATH'] = os.path.join('bin', 'python', 'lib', 'python%s'
            % self.build_config.LLDB_PY_VERSION)
        llvm_defines['LLDB_PYTHON_EXE_RELATIVE_PATH'] = os.path.join('bin', self.build_config.LLDB_PYTHON)
        llvm_defines['SWIG_EXECUTABLE'] = self.find_program('swig')
        llvm_defines['LLDB_ENABLE_CURSES'] = 'OFF'

        if self.build_config.build_ncurses and self.get_ncurses_version() is not None:
            llvm_defines['LLDB_ENABLE_CURSES'] = 'ON'
            llvm_defines['CURSES_INCLUDE_DIRS'] = os.path.join(self.get_prebuilts_dir('ncurses'), 'include')
        if self.build_config.build_libedit:
            llvm_defines['LLDB_ENABLE_LIBEDIT'] = 'ON'
            llvm_defines['LibEdit_INCLUDE_DIRS'] = os.path.join(self.get_prebuilts_dir('libedit'), 'include')

        if self.build_config.build_libxml2:
            llvm_defines['LLDB_ENABLE_LIBXML2'] = 'ON'
            llvm_defines['LIBXML2_INCLUDE_DIR'] = os.path.join(self.get_prebuilts_dir('libxml2'), self.use_platform(), 'include', 'libxml2')

    def llvm_compile(self,
                     build_name,
                     out_dir,
                     debug_build=False,
                     no_lto=False,
                     build_instrumented=False,
                     xunit_xml_output=None):

        llvm_clang_install = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR,
                                                          'prebuilts/clang/ohos', self.use_platform(),
                                                          'clang-%s' % self.build_config.CLANG_VERSION))
        llvm_path = self.merge_out_path('llvm_make')
        llvm_profdata = os.path.join(llvm_clang_install, 'bin', 'llvm-profdata')

        if self.host_is_darwin():
            ldflags = ''
        else:
            ldflags = '-fuse-ld=lld'
        ldflags = '%s -L%s' % (ldflags, os.path.join(llvm_clang_install, 'lib'))

        llvm_extra_env = {}
        llvm_extra_env['LD_LIBRARY_PATH'] = os.path.join(llvm_clang_install, 'lib')

        llvm_defines = {}

        self.llvm_compile_darwin_defines(llvm_defines)
        self.llvm_compile_linux_defines(llvm_defines, debug_build, no_lto, build_instrumented)

        if self.host_is_linux():
            ldflags += ' -l:libunwind.a -l:libc++abi.a --rtlib=compiler-rt -stdlib=libc++ -static-libstdc++'

        if xunit_xml_output:
            llvm_defines['LLVM_LIT_ARGS'] = "--xunit-xml-output={} -sv".format(xunit_xml_output)

        if self.build_config.enable_assertions:
            llvm_defines['LLVM_ENABLE_ASSERTIONS'] = 'ON'

        if debug_build:
            llvm_defines['CMAKE_BUILD_TYPE'] = 'Debug'

        if build_instrumented:
            llvm_defines['LLVM_BUILD_INSTRUMENTED'] = 'ON'
            llvm_defines['LLVM_PROFDATA'] = llvm_profdata

            resource_dir = "lib/clang/10.0.1/lib/linux/libclang_rt.profile-x86_64.a"
            ldflags += ' %s' % os.path.join(llvm_clang_install, resource_dir)

        cflags = '-fstack-protector-strong'
        if not self.host_is_darwin():
            ldflags += ' -Wl,-z,relro,-z,now -pie'
            if self.build_config.strip:
                ldflags += ' -s'

        self.llvm_compile_llvm_defines(llvm_defines, llvm_clang_install, cflags, ldflags)

        linker_path = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts', 'clang',
            'ohos', 'linux-x86_64', 'llvm', 'bin', 'ld.lld'))
        llvm_defines['CMAKE_LINKER'] = linker_path

        self.build_llvm(targets=self.build_config.TARGETS,
                        build_dir=llvm_path,
                        install_dir=out_dir,
                        build_name=build_name,
                        extra_defines=llvm_defines,
                        extra_env=llvm_extra_env)

    def llvm_compile_windows_defines(self,
                                     windows_defines,
                                     cc,
                                     cxx,
                                     windows_sysroot):

        if self.build_config.enable_assertions:

            windows_defines['LLVM_ENABLE_ASSERTIONS'] = 'ON'

        mingw_python_dir = self.get_mingw_python_dir()
        py_dir = mingw_python_dir if mingw_python_dir else windows_sysroot
        windows_defines['LLDB_RELOCATABLE_PYTHON'] = 'OFF'
        windows_defines['LLDB_ENABLE_PYTHON'] = 'ON'
        windows_defines['LLDB_PYTHON_HOME'] = 'python'
        windows_defines['LLDB_PYTHON_RELATIVE_PATH'] = \
            'bin/python/lib/python%s' % (self.build_config.LLDB_PY_VERSION)
        windows_defines['LLDB_PYTHON_EXE_RELATIVE_PATH'] = 'bin/python'
        windows_defines['LLDB_PYTHON_EXT_SUFFIX'] = '.pys'
        windows_defines['Python3_INCLUDE_DIRS'] = os.path.join(py_dir, 'include',
            'python%s' % self.build_config.LLDB_PY_VERSION)
        windows_defines['Python3_LIBRARIES'] = os.path.join(py_dir, 'lib', 'libpython%s.dll.a'
            % self.build_config.LLDB_PY_VERSION)
        windows_defines['Python3_EXECUTABLE'] = os.path.join(self.get_python_dir(), 'bin',
            self.build_config.LLDB_PYTHON)
        windows_defines['SWIG_EXECUTABLE'] = self.find_program('swig')

        windows_defines['CMAKE_C_COMPILER'] = cc
        windows_defines['CMAKE_CXX_COMPILER'] = cxx
        windows_defines['CMAKE_SYSTEM_NAME'] = 'Windows'
        windows_defines['CMAKE_BUILD_TYPE'] = 'Release'
        windows_defines['LLVM_BUILD_RUNTIME'] = 'OFF'
        windows_defines['LLVM_TOOL_CLANG_TOOLS_EXTRA_BUILD'] = 'ON'
        windows_defines['LLVM_TOOL_OPENMP_BUILD'] = 'OFF'
        windows_defines['LLVM_INCLUDE_TESTS'] = 'OFF'
        windows_defines['LLVM_ENABLE_LIBCXX'] = 'ON'
        windows_defines['LLVM_ENABLE_PROJECTS'] = 'clang;clang-tools-extra;lld;lldb'
        windows_defines['LLVM_BUILD_LLVM_DYLIB'] = 'OFF'
        windows_defines['CLANG_BUILD_EXAMPLES'] = 'OFF'
        windows_defines['CMAKE_SYSROOT'] = windows_sysroot
        windows_defines['CMAKE_FIND_ROOT_PATH_MODE_INCLUDE'] = 'ONLY'
        windows_defines['CMAKE_FIND_ROOT_PATH_MODE_LIBRARY'] = 'ONLY'
        windows_defines['CMAKE_FIND_ROOT_PATH_MODE_PACKAGE'] = 'ONLY'
        windows_defines['CMAKE_FIND_ROOT_PATH_MODE_PROGRAM'] = 'NEVER'
        windows_defines['LLDB_ENABLE_LIBEDIT'] = 'OFF'
        windows_defines['LLDB_RELOCATABLE_PYTHON'] = 'OFF'

        if self.build_config.build_libxml2:
            windows_defines['LLDB_ENABLE_LIBXML2'] = 'ON'
            windows_defines['LIBXML2_INCLUDE_DIR'] = os.path.join(self.get_prebuilts_dir('libxml2'), 'windows-x86_64', 'include', 'libxml2')
            windows_defines['LIBXML2_LIBRARY'] = os.path.join(self.get_prebuilts_dir('libxml2'), 'windows-x86_64', 'lib', 'libxml2.dll.a')

    def llvm_compile_windows_flags(self,
                                   windows_defines,
                                   windowstool_path,
                                   windows64_install,
                                   ldflags,
                                   cflags):

        windows_defines['CROSS_TOOLCHAIN_FLAGS_NATIVE'] = ';'.join([
            '-DCMAKE_PREFIX_PATH=%s' % self.CMAKE_BIN_DIR,
            '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
            '-DLLVM_ENABLE_LIBCXX=ON',
            '-DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE',
            '-DCMAKE_INSTALL_RPATH=%s' % os.path.join(windowstool_path, 'lib')]
        )

        ldflag = ['-fuse-ld=lld',
                  '-stdlib=libc++',
                  '--rtlib=compiler-rt',
                  '-lunwind', '-Wl,--dynamicbase',
                  '-Wl,--nxcompat',
                  '-lucrt',
                  '-lucrtbase',
                  '-L',
                  os.path.join(windows64_install, 'lib'),
                  '-Wl,--high-entropy-va']
        ldflags.extend(ldflag)

        cflag = ['-stdlib=libc++',
                 '--target=x86_64-pc-windows-gnu',
                 '-D_LARGEFILE_SOURCE',
                 '-D_FILE_OFFSET_BITS=64',
                 '-D_WIN32_WINNT=0x0600',
                 '-DWINVER=0x0600',
                 '-D__MSVCRT_VERSION__=0x1400',
                 '-DMS_WIN64']
        cflags.extend(cflag)

    def llvm_compile_windows_cmake(self,
                                   cflags,
                                   cxxflags,
                                   ldflags,
                                   windows_defines):


        zlib_path = self.merge_out_path('../', 'prebuilts', 'clang', 'host', 'windows-x86', 'toolchain-prebuilts',
                                        'zlib')
        zlib_inc = os.path.join(zlib_path, 'include')
        zlib_lib = os.path.join(zlib_path, 'lib')

        cflags.extend(['-I', zlib_inc])
        cxxflags.extend(['-I', zlib_inc])
        ldflags.extend(['-L', zlib_lib])

        windows_defines['CMAKE_ASM_FLAGS'] = ' '.join(cflags)
        windows_defines['CMAKE_C_FLAGS'] = ' '.join(cflags)
        windows_defines['CMAKE_CXX_FLAGS'] = ' '.join(cxxflags)
        windows_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags)
        windows_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        windows_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)


    def llvm_compile_for_windows(self,
                                 targets,
                                 enable_assertions,
                                 build_name):

        self.logger().info('Building llvm for windows.')

        build_dir = self.merge_out_path("windows-x86_64")
        windowstool_path = self.merge_out_path('llvm-install')
        windows64_install = self.merge_out_path('windows-x86_64-install')
        windows_sysroot = self.merge_out_path('mingw', self.build_config.MINGW_TRIPLE)

        self.check_create_dir(build_dir)

        # Write a NATIVE.cmake in windows_path that contains the compilers used
        # to build native tools such as llvm-tblgen and llvm-config.  This is
        # used below via the CMake variable CROSS_TOOLCHAIN_FLAGS_NATIVE.
        cc = os.path.join(windowstool_path, 'bin', 'clang')
        cxx = os.path.join(windowstool_path, 'bin', 'clang++')

        # Extra cmake defines to use while building for Windows
        windows_defines = {}
        self.llvm_compile_windows_defines(windows_defines, cc, cxx, windows_sysroot)

        # Set CMake path, toolchain file for native compilation (to build tablegen
        # etc).  Also disable libfuzzer build during native compilation.

        ldflags = []
        cflags = []

        self.llvm_compile_windows_flags(windows_defines,
            windowstool_path, windows64_install, ldflags, cflags)

        cxxflags = list(cflags)

        windows_extra_env = dict()

        cxxflags.append('-fuse-cxa-atexit')
        cxxflags.extend(('-I', os.path.join(windows64_install, 'include', 'c++', 'v1')))

        self.llvm_compile_windows_cmake(cflags, cxxflags, ldflags, windows_defines)

        self.build_llvm(self.build_config.TARGETS,
                        build_dir,
                        windows64_install,
                        build_name,
                        extra_defines=windows_defines,
                        extra_env=windows_extra_env)


class SysrootComposer(BuildUtils):

    def __init__(self, build_config):
        super(SysrootComposer, self).__init__(build_config)

    def setup_cmake_platform(self, llvm_install):

        # OHOS.cmake already exsit on cmake prebuilts,
        # but it didn't contain these two lines, so we still need OHOS.cmake.
        ohos_cmake = 'OHOS.cmake'
        dst_dir = self.merge_out_path(
            '../prebuilts/cmake/%s/share/cmake-3.16/Modules/Platform' % self.platform_prefix())
        src_file = '%s/%s' % (self.build_config.LLVM_BUILD_DIR, ohos_cmake)
        if os.path.exists(os.path.join(dst_dir, ohos_cmake)):
            os.remove(os.path.join(dst_dir, ohos_cmake))
        shutil.copy2(src_file, dst_dir)


    def build_musl(self, llvm_install, target, *extra_args):
        cur_dir = os.getcwd()
        os.chdir(self.build_config.LLVM_BUILD_DIR)
        self.logger().info('build musl %s', self.merge_out_path('install'))
        args = ['./build_musl.sh', '-t', target,
                '-c', self.merge_out_path(llvm_install, 'bin'),
                '-o', self.merge_out_path(),
                '-T', self.build_config.REPOROOT_DIR] + list(extra_args)
        self.check_call(args)
        os.chdir(cur_dir)

    def install_linux_headers(self, arch, target):
        dir_suffix = arch
        if arch == 'x86_64':
            dir_suffix = 'x86'
        elif arch == 'mipsel':
            dir_suffix = 'mips'
        linux_kernel_dir = os.path.join('kernel_linux_patches', 'linux-5.10')
        linux_kernel_path = os.path.join(self.build_config.OUT_PATH, '..', linux_kernel_dir)
        ohosmusl_sysroot_dst = self.merge_out_path('sysroot', target, 'usr')
        headers_tmp_dir = os.path.join(linux_kernel_path, 'prebuilts', 'usr', 'include')
        self.check_copy_tree(os.path.join(headers_tmp_dir, 'linux'),
                             os.path.join(ohosmusl_sysroot_dst, 'include/linux'))
        self.check_copy_tree(os.path.join(headers_tmp_dir, 'asm-%s' % dir_suffix,'asm'),
                             os.path.join(ohosmusl_sysroot_dst, 'include', 'asm'))
        self.check_copy_tree(os.path.join(headers_tmp_dir, 'asm-generic'),
                             os.path.join(ohosmusl_sysroot_dst, 'include/asm-generic'))

    def copy_libz_to_sysroot(self, libz_path, llvm_triple):
        # Install to sysroot
        dest_usr = self.merge_out_path('sysroot', llvm_triple, 'usr')
        dest_usr_include = os.path.join(dest_usr, 'include')

        # Copy over usr/include.
        zlib_h = self.merge_out_path('../third_party/zlib', 'zlib.h')
        self.check_copy_file(zlib_h, dest_usr_include)

        zconf_h = os.path.join(libz_path, 'zconf.h')
        self.check_copy_file(zconf_h, dest_usr_include)

        # Copy over usr/lib.
        dest_usr_lib = os.path.join(dest_usr, 'lib')
        static_zlib = os.path.join(libz_path, 'libz.a')
        self.check_copy_file(static_zlib, dest_usr_lib)


class LlvmLibs(BuildUtils):

    def __init__(self, build_config, sysroot_composer, llvm_package):
        super(LlvmLibs, self).__init__(build_config)
        self.sysroot_composer = sysroot_composer
        self.llvm_package = llvm_package

    def build_crt_libs(self, configs, llvm_install):
        for (arch, target) in configs:
            self.sysroot_composer.build_musl(llvm_install, target)
            if target.endswith(self.build_config.OPENHOS_SFX):
                self.sysroot_composer.install_linux_headers(arch, target)
            self.build_libs(llvm_install,
                            target,
                            precompilation=True)
            self.sysroot_composer.build_musl(llvm_install, target, '-l')

    def build_libs_defines(self,
                           llvm_triple,
                           defines,
                           cc,
                           cxx,
                           ar,
                           llvm_config,
                           ldflags,
                           cflags,
                           extra_flags):

        sysroot = self.merge_out_path('sysroot')

        defines['CMAKE_C_COMPILER'] = cc
        defines['CMAKE_CXX_COMPILER'] = cxx
        defines['CMAKE_AR'] = ar
        defines['LLVM_CONFIG_PATH'] = llvm_config
        defines['CMAKE_SYSROOT'] = sysroot
        defines['CMAKE_FIND_ROOT_PATH_MODE_INCLUDE'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_LIBRARY'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_PACKAGE'] = 'ONLY'
        defines['CMAKE_FIND_ROOT_PATH_MODE_PROGRAM'] = 'NEVER'
        defines['CMAKE_POSITION_INDEPENDENT_CODE'] = 'ON'

        ldflag = [
                '-fuse-ld=lld',
                '-Wl,--gc-sections',
                '-Wl,--build-id=sha1',
                '--rtlib=compiler-rt',
                '-stdlib=libc++', ]

        if not self.host_is_darwin():
            ldflag.append('-Wl,-z,relro,-z,now -pie')
            if self.build_config.strip:
                ldflag.append('-s')
        
        ldflags.extend(ldflag)

        cflag = [
                '-fstack-protector-strong', 
                '--target=%s' % llvm_triple,
                '-ffunction-sections',
                '-fdata-sections',
                extra_flags, ]

        cflags.extend(cflag)

    def build_libs(self, llvm_install, llvm_build, precompilation=False):
        configs_list = [
            ('arm', self.liteos_triple('arm'), '-march=armv7-a -mfloat-abi=soft', ''),
            ('arm', self.liteos_triple('arm'), '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=soft', 'a7_soft'),
            ('arm', self.liteos_triple('arm'),
             '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4', 'a7_softfp_neon-vfpv4'),
            ('arm', self.liteos_triple('arm'),
             '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4', 'a7_hard_neon-vfpv4'),

            ('arm', self.open_ohos_triple('arm'), '-march=armv7-a -mfloat-abi=soft', ''),
            ('arm', self.open_ohos_triple('arm'), '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=soft', 'a7_soft'),
            ('arm', self.open_ohos_triple('arm'),
             '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4', 'a7_softfp_neon-vfpv4'),
            ('arm', self.open_ohos_triple('arm'),
             '-march=armv7-a -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4', 'a7_hard_neon-vfpv4'),
            ('aarch64', self.open_ohos_triple('aarch64'), '', ''),
            ('riscv64', self.open_ohos_triple('riscv64'), '', ''),
            ('mipsel', self.open_ohos_triple('mipsel'), '', ''),
            ('mipsel', self.open_ohos_triple('mipsel'), '-mnan=legacy', 'nanlegacy'),
            ('x86_64', self.open_ohos_triple('x86_64'), '', ''),]

        cc = os.path.join(llvm_install, 'bin', 'clang')
        cxx = os.path.join(llvm_install, 'bin', 'clang++')
        ar = os.path.join(llvm_install, 'bin', 'llvm-ar')
        llvm_config = os.path.join(llvm_install, 'bin', 'llvm-config')
        seen_arch_list = [self.liteos_triple('arm')]

        self.set_clang_version(llvm_install)
        for (arch, llvm_triple, extra_flags, multilib_suffix) in configs_list:
            if llvm_build != llvm_triple:
                continue

            has_lldb_tools = arch not in ['riscv64', 'mipsel']

            defines = self.base_cmake_defines()
            ldflags = []
            cflags = []
            self.logger().info('Build libs for %s', llvm_triple)
            if self.build_config.target_debug:
                defines['CMAKE_BUILD_TYPE'] = 'Debug'
            self.build_libs_defines(llvm_triple, defines, cc, cxx, ar, llvm_config, ldflags, cflags, extra_flags)
            if arch == 'mipsel':
                ldflags.append('-Wl,-z,notext')
                ldflags.append('-Wl,--no-check-dynamic-relocations')

            llvm_path = self.merge_out_path('llvm_make')
            arch_list = [self.liteos_triple('arm'), self.open_ohos_triple('arm'),
                         self.open_ohos_triple('aarch64'), self.open_ohos_triple('riscv64'),
                         self.open_ohos_triple('mipsel'), self.open_ohos_triple('x86_64')]
            if precompilation:
                self.build_crts(llvm_install, arch, llvm_triple, cflags, ldflags, multilib_suffix, defines)
                continue
            # libunwind is added to linker command line by OHOS toolchain, so we have to use two step build
            self.build_runtimes(llvm_install, "libunwind", ldflags, cflags, llvm_triple, arch, multilib_suffix, defines)
            self.build_runtimes(llvm_install, "libunwind;libcxxabi;libcxx", ldflags, cflags, llvm_triple, arch, multilib_suffix, defines)

            libcxx_ndk_install = self.merge_out_path('libcxx-ndk')
            self.check_create_dir(libcxx_ndk_install)
            self.build_runtimes(libcxx_ndk_install, "libunwind;libcxxabi;libcxx", ldflags, cflags, llvm_triple,
                                    arch, multilib_suffix, defines, True)

            self.build_crts(llvm_install, arch, llvm_triple, cflags, ldflags, multilib_suffix, defines,
                                first_time=False)

            if llvm_triple in arch_list:
                if self.build_config.need_lldb_tools and has_lldb_tools and llvm_triple not in seen_arch_list:
                    self.build_lldb_tools(llvm_install, llvm_path, arch, llvm_triple, cflags, ldflags,
                                                   defines)
                    seen_arch_list.append(llvm_triple)
                continue

            self.build_libomp(llvm_install, arch, llvm_triple, cflags, ldflags, multilib_suffix, defines)
            self.build_libz(arch, llvm_triple, cflags, ldflags, defines)
            if self.build_config.need_lldb_tools and has_lldb_tools and llvm_triple not in seen_arch_list:
                self.build_lldb_tools(llvm_install, llvm_path, arch, llvm_triple, cflags, ldflags, defines)
                seen_arch_list.append(llvm_triple)

    def build_runtimes(self,
                       llvm_install,
                       rt_list,
                       ldflags,
                       cflags,
                       llvm_triple,
                       arch,
                       multilib_suffix,
                       defines,
                       is_libcxx_ndk_install=False):

        self.logger().info('Building runtimes(%s) for %s', rt_list, arch)

        ndk_suffix = '-ndk' if is_libcxx_ndk_install else ''
        multi_suffix = '-' + multilib_suffix if multilib_suffix else ''
        out_path = self.merge_out_path('lib', rt_list.replace(';', '-') + ndk_suffix + '-' + str(llvm_triple) + multi_suffix)

        libcxx_install_include_path = self.merge_out_path(llvm_install, 'include', 'libcxx-ohos', 'include', 'c++', 'v1')

        rt_cflags = list(cflags)
        rt_cflags.append('-fstack-protector-strong')
        rt_cflags.append('-funwind-tables')
        rt_cflags.append('-fno-omit-frame-pointer')

        rt_defines = defines.copy()
        rt_defines['OHOS'] = '1'
        rt_defines['LLVM_ENABLE_PER_TARGET_RUNTIME_DIR'] = 'ON'
        rt_defines['LLVM_TARGET_MULTILIB_SUFFIX'] = multilib_suffix
        rt_defines['LLVM_DEFAULT_TARGET_TRIPLE'] = llvm_triple
        rt_defines['LLVM_ENABLE_RUNTIMES'] = rt_list
        rt_defines['LIBUNWIND_USE_COMPILER_RT'] = 'ON'
        rt_defines['LIBUNWIND_ENABLE_SHARED'] = 'OFF'
        rt_defines['LIBCXXABI_USE_COMPILER_RT'] = 'ON'
        rt_defines['LIBCXXABI_USE_LLVM_UNWINDER'] = 'ON'
        rt_defines['LIBCXXABI_ENABLE_STATIC_UNWINDER'] = 'ON'
        rt_defines['LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY'] = 'OFF'
        rt_defines['LIBCXXABI_HAS_CXA_THREAD_ATEXIT_IMPL'] = 'OFF'
        rt_defines['LIBCXXABI_ENABLE_SHARED'] = 'OFF'
        rt_defines['LIBCXXABI_LIBCXX_INCLUDES'] = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'libcxx', 'include'))
        rt_defines['LIBCXXABI_INSTALL_INCLUDE_DIR'] = libcxx_install_include_path
        rt_defines['LIBCXX_USE_COMPILER_RT'] = 'ON'
        rt_defines['LIBCXX_ENABLE_ABI_LINKER_SCRIPT'] = 'OFF'
        rt_defines['LIBCXX_ENABLE_STATIC_ABI_LIBRARY'] = 'ON'
        rt_defines['LIBCXX_INSTALL_INCLUDE_DIR'] = libcxx_install_include_path
        rt_defines['LIBCXX_INSTALL_INCLUDE_TARGET_DIR'] = libcxx_install_include_path
        rt_defines['CMAKE_ASM_FLAGS'] = ' '.join(rt_cflags)
        rt_defines['CMAKE_C_FLAGS'] = ' '.join(rt_cflags)
        rt_defines['CMAKE_CXX_FLAGS'] = ' '.join(rt_cflags)
        rt_defines['CMAKE_INSTALL_PREFIX'] = llvm_install
        rt_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        rt_defines['CMAKE_SYSTEM_NAME'] = 'OHOS'
        rt_defines['CMAKE_CROSSCOMPILING'] = 'True'
        rt_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'

        if is_libcxx_ndk_install:
            rt_defines['LIBCXX_ABI_NAMESPACE'] = '__n1'
            rt_defines['LIBCXX_OUTPUT_NAME'] = 'c++_shared'
            rt_defines['LIBCXX_OUTPUT_STATIC_NAME'] = 'c++_static'
            rt_defines['LIBCXXABI_INSTALL_LIBRARY'] = 'OFF'
            rt_defines['LIBUNWIND_INSTALL_LIBRARY'] = 'OFF'
        else:
            rt_defines['LIBCXX_ABI_NAMESPACE'] = '__h'
 
        self.check_rm_tree(out_path)
        cmake_rt = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'runtimes'))
 
        self.invoke_cmake(cmake_rt,
                          out_path,
                          rt_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=out_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=None,
                          install=True)

    def build_crts(self,
                   llvm_install,
                   arch,
                   llvm_triple,
                   cflags,
                   ldflags,
                   multilib_suffix,
                   defines,
                   first_time=True):

        self.logger().info('Building compiler-rt for %s', arch)

        suffix = '-' + multilib_suffix if multilib_suffix else ''
        crt_path = self.merge_out_path('lib', 'clangrt-%s%s' % (llvm_triple, suffix))
        crt_install = os.path.join(llvm_install, 'lib', 'clang', self.build_config.VERSION)

        crt_extra_flags = []
        if not self.build_config.target_debug:
            # Remove absolute paths from compiler-rt debug info emitted with -gline-tables-only
            crt_extra_flags = ['-ffile-prefix-map=%s=.' % self.build_config.REPOROOT_DIR]

        crt_defines = defines.copy()
        crt_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags)
        crt_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        crt_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)
        crt_defines['CMAKE_C_FLAGS'] = ' '.join(cflags + crt_extra_flags)
        crt_defines['CMAKE_ASM_FLAGS'] = ' '.join(cflags + crt_extra_flags)
        crt_defines['CMAKE_CXX_FLAGS'] = ' '.join(cflags + crt_extra_flags)
        crt_defines['COMPILER_RT_TEST_COMPILER_CFLAGS'] = ' '.join(cflags)
        crt_defines['OHOS'] = '1'
        crt_defines['COMPILER_RT_TEST_TARGET_TRIPLE'] = llvm_triple
        crt_defines['COMPILER_RT_TEST_STANDALONE_BUILD_LIBS'] = 'OFF'
        crt_defines['COMPILER_RT_INCLUDE_TESTS'] = 'ON'
        crt_defines['CMAKE_INSTALL_PREFIX'] = crt_install
        crt_defines['LLVM_TARGET_MULTILIB_SUFFIX'] = multilib_suffix
        if first_time or llvm_triple == self.liteos_triple('arm'):
            crt_defines['COMPILER_RT_BUILD_LIBFUZZER'] = 'OFF'
        else:
            crt_defines['COMPILER_RT_BUILD_LIBFUZZER'] = 'ON'
        crt_defines['COMPILER_RT_BUILD_ORC'] = 'OFF'
        crt_defines['LLVM_ENABLE_PER_TARGET_RUNTIME_DIR'] = 'ON'
        crt_defines['COMPILER_RT_USE_BUILTINS_LIBRARY'] = 'ON'
        crt_defines['CMAKE_SYSTEM_NAME'] = 'OHOS'
        crt_defines['CMAKE_CROSSCOMPILING'] = 'True'
        crt_defines['SANITIZER_CXX_ABI'] = 'libcxxabi'
        crt_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'
        crt_defines['COMPILER_RT_HWASAN_WITH_INTERCEPTORS'] = 'OFF'
        crt_defines['COMPILER_RT_BUILD_SANITIZERS'] = \
            'OFF' if llvm_triple == self.liteos_triple('arm') or first_time else 'ON'
        crt_defines['COMPILER_RT_DEFAULT_TARGET_TRIPLE'] = llvm_triple
        crt_cmake_path = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'compiler-rt'))
        self.rm_cmake_cache(crt_path)

        self.invoke_cmake(crt_cmake_path,
                          crt_path,
                          crt_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=crt_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=None,
                          install=True)

    def build_libomp(self,
                     llvm_install,
                     arch,
                     llvm_triple,
                     cflags,
                     ldflags,
                     multilib_suffix,
                     defines):

        self.logger().info('Building libomp for %s', arch)

        libomp_path = self.merge_out_path('lib', 'libomp-%s' % llvm_triple)
        out_dir = os.path.join(libomp_path, 'lib')

        libomp_cflags = list(cflags)
        libomp_cflags.append('-fPIC')

        libomp_defines = defines.copy()
        libomp_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags)
        libomp_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        libomp_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)

        libomp_defines['OHOS'] = '1'
        libomp_defines['CMAKE_ASM_FLAGS'] = ' '.join(libomp_cflags)
        libomp_defines['CMAKE_C_FLAGS'] = ' '.join(libomp_cflags)
        libomp_defines['CMAKE_CXX_FLAGS'] = ' '.join(libomp_cflags)
        libomp_defines['OPENMP_ENABLE_LIBOMPTARGET'] = 'FALSE'

        libomp_defines['OPENMP_LIBDIR_SUFFIX'] = os.path.join(os.sep, llvm_triple, multilib_suffix)
        libomp_defines['LIBOMP_ENABLE_SHARED'] = 'FALSE'
        libomp_defines['CMAKE_POLICY_DEFAULT_CMP0056'] = 'NEW'
        libomp_defines['CMAKE_INSTALL_PREFIX'] = llvm_install
        libomp_defines['COMPILER_RT_USE_BUILTINS_LIBRARY'] = 'ON'
        libomp_defines['CMAKE_SYSTEM_NAME'] = 'OHOS'
        libomp_defines['CMAKE_CROSSCOMPILING'] = 'True'
        libomp_defines['LLVM_ENABLE_LIBCXX'] = 'ON'
        libomp_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'

        libomp_cmake_path = os.path.join(self.build_config.LLVM_PROJECT_DIR, 'openmp')
        self.rm_cmake_cache(libomp_path)

        self.invoke_cmake(libomp_cmake_path,
                          libomp_path,
                          libomp_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=libomp_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=None,
                          install=True)

    def build_libz(self,
                   arch,
                   llvm_triple,
                   cflags,
                   ldflags,
                   defines):

        self.logger().info('Building libz for %s ', arch)

        libz_path = self.merge_out_path('lib', 'libz-%s' % llvm_triple)
        if llvm_triple == self.hos_triple('arm'):
            ldflags.append('-lunwind')

        libz_cflags = list(cflags)
        libz_cflags.append('-fPIC')

        libz_defines = defines.copy()
        libz_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags)
        libz_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        libz_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)
        libz_defines['OHOS'] = '1'
        libz_defines['CMAKE_ASM_FLAGS'] = ' '.join(libz_cflags)
        libz_defines['CMAKE_C_FLAGS'] = ' '.join(libz_cflags)
        libz_defines['CMAKE_CXX_FLAGS'] = ' '.join(libz_cflags)
        libz_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'

        libz_cmake_path = os.path.join(self.build_config.REPOROOT_DIR, 'third_party/zlib')
        self.rm_cmake_cache(libz_path)

        self.invoke_cmake(libz_cmake_path,
                          libz_path,
                          libz_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=libz_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=None,
                          install=False)

        self.sysroot_composer.copy_libz_to_sysroot(libz_path, llvm_triple)

    def build_lldb_tools(self,
                          llvm_install,
                          llvm_path,
                          arch,
                          llvm_triple,
                          cflags,
                          ldflags,
                          defines):

        self.logger().info('Building lldb for %s', arch)

        lldb_path = self.merge_out_path('lib', 'lldb-server-%s' % llvm_triple)
        crt_install = os.path.join(llvm_install, 'lib', 'clang', self.build_config.VERSION)
        out_dir = os.path.join(lldb_path, 'bin')

        lldb_ldflags = list(ldflags)
        lldb_cflags = list(cflags)

        lldb_ldflags.append('-lunwind')
        lldb_ldflags.append('-static')

        lldb_defines = defines.copy()

        lldb_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(lldb_ldflags)
        lldb_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(lldb_ldflags)
        lldb_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(lldb_ldflags)

        lldb_defines['OHOS'] = '1'
        lldb_defines['CMAKE_C_FLAGS'] = ' '.join(lldb_cflags)
        lldb_defines['CMAKE_ASM_FLAGS'] = ' '.join(lldb_cflags)
        lldb_defines['CMAKE_CXX_FLAGS'] = ' '.join(lldb_cflags)
        lldb_defines['CMAKE_INSTALL_PREFIX'] = crt_install
        lldb_defines['LLVM_ENABLE_PER_TARGET_RUNTIME_DIR'] = 'ON'
        lldb_defines['LLDB_ENABLE_PYTHON'] = 'OFF'
        lldb_defines['LLDB_PYTHON_HOME'] = os.path.join('..', self.build_config.LLDB_PYTHON)
        lldb_defines['LLDB_PYTHON_RELATIVE_PATH'] = os.path.join('bin', self.build_config.LLDB_PYTHON, 'lib',
            'python%s' % self.build_config.LLDB_PY_VERSION, llvm_triple)
        lldb_defines['LLDB_PYTHON_EXE_RELATIVE_PATH'] = os.path.join('bin', self.build_config.LLDB_PYTHON)
        lldb_defines['LLDB_PYTHON_EXT_SUFFIX'] = '.so'
        lldb_defines['COMPILER_RT_USE_BUILTINS_LIBRARY'] = 'ON'
        lldb_defines['CMAKE_SYSTEM_NAME'] = 'OHOS'
        lldb_defines['CMAKE_CROSSCOMPILING'] = 'True'
        lldb_defines['LLVM_DEFAULT_TARGET_TRIPLE'] = llvm_triple
        lldb_defines['LLVM_ENABLE_LIBCXX'] = 'ON'
        lldb_defines['LLVM_ENABLE_PROJECTS'] = 'clang;lldb'
        lldb_defines['LLVM_TABLEGEN'] = os.path.join(llvm_install, 'bin', 'llvm-tblgen')
        lldb_defines['CLANG_TABLEGEN'] = os.path.join(llvm_install, '..', llvm_path, 'bin', 'clang-tblgen')
        lldb_defines['LLDB_TABLEGEN'] = os.path.join(llvm_install, '..', llvm_path, 'bin', 'lldb-tblgen')
        lldb_defines['LLVM_HOST_TRIPLE'] = llvm_triple
        lldb_defines['LLVM_TARGET_ARCH'] = arch
        lldb_defines['LLVM_TARGETS_TO_BUILD'] = self.build_config.TARGETS

        lldb_target = ['lldb-server'] if self.build_config.need_lldb_server else []

        if self.build_config.build_lldb_static and llvm_triple in ['aarch64-linux-ohos', 'arm-linux-ohos']:
            lldb_defines['LIBLLDB_BUILD_STATIC'] = 'ON'
            lldb_target.append('lldb')

        if self.build_config.lldb_timeout:
            lldb_defines['LLDB_ENABLE_TIMEOUT'] = 'True'

        lldb_cmake_path = os.path.join(self.build_config.LLVM_PROJECT_DIR, 'llvm')

        self.invoke_cmake(lldb_cmake_path,
                          lldb_path,
                          lldb_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=lldb_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=lldb_target,
                          install=False)

        self.llvm_package.copy_lldb_tools_to_llvm_install(lldb_target, lldb_path, crt_install, llvm_triple)

    def build_runtimes_for_windows(self, enable_assertions):

        self.logger().info('Building libs for windows.')
        toolchain_dir = self.merge_out_path('llvm-install')
        install_dir = self.merge_out_path('windows-x86_64-install')
        windows_sysroot = self.merge_out_path('mingw', self.build_config.MINGW_TRIPLE)

        cflags = ['-stdlib=libc++', '--target=x86_64-pc-windows-gnu', '-D_LARGEFILE_SOURCE',
                  '-D_FILE_OFFSET_BITS=64', '-D_WIN32_WINNT=0x0600', '-DWINVER=0x0600', '-D__MSVCRT_VERSION__=0x1400']

        cmake_defines = {}
        cmake_defines['CMAKE_C_COMPILER'] = os.path.join(toolchain_dir, 'bin', 'clang')
        cmake_defines['CMAKE_CXX_COMPILER'] = os.path.join(toolchain_dir, 'bin', 'clang++')
        cmake_defines['CMAKE_CROSSCOMPILING'] = 'True'
        cmake_defines['CMAKE_SYSTEM_NAME'] = 'Windows'
        cmake_defines['CMAKE_SYSROOT'] = windows_sysroot
        cmake_defines['CMAKE_ASM_FLAGS'] = ' '.join(cflags)
        cmake_defines['CMAKE_C_FLAGS'] = ' '.join(cflags)
        cmake_defines['CMAKE_CXX_FLAGS'] = ' '.join(cflags)
        cmake_defines['CMAKE_BUILD_TYPE'] = 'Release'
        cmake_defines['CMAKE_TRY_COMPILE_TARGET_TYPE'] = 'STATIC_LIBRARY'
        cmake_defines['CMAKE_INSTALL_PREFIX'] = install_dir
        cmake_defines['LLVM_CONFIG_PATH'] = os.path.join(toolchain_dir, 'bin', 'llvm-config')
        cmake_defines['LIBCXX_ENABLE_SHARED'] = 'OFF'
        cmake_defines['LIBCXX_ENABLE_STATIC_ABI_LIBRARY'] = 'ON'
        cmake_defines['LIBCXX_ENABLE_NEW_DELETE_DEFINITIONS'] = 'ON'
        cmake_defines['LIBCXXABI_ENABLE_SHARED'] = 'OFF'
        cmake_defines['LIBUNWIND_ENABLE_SHARED'] = 'OFF'
        cmake_defines['LIBCXXABI_USE_LLVM_UNWINDER'] = 'ON'
        cmake_defines['LLVM_ENABLE_RUNTIMES'] = 'libunwind;libcxxabi;libcxx'
        cmake_defines['LLVM_ENABLE_ASSERTIONS'] = 'ON' if enable_assertions else 'OFF'
        cmake_defines['Python3_EXECUTABLE'] = os.path.join(self.get_python_dir(), 'bin',
            self.build_config.LLDB_PYTHON)

        out_path = self.merge_out_path('lib', 'windows-runtimes')
        cmake_path = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'runtimes'))

        self.invoke_cmake(cmake_path,
                          out_path,
                          cmake_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path,
                          env=dict(self.build_config.ORIG_ENV),
                          install=True)

    def build_ncurses(self, llvm_make, llvm_install):
        self.logger().info('Building ncurses.')

        libncurses_src_dir = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'ncurses'))
        libncurses_build_path = self.merge_out_path('ncurses')
        libncurses_install_path = self.get_prebuilts_dir('ncurses')
        prebuilts_path = os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts')

        self.check_rm_tree(libncurses_build_path)
        self.rm_cmake_cache(libncurses_build_path)
        self.check_rm_tree(libncurses_install_path)
        self.rm_cmake_cache(libncurses_install_path)

        cur_dir = os.getcwd()
        os.chdir(self.build_config.LLVM_BUILD_DIR)
        clang_version = self.build_config.CLANG_VERSION

        ncurses_version = self.get_ncurses_version()
        if ncurses_version is not None:
            args = ['./build_ncurses.sh', libncurses_src_dir, libncurses_build_path, libncurses_install_path,
                    prebuilts_path, clang_version, ncurses_version]
            self.check_call(args)
            os.chdir(cur_dir)

            self.llvm_package.copy_ncurses_to_llvm(llvm_make)
            self.llvm_package.copy_ncurses_to_llvm(llvm_install)

    def build_libedit(self, llvm_make, llvm_install):
        self.logger().info('Building libedit')

        libedit_src_dir = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'libedit'))
        libedit_build_path = self.merge_out_path('libedit')
        libedit_install_path = self.get_prebuilts_dir('libedit')
        prebuilts_path = os.path.join(self.build_config.REPOROOT_DIR, 'prebuilts')

        self.check_rm_tree(libedit_build_path)
        self.rm_cmake_cache(libedit_build_path)
        self.check_rm_tree(libedit_install_path)
        self.rm_cmake_cache(libedit_install_path)

        libncurses_path = self.get_prebuilts_dir('ncurses')

        cur_dir = os.getcwd()
        os.chdir(self.build_config.LLVM_BUILD_DIR)
        clang_version = self.build_config.CLANG_VERSION
        args = ['./build_libedit.sh', libedit_src_dir, libedit_build_path , libedit_install_path, libncurses_path, prebuilts_path, clang_version]
        self.check_call(args)
        os.chdir(cur_dir)

        self.llvm_package.copy_libedit_to_llvm(llvm_make)
        self.llvm_package.copy_libedit_to_llvm(llvm_install)

    def build_libxml2_defines(self):
        libxml2_defines = {}
        libxml2_defines['LIBXML2_WITH_PYTHON'] = 'OFF'
        libxml2_defines['LIBXML2_WITH_PROGRAMS'] = 'OFF'
        libxml2_defines['LIBXML2_WITH_LZMA'] = 'OFF'
        libxml2_defines['LIBXML2_WITH_ICONV'] = 'OFF'
        libxml2_defines['LIBXML2_WITH_ZLIB'] = 'OFF'

        return libxml2_defines

    def build_libxml2(self, llvm_make, llvm_install):
        self.logger().info('Building libxml2')

        libxml2_defines = self.build_libxml2_defines()

        libxml2_cmake_path = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'libxml2'))

        libxml2_build_path = self.merge_out_path('libxml2')
        libxml2_install_path = os.path.join(self.get_prebuilts_dir('libxml2'), self.use_platform())
        libxml2_defines['CMAKE_INSTALL_PREFIX'] = libxml2_install_path

        self.rm_cmake_cache(libxml2_build_path)

        self.invoke_cmake(libxml2_cmake_path,
                         libxml2_build_path,
                         libxml2_defines,
                         env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(out_path=libxml2_build_path,
                         env=dict(self.build_config.ORIG_ENV),
                         target=None,
                         install=True)

        self.llvm_package.copy_libxml2_to_llvm(llvm_make)
        self.llvm_package.copy_libxml2_to_llvm(llvm_install)

    def build_libxml2_for_windows(self, windows64_install):
        self.logger().info('Building libxml2 for windows')

        windows_sysroot = self.merge_out_path('mingw', self.build_config.MINGW_TRIPLE)
        windowstool_path = self.merge_out_path('llvm-install')
        libxml2_build_path = self.merge_out_path('libxml2-windows-build')
        libxml2_install_path = os.path.join(self.get_prebuilts_dir('libxml2'), 'windows-x86_64')

        cflags = ['--target=x86_64-pc-windows-gnu']
        cflags.extend(('-I', os.path.join(windows_sysroot, 'include')))

        ldflags = ['-fuse-ld=lld',
                  '--rtlib=compiler-rt']
        ldflags.extend(('-L', os.path.join(windows_sysroot, 'lib')))

        rcflags = []
        rcflags.extend(('-I', os.path.join(windows_sysroot, 'include')))

        libxml2_defines = self.build_libxml2_defines()

        libxml2_defines['LIBXML2_WITH_TESTS'] = 'OFF'
        libxml2_defines['CMAKE_C_COMPILER'] = os.path.join(windowstool_path, 'bin', 'clang')
        libxml2_defines['CMAKE_C_FLAGS'] = ' '.join(cflags)
        libxml2_defines['CMAKE_SYSTEM_NAME'] = 'Windows'
        libxml2_defines['CMAKE_SYSROOT'] = windows_sysroot
        libxml2_defines['CMAKE_INSTALL_PREFIX'] = libxml2_install_path
        libxml2_defines['CMAKE_EXE_LINKER_FLAGS'] = ' '.join(ldflags)
        libxml2_defines['CMAKE_SHARED_LINKER_FLAGS'] = ' '.join(ldflags)
        libxml2_defines['CMAKE_MODULE_LINKER_FLAGS'] = ' '.join(ldflags)
        libxml2_defines['CMAKE_RC_COMPILER'] = os.path.join(windowstool_path, 'bin', 'llvm-windres')
        libxml2_defines['CMAKE_RC_FLAGS'] = ' '.join(rcflags)
        libxml2_defines['XML_INCLUDEDIR'] = os.path.join(windows_sysroot, 'include')

        libxml2_cmake_path = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'third_party', 'libxml2'))
                
        self.invoke_cmake(libxml2_cmake_path,
                          libxml2_build_path,
                          libxml2_defines,
                          env=dict(self.build_config.ORIG_ENV))

        self.invoke_ninja(libxml2_build_path,
                          env=dict(self.build_config.ORIG_ENV),
                          target=None,
                          install=True)
        
        if not os.path.exists(os.path.join(windows64_install, 'bin')):
            os.makedirs(os.path.join(windows64_install, 'bin'))
        shutil.copyfile(os.path.join(libxml2_build_path, 'libxml2.dll'), os.path.join(windows64_install, 'bin', 'libxml2.dll'))


class LlvmPackage(BuildUtils):

    def __init__(self, build_config):
        super(LlvmPackage, self).__init__(build_config)

    def copy_lldb_tools_to_llvm_install(self, tools, lldb_path, crt_install, llvm_triple):
        dst_dir = os.path.join(crt_install, 'bin', llvm_triple)
        self.check_create_dir(dst_dir)
        for tool in tools:
        # Copy lldb-tool
            src_bin = os.path.join(lldb_path, 'bin', tool)
            self.check_copy_file(src_bin, os.path.join(dst_dir, tool))

    def package_libcxx(self):
        libcxx_ndk_install=self.merge_out_path('libcxx-ndk')
        libcxx_ndk_install_include=self.merge_out_path(libcxx_ndk_install, 'include', 'libcxx-ohos', 'include', 'c++', 'v1')
        hosts_list=['linux-x86_64', 'darwin-x86_64', 'windows-x86_64', 'darwin-arm64']

        if os.path.exists(libcxx_ndk_install):
            for headerfile in os.listdir(libcxx_ndk_install_include):
                if headerfile not in ['__config', '__config_site']:
                    if os.path.isdir(os.path.join(libcxx_ndk_install_include, headerfile)):
                        shutil.rmtree(os.path.join(libcxx_ndk_install_include, headerfile))
                    else:
                        os.remove(os.path.join(libcxx_ndk_install_include, headerfile))

            #Package libcxx-ndk
            for host in hosts_list:
                tarball_name = 'libcxx-ndk-%s-%s' % (self.build_config.build_name, host)
                package_path = '%s%s' % (self.merge_packages_path(tarball_name), '.tar.bz2')
                self.logger().info('Packaging %s', package_path)
                args = ['tar', '-chjC', self.build_config.OUT_PATH, '-f', package_path, 'libcxx-ndk']
                self.check_create_dir(self.build_config.PACKAGES_PATH)
                self.check_call(args)

    @staticmethod
    def merge_tree(src_dir, dst_dir):
        for item in os.listdir(src_dir):
            src_path = os.path.join(src_dir, item)
            dst_path = os.path.join(dst_dir, item)
            if os.path.isfile(src_path):
                shutil.copyfile(src_path, dst_path)
            else:
                shutil.copytree(src_path, dst_path)

    def install_mingw_python(self, install_dir):
        py_version = f'python{self.build_config.LLDB_PY_VERSION}'
        py_dll_name = 'libpython' + self.build_config.LLDB_PY_VERSION + '.dll'
        mingw_python_dir = self.get_mingw_python_dir()
        if mingw_python_dir is None:
            py_root = self.merge_out_path(
                '..', 'third_party', 'mingw-w64', 'mingw-w64-python',
                self.build_config.LLDB_PY_VERSION)
            py_dll_path = os.path.join(py_root, py_dll_name)
            py_lib_path = os.path.join(py_root, 'lib')
        else:
            py_root = mingw_python_dir
            py_dll_path = os.path.join(py_root, 'bin', py_dll_name)
            py_lib_path = os.path.join(py_root, 'lib', py_version)

        bin_root = os.path.join(install_dir, 'bin')
        shutil.copyfile(py_dll_path, os.path.join(bin_root, py_dll_name))
        self.merge_tree(py_lib_path,
                        os.path.join(bin_root, 'python', 'lib', py_version))

    def copy_python_to_host(self, install_dir):
        if(self.host_is_linux()):
            need_file = 'libpython' + self.build_config.LLDB_PY_VERSION + '.so.1.0'
        if(self.host_is_darwin()):
            need_file = 'libpython' + self.build_config.LLDB_PY_VERSION + '.dylib'
        shutil.copyfile(os.path.join(self.get_python_dir(), "lib", need_file), os.path.join(install_dir, 'lib', need_file))
        self.check_copy_tree(self.get_python_dir(), os.path.join(install_dir, self.build_config.LLDB_PYTHON))

    def windows_lib_files_operation(self, lib_files, lib_dir, install_dir):

            # Remove unnecessary Windows lib files.
            windows_necessary_lib_files = ['libc++.a', 'libc++abi.a']

            for lib_file in lib_files:
                if lib_file.endswith('.a') and lib_file not in windows_necessary_lib_files:
                    static_library = os.path.join(lib_dir, lib_file)
                    os.remove(static_library)

            for necessary_lib_file in windows_necessary_lib_files:
                if not os.path.isfile(os.path.join(lib_dir, necessary_lib_file)):
                    raise RuntimeError('Did not find %s under %s' % (necessary_lib_file, lib_dir))
            self.install_mingw_python(install_dir)


    def darwin_stripped_xargs(self, bin_dir, necessary_bin_files, script_bins, need_x_bins_darwin):
        for bin_filename in os.listdir(bin_dir):
            binary = os.path.join(bin_dir, bin_filename)
            if not os.path.isfile(binary):
                continue
            if bin_filename not in necessary_bin_files:
                os.remove(binary)
            elif bin_filename not in script_bins and self.build_config.strip:
                if bin_filename not in need_x_bins_darwin and self.host_is_darwin():
                    self.check_call(['strip', '-x', binary])
                else:
                    self.check_call(['strip', binary])


    def strip_lldb_server(self, host, install_dir):
        clang_version_bin_dir = os.path.join(install_dir, 'lib', 'clang', self.build_config.CLANG_VERSION, 'bin')

        if not host.startswith('linux') or not os.path.exists(clang_version_bin_dir) or not self.build_config.strip:
            return
        llvm_strip = os.path.join(install_dir, 'bin', 'llvm-strip')
        for llvm_triple_dir in os.listdir(clang_version_bin_dir):
            llvm_triple_bin_dir = os.path.join(clang_version_bin_dir, llvm_triple_dir)

            if not os.path.isdir(llvm_triple_bin_dir):
                continue

            for bin_filename in os.listdir(llvm_triple_bin_dir):
                binary = os.path.join(llvm_triple_bin_dir, bin_filename)
                if os.path.isfile(binary):
                    self.check_call([llvm_strip, binary])


    def notice_prebuilts_file(self, host, projects, install_dir):

        # Fetch all the LICENSE.* files under our projects and append them into a
        # Single NOTICE file for the resulting prebuilts.
        notices = []
        for project in projects:
            license_pattern = os.path.abspath(os.path.join(self.build_config.LLVM_PROJECT_DIR, project, 'LICENSE.*'))
            for license_file in glob.glob(license_pattern):
                with open(license_file) as notice_file:
                    notices.append(notice_file.read())

        zlib_license_file = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR, 'third_party/zlib/LICENSE'))
        with open(zlib_license_file) as notice_file:
            notices.append(notice_file.read())

        if host.startswith('windows'):
            mingw_license_file = os.path.abspath(os.path.join(self.build_config.REPOROOT_DIR,
                                                                'third_party/mingw-w64/COPYING'))
            with open(mingw_license_file) as notice_file:
                notices.append(notice_file.read())

        with open(os.path.join(install_dir, 'NOTICE'), 'w') as notice_file:
            notice_file.write('\n'.join(notices))


    def package_clang_bin_file(self, necessary_bin_files, ext):
        necessary_bin_file = [
            'clang%s' % ext,
            'clang++%s' % ext,
            'clang-cpp%s' % ext,
            'clang-%s%s' % (self.build_config.VERSION.split('.')[0], ext),
            'clang-check%s' % ext,
            'clang-cl%s' % ext,
            'clang-format%s' % ext,
            'clang-tidy%s' % ext,
            'clangd%s' % ext,
            'count%s' % ext,
            'llc%s' % ext,
            'opt%s' % ext,

            ]
            
        necessary_bin_files.extend(necessary_bin_file)

    @staticmethod
    def package_lldb_bin_file(necessary_bin_files, ext):
        necessary_bin_file = [
            'dsymutil%s' % ext,
            'FileCheck%s' % ext,
            'git-clang-format',
            'ld.lld%s' % ext,
            'ld64.lld%s' % ext,
            'lld%s' % ext,
            'lld-link%s' % ext,
            'lldb%s' % ext,
            'lldb-argdumper%s' % ext,
            'lldb-server%s' % ext,
            'lldb-vscode%s' % ext,
            ]
        necessary_bin_files.extend(necessary_bin_file)

    @staticmethod
    def package_llvm_bin_file(necessary_bin_files, ext):
        necessary_bin_file = [
            'llvm-addr2line%s' % ext,
            'llvm-ar%s' % ext,
            'llvm-as%s' % ext,
            'llvm-cfi-verify%s' % ext,
            'llvm-config%s' % ext,
            'llvm-cov%s' % ext,
            'llvm-cxxfilt%s' % ext,
            'llvm-dis%s' % ext,
            'llvm-lib%s' % ext,
            'llvm-link%s' % ext,
            'llvm-modextract%s' % ext,
            'llvm-nm%s' % ext,
            'llvm-objcopy%s' % ext,
            'llvm-objdump%s' % ext,
            'llvm-profdata%s' % ext,
            'llvm-ranlib%s' % ext,
            'llvm-readelf%s' % ext,
            'llvm-readobj%s' % ext,
            'llvm-rc%s' % ext,
            'llvm-size%s' % ext,
            'llvm-strings%s' % ext,
            'llvm-strip%s' % ext,
            'llvm-symbolizer%s' % ext,
            ]
        necessary_bin_files.extend(necessary_bin_file)

    @staticmethod
    def package_scan_bin_file(necessary_bin_files, ext):
        necessary_bin_file = [
            'not%s' % ext,
            'sancov%s' % ext,
            'sanstats%s' % ext,
            'scan-build%s' % ext,
            'scan-view%s' % ext,
            'yaml2obj%s' % ext,
            ]
        necessary_bin_files.extend(necessary_bin_file)

    @staticmethod
    def package_license_project_tuple():
        # Install license files as NOTICE in the toolchain install dir.
        projects = (
            'llvm',
            'compiler-rt',
            'libcxx',
            'libcxxabi',
            'openmp',
            'clang',
            'clang-tools-extra',
            'lld',
            'lldb',
            'libunwind',
        )
        return projects

    def remove_unnecessary_bin(self, necessary_bin_files, host, lib_dir, install_dir, ext, shlib_ext):

        lib_files = []
        if os.path.isdir(lib_dir):
                lib_files = os.listdir(lib_dir)
        if host.startswith('windows'):
            vers_major = int(self.build_config.VERSION.split('.')[0])
            # Redefining necessary bin files for Windows.
            windows_forbidden_list_bin_files = ['clang-%s%s' % (vers_major, ext), 'scan-build%s' % ext,
                                           'scan-view%s' % ext]
            windows_additional_bin_files = ['liblldb%s' % shlib_ext, 'libpython3.10%s' % shlib_ext]
            if self.build_config.build_libxml2:
                windows_additional_bin_files += ['libxml2%s' % shlib_ext]

            new_necessary_bin_files = list(set(necessary_bin_files) - set(windows_forbidden_list_bin_files))
            new_necessary_bin_files.extend(windows_additional_bin_files)
            del necessary_bin_files[:]
            necessary_bin_files.extend(new_necessary_bin_files)

            self.windows_lib_files_operation(lib_files, lib_dir, install_dir)
        else:
            # Remove unnecessary static libraries.
            for lib_file in lib_files:
                if lib_file.endswith('.a'):
                    static_library = os.path.join(lib_dir, lib_file)
                    os.remove(static_library)

    def package_up_resulting(self, package_name, host, install_host_dir):
        # Package up the resulting trimmed install/ directory.
        tarball_name = '%s-%s' % (package_name, host)
        package_path = '%s%s' % (self.merge_packages_path(tarball_name), '.tar.bz2')
        self.logger().info('Packaging %s', package_path)
        args = ['tar', '-cjC', install_host_dir, '-f', package_path, package_name]
        if host.startswith('windows'):
            # windows do not support symlinks,
            # replace them with file copies
            args.insert(1, '--dereference')

        self.check_create_dir(self.build_config.PACKAGES_PATH)
        self.check_call(args)

        # Package ohos NDK
        if os.path.exists(self.merge_out_path('sysroot')):
            tarball_ndk_name = 'ohos-sysroot-%s' % self.build_config.build_name
            package_ndk_path = '%s%s' % (self.merge_packages_path(tarball_ndk_name), '.tar.bz2')
            self.logger().info('Packaging %s', package_ndk_path)
            args = ['tar', '-chjC', self.build_config.OUT_PATH, '-f', package_ndk_path, 'sysroot']
            self.check_call(args)

    def get_dependency_list(self, install_dir, lib):
        dependency_list = []
        cmd = "otool -L {}".format(lib)
        res = subprocess.run(cmd, stdout=subprocess.PIPE, shell=True).stdout.decode()
        # Remove the library itself
        res = res.split('\n')[1:]
        for line in res:
            line = line.strip()
            if line.startswith("/"):
                # Process lines similar to the following format:
                # /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1311.100.3)
                lib_path = line.split()[0]
                dependency_list.append(lib_path)
        return dependency_list

    def update_lib_id_link(self, install_dir, lib):
        self.logger().info('update_lib_id_link lib is %s', lib)
        if not self.host_is_darwin():
            return;
        else:
            dependency_list = self.get_dependency_list(install_dir, lib)

            # Update LC_ID_DYLIB, so that users of the library won't link with absolute path
            dylib_name = os.path.basename(lib)
            subprocess.check_call(["install_name_tool", "-id", "@rpath/%s" % dylib_name, lib])

            library_list = []

            if self.build_config.build_ncurses:
                library_list.extend(["libncurses", "libpanel", "libform"])

            if self.build_config.build_libedit:
                library_list.append("libedit")

            # Update LC_LOAD_DYLIB. The lib may already reference other libs.
            for dependency in dependency_list:
                lib_name =  os.path.basename(dependency)
                for library in library_list:
                    index_link = dependency.rfind(library)
                    if index_link != -1:
                        subprocess.check_call(["install_name_tool", "-change", dependency, "@loader_path/../lib/%s" % lib_name, lib])

    def copy_ncurses_to_llvm(self, install_dir):
        self.logger().info('copy_ncurses_to_llvm install_dir is %s', install_dir)

        if self.host_is_darwin():
            shlib_ext = '.6.dylib'
        if self.host_is_linux():
            shlib_ext = '.so.6'

        lib_dst_path = os.path.join(install_dir, 'lib')

        lib_src_path = self.merge_out_path('../prebuilts', 'ncurses', 'lib')
        libncurses_src = os.path.join(lib_src_path, 'libncurses%s' % shlib_ext)
        libpanel_src = os.path.join(lib_src_path, 'libpanel%s' % shlib_ext)
        libform_src = os.path.join(lib_src_path, 'libform%s' % shlib_ext)

        libncurses_dst = os.path.join(lib_dst_path, 'libncurses%s' % shlib_ext)
        libpanel_dst = os.path.join(lib_dst_path, 'libpanel%s' % shlib_ext)
        libform_dst =os.path.join(lib_dst_path, 'libform%s' % shlib_ext)

        if not os.path.exists(lib_dst_path):
            os.makedirs(lib_dst_path)

        for lib_file in (libncurses_src, libpanel_src, libform_src):
            self.update_lib_id_link(lib_src_path, lib_file)

        # Clear historical libraries
        for lib in (libncurses_dst, libpanel_dst, libform_dst):
            if os.path.exists(lib):
                os.remove(lib)

        for lib_src in (libncurses_src, libpanel_src, libform_src):
            self.check_copy_file(lib_src, lib_dst_path)

    def copy_libedit_to_llvm(self, install_dir):
        self.logger().info('LlvmPackage copy_libedit_to_llvm install_dir is %s', install_dir)

        if self.host_is_darwin():
            shlib_ext = '.0.dylib'
        if self.host_is_linux():
            shlib_ext = '.so.0'

        libedit_lib_path = self.merge_out_path('../prebuilts', 'libedit', 'lib')
        libedit_src = os.path.join(libedit_lib_path, 'libedit%s' % shlib_ext)

        lib_dst_path = os.path.join(install_dir, 'lib')

        libedit_dst = os.path.join(lib_dst_path, 'libedit%s' % shlib_ext)

        if not os.path.exists(lib_dst_path):
            os.makedirs(lib_dst_path)

        self.update_lib_id_link(libedit_lib_path, libedit_src)

        # Clear historical library
        if os.path.exists(libedit_dst):
            os.remove(libedit_dst)

        self.check_copy_file(libedit_src, lib_dst_path)

    def copy_wrappers(self, bin_dir):
        for sh_filename in os.listdir(os.path.join(self.build_config.LLVM_BUILD_DIR, 'wrappers')):
            shutil.copy(os.path.join(self.build_config.LLVM_BUILD_DIR, 'wrappers', sh_filename),
                            bin_dir)
            st = os.stat(os.path.join(bin_dir, sh_filename))
            os.chmod(os.path.join(bin_dir, sh_filename), st.st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)

    def copy_libxml2_to_llvm(self, install_dir):
        self.logger().info('LlvmPackage copy_libxml2_to_llvm install_dir is %s', install_dir)

        libxml2_version = self.get_libxml2_version()
        if libxml2_version is not None:
            if self.host_is_darwin():
                shlib_ext = f'.{libxml2_version}.dylib'
            if self.host_is_linux():
                shlib_ext = f'.so.{libxml2_version}'

            libxml2_lib_path = self.merge_out_path('libxml2')
            libxml2_src = os.path.join(libxml2_lib_path, 'libxml2%s' % shlib_ext)

            lib_dst_path = os.path.join(install_dir, 'lib')

            libxml2_dst = os.path.join(lib_dst_path, 'libxml2%s' % shlib_ext)

            if not os.path.exists(lib_dst_path):
                os.makedirs(lib_dst_path)

            self.update_lib_id_link(libxml2_lib_path, libxml2_src)

            # Clear historical library
            if os.path.isfile(libxml2_dst):
                os.remove(libxml2_dst)

            self.check_copy_file(libxml2_src, lib_dst_path)

    # Packing Operation.

    def package_operation(self, build_dir, host):

        self.logger().info('Packaging for other environments.')
        package_name = 'clang-%s' % self.build_config.build_name
        self.set_clang_version(build_dir)

        if host.startswith('windows'):
            ext = '.exe'
            shlib_ext = '.dll'
        elif host.startswith('linux'):
            ext = ''
            shlib_ext = '.so'
        else:
            ext = ''
            shlib_ext = '.dylib'

        install_dir = self.merge_out_path('install', host, package_name)
        bin_dir = os.path.join(install_dir, 'bin')
        lib_dir = os.path.join(install_dir, 'lib')
        install_host_dir = os.path.abspath(os.path.join(install_dir, '../'))
        self.check_rm_tree(install_host_dir)
        self.check_copy_tree(build_dir, install_dir)
        # copy readme file to install_dir
        shutil.copyfile(os.path.join(self.build_config.LLVM_BUILD_DIR, "toolchain_readme.md"),
                        os.path.join(install_dir, "README.md"))

        # Generate manifest in install_dir
        manifest = os.path.join(install_dir, 'manifest.xml')
        repo_tool = os.path.join(self.build_config.REPOROOT_DIR, '.repo', 'repo', 'repo')
        if os.path.isfile(repo_tool):
            self.logger().info('Generating manifest.')
            subprocess.run(['python2.7', repo_tool, 'manifest', '-r', '-o', manifest], shell=False,
                           stdout=subprocess.PIPE, cwd=self.build_config.REPOROOT_DIR)
        else:
            self.logger().error('Cannot generate manifest, repo tool not found.')

        # Remove unnecessary binaries.
        necessary_bin_files = []
        self.package_clang_bin_file(necessary_bin_files, ext)
        self.package_lldb_bin_file(necessary_bin_files, ext)
        self.package_llvm_bin_file(necessary_bin_files, ext)
        self.package_scan_bin_file(necessary_bin_files, ext)

        self.remove_unnecessary_bin(necessary_bin_files, host, lib_dir, install_dir, ext, shlib_ext)

        # Scripts that should not be stripped
        script_bins = ['git-clang-format', 'scan-build', 'scan-view']
        need_x_bins_darwin = ['clang', 'clang++', 'clang-9', 'clang-cl', 'clang-cpp']

        # Bin file in the list should be stripped with -x args when host=darwin
        self.darwin_stripped_xargs(bin_dir, necessary_bin_files, script_bins, need_x_bins_darwin)

        # Strip lldb-server
        self.strip_lldb_server(host, install_dir)
        
        # Copy lldb script
        lldb_script_file = 'lldb.cmd' if host.startswith('windows') else 'lldb.sh'
        self.check_copy_file(os.path.join(self.build_config.LLVM_PROJECT_DIR, 'lldb', 'scripts', lldb_script_file), os.path.join(install_dir, 'bin'))

        lldb_script_python_path = os.path.join(self.build_config.LLVM_PROJECT_DIR, 'lldb', 'scripts', 'lldb_python.py')

        self.check_create_dir(os.path.join(install_dir, 'script'))

        self.check_copy_file(lldb_script_python_path, os.path.join(install_dir, 'script'))

        for necessary_bin_file in necessary_bin_files:
            if not os.path.isfile(os.path.join(bin_dir, necessary_bin_file)):
                print('Did not find %s in %s' % (necessary_bin_file, bin_dir))
                raise RuntimeError('Did not find %s in %s' % (necessary_bin_file, bin_dir))

        cmake_dir = os.path.join(lib_dir, 'cmake')
        self.check_rm_tree(cmake_dir)

        self.notice_prebuilts_file(host, self.package_license_project_tuple(), install_dir)

        self.copy_wrappers(bin_dir)

        create_tar = True
        if create_tar:
            self.package_up_resulting(package_name, host, install_host_dir)

        return



def main():
    build_config = BuildConfig()
    build_utils = BuildUtils(build_config)
    sysroot_composer = SysrootComposer(build_config)
    llvm_core = LlvmCore(build_config)
    llvm_package = LlvmPackage(build_config)
    llvm_libs = LlvmLibs(build_config, sysroot_composer, llvm_package)

    args = build_config.parse_args()
    need_host = build_utils.host_is_darwin() or ('linux' not in args.no_build)
    need_windows = build_utils.host_is_linux() and \
                   ('windows' not in args.no_build)

    llvm_install = build_utils.merge_out_path('llvm-install')
    llvm_make = build_utils.merge_out_path('llvm_make')
    windows64_install = build_utils.merge_out_path('windows-x86_64-install')

    configs = []
    if not build_config.no_build_arm:
        configs.append(('arm', build_utils.liteos_triple('arm')))
        configs.append(('arm', build_utils.open_ohos_triple('arm')))

    if not build_config.no_build_aarch64:
        configs.append(('arm64', build_utils.open_ohos_triple('aarch64')))

    if not build_config.no_build_riscv64:
        configs.append(('riscv', build_utils.open_ohos_triple('riscv64')))

    if not build_config.no_build_mipsel:
        configs.append(('mipsel', build_utils.open_ohos_triple('mipsel')))

    if not build_config.no_build_x86_64:
        configs.append(('x86_64', build_utils.open_ohos_triple('x86_64')))

    if build_config.build_ncurses:
        llvm_libs.build_ncurses(llvm_make, llvm_install)

    if build_config.build_libedit:
        llvm_libs.build_libedit(llvm_make, llvm_install)

    if build_config.build_libxml2:
        build_utils.get_libxml2_version()
        llvm_libs.build_libxml2(llvm_make, llvm_install)
        
    if build_config.do_build and need_host:
        llvm_core.llvm_compile(
            build_config.build_name,
            llvm_install,
            build_config.debug,
            build_config.no_lto,
            build_config.build_instrumented,
            build_config.xunit_xml_output)
        llvm_package.copy_python_to_host(llvm_make)
        llvm_package.copy_python_to_host(llvm_install)

    llvm_core.set_clang_version(llvm_install)

    if build_config.do_build and build_utils.host_is_linux():
        sysroot_composer.setup_cmake_platform(llvm_install)
        llvm_libs.build_crt_libs(configs, llvm_install)

        if build_config.need_libs:
            for (arch, target) in configs:
                llvm_libs.build_libs(llvm_install, target)

    windows_python_builder = None

    if build_config.do_build and need_windows:
        mingw.main(build_config.VERSION)
        llvm_libs.build_runtimes_for_windows(build_config.enable_assertions)

        if build_config.build_libxml2:
            llvm_libs.build_libxml2_for_windows(windows64_install)

        if build_config.build_python:
            # Use just built LLVM and MinGW-w64 to cross-compile Python
            windows_python_builder = MinGWPythonBuilder(build_config)
            windows_python_builder.build()
            windows_python_builder.prepare_for_package()
            llvm_core.set_mingw_python_dir(windows_python_builder.install_dir)
        llvm_core.llvm_compile_for_windows(build_config.TARGETS,
                                          build_config.enable_assertions,
                                          build_config.build_name)


    if build_config.do_package:
        if build_utils.host_is_linux():
            llvm_package.package_libcxx()
        llvm_package.package_operation(llvm_install, build_utils.use_platform())

        if build_config.build_python and windows_python_builder:
            llvm_package.set_mingw_python_dir(windows_python_builder.install_dir)

        if need_windows:
            llvm_package.package_operation(windows64_install, 'windows-x86_64')

    if build_config.build_clean:
        build_utils.rm_build_output()

if __name__ == '__main__':
    main()
