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


def get_names_from_vars(vars):
    names = list()
    value_list = vars.splitlines()
    for value in value_list:
        start = value.find(")") + 2
        end = value.find("=") - 1
        name = value[start:end]
        names.append(name)
    return names


class TestFrameVariable(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)

        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")

    def verify_variable_names(self, description, vars, names):
        copy_names = list(names)
        actual_names = get_names_from_vars(vars)
        for name in actual_names:
            if name in copy_names:
                copy_names.remove(name)
            else:
                self.lldb_assert(False, "didn't find '%s' in %s" % (name, copy_names))
        self.lldb_assert(
            len(copy_names) == 0,
            "%s: we didn't find variables: %s in value list (%s)"
            % (description, copy_names, actual_names),
        )

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    def test_frame_variable(self):
        # Create a target by the debugger.
        exe = self.getBuildArtifact(self._testMethodName)
        d = {"EXE": exe}
        self.build(dictionary=d)
        self.setTearDownCleanup(dictionary=d)
        target = self.dbg.CreateTarget(exe)
        self.lldb_assert(target, "target is not valid")

        # Set breakpoint
        line = line_number("frame_variable.c", "// breakpoint 1")
        breakpoint = target.BreakpointCreateByLocation("frame_variable.c", line)
        self.lldb_assert(breakpoint.GetNumLocations() >= 1, "process is not valid")

        process = target.LaunchSimple(
            None, None, lldb.remote_platform.GetWorkingDirectory()
        )
        self.lldb_assert(process and process.IsValid(), "process is not valid")

        threads = lldbutil.get_threads_stopped_at_breakpoint_id(
            process, breakpoint.GetID()
        )
        self.lldb_assert(
            len(threads) == 1, "There should be a thread stopped at breakpoint 1"
        )
        thread = threads[0]
        self.lldb_assert(thread.IsValid(), "Thread must be valid")
        frame = thread.GetFrameAtIndex(0)
        self.lldb_assert(frame.IsValid(), "Frame must be valid")

        arg_names = ["argc", "argv"]
        local_names = ["i", "j", "k"]

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Frame variable
        vars = self.handle_cmd("frame variable")

        # Verify if we ask for arguments and locals that we got what we expect
        desc = "arguments + locals"
        names = arg_names + local_names
        count = len(names)
        self.lldb_assert(
            len(vars.splitlines()) == count,
            "There should be %i %s (%s) but we are reporting %i (%s)"
            % (count, desc, names, len(vars.splitlines()), get_names_from_vars(vars)),
        )
        self.verify_variable_names("check names of %s" % (desc), vars, names)

        self.lldb_assert_total_time(
            "HandleCommand: frame variable", "Completed frame variable."
        )
