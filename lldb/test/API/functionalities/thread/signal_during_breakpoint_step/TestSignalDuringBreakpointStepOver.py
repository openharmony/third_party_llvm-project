import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil
from SignalDuringBreakpointStepTestCase import SignalDuringBreakpointStepTestCase


@skipIfWindows
class SignalDuringBreakpointStepOverTestCase(SignalDuringBreakpointStepTestCase):

    mydir = TestBase.compute_mydir(__file__)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    def test_step_over_breakpoint(self):
        self.run_to_breakpoint_and_step('step-over', False)

    @skipIf(triple='^mips')
    def test_step_over_no_breakpoint(self):
        self.run_to_breakpoint_and_step('step-over', True)
