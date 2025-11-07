"""
Copyright (C) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import unittest2
import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class MixedArkTSDebuggerTest(TestBase):

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line number for our breakpoint.
        self.breakpoint = line_number('main.c', '// break on this line')

    def test(self):
        """Test getting js backtrace."""
        self.build()

        exe = self.getBuildArtifact("a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        # This should create a breakpoint in the main thread.
        lldbutil.run_break_set_by_file_and_line(
            self, "main.c", self.breakpoint, num_expected_locations=1)
        
        # The breakpoint list should show 1 location.
        self.expect(
            "breakpoint list -f",
            "Breakpoint location shown correctly",
            substrs=[
                "1: file = 'main.c', line = %d, exact_match = 0, locations = 1" %
                self.breakpoint])
        
        # Run the program.
        self.runCmd("run", RUN_SUCCEEDED)

        # The stop reason of the thread should be breakpoint.
        self.expect("thread list", STOPPED_DUE_TO_BREAKPOINT,
                    substrs=['stopped',
                             'stop reason = breakpoint'])
        
        # Get the target and process
        target = self.dbg.GetSelectedTarget()
        process = target.GetProcess()

        # Get arkts backtrace
        arkdb = lldb.SBMixedArkTSDebugger(target)
        er = lldb.SBError()
        bt = arkdb.GetBackTrace(er)
        message = ""
        var = arkdb.OperateDebugMessage(message, er)

        # Check the result
        self.assertTrue(er.Success(), "ArkTS debugger get backtrace failed.")
        er.Clear()
        self.assertTrue(bt.GetString(er, 0) == 'This is a ArkTS backtrace',
                        'ArkTS debugger get wrong backtrace.')

        # Check the arkTs variable result
        self.assertTrue(er.Success(), "ArkTS debugger get operate debug message result failed.")
        er.Clear()
        self.assertTrue(var.GetString(er, 0) == 'This is a ArkTS operate debug message result',
                        'ArkTS debugger get wrong operate debug message result.')

        # Run to completion
        self.runCmd("continue")

        # If the process hasn't exited, collect some information
        if process.GetState() != lldb.eStateExited:
            self.runCmd("thread list")
            self.runCmd("process status")

        # At this point, the inferior process should have exited.
        self.assertEqual(
            process.GetState(), lldb.eStateExited,
            PROCESS_EXITED)
