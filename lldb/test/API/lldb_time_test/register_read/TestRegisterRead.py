# Copyright (C) 2023 Huawei Device Co., Ltd.
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

import lldb
from lldb_time_test.TimeTestBase import TimeTestBase
from lldbsuite.test.lldbtest import line_number
from lldbsuite.test.decorators import skipOnOpenHarmonyCI, skipIf


class TestRegisterRead(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)

        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")

        # Find the line number to break inside main().
        self.line = line_number("register_read.cpp", "// Set break point at this line.")

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_register_read(self):
        # Create a target by the debugger.
        exe = self.getBuildArtifact(self._testMethodName)
        d = {"EXE": exe}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        target = self.dbg.CreateTarget(exe)
        self.lldb_assert(target, "target is not valid")

        breakpoint = target.BreakpointCreateByLocation("register_read.cpp", self.line)
        self.lldb_assert(breakpoint, "breakpoint is not valid")

        # Now launch the process, and do not stop at entry point.
        process = target.LaunchSimple(
            None, None, lldb.remote_platform.GetWorkingDirectory()
        )
        self.lldb_assert(process and process.IsValid(), "process is not valid")

        for thread in process:
            if thread.GetStopReason() == lldb.eStopReasonBreakpoint:
                for frame in thread:
                    # Log timers reset
                    self.handle_cmd("log timers reset")
                    # Register read
                    registers = self.handle_cmd("register read")

                    reg_list = registers.splitlines()
                    self.lldb_assert(
                        reg_list[0] == "General Purpose Registers:",
                        "Command(register read) execution output didn't meet expectation",
                    )

                    # We've finished dumping the registers for frame #0.
                    break

        self.lldb_assert_total_time(
            "HandleCommand: register read", "Completed register read."
        )
