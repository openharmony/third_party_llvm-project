#! /bin/bash

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

echo "Params: $*"

# 初始化变量
BRANCH='master'
ENABLE_INIT=0
ENABLE_SYNC=0
ENABLE_BUILD=0
PR_NUMBER=''
BUILD_TAG=''
BUILD_URL=''
START_TIME=$(date +%s)

# 逐个处理参数
for arg in "$@"
do
    case $arg in
    --branch=*)
        BRANCH="${arg#*=}"
        ENABLE_INIT=1
        shift # 移除已处理的参数
        ;;
    --build=*)
        ENABLE_BUILD="${arg#*=}"
        shift
        ;;
    --sync=*)
        ENABLE_SYNC="${arg#*=}"
        shift
	;;
    --buildTag=*)
        BUILD_TAG="${arg#*=}"
        shift
	;;
    --buildURL=*)
        BUILD_URL="${arg#*=}"
        shift
	;;
    --pr_number=*)
        PR_NUMBER="${arg#*=}"
        shift
        ;;
        *)
    esac
done

if [ "$ENABLE_INIT" = 1 ]; then
    echo "llvm-project switch to branch: $BRANCH"
    cd ~/llvm/toolchain/llvm-project
    git fetch
    git reset --hard
    git switch -c $BRANCH
    git checkout $BRANCH
    git pull origin $BRANCH
else
    echo "current branch $BRANCH"
fi

if [ "$ENABLE_SYNC" = 1 ]; then
    cd ~/llvm
    echo "repo sync..."
	repo sync -c
else
    echo "skip repo sync..."
fi

if [ -n "$PR_NUMBER" ]; then
    echo "pr编号为 $PR_NUMBER..."
    cd ~/llvm/toolchain/llvm-project
    git fetch https://gitee.com/openharmony/third_party_llvm-project.git pull/$PR_NUMBER/head:pr_$PR_NUMBER
    git checkout pr_$PR_NUMBER
else
    echo "无--pr_number参数"
fi

if [ "$ENABLE_BUILD" = 1 ]; then
    echo "build start..."
    cd ~/llvm
    python3 ./toolchain/llvm-project/llvm-build/build.py --no-lto --enable-assertions --no-build-x86_64 --no-build-mipsel --no-build-arm --no-build-riscv64 --no-build windows,lldb-server,check-api
else
    echo "skip build..."
fi

echo "running compiler-rt sanitizer tests..."
TESTS=("cfi" "asan" "ubsan" "hwasan" "tsan" "gwp_asan")
FAILED_TESTS=()
EXIT_CODE=0
SUMMARY=()

cd ~/llvm/test-suites/
rm -rf ~/llvm/test-suites/build
mkdir -p ~/llvm/test-suites/build

# set lit timeout to 120s
sed -i 's/timeout=[0-9]\+/timeout=120/g' ~/llvm/toolchain/llvm-project/compiler-rt/test/sanitizer_common/ohos_family_commands/ohos_common.py

# run TESTS
for TEST in "${TESTS[@]}"; do
    TEST_RESULT="$TEST.result"
    touch $TEST_RESULT
    > $TEST_RESULT

    TEST_LOG="$TEST.log"
    touch $TEST_LOG
    > $TEST_LOG

    ~/llvm/test-suites/run_test.sh $TEST 0 |& tee $TEST_LOG

    if grep -q "Failed Tests"  $TEST_LOG; then
        echo "sanitizer $TEST has failed tests."
        grep -A 500 "Failed Tests" $TEST_LOG >> $TEST_RESULT
        FAILED_TESTS+=("$TEST")
    elif grep -q "Unexpectedly" $TEST_LOG; then
        echo "sanitizer $TEST has unexpected behavior."
        grep -A 500 "Unexpectedly" $TEST_LOG >> $TEST_RESULT
        FAILED_TESTS+=("$TEST")
    else        
        echo "All $TEST tests passed."
        grep -A 20 "Testing Time" $TEST_LOG >> $TEST_RESULT
    fi
done

# gen test summary
if [ ${#FAILED_TESTS[@]} -ne 0 ]; then
    SUMMARY+="The following tests failed: "
    for FAILED_TEST in "${FAILED_TESTS[@]}"; do
        SUMMARY+=" [$FAILED_TEST] "
    done
    EXIT_CODE=1
else
    SUMMARY+="All sanitizer tests passed."
fi

END_TIME=$(date +%s)  # 记录结束时间，以秒为单位  
RUNTIME=$((END_TIME - START_TIME))  # 计算时间差，以秒为单位 

HOURS=$((RUNTIME / 3600))  
MINUTES=$(((RUNTIME % 3600) / 60))  
SECONDS=$((RUNTIME % 60))  
  
# 格式化时间字符串，注意处理小时数为0的情况  
if [ $HOURS -gt 0 ]; then  
    TIME_STR="${HOURS}h ${MINUTES}m ${SECONDS}s"  
else  
    TIME_STR="${MINUTES}m ${SECONDS}s"  
fi  

RESULT="${SUMMARY} (Cost ${TIME_STR}). see $BUILD_URL for detail."  
SUBJECT="[LLVM-sanitizers][$BUILD_TAG]"  
echo "$SUBJECT $RESULT"

# send email
python3 ~/llvm/test-suites/send_email.py "${SUBJECT}" "${RESULT}" "${TESTS[@]}"

exit $EXIT_CODE
