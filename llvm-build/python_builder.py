# Copyright (C) 2023 Huawei Device Co., Ltd.
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

import logging
import os
from pathlib import Path
import shutil
import subprocess
from typing import List, Mapping
import binascii

class PythonBuilder:
    target_platform = ""
    patches = []

    def __init__(self, build_config) -> None:
        self.repo_root = Path(build_config.REPOROOT_DIR).resolve()
        self._out_dir = Path(build_config.OUT_PATH).resolve()
        self._clang_toolchain_dir = self._out_dir / 'llvm-install'
        self._lldb_py_version = build_config.LLDB_PY_VERSION
        self._version = build_config.LLDB_PY_DETAILED_VERSION
        version_parts = self._version.split('.')
        self._major_version = version_parts[0] + '.' + version_parts[1]
        self._source_dir = self.repo_root / 'third_party' / 'python'
        self._patch_dir = self._source_dir / 'patches'
        self._prebuilts_path = os.path.join(self.repo_root, 'prebuilts', 'python3', 'linux-x86', self._version)
        self._prebuilts_python_path = os.path.join(self._prebuilts_path, 'bin', 'python%s' % self._lldb_py_version)
        self._install_dir = ""

        self._clean_patches()

    @property
    def _logger(self) -> logging.Logger:
        return logging.getLogger(__name__)

    @property
    def _cc(self) -> Path:
        return self._clang_toolchain_dir/ 'bin' / 'clang'

    @property
    def _cflags(self) -> List[str]:
        return []

    @property
    def _ldflags(self) -> List[str]:
        return []

    @property
    def _cxx(self) -> Path:
        return self._clang_toolchain_dir / 'bin' / 'clang++'

    @property
    def _strip(self) -> Path:
        return self._clang_toolchain_dir / 'bin' / 'llvm-strip'

    @property
    def _cxxflags(self) -> List[str]:
        return self._cflags.copy()

    @property
    def _rcflags(self) -> List[str]:
        return []

    @property
    def _env(self) -> Mapping[str, str]:
        clang_bin_dir = self._clang_toolchain_dir / 'bin'
        env = os.environ.copy()

        env.update({
            'CC': str(self._cc),
            'CXX': str(self._cxx),
            'WINDRES': str(clang_bin_dir / 'llvm-windres'),
            'AR': str(clang_bin_dir / 'llvm-ar'),
            'READELF': str(clang_bin_dir / 'llvm-readelf'),
            'LD': str(clang_bin_dir / 'ld.lld'),
            'DLLTOOL': str(clang_bin_dir / 'llvm-dlltoo'),
            'RANLIB': str(clang_bin_dir / 'llvm-ranlib'),
            'STRIP': str(self._strip),
            'CFLAGS': ' '.join(self._cflags),
            'CXXFLAGS': ' '.join(self._cxxflags),
            'LDFLAGS': ' '.join(self._ldflags),
            'RCFLAGS': ' '.join(self._rcflags),
            'CPPFLAGS': ' '.join(self._cflags),
        })
        return env

    def _configure(self) -> None:
        return

    def _clean_patches(self) -> None:
        subprocess.check_call(['git', 'reset', '--hard', 'HEAD'], cwd=self._source_dir)
        subprocess.check_call(['git', 'clean', '-df'], cwd=self._source_dir)

    def _pre_build(self) -> None:
        if self._patch_ignore_file.is_file():
            self._logger.warning('Patches for Python have being applied, skip patching')
            return

        if not self._patch_dir.is_dir():
            self._logger.warning('Patches are not found, skip patching')
            return

        for patch in self._patch_dir.iterdir():
            if patch.is_file() and patch.name in self.patches:
                cmd = [ 'git', 'apply', str(patch) ]
                subprocess.check_call(cmd, cwd=self._source_dir)

    def build(self) -> None:
        self._pre_build()
        if self._build_dir.exists():
            shutil.rmtree(self._build_dir)
        if self._install_dir.exists():
            shutil.rmtree(self._install_dir)
        self._build_dir.mkdir(parents=True)
        self._install_dir.mkdir(parents=True)
        self._configure()
        self._install()

    def _install(self) -> None:
        num_jobs = os.cpu_count() or 8
        cmd = [ 'make', f'-j{num_jobs}', 'install']
        subprocess.check_call(cmd, cwd=self._build_dir)

    def _strip_in_place(self, file: Path) -> None:
        cmd = [
            str(self._strip),
            str(file),
        ]
        subprocess.check_call(cmd)

    def _clean_bin_dir(self) -> None:
        python_bin_dir = self._install_dir / 'bin'
        if not python_bin_dir.is_dir():
            return

        windows_suffixes = ( '.exe', '.dll' )
        for f in python_bin_dir.iterdir():
            if f.suffix not in windows_suffixes or f.is_symlink():
                f.unlink()
                continue
            self._strip_in_place(f)

    def _remove_dir(self, dir_path: Path) -> None:
        if dir_path.is_dir():
            shutil.rmtree(dir_path)

    def _clean_share_dir(self) -> None:
        share_dir = self._install_dir / 'share'
        self._remove_dir(share_dir)

    def _clean_lib_dir(self) -> None:
        python_lib_dir = self._install_dir / 'lib'
        pkgconfig_dir = python_lib_dir / 'pkgconfig'
        self._remove_dir(pkgconfig_dir)

    def _remove_exclude(self) -> None:
        exclude_dirs_tuple = (
            'config-' + self._lldb_py_version,
            '__pycache__',                        # EXCLUDE_FROM_LIB
            'idlelib',                            # IDLE_DIRS_ONLY
            'tkinter', 'turtledemo',              # TCLTK_DIRS_ONLY
            'test', 'tests'                       # TEST_DIRS_ONLY
        )
        exclude_files_tuple = (
            'bdist_wininst.py',                   # BDIST_WININST_FILES_ONLY
            'turtle.py',                          # TCLTK_FILES_ONLY
            '.whl',
            '.pyc', '.pickle'                   # EXCLUDE_FROM_LIB
        )

        for root, dirs, files in os.walk(self._install_dir / 'lib'):
            for item in dirs:
                if item.startswith(exclude_dirs_tuple):
                    shutil.rmtree(os.path.join(root, item))
            for item in files:
                if item.endswith(exclude_files_tuple):
                    os.remove(os.path.join(root, item))

    def _is_elf_file(self, file_path: Path) -> None:
        with open(file_path, 'rb') as f:
            magic_numbers = f.read(4)
            hex_magic_number = binascii.hexlify(magic_numbers).decode('utf-8')
            if hex_magic_number == '7f454c46':
                return True
            else:
                return False

    @property
    def install_dir(self) -> str:
        return str(self._install_dir)


