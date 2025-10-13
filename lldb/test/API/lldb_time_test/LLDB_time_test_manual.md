# LLDB TIME TEST MANUAL

## About LLDB time test

### Purpose

Automated acquisition of time-consuming LLDB debugging data, including use cases related to attach and single-step scenarios.

### About test case

This test is not aimed at functionality but rather performance, utilizing some test cases referenced from `lldb/test/API/python_api`. The sole criterion for determining the success of each use case is the total time. If the time consumption data of other modules exceeds the acceptable fluctuation range, it will only save in report files.

### About benchmark file

The benchmark data can be customized according to specific conditions and is stored in the `benchmark.json` file within each test module's folder.

```
{
    "test_attach_to_process_with_id_api":{    // Case name
        "TotalTime": {                        // Monitored module or tag
            "benchmark": t,                   // Baseline data, in seconds
            "range": [x, y]                   // Anomalies will be identified if the error range falls below the baseline
                                              // multiplied by 'x' or exceeds the baseline multiplied by 'y'.
        }
    }
}
```

A template file for benchmark data is provided. The data in this file is associated with the HOST, devices, and network environment, so when deploying it on CI, it may be necessary to modify these data and the permissible fluctuation range.

## How to run the test

### 1. Build LLVM project

##### Build：

```
# build
$ ./prebuilts/python3/linux-86/3.10.2/bin/python3 ./toolchain/llvm-project/llvm-build/build.py --enable-monitoring --build-python
```

Reference: [LLVM Build Guide](https://gitcode.com/openharmony/third_party_llvm-project/blob/master/llvm-build/README.md)

### 2. HDC Tool

Download the SDK package to obtain HDC tool from [Daily Builds](http://ci.openharmony.cn/dailybuilds),  path in SDK package: `**/ohos-sdk/linux/toolchains`, you can copy HDC tool to /usr/bin directory through a symbolic link.

HDC Usage Reference: [Linux USB Device Permission](https://gitcode.com/openharmony/developtools_hdc/blob/master/README_zh.md#linux%E7%AB%AFusb%E8%AE%BE%E5%A4%87%E6%9D%83%E9%99%90%E8%AF%B4%E6%98%8E)

### 3. Hardware Platform

Host: Linux x86_64 Ubuntu20.04

Target: RK3568 AARCH64/ARM

### 4. Configure and Run Test Suite

##### Options: 

-llvm-root: the absolute path to the LLVM Project;
-hdc-path: the absolute path to the HDC tool;
-use-module-cache: True or False, determines whether run tests with module cache.

##### Run the script:
For example: 

```
./run_lldb_time_test.sh -llvm-root {LLVM_PROJECT} -hdc-path {HDC_PATH} -use-module-cache False
```

##### Report:

After the test script has been executed, reports are generated in the  report folder located in the same directory as each test case.

For example: `lldb_time_test/attach/report/test_attach_to_process_with_name_api.csv`
