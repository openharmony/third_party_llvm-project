#!/bin/bash
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

# LLVM Toolchain Compiled Product File Permission Check Script
## Purpose
To verify whether the permissions of all files in the folders of LLVM compiled products, Clang, and NDK meet the requirements.

## Version Description
The permission check list included in the current submission is based on the version 15.0.4-6fe50d generated from the mirror site (https://mirrors.huaweicloud.com/harmonyos/compiler/clang/15.0.4-6fe50d/linux/).

## Usage
Checks are performed separately on the folders of Clang and NDK. The {file_path} should be the absolute path or relative path (relative to the location of the Python script).

### Generate SO File Permission List
Search and check each  file in the folder, and record the permission list to the output file.

```shell
python file_permission_check.py --generate-list {file_path} {filename}
```
### Generate Default SO File Permission List
Search and check each file in the folder, and record the permission list to the output file. The default check lists are default_clang_file_permission.checklist and default_ndk_flie_permission.checklist.
```shell
# Generate default permission list for Clang
python file_permission_check.py --generate-default-list {file_path}
# Generate default permission list for NDK
python file_permission_check.py --generate-ndk-list {file_path}
```
### Check Custom List
Check the permissions of the files in the input folder based on the provided permission check list.
```shell
python file_permission_check.py --file-check {file_path} {filename}
```
### Check Default List
Verify the permissions of the files in the input folder against the permission list of the baseline version (before running this command, ensure that the directory contains default_clang_file_permission.checklist and default_ndk_flie_permission.checklist).
```shell
# Check Clang 
python file_permission_check.py --check-default-clang {file_path}
# Check NDK 
python file_permission_check.py --check-default-ndk {file_path}
```

### Customize comparing two directories
Provide two custom directories and verify the file differences between the two folders, with the old folder at the front and the new folder at the back.
```shell
# Customize directories
python file_permission_check.py --compare-between-path {old_file_path} {new_file_path}
```
