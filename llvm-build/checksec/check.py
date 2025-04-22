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

import json
import os
import time
import datetime
import argparse
import jinja2
import subprocess
from typing import List, Dict, Union

# Constants for check statuses
CHECK_TRUE = 'true'
CHECK_FALSE = 'false'
CHECK_PASS = 'passed'
CHECK_FAILED = 'failed'
CHECK_ITEM_NUM = 11

class HtmlTitle:
    """Class representing summary data for the report."""
    
    _start_time = 0
    _end_time = 0
    _execution_time = 0
    _total_file_num = 0
    _file_size_sum = 0
    _check_file_list_num = 0

def is_executable_or_library(file_name: str) -> bool:
    """
    Check if the given file is an executable or library file.
    
    :param file_name: The name of the file to check
    :return: True if the file is executable or a library, False otherwise
    """
    # Check if the file exists first.
    if not os.path.isfile(file_name):
        return False
    # Filter by file extension.
    # valid_extensions = ('.out', '.bin', '.elf', '.so', '.a')  
    # if file_name.endswith(valid_extensions) or os.access(file_name, os.X_OK):
    #     return True
    
    # Use the file command to determine the file type.
    try:
        file_type = subprocess.check_output(['file', '--mime-type', file_name]).decode('utf-8').strip()
        # Check if the file is an executable file, shared object file, or library file.
        if 'executable' in file_type or 'shared object' in file_type or 'library' in file_type:
            return True
    except subprocess.CalledProcessError:
        pass  
    
    return False

class GenerateReport:
    """Class for generating HTML and JSON reports from checksec results."""

    @staticmethod
    def _generate_failed_files_html_report(output_path: str, failed_file_list: List[Dict]):
        """
        Generate an HTML report for failed files.

        Args:
            output_path (str): Directory where the HTML file will be saved.
            failed_file_list (list): List of dictionaries containing failed file details.
        """
        env = jinja2.Environment(loader = jinja2.FileSystemLoader('./templates'))
        template = env.get_template('failed_files_template.html')
        data = {
            'check_file_list' : failed_file_list
        }
        document = template.render(data)
        file_path=os.path.join(output_path, 'failed files list.html')
        with open(file_path, 'w', encoding='utf-8') as failed_file:
            failed_file.write(document)

    @staticmethod
    def _generate_total_files_html_report(output_path: str, total_file_info: List[Dict]):
        """
        Generate an HTML report for all scanned files.

        Args:
            output_path (str): Directory where the HTML file will be saved.
            total_file_info (list): List of all files and their check details.
        """
        env = jinja2.Environment(loader = jinja2.FileSystemLoader('./templates'))
        template = env.get_template('total_files_template.html')
        data = {
            'total_file_info' : total_file_info,
        }
        document = template.render(data)
        file_path = os.path.join(output_path, 'total file list.html')
        with open(file_path, 'w', encoding='utf-8') as total_file:
            total_file.write(document)

    @staticmethod
    def _generate_summary_html_report(output_path: str, check_items: List['CheckItemData'], summary_data: 'HtmlTitle',
                                     total_file_info: List[Dict]):
        """
        Generate a summary HTML report.

        Args:
            output_path (str): Directory where the HTML file will be saved.
            check_items (list): List of check items with results.
            summary_data (HtmlTitle): Summary data of the scan.
            total_file_info (list): List of all files and their check details.
        """
        env = jinja2.Environment(loader = jinja2.FileSystemLoader('./templates'))
        template = env.get_template('index_files_template.html')
        data = {
            'summary_data' : summary_data,
            'total_file_info' : total_file_info,
            'check_items' : check_items
        }
        document = template.render(data)
        file_path = os.path.join(output_path, 'Index.html')
        with open(file_path, 'w', encoding='utf-8') as index_html_file:
            index_html_file.write(document)

    @staticmethod
    def _generate_failed_files_json_report(output_path: str, failed_file_list: List[Dict]):
        """
        Generate a JSON report for failed files.

        Args:
            output_path (str): Directory where the JSON file will be saved.
            failed_file_list (list): List of failed files with details.
        """
        data = {
                'check_file_list' : failed_file_list
            }
        json_str = json.dumps(data,indent=1)
        file_path = os.path.join(output_path, 'check_file_list.json')
        with open(file_path, 'w' ,encoding='utf-8') as json_file:
            json_file.write(json_str)

    @staticmethod
    def _generate_total_files_json_report(output_path: str, total_file_info: List[Dict]):
        """
        Generate a JSON report for all scanned files.

        Args:
            output_path (str): Directory where the JSON file will be saved.
            total_file_info (list): List of all files and their check details.
        """
        data = {
            'total_file_info' : total_file_info
        }
        json_str =json.dumps(data,indent=1)
        file_path = os.path.join(output_path, 'total_file_list.json')
        with open(file_path, 'w' ,encoding ='utf-8') as json_file:
            json_file.write(json_str)

    @staticmethod
    def _generate_summary_json_report(output_path: str, check_items: List['CheckItemData'], summary_data: 'HtmlTitle',
                                     total_file_info: List[Dict]):
        """
        Generate a summary JSON report.

        Args:
            output_path (str): Directory where the JSON file will be saved.
            check_items (list): List of check items with results.
            summary_data (HtmlTitle): Summary data of the scan.
            total_file_info (list): List of all files and their check details.
        """
        summary_data_dict = summary_data.__dict__
        check_items_dict = {}
        for i in check_items:
            check_items_dict[i._name] = i.__dict__
        data = {
            'summary_data' : summary_data_dict,
            'total_file_info' : total_file_info,
            'check_items' : check_items_dict,
        }
        json_str = json.dumps(data ,indent=1)
        file_path = os.path.join(output_path, 'Index.json')
        with open(file_path, 'w',encoding='utf-8') as json_file:
            json_file.write(json_str)

