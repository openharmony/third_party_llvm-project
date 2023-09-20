"""
Test change libc++ std::atomic values.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class LibcxxChangeBitsetValueTestCase(TestBase):

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)

    def do_test_var(self, var_name, frame, expected_child_cnt):
        value = frame.FindVariable(var_name)
        self.assertTrue(value.IsValid(), "Got the SBValue for val")
        child_cnt = value.GetNumChildren()
        self.assertTrue(child_cnt == expected_child_cnt,
                        "Expected child cnt = {}".format(expected_child_cnt))
        for i in range(child_cnt):
            child_val = value.GetChildAtIndex(i)
            self.assertTrue(child_val.IsValid(), "Got the SBValue for inner atomic val")
            new_val = 1000+i
            result = child_val.SetValueFromCString(str(new_val))
            self.assertTrue(result, "Setting val returned True.")
            self.assertTrue(child_val.GetValueDidChange(), "LLDB noticed that value changed")
            result = child_val.GetValueAsUnsigned()
            self.assertTrue(result == new_val, "Got correct value ({})".format(new_val))

    @add_test_categories(["libc++", "libstdcxx"])
    @expectedFailureAll(oslist=["windows"], bugnumber="llvm.org/pr24772")
    def test(self):
        """Test that we can change values of libc++ std::bitset."""
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

        self.do_test_var('empty', frame0, 0)
        self.do_test_var('small', frame0, 1)
        self.do_test_var('big', frame0, 2)
