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

class FilePermissionCheck:
    SO_PERMISSION_CHECK_MIN_ARGS_NUMBER = 3

    DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH = "default_clang_file_permission.checklist"
    DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH = "default_ndk_flie_permission.checklist"
    TEMP_CLANG_FILE_PERMISSION_CHECKLIST_PATH = "temp_clang_file_permission.checklist"
    logger = None
    
    def __init__(self) -> None:
        self.logger = logging.getLogger("file permission check")
        self.logger.setLevel(logging.INFO)
        ch = logging.StreamHandler()
        ch.setLevel(logging.INFO)
        formatter = logging.Formatter('%(asctime)s: %(levelname)s: %(name)s: %(message)s')
        ch.setFormatter(formatter)
        self.logger.addHandler(ch)

    def get_file_size(file_path):
        """返回文件的大小，以字节为单位"""
        if os.path.exists(file_path):
            return os.path.getsize(file_path)
        return 0

    def file_list_generate(self, directory: str, output_file: str):
        '''
        Args: 
            directory(str): the clang path or the ndk path
            output_file(str): path and filename of the output file to save the permission check list
        '''
        with open(output_file, 'w') as ff:
            pass
        total = 0
        for root, dirs, files in os.walk(directory):
            for file in files:
                total += 1
                file_path = os.path.join(root, file)
                file_stat = os.stat(file_path)
                file_mode = stat.filemode(file_stat.st_mode)
                file_size_bytes = os.path.getsize(file_path)
                file_size = round(file_size_bytes / (1024 * 1024), 1)
                with open(output_file, 'a') as f:
                    f.write('lib' + file_path.replace(directory, '').replace('\\', '/') + ',' + file_mode + ',' + str(file_size) + '\n')
        self.logger.info(f"Total {total} files in {directory}")

    def file_list_check(self, directory: str, checklist_file: str) -> bool:
        '''
        Args: 
            directory(str): the clang path or the ndk lib path
            checklist_file(str): path and filename of the checklist file saved the permission check list
        Return:
            True(all permission check succeeded) or False(one or more check failed). 
        '''
        lack = 0
        total = 0
        fail = 0
        success = 0

        timestamp = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
        log_file_path = "file_permissions_check_log" + timestamp + ".txt"

        success_log = ""
        error_log = ""
        lack_log = ""
        with open(checklist_file, 'r') as ff:
            for line in ff:
                lib_file_path = os.path.join(directory, line.split(',')[0].split('lib/', 1)[1]).replace('\\', '/')
                file_stat_expected = line.split(',')[1].replace('\n', '')
                file_size_expected = line.split(',')[2].replace('\n', '')
                file_stat_real = self.get_file_permissions(lib_file_path)
                if file_stat_real is None:
                    lack += 1
                    total += 1
                    lack_log += f"{lib_file_path} check failed, the file is missed, missing file size is {file_size_expected} MB.\n"
                elif file_stat_expected == stat.filemode(os.stat(lib_file_path).st_mode):
                    success += 1
                    total += 1
                    success_log += f"{lib_file_path} check succeeded.\n"
                else:
                    fail += 1
                    total += 1
                    error_log += f"{lib_file_path} check failed, expected: {file_stat_expected}, real: {stat.filemode(os.stat(lib_file_path).st_mode)}\n"

        with open(log_file_path, 'w') as f:
            f.write(f'llvm file permissions check complete, Success: {success}/{total}, Failed: {fail}/{total}, Lack: {lack}/{total}\n')
            f.write(f'\n{fail}/{total} checks failed.\n')
            f.write(error_log)
            f.write(f'\n{lack}/{total} file missed.\n')
            f.write(lack_log)
            f.write(f'\n{success}/{total} checks succeed.\n')
            f.write(success_log)
            
        if success == total:
            self.logger.info(f"{directory} files permissions check passed.")
            return True
        else:
            self.logger.error(f"{directory} files permissions check failed.")
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
        return self.file_list_check(clang_lib_path, self.DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH) \
            & self.file_list_check(ndk_lib_path, self.DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH)

    def cmd_handler(self) -> None:
        '''Perform parameter recognition and execute corresponding tasks.'''
        args = sys.argv
        if len(args) < self.SO_PERMISSION_CHECK_MIN_ARGS_NUMBER:
            self.logger.info("Missing arguments. Correct eg: python file_permission_check.py --generate-default-list clang/lib")
            return
        
        if len(args) > 1:
            if args[1] == '--generate-list':
                self.file_list_generate(args[2], args[3])
            elif args[1] == '--generate-default-list':
                self.file_list_generate(args[2], self.DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            elif args[1] == '--generate-ndk-list':
                self.file_list_generate(args[2], self.DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH)
            elif args[1] == '--file-check':
                self.file_list_check(args[2], args[3])
            elif args[1] == '--check-default-clang':
                self.file_list_check(args[2], self.DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            elif args[1] == '--check-default-ndk':
                self.file_list_check(args[2], self.DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH)
            elif args[1] == '--compare-between-path':
                self.file_list_generate(args[2], self.TEMP_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
                self.file_list_check(args[3], self.TEMP_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            else:
                self.logger.info("The input arguments are incorrect. Check the input arguments by reading the README document.")
def main():
    file_permission_check = FilePermissionCheck()
    file_permission_check.cmd_handler()
if __name__ == '__main__':
    main()
