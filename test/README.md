# Test Scripts

This folder hosts a collection of example scripts designed for testing various components of the LLVM project. These scripts facilitate the building, testing, and reporting processes for `llvm-ggw`, `sanitizer-ggw`, and local Jenkins setups.

## Directory Structure

*   **`llvm-ggw/`**: Contains scripts and configuration files for triggering and running tests related to the `llvm-ggw` component.
    *   `job_runner.Jenkinsfile`: Jenkins pipeline script that triggers the `llvm-ggw` test suite.
    *   `job_wrapper.groovy`: Groovy script used for configuring Jenkins jobs specifically for `llvm-ggw` testing.

*   **`sanitizer-ggw/`**: Holds scripts responsible for building and testing the `sanitizer-ggw` project.
    *   `build_project.sh`: Shell script to build the `sanitizer-ggw` project.
    *   `job_runner.Jenkinsfile`: Jenkins pipeline script that triggers the `sanitizer-ggw` test suite.
    *   `job_wrapper.groovy`: Groovy script used for configuring Jenkins jobs specifically for `sanitizer-ggw` testing.
    *   `test_project.sh`: Shell script to execute tests on the `sanitizer-ggw` project.

*   **`local-jenkins/`**: Houses scripts used in a local Jenkins environment.
    *   `ci_daily.sh`: Shell script for daily continuous integration tasks.
    *   `run_test.sh`: Shell script to execute the test suite.
    *   `send_email.py`: Python script to send email notifications (presumably for test results).

## Test suites

*   **`llvm-ggw`**: Test suite for llvm regression tests、unit tests、ace super test、llvm benchmark、llvmts.
    *   `unit tests`: Unit tests are written using Google Test and Google Mock and are located in the llvm/unittests directory.
    *   `regression tests`: The regression tests are small pieces of code that test a specific feature of LLVM or trigger a specific bug in LLVM. These tests are located in the llvm/test directory.
    *   `ace super test`: Comprehensive test and validation suite for compilers and library support for the programming languages C and C++.
    *   `llvm benchmark`: The test-suite contains benchmark and test programs. It comes with tools to collect metrics such as benchmark runtime, compilation time and code size.
    *   `llvmts`: The suite comes with tools to collect metrics such as benchmark runtime, compilation time and code size, see:https://llvm.org/docs/TestSuiteGuide.html.

*   **`sanitizer-ggw`**: Test suite for compiler-rt, currently we support asan、 cfi、 gwp_asan、hwasan、tsan、ubsan, These tests are located in the llvm/compiler-rt/test directory.

*   **`local-jenkins`**: Local copy of sanitizer daily tests.

The 'llvm-ggw' and 'sanitizer-ggw' test suite are used for blue-zone gitee verification, and 'local-jenkins' is used for yellow-zone development.

## Usage

These scripts are intended to be used as examples and starting points. You should modify them according to your specific project requirements and testing environment. Refer to the individual script files for more detailed usage instructions.

## Prerequisites

Before using these scripts, ensure you have the following prerequisites in place:

*   Jenkins
*   Python
*   Necessary build tools and dependencies for LLVM and related projects.

## Contributing

Contributions are welcome! If you find any issues or have improvements to the scripts, please feel free to submit a pull request. Make sure to follow the existing coding style and include appropriate test cases where applicable.
