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
from lldbsuite.test.decorators import skipOnOpenHarmonyCI, skipIf
from lldb_time_test.TimeTestBase import TimeTestBase
from lldbsuite.test import lldbutil


class TestAttachAPI(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)

        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_attach_to_process_with_id_api(self):
        """Create target, spawn a process, and attach to it with process id."""
        # Build
        exe = self._testMethodName
        d = {"EXE": exe}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        target = self.dbg.CreateTarget(self.getBuildArtifact(exe))

        # Spawn a new process
        token = exe + ".token"
        popen = self.spawnSubprocess(self.getBuildArtifact(exe), [token])
        lldbutil.wait_for_file_on_target(self, token)

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Attach to process with process id
        self.dbg.SetSelectedPlatform(lldb.selected_platform)
        listener = lldb.SBListener("my.attach.listener")
        error = lldb.SBError()
        process = target.AttachToProcessWithID(listener, popen.pid, error)
        self.lldb_assert(error.Success() and process, "process is valid")

        # Destroy process
        lldb.remote_platform.Kill(popen.pid)

        self.lldb_assert_total_time("Attach Start", "CompleteAttach end")

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_attach_to_process_with_name_api(self):
        """Create target, spawn a process, and attach to it with process name."""
        # Build
        exe = self._testMethodName
        d = {"EXE": exe}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        target = self.dbg.CreateTarget(self.getBuildArtifact(exe))

        # Spawn a new process.
        token = exe + ".token"
        popen = self.spawnSubprocess(self.getBuildArtifact(exe), [token])
        lldbutil.wait_for_file_on_target(self, token)

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Attach to process with process name
        self.dbg.SetSelectedPlatform(lldb.selected_platform)
        listener = lldb.SBListener("my.attach.listener")
        error = lldb.SBError()
        process = target.AttachToProcessWithName(listener, exe, False, error)

        self.lldb_assert(error.Success() and process, "process is valid")

        # Destroy process
        lldb.remote_platform.Kill(popen.pid)

        self.lldb_assert_total_time("Attach Start", "CompleteAttach end")