class MinGWPythonBuilder(PythonBuilder):
    def __init__(self, build_config) -> None:
        super().__init__(build_config)

        self.target_platform = "x86_64-w64-mingw32"
        self.patches = [ f'cpython_mingw_v{self._version}.patch' ]

        self._mingw_install_dir = self._out_dir / 'mingw' / build_config.MINGW_TRIPLE
        self._build_dir = self._out_dir / 'python-windows-build'
        self._install_dir = self._out_dir / 'python-windows-install'

        # This file is used to detect whether patches are applied
        self._patch_ignore_file = self._source_dir / 'mingw_ignorefile.txt'


        for directory in (self._clang_toolchain_dir, self._mingw_install_dir,
                          self._source_dir):
            if not directory.is_dir():
                raise ValueError(f'No such directory "{directory}"')

    @property
    def _cflags(self) -> List[str]:
        cflags = [
            f'-target {self.target_platform}',
            f'--sysroot={self._mingw_install_dir}',
            f'-fstack-protector-strong',
        ]
        return cflags

    @property
    def _ldflags(self) -> List[str]:
        ldflags = [
            f'--sysroot={self._mingw_install_dir}',
            f'-rtlib=compiler-rt',
            f'-target {self.target_platform}',
            f'-lucrt',
            f'-lucrtbase',
            f'-fuse-ld=lld',
        ]
        return ldflags

    @property
    def _rcflags(self) -> List[str]:
        return [ f'-I{self._mingw_install_dir}/include' ]

    def _configure(self) -> None:
        subprocess.check_call(['autoreconf', '-vfi'], cwd=self._source_dir)
        build_platform = subprocess.check_output(
            ['./config.guess'], cwd=self._source_dir).decode().strip()
        config_flags = [
            f'--prefix={self._install_dir}',
            f'--build={build_platform}',
            f'--host={self.target_platform}',
            f'--with-build-python={self._prebuilts_python_path}',
            '--enable-shared',
            '--without-ensurepip',
            '--enable-loadable-sqlite-extensions',
        ]
        cmd = [str(self._source_dir / 'configure')] + config_flags
        subprocess.check_call(cmd, env=self._env, cwd=self._build_dir)

    def prepare_for_package(self) -> None:
        self._clean_bin_dir()
        self._clean_share_dir()
        self._clean_lib_dir()
        self._remove_exclude()

    def package(self) -> None:
        archive = self._out_dir / f'cpython-mingw-clang-{self._version}.tar.gz'
        if archive.exists():
            archive.unlink()
        cmd = [
            'tar',
            '-czf',
            str(archive),
            '--exclude=__pycache__', # Do not package __pycache__ directory
            '--transform',
            # Pack under directory python3/windows-x86
            f's,^,python3/windows-x86/{self._version}/,',
        ] + [ f.name for f in self._install_dir.iterdir() ]
        subprocess.check_call(cmd, cwd=self._install_dir)


