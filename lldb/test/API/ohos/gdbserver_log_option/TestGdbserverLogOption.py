"""
Test that lldb-server platform recognizes the --gdbserver-log-file option
(OHOS !617).  The option is registered under #ifdef OHOS_LLVM but only
shown in usage text under #ifdef __OHOS_FAMILY__, so on x86 Linux we
verify the option is accepted (not rejected as "unrecognized").
"""

import os
import subprocess

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *


class TestGdbserverLogOption(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def _find_lldb_server(self):
        """Locate the lldb-server binary."""
        shlib_dir = lldb.SBHostOS.GetLLDBPath(lldb.ePathTypeLLDBShlibDir)
        if shlib_dir:
            for candidate in [
                os.path.join(str(shlib_dir), "lldb-server"),
                os.path.join(str(shlib_dir), "..", "bin", "lldb-server"),
            ]:
                candidate = os.path.normpath(candidate)
                if os.path.isfile(candidate):
                    return candidate
        import shutil
        return shutil.which("lldb-server")

    @skipIfWindows
    def test_gdbserver_log_file_accepted(self):
        """--gdbserver-log-file should be a recognized option when built
        with OHOS_LLVM=ON.  We verify it is not rejected as
        'unrecognized option'."""
        lldb_server = self._find_lldb_server()
        if not lldb_server:
            self.skipTest("lldb-server not found")

        # Pass --gdbserver-log-file with --server but without --listen.
        # lldb-server will fail immediately after option parsing because
        # --listen is required.  The important thing is the option was
        # parsed, not rejected as unknown.
        result = subprocess.run(
            [
                lldb_server, "platform",
                "--gdbserver-log-file", "/tmp/test-gdbserver-ohos.log",
                "--server",
            ],
            capture_output=True,
            text=True,
            timeout=5,
        )
        combined = result.stdout + result.stderr

        # The key assertion: the option must NOT be reported as unrecognized.
        self.assertNotIn(
            "unrecognized option",
            combined,
            "lldb-server platform should recognize --gdbserver-log-file "
            "when built with OHOS_LLVM=ON. Got:\n" + combined,
        )
