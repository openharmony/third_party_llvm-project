import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil
from SignalDuringBreakpointStepTestCase import SignalDuringBreakpointStepTestCase


@skipIfWindows
class SignalDuringBreakpointFuncStepUntilTestCase(SignalDuringBreakpointStepTestCase):

    mydir = TestBase.compute_mydir(__file__)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    def test_step_until_func_breakpoint(self):
        self.run_to_breakpoint_and_step('step-until-func', False)

    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    def test_step_until_func_no_breakpoint(self):
        self.run_to_breakpoint_and_step('step-until-func', True)