class OHOSPythonBuilder(PythonBuilder):
    def __init__(self, build_utils, target_platform) -> None:
        super().__init__(build_utils.build_config)

        self.target_platform = target_platform
        self.patches = ["cross_compile_support_ohos.patch"]

        self._build_dir = Path(build_utils.merge_python_build_dir(target_platform))
        self._install_dir = Path(build_utils.merge_python_install_dir(target_platform))

        # This file is used to detect whether patches are applied
        self._patch_ignore_file = self._source_dir / 'support_ohos_ignorefile.txt'

        self.build_utils = build_utils
        return

    @property
    def _cflags(self) -> List[str]:
        cflags = [
            f'--target={self.target_platform}',
            f'-fstack-protector-strong',
            '-nostdinc',
            '-I%s' % str(self._out_dir / 'sysroot' / self.target_platform / 'usr' / 'include'),
        ]
        if str(self.target_platform).find("arm") > 0:
            cflags.append('-march=armv7-a -mfloat-abi=soft')
        return cflags

    @property
    def _ldflags(self) -> List[str]:
        ldflags = [
            f'-rtlib=compiler-rt',
            f'--target={self.target_platform}',
            f'-fuse-ld=lld',
            f'-Wl,-z,relro,-z,now -Wl,-z,noexecstack',
	    '-fstack-protector-strong',
            '-L%s' % str(self._out_dir / 'sysroot' / self.target_platform / 'usr' / 'lib'),
            '-lc',
            '-Wl,-rpath,\\$$ORIGIN/../lib',
        ]
        return ldflags

    def _configure(self) -> None:
        subprocess.check_call(['autoreconf', '-vfi'], cwd=self._source_dir)
        build_platform = subprocess.check_output(
            ['./config.guess'], cwd=self._source_dir).decode().strip()
        config_flags = [
            f'--prefix={self._install_dir}',
            f'--build={build_platform}',
            f'--host={self.target_platform}',
            f'--with-build-python={self._prebuilts_python_path}',
            '--enable-shared',
            '--without-ensurepip',
            '--enable-loadable-sqlite-extensions',
            '--disable-ipv6',
            'ac_cv_file__dev_ptmx=no',
            'ac_cv_file__dev_ptc=no',
            '--without-system-ffi',
            '--without-pydebug',
            '--without-doc-strings',
            '--without-dtrace',
        ]
        cmd = [str(self._source_dir / 'configure')] + config_flags
        subprocess.check_call(cmd, env=self._env, cwd=self._build_dir)

    def copy_python_to_host(self, install_dir):
        libpython = f'libpython{self.build_utils.build_config.LLDB_PY_VERSION}.so.1.0'

        shutil.copyfile(os.path.join(self._install_dir, "lib", libpython), os.path.join(install_dir, 'lib', libpython))
        self.build_utils.check_copy_tree(self._install_dir, os.path.join(install_dir, self.build_utils.build_config.LLDB_PYTHON))

    def prepare_for_package(self) -> None:
        self._remove_exclude()
        if self.build_utils.build_config.strip:
            python_bin_dir = self._install_dir / 'bin'
            if not python_bin_dir.is_dir():
                return

            for f in python_bin_dir.iterdir():
                if f.is_symlink():
                    continue
                if self._is_elf_file(f):
                    self._strip_in_place(f)

            for root, dirs, files in os.walk(self._install_dir / 'lib'):
                for item in files:
                    f = os.path.join(root, item)
                    if os.path.islink(f):
                        continue
                    if self._is_elf_file(f):
                        self._strip_in_place(f)
