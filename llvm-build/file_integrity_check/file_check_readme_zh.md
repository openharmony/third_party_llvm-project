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

# llvm工具链编译产物中so文件权限校验脚本
## 用途
校验llvm编译产物clang和ndk中下所有的文件权限是否符合要求,以及文件完整性
## 版本说明
当前提交中包含的权限检查列表基于镜像网站中15.0.4-6fe50d版本生成（https://mirrors.huaweicloud.com/harmonyos/compiler/clang/15.0.4-6fe50d/linux/）
## 使用方法
针对clang和ndk中文件夹分别进行校验，`{file_path}`为clang或ndk文件夹对应绝对路径或相较于py脚本所在位置的相对路径
### 文件权限列表生成
搜索并检查文件夹内的每个文件，记录权限列表到输出文件中
```shell
python file_permission_check.py --generate-list {file_path} {filename}
```
### 默认文件权限列表生成
搜索并检查文件夹内的每个文件，记录权限列表到输出文件中，默认检查列表为default_clang_file_permission.checklist和default_ndk_flie_permission.checklist
```shell
# clang 默认权限列表生成
python file_permission_check.py --generate-default-list {file_path}
# ndk 默认权限列表生成
python file_permission_check.py --generate-ndk-list {file_path}
```
### 校验自定义列表
根据给定的权限校验列表，校验输入文件夹内文件的权限是否正确
```shell
python file_permission_check.py --file-check {file_path} {filename}
```
### 校验默认列表
根据基准版本的权限列表，校验输入文件夹内文件的权限是否正确(运行该指令之前，确保目录中包含default_clang_file_permission.checklist和default_ndk_flie_permission.checklist)
```shell
# clang 校验
python file_permission_check.py --check-default-clang {file_path}
# ndk 校验
python file_permission_check.py --check-default-ndk {file_path}
```

### 自定义比较两个目录
给出自定义的两个目录，校验两个文件夹的文件差异，需要旧的文件夹在前，新的文件夹在后
```shell
# 自定义目录
python file_permission_check.py --compare-between-path {old_file_path} {new_file_path}
```
