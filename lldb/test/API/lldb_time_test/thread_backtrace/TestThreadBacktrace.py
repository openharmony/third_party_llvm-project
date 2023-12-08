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


class TestBacktrace(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)

        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")

        # Find the line numbers within thread_backtrace.cpp for thread_backtrace.
        self.backtrace_breakpoint = line_number(
            "thread_backtrace.cpp", "// breakpoint for thread backtrace."
        )

        # We'll use the test method name as the exe_name for executable compiled from thread_backtrace.cpp.
        self.exe_name = self._testMethodName

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_thread_backtrace(self):
        """Test 'thread backtrace'."""
        # We build a different executable than the default build() does.
        d = {"CXX_SOURCES": "thread_backtrace.cpp", "EXE": self.exe_name}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        self.thread_backtrace()

    def thread_backtrace(self):
        # Create a target by the debugger.
        exe = self.getBuildArtifact(self.exe_name)
        target = self.dbg.CreateTarget(exe)
        self.lldb_assert(target, "target is not valid")

        breakpoint = target.BreakpointCreateByLocation(
            "thread_backtrace.cpp", self.backtrace_breakpoint
        )
        self.lldb_assert(breakpoint, "breakpoint is not valid")

        # Launch the process, and do stop at breakpoint.
        process = target.LaunchSimple(
            None, None, lldb.remote_platform.GetWorkingDirectory()
        )
        self.lldb_assert(process and process.IsValid(), "process is not valid")
        self.lldb_assert(
            process.GetState() == lldb.eStateStopped, "Process state is not stopped"
        )
        thread = lldbutil.get_stopped_thread(process, lldb.eStopReasonBreakpoint)
        self.lldb_assert(
            thread.IsValid(),
            "There should be a thread stopped due to breakpoint condition",
        )

        # Verify that we are stopped at the correct source line number in thread_backtrace.cpp.
        frame0 = thread.GetFrameAtIndex(0)
        lineEntry = frame0.GetLineEntry()
        self.lldb_assert(
            lineEntry.GetLine() == self.backtrace_breakpoint,
            "Thread didn't stop at the correct source line number where we expect.",
        )

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Thread backtrace
        self.handle_cmd("thread backtrace")

        self.lldb_assert_total_time(
            "HandleCommand: thread backtrace", "Completed backtrace."
        )
