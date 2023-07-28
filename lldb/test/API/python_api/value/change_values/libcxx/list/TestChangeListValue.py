# OHOS_LOCAL begin
"""
Test change libc++ list values.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class LibcxxChangeValueTestCase(TestBase):

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)

    def do_single_test(self, var_name, frame):
        var = frame.FindVariable(var_name)
        self.assertTrue(var.IsValid(), "Got the SBValue for val")
        element0 = var.GetChildMemberWithName("[0]")
        self.assertTrue(element0.IsValid(), "Got the SBValue for [0]")
        result = element0.SetValueFromCString("12345")
        self.assertTrue(result, "Setting val returned True.")
        result = element0.GetValueAsUnsigned()
        self.assertTrue(result == 12345, "Got correct value (12345)")
        self.assertTrue(element0.GetValueDidChange(), "LLDB noticed that value changed")

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
        
        self.do_single_test("L", frame0)
        self.do_single_test("FL", frame0)
# OHOS_LOCAL end
