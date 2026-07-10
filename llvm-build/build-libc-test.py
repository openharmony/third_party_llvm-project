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

from build import BuildConfig, SysrootComposer

def main():
    print('Start building musl libc tests')
    build_config = BuildConfig()
    sysroot_composer = SysrootComposer(build_config)

    product_name = 'llvm_build'
    target_cpu = 'arm64'
    gn_args = ''

    os.chdir(build_config.LLVM_BUILD_DIR)
    sysroot_composer.run_hb_build(product_name, target_cpu, 'libctest', gn_args)
    sysroot_composer.run_hb_build(product_name, target_cpu, 'libc_gtest', gn_args)


if __name__ == "__main__":
    main()



