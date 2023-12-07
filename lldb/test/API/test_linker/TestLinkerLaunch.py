#  Copyright (c) 2023 Huawei Device Co., Ltd.
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

"""
Test lldb hit the breakpoint in the linker or musl. This test case only runs
when the local ubuntu is connected to the phone or other device and there is
a remote-ohos connection, will skip on CI or remote = false.
"""

import subprocess
import sys

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class LaunchLinker(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Get the full path to our executable to be debugged.
        self.exe = "%s_%d" % (self.getBuildArtifact(self.testMethodName), os.getpid())
        d = {"EXE": self.exe}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)

    def tearDown(self):
        # Call super's tearDown().
        TestBase.tearDown(self)

    @skipIf(remote=False)
    @skipUnlessPlatform(["ohos"])
    def test_linker_with_breakpoint(self):
        """Create target, breakpoint, launch a process, and then kill it."""
        target = self.dbg.CreateTarget(self.exe)

        breakpoint = target.BreakpointCreateByName("__dls2b")
        # The default state after breakpoint creation should be enabled.
        self.assertTrue(
            breakpoint.IsEnabled(), "Breakpoint should be enabled after creation"
        )
        process = target.LaunchSimple(None, None, self.get_process_working_directory())
        self.assertTrue(process, PROCESS_IS_VALID)

        thread = lldbutil.get_stopped_thread(process, lldb.eStopReasonBreakpoint)
        self.assertIsNotNone(thread)

        # The breakpoint should have a hit count of 1.
        self.assertEqual(breakpoint.GetHitCount(), 1, BREAKPOINT_HIT_ONCE)

        # Destroy process before TestBase.tearDown()
        self.dbg.GetSelectedTarget().GetProcess().Destroy()

    @skipIf(remote=False)
    @skipUnlessPlatform(["ohos"])
    def test_musl_with_breakpoint(self):
        """Create target, breakpoint, launch a process, and then kill it."""
        target = self.dbg.CreateTarget(self.exe)

        breakpoint = target.BreakpointCreateByName("printf")
        # The default state after breakpoint creation should be enabled.
        self.assertTrue(
            breakpoint.IsEnabled(), "Breakpoint should be enabled after creation"
        )
        process = target.LaunchSimple(None, None, self.get_process_working_directory())
        self.assertTrue(process, PROCESS_IS_VALID)

        thread = lldbutil.get_stopped_thread(process, lldb.eStopReasonBreakpoint)
        self.assertIsNotNone(thread)

        # The breakpoint should have a hit count of 1.
        self.assertEqual(breakpoint.GetHitCount(), 1, BREAKPOINT_HIT_ONCE)

        # Destroy process before TestBase.tearDown()
        self.dbg.GetSelectedTarget().GetProcess().Destroy()
