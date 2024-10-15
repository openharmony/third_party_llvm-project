# llvm工具链编译产物中so文件权限校验脚本
## 用途
校验llvm编译产物clang和ndk中lib文件夹下所有的so文件权限是否符合要求
## 版本说明
当前提交中包含的权限检查列表基于镜像网站中15.0.4-6fe50d版本生成（https://mirrors.huaweicloud.com/harmonyos/compiler/clang/15.0.4-6fe50d/linux/）
## 使用方法
针对clang和ndk中lib文件夹分别进行校验，`{lib_path}`为clang或ndk下第一层lib对应绝对路径或相较于py脚本所在位置的相对路径,且以lib结尾
### so文件权限列表生成
搜索并检查lib文件夹内的每个.so文件，记录权限列表到输出文件中
```shell
python so_permission_check.py -g {lib_path} {filename}
```
### 默认so文件权限列表生成
搜索并检查lib文件夹内的每个.so文件，记录权限列表到输出文件中，默认检查列表为default_clang_lib_so_permission.checklist和default_ndk_lib_so_permission.checklist
```shell
# clang lib默认权限列表生成
python so_permission_check.py -cdg {lib_path}
# ndk lib默认权限列表生成
python so_permission_check.py -ndg {lib_path}
```
### 校验自定义列表
根据给定的权限校验列表，校验输入lib文件夹内.so文件的权限是否正确
```shell
python so_permission_check.py -v {lib_path} {filename}
```
### 校验默认列表
根据基准版本的.so权限列表，校验输入lib文件夹内.so文件的权限是否正确(运行该指令之前，确保目录中包含default_clang_lib_so_permission.checklist和default_ndk_lib_so_permission.checklist)
```shell
# clang lib校验
python so_permission_check.py -cdv {lib_path}
# ndk lib校验
python so_permission_check.py -ndv {lib_path}
```

# LLVM Toolchain Compiled Product SO File Permission Check Script
## Purpose
To verify whether the permissions of all .so files in the lib folders of LLVM compiled products, Clang, and NDK meet the requirements.

## Version Description
The permission check list included in the current submission is based on the version 15.0.4-6fe50d generated from the mirror site (https://mirrors.huaweicloud.com/harmonyos/compiler/clang/15.0.4-6fe50d/linux/).

## Usage
Checks are performed separately on the lib folders of Clang and NDK. The {lib_path} should be the absolute path or relative path (relative to the location of the Python script) corresponding to the first-level lib directory, and it should end with lib.

### Generate SO File Permission List
Search and check each .so file in the lib folder, and record the permission list to the output file.

```shell
python so_permission_check.py -g {lib_path} {filename}
```
### Generate Default SO File Permission List
Search and check each .so file in the lib folder, and record the permission list to the output file. The default check lists are default_clang_lib_so_permission.checklist and default_ndk_lib_so_permission.checklist.
```shell
# Generate default permission list for Clang lib
python so_permission_check.py -cdg {lib_path}
# Generate default permission list for NDK lib
python so_permission_check.py -ndg {lib_path}
```
### Check Custom List
Check the permissions of the .so files in the input lib folder based on the provided permission check list.
```shell
python so_permission_check.py -v {lib_path} {filename}
```
### Check Default List
Verify the permissions of the .so files in the input lib folder against the permission list of the baseline version (before running this command, ensure that the directory contains default_clang_lib_so_permission.checklist and default_ndk_lib_so_permission.checklist).
```shell
# Check Clang lib
python so_permission_check.py -cdv {lib_path}
# Check NDK lib
python so_permission_check.py -ndv {lib_path}
```