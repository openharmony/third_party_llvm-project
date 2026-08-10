"""
Test that the error message for a missing module file includes
'does not exist' (OHOS !931 adds 'or format error' to distinguish
missing files from wrong-format files).
"""

import os

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *


class TestModuleErrorMessage(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    @skipIfWindows
    def test_nonexistent_target(self):
        """Creating a target with a non-existent file should produce an
        error message that mentions 'does not exist'."""
        fake_path = "/tmp/lldb_ohos_test_nonexistent_931.so"
        if os.path.exists(fake_path):
            os.remove(fake_path)

        # Use runCmd which captures output; target create may fail
        # or succeed with a warning.
        self.runCmd("target create " + fake_path, check=False)
        output = self.res.GetOutput()
        error = self.res.GetError()
        combined = output + error

        # The error path in ModuleList::GetSharedModule should be
        # exercised.  The message should contain "does not exist".
        self.assertIn(
            "does not exist",
            combined,
            "target create with non-existent file should mention "
            "'does not exist' in the output",
        )
