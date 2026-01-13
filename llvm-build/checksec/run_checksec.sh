#!/bin/bash
# Copyright (C) 2024 Huawei Device Co., Ltd.
#
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
WORK_DIR=$(dirname "$(realpath "$0")")
check_dir="" 
output_path=""
while [[ $# -gt 0 ]]; do
    case $1 in
        # set the check directory
        -c|--check_dir)
             check_dir="$(realpath "$2")"
            shift 2
            ;;
        # set the report output directory
        -o|--output_path)
             output_path="$(realpath "$2")"
            shift 2
            ;;
        # print the usage of 'run_checksec.sh'
        -h|--help)
            echo "Usage: ./run_checksec.sh [-s][-c check_dir] [-o ./llvmts-log/checksec] [OPTION]"
            echo
            echo "Options are:"
            echo "  -c/--check_dir <check_dir>       check_dir: Path to the binary file that you want to check using checksec."  
            echo "  -o/--output_path  <output_path>  output_path: Path to store the results of the binary security compilation option checks."
            echo "  -h/--help                        print usage of run_checksec.sh"
            exit 0
            ;;
        # other unsupport option
        *)
            echo "Illegal option $1"
            exit 1
            ;;
    esac
done

# If --check_dir/-c is not provided, use the default value
default_check_dir="$(realpath "${WORK_DIR}/../../../../out/llvm-install")"   # default check path
if [ -z "$check_dir" ]; then
    check_dir="$default_check_dir"
fi

TIME=$(date +%Y%m%d%H%M)
time_dir=$(date +%Y-%m-%d-%H-%M)
# If --output_path/-o is not provided, use the default value
default_output_path="${WORK_DIR}/llvmsec-log"   
if [ -z "$output_path" ]; then
    output_path="$default_output_path"
fi
llvmts_logdir="${output_path}/${time_dir}/"

echo "The check path is  ${check_dir}"
echo "The check result report is stored in   ${llvmts_logdir}"
echo "run checksec test"
# run check.py to run ./checksec/checksec                          
cd ${WORK_DIR}
if [ ! -d "checksec" ]; then
    echo "Need to look up README to use the script."
    exit 1
fi
mkdir -p  ${llvmts_logdir}
python3 ${WORK_DIR}/check.py --check_dir ${check_dir} --output_path ${llvmts_logdir} 2>&1 | tee ${WORK_DIR}/check.txt
cp ${WORK_DIR}/templates/bootstrap.bundle.min.js ${llvmts_logdir}
cp ${WORK_DIR}/templates/jquery.min.js ${llvmts_logdir}
cp -r ${WORK_DIR}/templates/css ${llvmts_logdir}
cp -r ${WORK_DIR}/templates/fonts ${llvmts_logdir}
if (grep -q "check error" ${WORK_DIR}/check.txt); then
    exit 1
fi
if [ -f ${WORK_DIR}/failed_list.txt ];then
    echo exist failed files
    exit 1
fi