class CheckItemData:
    """Class representing data for a single security check item."""

    def __init__(self, name: str, do_check: str = CHECK_FALSE, check_flag: Union[str, None] = None):
        """
        Initialize a check item.

        Args:
            name (str): Name of the check item.
            do_check (str): Whether this check is enabled. Defaults to CHECK_FALSE.
            check_flag (str | None): Expected value to compare during the check.
        """
        self._name = name
        self._do_check = do_check
        self._check_flag = check_flag
        self._passed_list = []
        self._failed_list = []
        self._passed_num = 0
        self._failed_num = 0

class CheckData:
    """Class for handling security checks and generating check data."""

    def __init__(self) -> None:
        """
        Initialize the CheckData object with default check items.
        """
        self._relro_check = CheckItemData('relro', CHECK_TRUE, 'full')
        self._nx_check = CheckItemData('nx', CHECK_TRUE, 'yes')
        self._rpath_check = CheckItemData('rpath', CHECK_TRUE, 'no')
        self._canary_check = CheckItemData('canary')
        self._pie_check = CheckItemData('pie')
        self._fortify_check = CheckItemData('fortify')
        self._symbols_check = CheckItemData('symbols')
        self._clangcfi_check = CheckItemData('clangcfi')
        self._clang_safestack_check = CheckItemData('safestack')
        self._fortified_check = CheckItemData('fortified')
        self._fortifiable_check = CheckItemData('fortify-able')

    def _check_security(self, check_file_path: str, check_dir: str, check_items: List[CheckItemData], 
                        summary_data: 'HtmlTitle', file_name_list: List[str], total_file_info: List[Dict]):
        """
        Perform security checks on files in a directory.

        Args:
            check_file_path (str): Path to the JSON file storing file information.
            check_dir (str): Directory containing the files to be checked.
            check_items (list): List of CheckItemData objects representing check configurations.
            summary_data (HtmlTitle): Summary data object to store aggregate information.
            file_name_list (list): List to store the names of checked files.
            total_file_info (list): List to store detailed information about all files.
        """
        check_path=check_dir
        if not os.path.exists(check_path):
            print("check error: check file is not exist")
            exit()
        checksec_statement = './checksec/checksec --dir={0} --extended --output=json'.format(check_path)
        print(checksec_statement)

        checksec_output = json.loads(os.popen(checksec_statement).read())
        self.__generate_summary_data(checksec_output, check_items, summary_data, file_name_list, total_file_info)

    def __generate_summary_data(self, checksec_output: Dict, check_items: List[CheckItemData], 
                                summary_data: 'HtmlTitle', file_name_list: List[str], total_file_info: List[Dict]):
        """
        Generate summary data from checksec output.

        Args:
            checksec_output (dict): Parsed JSON output from checksec.
            check_items (list): List of CheckItemData objects representing check configurations.
            summary_data (HtmlTitle): Summary data object to store aggregate information.
            file_name_list (list): List to store the names of checked files.
            total_file_info (list): List to store detailed information about all files.
        """
        file_size_sum = 0
        for file_info in checksec_output.values():
            if 'name' not in file_info:
                file_name = file_info.get('filename')
                if not is_executable_or_library(file_name):   # if the given file isn't an executable or library file, filter out
                    continue
                file_name_list.append(''.join(list(key for key, value in checksec_output.items() if value == file_info)))
                summary_data._total_file_num = len(file_name_list)
                self.__generate_check_items_data(file_info, check_items)
                file_info['file_single_size'] = get_file_size(os.path.getsize(file_info['filename']))
                file_info['fortify_able'] = file_info['fortify-able']
                total_file_info.append(file_info)
        for check_item in check_items:
            check_item._passed_num = len(check_item._passed_list)
            check_item._failed_num = len(check_item._failed_list)
        for file_name in file_name_list:
            file_size_sum += os.path.getsize(file_name)
        summary_data._file_size_sum = get_file_size(file_size_sum)

    def __generate_check_items_data(self, file_info: Dict, check_items: List[CheckItemData]):
        """
        Populate check item results for a single file.

        Args:
            file_info (dict): Information about a single file.
            check_items (list): List of CheckItemData objects representing check configurations.
        """
        for i in range(0, CHECK_ITEM_NUM):
            if check_items[i]._do_check == CHECK_TRUE:
                if check_items[i]._check_flag == list(file_info.values())[i]:
                    check_items[i]._passed_list.append(file_info['filename'])
                else:
                    check_items[i]._failed_list.append(file_info['filename'])
            else:
                check_items[i]._passed_list.append(file_info['filename'])

    def _generate_failed_files_data(self, check_items: List[CheckItemData], summary_data: 'HtmlTitle', 
                                    file_name_list: List[str], failed_file_list: List[Dict]):
        """
        Generate data for files that failed security checks.

        Args:
            check_items (list): List of CheckItemData objects representing check configurations.
            summary_data (HtmlTitle): Summary data object to store aggregate information.
            file_name_list (list): List of all checked file names.
            failed_file_list (list): List to store information about failed files.
        """
        for file_name in file_name_list:
            if not is_executable_or_library(file_name):            #if the given file isn't an executable or library file, filter out
                continue
            file_info = {}
            file_info['file_name'] = file_name
            for check_item in check_items:
                if file_name in check_item._failed_list:
                    if check_item._name == 'fortify-able':
                        file_info['fortifiable'] = CHECK_FAILED
                    else:
                        file_info[check_item._name] = CHECK_FAILED
                else:
                    if check_item._name == 'fortify-able':
                        file_info['fortifiable'] = CHECK_PASS
                    else:
                        file_info[check_item._name] = CHECK_PASS
            if CHECK_FAILED in file_info.values():
                failed_file_list.append(file_info)
        summary_data._check_file_list_num = len(failed_file_list)


