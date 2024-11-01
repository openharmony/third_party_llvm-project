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