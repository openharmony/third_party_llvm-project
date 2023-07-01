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

from enum import Enum
import lldb
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import time
from typing import Mapping, Optional, Tuple

class Arch(Enum):
    ARM = "arm"
    AARCH64 = "aarch64"
    X86_64 = "x86_64"
    UNKNOWN = "unknown"

class LLDBException(Exception):
    _err_msg: str

    def __init__(self, err_msg: str) -> None:
        super().__init__(err_msg)
        self._err_msg = err_msg

    @property
    def err_msg(self) -> str:
        return self._err_msg

class _ServerConfig:
    arch: Arch = Arch.UNKNOWN
    install_path: PurePosixPath = PurePosixPath("/data/local/tmp/lldb/lldb-server")
    tcp_listen_port: int = 1234
    platform: str = "remote-ohos"

_server_config_item_mapping: Mapping[str, Tuple[str, type]] = {
    "arch": ("arch", Arch),
    "install-path": ("install_path", PurePosixPath),
    "tcp-listen-port": ("tcp_listen_port", int),
    "platform": ("platform", str)
}

_config = _ServerConfig()

def show_server_configs() -> None:
    for config_name, mapping in _server_config_item_mapping.items():
        attr_name = mapping[0]
        attr_value = getattr(_config, attr_name)
        value = attr_value.value if isinstance(attr_value, Enum) else attr_value
        print(f"{config_name}='{value}'")

def set_server_config(name: str, value: str) -> None:
    mapping = _server_config_item_mapping.get(name)
    if mapping is None:
        print(f"Error: unknown config name '{name}'")
        return
    attr_name, attr_type = mapping
    setattr(_config, attr_name, attr_type(value))

def _check_hdc() -> None:
    if shutil.which("hdc") is None:
        raise LLDBException("hdc is not found. Make sure 'PATH' environment variable is properly set.")

def _guess_arch() -> Arch:
    ret = subprocess.check_output('hdc shell file /system/bin/appspawn', shell = True)
    str_arch = ret.decode(encoding = "utf-8", errors = "strict")
    match_res = re.search("musl-(.*?)(?:\.so)", str_arch)

    if match_res is None:
        return Arch.UNKNOWN
    try:
        arch = Arch(match_res.group(1))
    except ValueError:
        arch = Arch.UNKNOWN
    return arch

def _parse_lldb_version(version_string: str) -> Optional[str]:
    # An example of LLDB's version string:
    #
    # lldb version 15.0.4 (https://gitee.com/openharmony/third_party_llvm-project revision edacc9e94de5f6d415b1b1ad13f420194739b85a)
    #   clang revision edacc9e94de5f6d415b1b1ad13f420194739b85a
    #     llvm revision edacc9e94de5f6d415b1b1ad13f420194739b85a
    parts = version_string.split()
    if len(parts) < 3:
        return None
    return parts[2]

def _find_lldb_server(arch: Arch) -> Path:
    if arch is Arch.UNKNOWN:
        arch = _guess_arch()
    if arch is Arch.UNKNOWN:
        raise LLDBException("failed to get hardware archtecture for your device")

    curr_script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
    version = _parse_lldb_version(lldb.debugger.GetVersionString())
    if version is None:
        raise LLDBException("failed to get version of LLDB")

    lldb_server_path = curr_script_dir / ".." / "lib" / "clang" / version / "bin" / f"{arch.value}-linux-ohos" / "lldb-server"
    return lldb_server_path

def _install_lldb_server(lldb_server_path: Path, install_path: PurePosixPath) -> None:
    if not lldb_server_path.exists():
        raise LLDBException(f"file '{lldb_server_path}' does not exists")
    if not lldb_server_path.is_file():
        raise LLDBException(f"file 'f{lldb_server_path}' is not a regular file")

    subprocess.run(['hdc', 'shell', 'mkdir', '-p', str(install_path.parent)])
    subprocess.run(['hdc', 'file', 'send', str(lldb_server_path), str(install_path)])
    subprocess.run(['hdc', 'shell', 'chmod', 'u+x', str(install_path)])

def _start_lldb_server(lldb_server_path: PurePosixPath, tcp_listen_port: int) -> None:
    subprocess.Popen(['hdc', 'shell', str(lldb_server_path), 'platform', '--listen', f'*:{tcp_listen_port}'], stdout=subprocess.PIPE)
    time.sleep(3)

# connect to lldb-server
def _connect_server(platform: str, server_port: int) -> None:
    remote_platform = lldb.SBPlatform(platform)
    server_url = f"connect://localhost:{server_port}"
    platform_connect_options = lldb.SBPlatformConnectOptions(server_url)
    err = remote_platform.ConnectRemote(platform_connect_options)
    if not err.Success():
        raise LLDBException(f"failed to connect to platform '{platform}' using URL '{server_url}': {err}")

    lldb.debugger.SetSelectedPlatform(remote_platform)
    print("platform connected.")

def main() -> None:
    try:
        _check_hdc()
        lldb_server_path = _find_lldb_server(_config.arch)
        _install_lldb_server(lldb_server_path, _config.install_path)
        _start_lldb_server(_config.install_path, _config.tcp_listen_port)
        _connect_server(_config.platform, _config.tcp_listen_port)
    except LLDBException as e:
        print(f"Error: {e.err_msg}")

if __name__ == '__main__':
    main()
