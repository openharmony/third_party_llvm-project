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

import argparse
import subprocess
import sys
import logging


class AbiCheck:

    def __init__(self, args) -> None:
        self.args = argparse.Namespace(
            action=None,
            suppressions=None,
            no_show_locs=None,
            headers_dir=None,
            debug_info_dir=None,
            load_all_types=None,
            annotate=None,
            abi_file=None,
            elf_file=None,
            debug_info_dir1=None,
            debug_info_dir2=None,
            headers_dir1=None,
            headers_dir2=None,
            show_size_offset=None,
            compare_files=None 
        )
        source_dict = vars(args)
        self.args.__dict__.update(source_dict)
        logging.basicConfig(level=logging.INFO)
        return

    @classmethod
    def exec_command(cls, command, timeout=10):
        with subprocess.Popen(command, stderr=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True,
                              start_new_session=True) as process:
            output, errs = process.communicate(timeout=timeout)
            out_str = output.decode()
            errs_str = errs.decode()
            if out_str != '':
                logging.info(out_str)
            if errs_str != '':
                logging.error(errs_str)

    @classmethod
    def get_show_size_offset(cls, argument):
        if argument == "bytes":
            return "bytes"
        elif argument == "bits":
            return "bits"
        elif argument == "hex" or argument == "hexadecimal":
            return "hex"
        elif argument == "dec" or argument == "decimal":
            return "dec"
        else:
            return "bytes"

    def run(self) -> None:
        if self.args.action == "gen_abi":
            self.gen_abi_file()
        elif self.args.action == "compare_abi":
            self.compare_abi_files()
        else:
            logging.info("do nothing")

    def build_gen_command(self):
        all_args = ["--no-corpus-path"]
        if self.args.suppressions is not None:
            all_args.extend(["--suppressions", self.args.suppressions])
        if self.args.debug_info_dir is not None:
            all_args.extend(["--debug-info-dir", self.args.debug_info_dir])
        if self.args.headers_dir is not None:
            all_args.extend(["--headers-dir", self.args.headers_dir])
        if self.args.no_show_locs is not None and self.args.no_show_locs is not False:
            all_args.extend(["--no-show-locs"])
        if self.args.load_all_types is not None and self.args.load_all_types is not False:
            all_args.extend(["--load-all-types"])
        if self.args.annotate is not None and self.args.annotate is not False:
            all_args.extend(["--annotate"])
        all_args.extend(["--out-file", self.args.abi_file])
        all_args.extend([self.args.elf_file])
        command = ["abidw"] + all_args
        return command

    def gen_abi_file(self):
        if self.args.elf_file is None:
            logging.error("elf-file is None, please check your file")
            return 
        if self.args.abi_file is None:
            logging.error("abi-file path is None, please check your config")
            return
        command = self.build_gen_command()
        self.exec_command(command)

    def build_compare_command(self):
        all_args = ["--no-corpus-path",
                    "--deleted-fns",
                    "--drop-private-types",
                    "--changed-fns",
                    "--added-fns",
                    "--deleted-vars", 
                    "--changed-vars",
                    "--added-vars",
                    "--harmless",
                    "--no-redundant"]
        if self.args.suppressions is not None:
            all_args.extend(["--suppressions", self.args.suppressions])
        if self.args.debug_info_dir1 is not None:
            all_args.extend(["--debug-info-dir1", self.args.debug_info_dir1])
        if self.args.debug_info_dir2 is not None:
            all_args.extend(["--debug-info-dir2", self.args.debug_info_dir2])
        if self.args.headers_dir1 is not None:
            all_args.extend(["--headers-dir1", self.args.headers_dir1])
        if self.args.headers_dir2 is not None:
            all_args.extend(["--headers-dir2", self.args.headers_dir2])
        if self.args.show_size_offset is not None:
            size_offset = self.get_show_size_offset(self.args.show_size_offset)
            all_args.extend([f"--show-{size_offset}"])
        all_args.extend(self.args.compare_files)
        command = ["abidiff"] + all_args
        return command

    def compare_abi_files(self, timeout=10):
        if self.args.compare_files is None:
            logging.error("elf_file is None, please check your file")
            return False
        command = self.build_compare_command()
        cmd_string = " ".join(command)
        with subprocess.Popen(command, stderr=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True,
                              start_new_session=True) as process:
            output, errs = process.communicate(timeout=timeout)
            out_str = output.decode()
            if out_str is not None and out_str != "":
                logging.warning("cmd_string is : %s", cmd_string)
                logging.warning(out_str)
                return True
            else:
                return False


def parse_abi_check_args():
    parser = argparse.ArgumentParser(description='Generate ABI files and compare ABI files')
    parser.add_argument('--action', help='gen_abi or compare_abi')
    parser.add_argument('--suppressions', help='<path> specify a suppression file')
    parser.add_argument('--no-show-locs', action='store_true', default=False, help='do not show location information')
    gen_abi = parser.add_argument_group("gen_abi_file")
    gen_abi.add_argument(
        '--elf-file', 
        help='elf-file to gen abi-file')
    gen_abi.add_argument(
        '--abi-file',
        help='<file-path> write the output to file-path')
    gen_abi.add_argument(
        '--debug-info-dir',
        help='<dir-path> look for debug info under dir-path')
    gen_abi.add_argument(
        '--headers-dir',
        help='<path> the path to headers of the elf file')
    gen_abi.add_argument(
        '--load-all-types',
        action='store_true',
        default=False,
        help='read all types including those not reachable from exported declarations')
    gen_abi.add_argument(
        '--annotate',
        action='store_true',
        default=False, 
        help='annotate the ABI artifacts emitted in the output')
    compare_abi = parser.add_argument_group("compare_abi")
    compare_abi.add_argument(
        '--compare-files',
        nargs=2,
        metavar='FILE', help='compare two abi files')
    compare_abi.add_argument(
        '--debug-info-dir1',
        help=' <path> the root for the debug info of file1')
    compare_abi.add_argument(
        '--debug-info-dir2',
        help=' <path> the root for the debug info of file1')
    compare_abi.add_argument(
        '--headers-dir1',
        help='<path>  the path to headers of file1')
    compare_abi.add_argument(
        '--headers-dir2',
        help='<path>  the path to headers of file2')
    compare_abi.add_argument(
        '--show-size-offset',
        help='show size offsets in bytes/bits/hexadecimal/decimal')
    return parser.parse_args()


def main(args):
    abi_check = AbiCheck(args)
    abi_check.run()


if __name__ == '__main__':
    sys.exit(main(parse_abi_check_args()))