def generate_failed_list(failed_file_list: List[Dict]):
    """
    Generates a text file named 'failed_list.txt' containing details of files that failed checks.

    Args:
        failed_file_list (list[dict]): A list of dictionaries containing failed file information.
    """
    if os.path.isfile('failed_list.txt'):
        os.system('rm failed_list.txt')
    if len(failed_file_list) != 0:
        with open('failed_list.txt', 'w') as txt_file:
            for failed_file_info in failed_file_list:
                txt_file.write('{}\n'.format(failed_file_info))

def get_file_size(file_size: int) -> str:
    """
    Format file size into human-readable format.

    Args:
        file_size (int): File size in bytes.

    Returns:
        str: File size as a human-readable string.
    """
    if file_size < 1024:
        return str(round(file_size, 2)) + 'B'
    else:
        file_size /= 1024
        if file_size < 1024:
            return str(round(file_size, 2)) + 'K'
        else:
            file_size /= 1024
            if file_size < 1024:
                return str(round(file_size, 2)) + 'M'
            else:
                file_size /= 1024
                return str(round(file_size, 2)) + 'G'

def update_attributes(args: argparse.Namespace, check_items: List[CheckItemData]):
    """
    Updates the attributes of check items based on arguments.

    Args:
        args (argparse.Namespace): Parsed arguments.
        check_items (list[CheckItemData]): List of security check items to update.
    """
    if args.relro:
        check_items[0]._do_check = CHECK_TRUE
        check_items[0]._check_flag = args.relro
    if args.canary:
        check_items[1]._do_check = CHECK_TRUE
        check_items[1]._check_flag = args.canary
    if args.nx:
        check_items[2]._do_check = CHECK_TRUE
        check_items[2]._check_flag = args.nx
    if args.pie:
        check_items[3]._do_check = CHECK_TRUE
        check_items[3]._check_flag = args.pie
    if args.Clang_Cfi:
        check_items[4]._do_check = CHECK_TRUE
        check_items[4]._check_flag = args.Clang_Cfi
    if args.safestack:
        check_items[5]._do_check = CHECK_TRUE
        check_items[5]._check_flag = args.safestack
    if args.rpath:
        check_items[6]._do_check = CHECK_TRUE
        check_items[6]._check_flag = args.rpath
    if args.symbols:
        check_items[7]._do_check = CHECK_TRUE
        check_items[7]._check_flag = args.symbols
    if args.fortity:
        check_items[8]._do_check = CHECK_TRUE
        check_items[8]._check_flag = args.fortity
    if args.fortified:
        check_items[9]._do_check = CHECK_TRUE
        check_items[9]._check_flag = args.fortified
    if args.fortifiable:
        check_items[10]._do_check = CHECK_TRUE
        check_items[10]._check_flag = args.fortifiable

