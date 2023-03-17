import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil
from SignalDuringBreakpointStepTestCase import SignalDuringBreakpointStepTestCase


@skipIfWindows
class SignalDuringBreakpointFuncStepInTestCase(SignalDuringBreakpointStepTestCase):

    mydir = TestBase.compute_mydir(__file__)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    # Currently, lldb always steps over the handler from a breakpoint line.
    @skipIf(triple='^mips')
    def test_step_in_func_breakpoint(self):
        self.run_to_breakpoint_and_step('step-in-func', False)

    @skipIf(triple='^mips')
    # Currently, lldb always steps into the handler from a non-breakpoint line,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    def test_step_in_func_no_breakpoint(self):
        self.run_to_breakpoint_and_step('step-in-func', True)
