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
import stat
import sys
from datetime import datetime

DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH = "default_clang_lib_so_permission.checklist"
DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH = "default_ndk_lib_so_permission.checklist"

def main():
    args = sys.argv
    if len(args) < 3:
        print("Missing arguments. Correct eg: python so_permission_check.py -g clang/lib")
        return
    if args[1] == '-g':
        so_permissions_list_generate(args[2], args[3])
    elif args[1] == '-cdg':
        so_permissions_list_generate(args[2], DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH)
    elif args[1] == '-ndg':
        so_permissions_list_generate(args[2], DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH)
    elif args[1] == '-v':
        so_files_permissions_check(args[2], args[3])
    elif args[1] == '-cdv':
        so_files_permissions_check(args[2], DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH)
    elif args[1] == '-ndv':
        so_files_permissions_check(args[2], DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH)
    else:
        print("The input parameter is incorrect. Check the input parameter by reading the README document.")

# Read all .so files in the lib folder and generate a verification file (typically used for generating new verification files after a version update).
def so_permissions_list_generate(directory, output_file):

    with open(output_file, 'w') as ff:
        pass
    total = 0
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.so'):
                total += 1
                file_path = os.path.join(root, file)
                file_stat = os.stat(file_path)
                file_mode = stat.filemode(file_stat.st_mode)
                with open(output_file, 'a') as f:
                    f.write('lib' + file_path.replace(directory, '').replace('\\', '/') + ',' + file_mode+ '\n')
    print(f"Total {total} .so files in {directory}")

# Check if the permissions of the binary files match those in the verification file.
def so_files_permissions_check(directory, standard_file):
    lack = 0
    total = 0
    fail = 0
    success = 0

    timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    log_file_path = "so_permissions_check_log" + timestamp + ".txt"

    success_log = ""
    error_log = ""
    lack_log = ""
    with open(standard_file, 'r') as ff:
        for line in ff:
            lib_file_path = os.path.join(directory, line.split(',')[0].split('lib/', 1)[1]).replace('\\', '/')
            so_file_stat_expected = line.split(',')[1].replace('\n', '')
            so_file_stat_real = get_file_permissions(lib_file_path)
            if so_file_stat_real == None:
                lack += 1
                total += 1
                lack_log += lib_file_path + ' check failed, the file is missed.\n'
            elif so_file_stat_expected == stat.filemode(os.stat(lib_file_path).st_mode):
                success += 1
                total += 1
                success_log += lib_file_path + ' check succeeded.\n'
            else:
                fail += 1
                total += 1
                error_log += lib_file_path + ' check failed, expected:' + so_file_stat_expected + ', real: ' + stat.filemode(os.stat(lib_file_path).st_mode) + '\n'
    if success == total:
        print("So Permissions check passed.")
    else:
        print("So Permissions check failed.")
        print(str(fail) + '/' + str(total) + ' checks failed.\n')
        print(error_log)
        print(str(lack) + '/' + str(total) + ' file missed.\n')
        print(lack_log)
        print(str(fail + lack) + '/' + str(total) + ' checks failed, check logs are in ' + os.path.join(os.getcwd(), log_file_path))
    with open(log_file_path, 'w') as f:
        f.write('llvm so permissions check complete, ' + 'Success:' + str(success) + '/' + str(total) + ', ' +  'Failed:' + str(fail) + '/' + str(total) + ', Lack:' + str(lack) + '/' + str(total))
        f.write('\n\n' + str(fail) + '/' + str(total) + ' checks failed.\n')
        f.write(error_log)
        f.write('\n\n' + str(lack) + '/' + str(total) + ' file missed.\n')
        f.write(lack_log)
        f.write('\n\n' + str(success) + '/' + str(total) + ' checks succeed.\n')
        f.write(success_log)
    print('Success:' + str(success) + '/' + str(total) + ', ' +  'Failed:' + str(fail) + '/' + str(total) + ', Lack:' + str(lack) + '/' + str(total))

def get_file_permissions(file_path):
    try:
        file_stat = os.stat(file_path)
        permissions = stat.filemode(file_stat.st_mode)
        return permissions
    except FileNotFoundError:
        print(f"error: file '{file_path}' not found.")
    except PermissionError:
        print(f"error: no read permission for '{file_path}'.")
    except Exception as e:
        print(f"error: unknow error - {e}")
    return None

if __name__ == '__main__':
    main()