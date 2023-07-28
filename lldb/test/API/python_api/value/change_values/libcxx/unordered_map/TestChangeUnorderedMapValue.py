# OHOS_LOCAL begin
"""
Test change libc++ unordered map values.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class LibcxxChangeValueTestCase(TestBase):

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)

    @add_test_categories(["libc++"])
    @expectedFailureAll(oslist=["windows"], bugnumber="llvm.org/pr24772")
    def test(self):
        """Test that we can change values of libc++ map."""
        self.build()
        self.runCmd("file " + self.getBuildArtifact("a.out"), CURRENT_EXECUTABLE_SET)

        bkpt = self.target().FindBreakpointByID(
            lldbutil.run_break_set_by_source_regexp(
                self, "Set break point at this line."))

        self.runCmd("run", RUN_SUCCEEDED)

        # Get Frame #0.
        target = self.dbg.GetSelectedTarget()
        process = target.GetProcess()
        self.assertState(process.GetState(), lldb.eStateStopped)
        thread = lldbutil.get_stopped_thread(
            process, lldb.eStopReasonBreakpoint)
        self.assertTrue(
            thread.IsValid(),
            "There should be a thread stopped due to breakpoint condition")
        frame0 = thread.GetFrameAtIndex(0)
        self.assertTrue(frame0.IsValid(), "Got a valid frame.")

        val_value = frame0.FindVariable("UM")
        self.assertTrue(val_value.IsValid(), "Got the SBValue for val")
        element0 = val_value.GetChildMemberWithName("[0]")
        self.assertTrue(element0.IsValid(), f"Got the SBValue for [0]")
        pair0 = element0.GetChildMemberWithName("__cc")
        self.assertTrue(pair0.IsValid(), f"Got the SBValue for __cc")
        self.assertTrue(pair0.GetNumChildren() == 2, f"Got 2 children")
        pair0_second = pair0.GetChildMemberWithName("second")
        self.assertTrue(pair0_second.IsValid(), "Got the SBValue for [0].second")
        result = pair0_second.SetValueFromCString("12345")
        self.assertTrue(result, "Setting val returned True.")
        result = pair0_second.GetValueAsUnsigned()
        self.assertTrue(result == 12345, "Got correct value (12345)")
        self.assertTrue(pair0_second.GetValueDidChange(), "LLDB noticed that value changed")
# OHOS_LOCAL end
