"""
This test is intended to create a situation in which signals are received by
a thread while it is stepping off a breakpoint. If a breakpoint is hit inside
the handler, the breakpoint on the line with the original instruction should
be hit when the handler is finished.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil
from SignalDuringBreakpointStepTestCase import SignalDuringBreakpointStepTestCase


@skipIfWindows
class TestSignalStepOverHandlerWithBreakpoint(SignalDuringBreakpointStepTestCase):

    mydir = TestBase.compute_mydir(__file__)

    num_iters = 10
    num_signal_iters = 5
    func_name_target = ''

    def set_up_step_over_handler_with_breakpoint(self):
        self.action = 'step-over'
        self.handler_bp = self.target.BreakpointCreateBySourceRegex(
            'Break here in signal handler',
            self.main_source_spec)

    def set_func_name_target(self):
        self.func_name_target = self.thread.GetFrameAtIndex(0).GetFunctionName()

    def step_to_target(self, action):
        while self.thread.GetFrameAtIndex(0).GetFunctionName() != self.func_name_target:
            self.runCmd(action)

    def check_handler_bp_step_out(self, action, is_breakpoint_disabled):
        # Start at the relevand line with breakpoint, enable the handler breakpoint for this iteration
        self.handler_bp.SetEnabled(True)
        self.set_func_name_target()

        # Step
        self.runCmd("thread %s" % self.action2cmd(action))

        stopped_in_handler = \
            (lldbutil.get_one_thread_stopped_at_breakpoint(self.process, self.handler_bp) == self.thread)
        # Disable the handler breakpoint until next iteration
        self.handler_bp.SetEnabled(False)
        # Try to get to the next line through the handler
        if stopped_in_handler:
            # Stopped inside the handler, step out until we are back in our parent function
            self.step_to_target("thread step-out")

            if is_breakpoint_disabled:
                # Should be stopped at the breakpoint line because of step-out, try again
                self.check_stopped_at_line(
                    self.breakpoints[action].GetLocationAtIndex(0).GetAddress().GetLineEntry().GetLine())
            else:
                # Should be stopped at our initial action breakpoint again, try again
                self.check_stopped_at_action_breakpoint(action)
            self.runCmd("continue")

        # Now stopped at the corresponding line
        self.check_stopped_at_action_line(action)

    def check_handler_bp_continue(self, action, is_breakpoint_disabled):
        # Start at the relevand line with breakpoint, enable the handler breakpoint for this iteration
        self.handler_bp.SetEnabled(True)
        self.set_func_name_target()

        # Step
        self.runCmd("thread %s" % self.action2cmd(action))

        stopped_in_handler = \
            (lldbutil.get_one_thread_stopped_at_breakpoint(self.process, self.handler_bp) == self.thread)
        # Disable the handler breakpoint until next iteration
        self.handler_bp.SetEnabled(False)
        # Try to get to the next line through the handler
        if stopped_in_handler:
            # Stopped inside the handler, continue until we are back in our parent function
            self.step_to_target("continue")

            if not is_breakpoint_disabled:
                # Should be stopped at initial breakpoint again
                self.check_stopped_at_action_breakpoint(action)
                self.runCmd("continue")

        # Now stopped at the corresponding line
        self.check_stopped_at_action_line(action)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    # Currently on arm, lldb might get wrong return addresses from a signal handler
    # and fail with 'could not create return address breakpoint'
    @skipIf(archs='arm')
    def test_breakpoint_inside_handler_step_out(self):
        self.set_up_step_over_handler_with_breakpoint()
        self.set_up_and_iterate(self.action, True, self.check_handler_bp_step_out)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    # Currently on arm, lldb might get wrong return addresses from a signal handler
    # and fail with 'could not create return address breakpoint'
    @skipIf(archs='arm')
    def test_breakpoint_inside_handler_step_out_to_breakpoint(self):
        self.set_up_step_over_handler_with_breakpoint()
        self.set_up_and_iterate(self.action, False, self.check_handler_bp_step_out)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    def test_breakpoint_inside_handler_continue(self):
        self.set_up_step_over_handler_with_breakpoint()
        self.set_up_and_iterate(self.action, True, self.check_handler_bp_continue)

    # Atomic sequences are not supported yet for MIPS in LLDB.
    # (Copied from concurrent_events/TestConcurrentSignalBreak.py)
    @skipIf(triple='^mips')
    # Currently, lldb might skip the next stop when stepping out of the func,
    # if the single-stepping is not emulated
    @skipIf(archs=no_match(['arm']))
    def test_breakpoint_inside_handler_continue_to_breakpoint(self):
        self.set_up_step_over_handler_with_breakpoint()
        self.set_up_and_iterate(self.action, False, self.check_handler_bp_continue)
