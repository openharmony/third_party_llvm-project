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
from lldbsuite.test import lldbutil
from lldbsuite.test.lldbtest import line_number
from lldbsuite.test.decorators import skipOnOpenHarmonyCI, skipIf


class TestStepOut(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)

        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")

        # Find the line numbers within step.cpp for step_out().
        self.step_in_c = line_number(
            "step.cpp", "// breakpoint for thread step-in function c."
        )
        # We'll use the test method name as the exe_name for executable compiled from step.cpp.
        self.exe_name = self._testMethodName

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_step_out_of_function_c(self):
        """Test 'thread step-out' to step out of function c()."""
        # We build a different executable than the default build() does.
        d = {"CXX_SOURCES": "step.cpp", "EXE": self.exe_name}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        self.step_out_of_function_c()

    def step_out_of_function_c(self):
        # Create a target by the debugger.
        exe = self.getBuildArtifact(self.exe_name)
        target = self.dbg.CreateTarget(exe)
        self.lldb_assert(target, "target is not valid")

        breakpoint = target.BreakpointCreateByName("c")
        self.lldb_assert(breakpoint, "breakpoint is not valid")

        # Launch the process, and do not stop at the entry point.
        process = target.LaunchSimple(
            None, None, lldb.remote_platform.GetWorkingDirectory()
        )

        thread = lldbutil.get_stopped_thread(process, lldb.eStopReasonBreakpoint)
        self.lldb_assert(
            thread.IsValid(), "There should be a thread stopped due to breakpoint"
        )
        target.BreakpointDelete(breakpoint.GetID())

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Thread step-out
        self.handle_cmd("thread step-out")

        self.lldb_assert(
            thread.GetFrameAtIndex(0).GetLineEntry().GetLine() == self.step_in_c,
            "step out function c() is successful",
        )

        self.lldb_assert_total_time(
            "HandleCommand: thread step-out", "Completed step out."
        )
