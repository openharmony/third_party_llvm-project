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

import subprocess
import lldb
import os
from lldb_time_test.TimeTestBase import TimeTestBase
from lldbsuite.test.decorators import skipOnOpenHarmonyCI, skipIf, skipUnlessPlatform


class TestAttachHap(TimeTestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        # Call super's setUp().
        TimeTestBase.setUp(self)
        # Init benchmark JSON file
        self.init_benchmark("benchmark_usb.json")
        self.package_name = "com.hw.lldbperformancetestapp"

    @skipOnOpenHarmonyCI
    @skipIf(remote=False)
    @skipUnlessPlatform(["ohos"])
    def test_ohos_attach_to_hap(self):
        # install app
        hap_path = os.path.join(os.path.dirname(__file__), "entry-default-signed.hap")
        subprocess.run(
            ["hdc", "install", "-r", hap_path],
            stdout=subprocess.PIPE,
        )
        # start hap
        subprocess.run(
            [
                "hdc",
                "shell",
                "aa",
                "start",
                "-a",
                "%s.MainAbility" % self.package_name,
                "-b",
                self.package_name,
            ],
            stdout=subprocess.PIPE,
        )
        ret = subprocess.run(
            ["hdc", "shell", "pidof", self.package_name],
            stdout=subprocess.PIPE,
        )
        pid = int(ret.stdout.decode("utf-8").strip())
        self.lldb_assert(pid > 0, "hap's pid is illegal.")

        # Log timers reset
        self.handle_cmd("log timers reset")
        # Attach hap process by pid
        self.handle_cmd("attach -p %s" % str(pid))

        self.lldb_assert_total_time("Attach Start", "CompleteAttach end")