def add_parse() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        argparse.Namespace: The parsed command-line arguments.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--check_dir',
        required = True,
        type = str,
        help='check_dir')

    parser.add_argument(
        '--output_path',
        required = True,
        type = str,
        help='output path to save the report')

    parser.add_argument(
        '--relro',
        type = str,
        help = 'set relro expected value and compare actual value with expected value')

    parser.add_argument(
        '--nx',
        type = str,
        help = 'set nx expected value and compare actual value with expected value')

    parser.add_argument(
        '--rpath',
        type = str,
        help = 'set rpath expected value and compare actual value with expected value')

    parser.add_argument(
        '--canary',
        type = str,
        help = 'set canary expected value and compare actual value with expected value')

    parser.add_argument(
        '--pie',
        type = str,
        help = 'set pie expected value and compare actual value with expected value')

    parser.add_argument(
        '--symbols',
        type = str,
        help = 'set symbols expected value and compare actual value with expected value')

    parser.add_argument(
        '--fortity',
        type = str,
        help = 'set fortity expected value and compare actual value with expected value')

    parser.add_argument(
        '--safestack',
        type = str,
        help = 'set safestack expected value and compare actual value with expected value')

    parser.add_argument(
        '--Clang_Cfi',
        type = str,
        help = 'set Clang-Cfi expected value and compare actual value with expected value')

    parser.add_argument(
        '--fortified',
        type = str,
        help = 'set fortified expected value and compare actual value with expected value')

    parser.add_argument(
        '--fortifiable',
        type = str,
        help = 'set fortify-able expected value and compare actual value with expected value')
    return parser.parse_args()

def main():
    """
    Main function to execute the security check process and generate reports.
    """
    start_time = time.time()
    summary_data = HtmlTitle()
    summary_data._start_time = datetime.datetime.now().strftime('%Y/%m/%d %H:%M:%S')
    check_file_path = 'check_file_path.json'
    args=add_parse()
    output_path = args.output_path

    check_dir = args.check_dir
    if check_dir[-1] == '/':
        check_dir = check_dir[:-1]

    check_data = CheckData()
    check_items = [check_data._relro_check, check_data._canary_check, check_data._nx_check, check_data._pie_check,
                   check_data._clangcfi_check, check_data._clang_safestack_check, check_data._rpath_check, check_data._symbols_check,
                   check_data._fortify_check, check_data._fortified_check, check_data._fortifiable_check]
    update_attributes(args, check_items)
    file_name_list = []
    failed_file_list = []
    total_file_info = []
    check_data._check_security(check_file_path, check_dir, check_items, summary_data, file_name_list, total_file_info)
    check_data._generate_failed_files_data(check_items, summary_data, file_name_list, failed_file_list)

    summary_data._end_time = datetime.datetime.now().strftime('%Y/%m/%d %H:%M:%S')
    end_time = time.time()
    run_time = round(end_time - start_time, 2)
    summary_data._execution_time = str(run_time) + 's'
    if output_path[-1] != '/':
        output_path = os.path.join(output_path, '')
    report = GenerateReport()
    report._generate_summary_html_report(output_path, check_items, summary_data, total_file_info)
    report._generate_total_files_html_report(output_path, total_file_info)
    report._generate_failed_files_html_report(output_path, failed_file_list)

    report._generate_summary_json_report(output_path, check_items, summary_data, total_file_info)
    report._generate_total_files_json_report(output_path, total_file_info)
    report._generate_failed_files_json_report(output_path, failed_file_list)

    generate_failed_list(failed_file_list)

if __name__ == '__main__':
    main()
