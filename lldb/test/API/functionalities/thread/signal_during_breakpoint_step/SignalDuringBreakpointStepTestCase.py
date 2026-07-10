"""
This test is intended to create a situation in which signals are received by
a thread while it is stepping off a breakpoint. The intended behavior is to
skip the handler and to not stop at the breakpoint we are trying to step off
the second time, as the corresponding instruction was not executed anyway.
If a breakpoint is hit inside the handler, the breakpoint on the line with
the original instruction should be hit when the handler is finished.

This checks stepping off breakpoints set on single instructions and function
calls as well, to see a potential pc change when single-stepping.
"""



import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil


@skipIfWindows
class SignalDuringBreakpointStepTestCase(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    actions = [
        'step-in-func',
        'step-over',
        'step-over-func',
        'step-out',
        'step-out-func',
        'step-until',
        'step-until-func'
    ]

    num_iters = 100
    num_signal_iters = 20
    cur_iter = 0

    def action2cmd(self, action):
        res = action
        if res.endswith('-func'):
            res = res[:-5]
        if res == 'step-until':
            res = 'until %s' % self.lines[action]
        return res

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line numbers and set the breakpoints
        self.main_source = 'main.cpp'
        self.main_source_spec = lldb.SBFileSpec(self.main_source)

        self.build()
        exe = self.getBuildArtifact("a.out")

        self.target = self.dbg.CreateTarget(exe)
        self.assertTrue(self.target, VALID_TARGET)

        self.setup_breakpoint = self.target.BreakpointCreateBySourceRegex(
            'Break here and adjust',
            self.main_source_spec)

        self.breakpoints = {}
        self.lines = {}
        for action in self.actions:
            bp = self.target.BreakpointCreateBySourceRegex(
                '%s breakpoint' % action,
                self.main_source_spec)
            self.assertTrue(bp, VALID_BREAKPOINT)
            self.breakpoints[action] = bp
            self.lines[action] = line_number(self.main_source, '%s line' % action)
        self.fall_out_breakpoint = self.target.BreakpointCreateBySourceRegex(
            'Break here to not fall out',
            self.main_source_spec)

    def get_thread_stopped_at(self):
        frame = self.thread.GetFrameAtIndex(0)
        desc = lldbutil.get_description(frame.GetLineEntry())
        return '(stopped at %s for iteration %d)' % (desc, self.cur_iter)

    def check_iteration(self):
        cur_iter = self.thread.GetSelectedFrame().EvaluateExpression('g_cur_iter').GetValueAsSigned()
        skipped = cur_iter > self.cur_iter
        self.assertEquals(self.cur_iter, cur_iter,
            'Expected action iteration %d to %s (was a breakpoint %s?) %s' %
                (cur_iter, 'continue' if skipped else 'end',
                    'skipped' if skipped else 'hit twice', self.get_thread_stopped_at()))

    def check_stopped_at_breakpoint(self, bp, msg):
        self.check_iteration()
        thread1 = lldbutil.get_one_thread_stopped_at_breakpoint(self.process, bp)
        self.assertEquals(self.thread, thread1,
            "%s %s." % (msg, self.get_thread_stopped_at()))

    def check_stopped_at_line(self, line):
        self.check_iteration()
        desc = self.get_thread_stopped_at()
        expect = "%s:%d" % (self.main_source, line)
        self.assertTrue(expect in desc, "Expected to stop at %s %s" % (expect, desc))
        self.assertEquals(self.thread.GetStopReason(), lldb.eStopReasonPlanComplete,
            "Expected stop reason to be step into/over/out %s." % desc)

    def check_stopped_at_action_breakpoint(self, action):
        self.check_stopped_at_breakpoint(self.breakpoints[action],
            "Didn't stop at breakpoint for %s action" % action)

    def check_stopped_at_action_line(self, action):
        self.check_stopped_at_line(self.lines[action])


    def set_up_for_action(self, action):
        # Run the program.
        self.runCmd("run", RUN_SUCCEEDED)

        # Check we are at line self.setup_breakpoint
        self.expect("thread backtrace", STOPPED_DUE_TO_BREAKPOINT,
                    substrs=["stop reason = breakpoint 1."])

        # Get the process
        self.process = self.target.GetProcess()

        # We should be stopped at the setup site where we can set the action
        self.assertEqual(
            self.process.GetNumThreads(),
            1,
            'Expected to stop before any additional threads are spawned.')

        action_idx = self.actions.index(action)
        self.runCmd("expr action_idx=%d" % action_idx)
        self.runCmd("expr NUM_ITERS=%d" % self.num_iters)
        self.runCmd("expr NUM_SIGNAL_ITERS=%d" % self.num_signal_iters)

        self.thread = self.process.GetThreadAtIndex(0)

        # Configure signal settings
        self.runCmd("process handle SIGRTMIN -p true -s false -n true")
        # TODO: signal numbering is wrong for linux musl right now (SIGRTMIN=35!=34)
        # For now just configure multiple rt signals
        self.runCmd("process handle SIGRTMIN+1 -p true -s false -n true")

        # Continue the inferior so threads are spawned
        self.runCmd("continue")

    def set_up_and_iterate(self, action, do_disable_breakpoint, checker):
        # Set up
        self.set_up_for_action(action)

        # Iterate and check
        for i in range(self.num_iters):
            self.cur_iter = i
            # Check if stopped at the right breakpoint
            self.check_stopped_at_action_breakpoint(action)
            # Disable the breakpoint, if needed
            if do_disable_breakpoint:
                self.breakpoints[action].SetEnabled(False)
            # Delegate to custom checker
            checker(action, do_disable_breakpoint)
            # Enable the breakpoint, if we disabled it
            if do_disable_breakpoint:
                self.breakpoints[action].SetEnabled(True)
            # Continue
            self.runCmd("continue")

        # Should be at the last breakpoint before exit
        self.cur_iter += 1
        self.check_stopped_at_breakpoint(self.fall_out_breakpoint, 'Expected to stop at the last fall-out breakpoint')

    def check_step_off(self, action, is_breakpoint_disabled):
        # Step off the breakpoint
        self.runCmd("thread %s" % self.action2cmd(action))

        # Should be stopped at the corresponding line with 'plan complete' reason
        self.check_stopped_at_action_line(action)

    def run_to_breakpoint_and_step(self, action, do_disable_breakpoint):
        self.set_up_and_iterate(action, do_disable_breakpoint, self.check_step_off)
