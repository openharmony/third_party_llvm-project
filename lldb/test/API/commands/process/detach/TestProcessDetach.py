"""
Tests for process detach and unload-modules option.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class ProcessDetachTestCase(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        TestBase.setUp(self)
        self.line = line_number("main.cpp", "// break here")
        self.runCmd("platform select host")

    def _launch_and_stop(self):
        self.build()
        try:
            _, process, _, _ = lldbutil.run_to_source_breakpoint(
                self, "// break here", lldb.SBFileSpec("main.cpp"))
        except AssertionError as e:
            if "initial handshake packet" in str(e):
                self.skipTest("process launch unavailable in current test environment")
            raise
        self.assertTrue(process and process.IsValid(), PROCESS_IS_VALID)
        self.assertEqual(process.GetState(), lldb.eStateStopped)
        return process

    @skipIfiOSSimulator
    def test_process_detach_invalid_unload_modules_value(self):
        process = self._launch_and_stop()
        self.expect("process detach -u maybe",
                    error=True,
                    substrs=['invalid boolean option: "maybe"'])
        self.assertEqual(process.GetState(), lldb.eStateStopped)
        self.expect("process detach", error=False)

    @skipIfiOSSimulator
    def test_process_detach_with_unload_modules_option(self):
        self._launch_and_stop()
        self.expect("process detach -u true", error=False)

    @skipIfiOSSimulator
    def test_sbprocess_detach_overload_with_unload_modules(self):
        process = self._launch_and_stop()
        error = process.Detach(False, True)
        self.assertTrue(error.Success(), error.GetCString())
