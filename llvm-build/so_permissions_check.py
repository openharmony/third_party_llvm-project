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
import logging

class SoPermissionCheck:
    SO_PERMISSION_CHECK_MIN_ARGS_NUMBER = 3
    DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH = "default_clang_lib_so_permission.checklist"
    DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH = "default_ndk_lib_so_permission.checklist"
    logger = None

    def __init__(self) -> None:
        self.logger = logging.getLogger("so permission check")
        self.logger.setLevel(logging.INFO)
        ch = logging.StreamHandler()
        ch.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s: %(levelname)s: %(name)s: %(message)s')
        ch.setFormatter(formatter)
        self.logger.addHandler(ch)

    def so_permissions_list_generate(self, directory: str, output_file: str):
        '''
        Read all .so files in the lib folder and generate a verification file.
        Typically used for generating new verification files after a version update.

        Args: 
            directory(str): the clang lib path or the ndk lib path
            output_file(str): path and filename of the output file to save the permission check list
        Return:
            No return value but a checklist file will be generated. 
        '''
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
        self.logger.info(f"Total {total} .so files in {directory}")

    def so_files_permissions_check(self, directory: str, checklist_file: str) -> bool:
        '''
        Check if the permissions of the binary files match those in the checklist file.

        Args: 
            directory(str): the clang lib path or the ndk lib path
            checklist_file(str): path and filename of the checklist file saved the permission check list
        Return:
            True(all permission check succeeded) or False(one or more check failed). 
        '''
        lack = 0
        total = 0
        fail = 0
        success = 0

        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        log_file_path = "so_permissions_check_log" + timestamp + ".txt"

        success_log = ""
        error_log = ""
        lack_log = ""
        with open(checklist_file, 'r') as ff:
            for line in ff:
                lib_file_path = os.path.join(directory, line.split(',')[0].split('lib/', 1)[1]).replace('\\', '/')
                so_file_stat_expected = line.split(',')[1].replace('\n', '')
                so_file_stat_real = self.get_file_permissions(lib_file_path)
                if so_file_stat_real is None:
                    lack += 1
                    total += 1
                    lack_log += f"{lib_file_path} check failed, the file is missed.\n"
                elif so_file_stat_expected == stat.filemode(os.stat(lib_file_path).st_mode):
                    success += 1
                    total += 1
                    success_log += f"{lib_file_path} check succeeded.\n"
                else:
                    fail += 1
                    total += 1
                    error_log += f"{lib_file_path} check failed, expected: {so_file_stat_expected}, real: {stat.filemode(os.stat(lib_file_path).st_mode)}\n"

        with open(log_file_path, 'w') as f:
            f.write(f'llvm so permissions check complete, Success: {success}/{total}, Failed: {fail}/{total}, Lack: {lack}/{total}\n')
            f.write(f'\n{fail}/{total} checks failed.\n')
            f.write(error_log)
            f.write(f'\n{lack}/{total} file missed.\n')
            f.write(lack_log)
            f.write(f'\n{success}/{total} checks succeed.\n')
            f.write(success_log)
            
        if success == total:
            self.logger.info(f"{directory} so permissions check passed.")
            return True
        else:
            self.logger.error(f"{directory} so permissions check failed.")
            self.logger.error(f"{fail}/{total} checks failed.\n")
            self.logger.error(error_log)
            self.logger.error(f"{lack}/{total} file missed.\n")
            self.logger.error(lack_log)
            self.logger.error(f"{fail + lack}/{total} checks failed, check logs are in {os.path.join(os.getcwd(), log_file_path)}")
            return False

    def get_file_permissions(self, file_path: str) -> str:
        '''Get file permissions and throw possible exceptions.'''
        try:
            file_stat = os.stat(file_path)
            permissions = stat.filemode(file_stat.st_mode)
            return permissions
        except FileNotFoundError:
            self.logger.error(f"file '{file_path}' not found.")
        except PermissionError:
            self.logger.error(f"no read permission for '{file_path}'.")
        except Exception as e:
            self.logger.error(f"unknown error - {e}")
        return None

    def check(self, clang_lib_path: str, ndk_lib_path: str) -> bool:
        '''
        Main build script calling interface. 
        Check the so file permissions in clang and ndk lib directories based on the default check list.
        '''
        return self.so_files_permissions_check(clang_lib_path, self.DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH) \
            & self.so_files_permissions_check(ndk_lib_path, self.DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH)

    def cmd_handler(self) -> None:
        '''Perform parameter recognition and execute corresponding tasks.'''
        args = sys.argv
        if len(args) < self.SO_PERMISSION_CHECK_MIN_ARGS_NUMBER:
            self.logger.info("Missing arguments. Correct eg: python so_permission_check.py -g clang/lib")
            return
        
        match args[1]:
            case '-g':
                self.so_permissions_list_generate(args[2], args[3])
            case '-cdg':
                self.so_permissions_list_generate(args[2], self.DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH)
            case '-ndg':
                self.so_permissions_list_generate(args[2], self.DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH)
            case '-v':
                self.so_files_permissions_check(args[2], args[3])
            case '-cdv':
                self.so_files_permissions_check(args[2], self.DEFAULT_CLANG_LIB_SO_PERMISSION_CHECKLIST_PATH)
            case '-ndv':
                self.so_files_permissions_check(args[2], self.DEFAULT_NDK_LIB_SO_PERMISSION_CHECKLIST_PATH)
            case _:
                self.logger.info("The input arguments are incorrect. Check the input arguments by reading the README document.")
def main():
    so_permission_check = SoPermissionCheck()
    so_permission_check.cmd_handler()
if __name__ == '__main__':
    main()
