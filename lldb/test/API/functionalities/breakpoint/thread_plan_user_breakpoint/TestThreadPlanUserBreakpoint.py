"""
Test that breakpoints (reason = breakpoint) have more priority than
plan completion (reason = step in/out/over) when reporting stop reason after step,
in particular 'step out' and 'step over', and in addition 'step in'.
Check for correct StopReason when stepping to the line with breakpoint,
which should be eStopReasonBreakpoint in general,
and eStopReasonPlanComplete when breakpoint's condition fails or it is disabled.
"""


import unittest2
import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class ThreadPlanUserBreakpointsTestCase(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    def setUp(self):
        TestBase.setUp(self)

        self.build()
        exe = self.getBuildArtifact("a.out")
        src = lldb.SBFileSpec("main.cpp")

        # Create a target by the debugger.
        self.target = self.dbg.CreateTarget(exe)
        self.assertTrue(self.target, VALID_TARGET)

        # Setup three breakpoints
        self.lines = [line_number('main.cpp', "breakpoint_%i" % i) for i in range(3)]

        self.breakpoints = [self.target.BreakpointCreateByLocation(src, line) for line in self.lines]
        self.assertTrue(
            self.breakpoints[0] and self.breakpoints[0].GetNumLocations() == 1,
            VALID_BREAKPOINT)

        # Start debugging
        self.process = self.target.LaunchSimple(
            None, None, self.get_process_working_directory())
        self.assertIsNotNone(self.process, PROCESS_IS_VALID)
        self.thread = lldbutil.get_one_thread_stopped_at_breakpoint(self.process, self.breakpoints[0])
        self.assertIsNotNone(self.thread, "Didn't stop at breakpoint 0.")

    def check_correct_stop_reason(self, breakpoint_idx, condition):
        self.assertEquals(self.process.GetState(), lldb.eStateStopped)
        if condition:
            # All breakpoints active, stop reason is breakpoint
            thread1 = lldbutil.get_one_thread_stopped_at_breakpoint(self.process, self.breakpoints[breakpoint_idx])
            self.assertEquals(self.thread, thread1, "Didn't stop at breakpoint %i." % breakpoint_idx)
        else:
            # Breakpoints are inactive, stop reason is plan complete
            self.assertEquals(self.thread.GetStopReason(), lldb.eStopReasonPlanComplete,
                "Expected stop reason to be step into/over/out for inactive breakpoint %i line." % breakpoint_idx)

    def change_breakpoints(self, action):
        for i in range(1, len(self.breakpoints)):
            action(self.breakpoints[i])

    def check_thread_plan_user_breakpoint(self, condition, set_up_breakpoint_func):
        # Make breakpoints active/inactive in different ways
        self.change_breakpoints(lambda bp: set_up_breakpoint_func(condition, bp))

        self.thread.StepInto()
        # We should be stopped at the breakpoint_1 line with the correct stop reason
        self.check_correct_stop_reason(1, condition)

        # This step-over creates a step-out from `func_1` plan
        self.thread.StepOver()
        # We should be stopped at the breakpoint_2 line with the correct stop reason
        self.check_correct_stop_reason(2, condition)

        # Check explicit step-out
        # Make sure we install the breakpoint at the right address:
        # on some architectures (e.g, aarch64), step-out stops before the next source line
        return_addr = self.thread.GetFrameAtIndex(1).GetPC()
        step_out_breakpoint = self.target.BreakpointCreateByAddress(return_addr)
        set_up_breakpoint_func(condition, step_out_breakpoint)
        self.breakpoints.append(step_out_breakpoint)

        self.thread.StepOut()
        # We should be stopped somewhere in the main frame with the correct stop reason
        self.check_correct_stop_reason(3, condition)

        # Run the process until termination
        self.process.Continue()
        self.assertEquals(self.process.GetState(), lldb.eStateExited)

    def set_up_breakpoints_condition(self, condition, bp):
        # Set breakpoint condition to true/false
        conditionStr = 'true' if condition else 'false'
        bp.SetCondition(conditionStr)

    def set_up_breakpoints_enable(self, condition, bp):
        # Enable/disable breakpoint
        bp.SetEnabled(condition)

    def set_up_breakpoints_callback(self, condition, bp):
        # Set breakpoint callback to return True/False
        bp.SetScriptCallbackBody('return %s' % condition)

    def test_thread_plan_user_breakpoint_conditional_active(self):
        # Test with breakpoints 1, 2, 3 having true condition
        self.check_thread_plan_user_breakpoint(condition=True,
                                               set_up_breakpoint_func=self.set_up_breakpoints_condition)

    def test_thread_plan_user_breakpoint_conditional_inactive(self):
        # Test with breakpoints 1, 2, 3 having false condition
        self.check_thread_plan_user_breakpoint(condition=False,
                                               set_up_breakpoint_func=self.set_up_breakpoints_condition)

    def test_thread_plan_user_breakpoint_unconditional_active(self):
        # Test with breakpoints 1, 2, 3 enabled unconditionally
        self.check_thread_plan_user_breakpoint(condition=True,
                                               set_up_breakpoint_func=self.set_up_breakpoints_enable)

    def test_thread_plan_user_breakpoint_unconditional_inactive(self):
        # Test with breakpoints 1, 2, 3 disabled unconditionally
        self.check_thread_plan_user_breakpoint(condition=False,
                                               set_up_breakpoint_func=self.set_up_breakpoints_enable)

    def test_thread_plan_user_breakpoint_callback_active(self):
        # Test with breakpoints 1, 2, 3 with callback that returns 'True'
        self.check_thread_plan_user_breakpoint(condition=True,
                                               set_up_breakpoint_func=self.set_up_breakpoints_callback)

    def test_thread_plan_user_breakpoint_callback_inactive(self):
        # Test with breakpoints 1, 2, 3 with callback that returns 'False'
        self.check_thread_plan_user_breakpoint(condition=False,
                                               set_up_breakpoint_func=self.set_up_breakpoints_callback)
