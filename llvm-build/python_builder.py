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

class MinGWPythonBuilder:
    target_platform = "x86_64-w64-mingw32"

    def __init__(self, build_config) -> None:
        repo_root = Path(build_config.REPOROOT_DIR).resolve()
        self._out_dir = Path(build_config.OUT_PATH).resolve()
        self._clang_toolchain_dir = self._out_dir / 'llvm-install'
        self._mingw_install_dir = self._out_dir / 'mingw' / build_config.MINGW_TRIPLE
        self._source_dir = repo_root / 'third_party' / 'python'
        self._build_dir = self._out_dir / 'python-windows-build'
        self._install_dir = self._out_dir / 'python-windows-install'
        self._version = build_config.LLDB_PY_DETAILED_VERSION
        version_parts = self._version.split('.')
        self._major_version = version_parts[0] + '.' + version_parts[1]
        # This file is used to detect whether patches are applied
        self._mingw_ignore_file = self._source_dir / 'mingw_ignorefile.txt'
        self._patch_dir = self._source_dir / 'patches'

        for directory in (self._clang_toolchain_dir, self._mingw_install_dir,
                          self._source_dir):
            if not directory.is_dir():
                raise ValueError(f'No such directory "{directory}"')

    @property
    def _logger(self) -> logging.Logger:
        return logging.getLogger(__name__)

    @property
    def _cc(self) -> Path:
        return self._clang_toolchain_dir/ 'bin' / 'clang'

    @property
    def _cxx(self) -> Path:
        return self._clang_toolchain_dir / 'bin' / 'clang++'

    @property
    def _strip(self) -> Path:
        return self._clang_toolchain_dir / 'bin' / 'llvm-strip'

    @property
    def _cflags(self) -> List[str]:
        cflags = [
            f'-target {self.target_platform}',
            f'--sysroot={self._mingw_install_dir}',
        ]
        return cflags

    @property
    def _cxxflags(self) -> List[str]:
        return self._cflags.copy()

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
        })
        return env

    def _configure(self) -> None:
        subprocess.check_call(['autoreconf', '-vfi'], cwd=self._source_dir)
        build_platform = subprocess.check_output(
            ['./config.guess'], cwd=self._source_dir).decode().strip()
        config_flags = [
            f'--prefix={self._install_dir}',
            f'--build={build_platform}',
            f'--host={self.target_platform}',
            '--enable-shared',
            '--without-ensurepip',
            '--enable-loadable-sqlite-extensions',
        ]
        cmd = [str(self._source_dir / 'configure')] + config_flags
        subprocess.check_call(cmd, env=self._env, cwd=self._build_dir)

    def _pre_build(self) -> None:
        if self._mingw_ignore_file.is_file():
            self._logger.warning('Patches for Python have being applied, skip patching')
            return

        if not self._patch_dir.is_dir():
            self._logger.warning('Patches are not found, skip patching')
            return

        for patch in self._patch_dir.iterdir():
            if patch.is_file():
                cmd = [ 'git', 'apply', str(patch) ]
                subprocess.check_call(cmd, cwd=self._source_dir)

    def build(self) -> None:
        self._pre_build()
        if self._build_dir.exists():
            shutil.rmtree(self._build_dir)
        if self._install_dir.exists():
            shutil.rmtree(self._install_dir)
        self._build_dir.mkdir()
        self._install_dir.mkdir()
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

    def _remove_pycache(self) -> None:
        pycaches = []
        for dir_path, dirnames, _ in os.walk(self._install_dir / 'lib'):
            for dirname in dirnames:
                if dirname == '__pycache__':
                    pycaches.append(os.path.join(dir_path, dirname))

        for cache_dir in pycaches:
            shutil.rmtree(cache_dir)

    def prepare_for_package(self) -> None:
        self._clean_bin_dir()
        self._clean_share_dir()
        self._clean_lib_dir()
        self._remove_pycache()

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

    @property
    def install_dir(self) -> str:
        return str(self._install_dir)
