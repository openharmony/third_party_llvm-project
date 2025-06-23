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
import argparse

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
        log_file_path = "file_permissions_check_log" + timestamp + ".html"

        success_log = []
        error_log = []
        lack_log = []
        add_log = []
        checklist_files = set()

        with open(checklist_file, 'r') as ff:
            for line in ff:
                checklist_files.add(line.split(',')[0].split('lib/', 1)[1].replace('\n', ''))
                lib_file_path = os.path.join(directory, line.split(',')[0].split('lib/', 1)[1]).replace('\\', '/')
                file_stat_expected = line.split(',')[1].replace('\n', '')
                file_size_expected = line.split(',')[2].replace('\n', '')
                try:
                    file_stat_real = stat.filemode(os.stat(lib_file_path).st_mode)
                    if file_stat_expected == file_stat_real:
                        success += 1
                        total += 1
                        success_log.append((lib_file_path, file_stat_real))
                    else:
                        fail += 1
                        total += 1
                        error_log.append((lib_file_path, file_stat_expected, file_stat_real))
                except FileNotFoundError:
                    lack += 1
                    total += 1
                    lack_log.append((lib_file_path, file_size_expected))

        # Check for add files in the directory
        directory_files = set()
        for root, dirs, files in os.walk(directory):
            for file in files:
                file_path = os.path.join(root, file).replace(directory, '').replace('\\', '/')[1:]
                directory_files.add(file_path)
                if file_path not in checklist_files:
                    add_log.append(file_path)

        with open(log_file_path, 'w') as f:
            f.write("<html><head><title>File Permissions Check</title></head><body>\n")
            f.write(f"<h1>File Permissions Check Report</h1>\n")
            f.write(f"<h2>Total {total} files in {directory}</h2>\n")
            f.write(f"<h3>Success: {success}/{total}</h3>\n")
            f.write(f"<h3>Failed: {fail}/{total}</h3>\n")
            f.write(f"<h3>Missed: {lack}/{total}</h3>\n")
            f.write(f"<h3>add: {len(add_log)}/{total}</h3>\n")

            f.write("<h2>Failed Checks</h2>\n")
            f.write("<table border='1'><tr><th>File</th><th>Expected</th><th>Real</th></tr>\n")
            for lib_file_path, expected, real in error_log:
                f.write(f"<tr><td>{lib_file_path}</td><td>{expected}</td><td>{real}</td></tr>\n")
            f.write("</table>\n")

            f.write("<h2>Missed Files</h2>\n")
            f.write("<table border='1'><tr><th>File</th><th>Expected Size (MB)</th></tr>\n")
            for lib_file_path, file_size_expected in lack_log:
                f.write(f"<tr><td>{lib_file_path}</td><td>{file_size_expected}</td></tr>\n")
            f.write("</table>\n")

            f.write("<h2>add Files</h2>\n")
            f.write("<table border='1'><tr><th>File</th></tr>\n")
            for file_path in add_log:
                f.write(f"<tr><td>{file_path}</td></tr>\n")
            f.write("</table>\n")

            f.write("<h2>Successful Checks</h2>\n")
            f.write("<table border='1'><tr><th>File</th><th>Permissions</th></tr>\n")
            for lib_file_path, permissions in success_log:
                f.write(f"<tr><td>{lib_file_path}</td><td>{permissions}</td></tr>\n")
            f.write("</table>\n")

            f.write("</body></html>\n")

        if success == total and len(add_log) == 0:
            self.logger.info(f"{directory} files permissions check passed.")
            return True
        else:
            self.logger.error(f"{directory} files permissions check failed.")
            self.logger.error(f"{fail}/{total} checks failed.\n")
            self.logger.error(f"{lack}/{total} file missed.\n")
            self.logger.error(f"{len(add_log)}/{total} add files found.\n")
            self.logger.error(f"Total {fail + lack + len(add_log)}/{total} checks failed, check logs are in {os.path.join(os.getcwd(), log_file_path)}")
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
        if len(sys.argv) < self.SO_PERMISSION_CHECK_MIN_ARGS_NUMBER:
            self.logger.info("Missing arguments. Correct example: python filepermissioncheck.py --generate-default-list clang/lib")
            return
        
        parser = argparse.ArgumentParser(description='File permission check tool.')
        parser.add_argument(
            '--generate-list',
            nargs=2,
            metavar=('DIRECTORY', 'OUTPUT_FILE'),
            help='Generate a file permission checklist for the given directory and save to the output file.')
        
        parser.add_argument(
            '--generate-default-list',
            nargs=1,
            metavar='DIRECTORY',
            help='Generate a default file permission checklist for the given directory.')
        
        parser.add_argument(
            '--generate-ndk-list',
            nargs=1,
            metavar='DIRECTORY',
            help='Generate a default NDK library file permission checklist for the given directory.')
        
        parser.add_argument(
            '--file-check',
            nargs=2,
            metavar=('DIRECTORY', 'CHECKLIST_FILE'),
            help='Check file permissions in the given directory based on the provided checklist file.')
        
        parser.add_argument(
            '--check-default-clang',
            nargs=1,
            metavar='DIRECTORY',
            help='Check file permissions in the given Clang directory based on the default checklist.')
        parser.add_argument(
            '--check-default-ndk',
            nargs=1,
            metavar='DIRECTORY',
            help='Check file permissions in the given NDK directory based on the default checklist.')
        
        parser.add_argument(
            '--compare-between-path',
            nargs=2,
            metavar=('SOURCE_DIRECTORY', 'TARGET_DIRECTORY'),
            help='Generate a checklist for the source directory and check the target directory against it.')

        args = parser.parse_args()

        if len(vars(args)) > 1:
            if args.generate_list:
                self.file_list_generate(args.generate_list[0], args.generate_list[1])
            elif args.generate_default_list:
                self.file_list_generate(args.generate_default_list[0], self.DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            elif args.generate_ndk_list:
                self.file_list_generate(args.generate_ndk_list[0], self.DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH)
            elif args.file_check:
                self.file_list_check(args.file_check[0], args.file_check[1])
            elif args.check_default_clang:
                self.file_list_check(args.check_default_clang[0], self.DEFAULT_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            elif args.check_default_ndk:
                self.file_list_check(args.check_default_ndk[0], self.DEFAULT_NDK_LIB_FILE_PERMISSION_CHECKLIST_PATH)
            elif args.compare_between_path:
                self.file_list_generate(args.compare_between_path[0], self.TEMP_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
                self.file_list_check(args.compare_between_path[1], self.TEMP_CLANG_FILE_PERMISSION_CHECKLIST_PATH)
            else:
                self.logger.info("The input arguments are incorrect. Check the input arguments by reading the README document.")

def main():
    file_permission_check = FilePermissionCheck()
    file_permission_check.cmd_handler()

if __name__ == '__main__':
    main()
