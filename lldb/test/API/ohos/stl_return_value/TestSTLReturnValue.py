from __future__ import print_function

import unittest2
import os
import lldb
from lldbsuite.test.lldbtest import *
import lldbsuite.test.lldbutil as lldbutil
from lldbsuite.test.decorators import *

class TestSTLReturnValue(TestBase):
    def test_stack_return_value(self):
        """Test some expressions involving STL data types."""
        self.build()
        (_, _, thread, _) = lldbutil.run_to_source_breakpoint(self, "// Set stack break point at this line.", lldb.SBFileSpec("main.cpp"))
        self.runCmd("finish")
        frame0 = thread.GetFrameAtIndex(0)
        th = frame0.GetThread()
        value = th.GetStopReturnValue()
        self.assertTrue(value.IsValid())

        child_value = value.GetChildMemberWithName("c")
        self.assertEqual(int(child_value.GetChildAtIndex(0).GetValue()), 1)
        self.assertEqual(int(child_value.GetChildAtIndex(1).GetValue()), 2)
        self.assertEqual(int(child_value.GetChildAtIndex(2).GetValue()), 3)

    def test_string_return_value(self):
        """Test some expressions involving STL data types."""
        self.build()
        (_, _, thread, _) = lldbutil.run_to_source_breakpoint(self, "// Set string break point at this line.", lldb.SBFileSpec("main.cpp"))
        self.runCmd("finish")
        frame0 = thread.GetFrameAtIndex(0)
        th = frame0.GetThread()
        value = th.GetStopReturnValue()
        self.assertTrue(value.IsValid())
        self.assertEqual(value.GetSummary(), '"Hello"')

    def test_map_return_value(self):
        """Test some expressions involving STL data types."""
        self.build()
        (_, _, thread, _) = lldbutil.run_to_source_breakpoint(self, "// Set map break point at this line.", lldb.SBFileSpec("main.cpp"))
        self.runCmd("finish")
        frame0 = thread.GetFrameAtIndex(0)
        th = frame0.GetThread()
        value = th.GetStopReturnValue()
        self.assertTrue(value.IsValid())
        self.assertEqual(value.GetChildAtIndex(0).GetChildAtIndex(0).GetSummary(), '"A"')
        self.assertEqual(int(value.GetChildAtIndex(0).GetChildAtIndex(1).GetValue()), 100)
        self.assertEqual(value.GetChildAtIndex(1).GetChildAtIndex(0).GetSummary(), '"B"')
        self.assertEqual(int(value.GetChildAtIndex(1).GetChildAtIndex(1).GetValue()), 50)
        self.assertEqual(value.GetChildAtIndex(2).GetChildAtIndex(0).GetSummary(), '"C"')
        self.assertEqual(int(value.GetChildAtIndex(2).GetChildAtIndex(1).GetValue()), 0)

    def test_class_return_value(self):
        """Test some expressions involving STL data types."""
        self.build()
        (_, _, thread, _) = lldbutil.run_to_source_breakpoint(self, "// Set class break point at this line.", lldb.SBFileSpec("main.cpp"))
        self.runCmd("finish")
        frame0 = thread.GetFrameAtIndex(0)
        th = frame0.GetThread()
        value = th.GetStopReturnValue()
        self.assertTrue(value.IsValid())
        self.assertEqual(value.GetChildAtIndex(0).GetName(), "a")
        self.assertEqual(int(value.GetChildAtIndex(0).GetValue()), 0)
        self.assertEqual(value.GetChildMemberWithName("s").GetSummary(), '"Hello"')
        child_vec = value.GetChildMemberWithName("vec")
        self.assertEqual(int(child_vec.GetChildAtIndex(0).GetValue()), 1)
        self.assertEqual(int(child_vec.GetChildAtIndex(1).GetValue()), 2)
        self.assertEqual(int(child_vec.GetChildAtIndex(2).GetValue()), 3)
        child_list = value.GetChildMemberWithName("l")
        self.assertEqual(int(child_list.GetChildAtIndex(0).GetValue()), 1)
        self.assertEqual(int(child_list.GetChildAtIndex(1).GetValue()), 1)
        self.assertEqual(int(child_list.GetChildAtIndex(2).GetValue()), 1)

    def test_child_class_return_value(self):
        """Test some expressions involving STL data types."""
        self.build()
        (_, _, thread, _) = lldbutil.run_to_source_breakpoint(self, "// Set child_class break point at this line.", lldb.SBFileSpec("main.cpp"))
        self.runCmd("finish")
        frame0 = thread.GetFrameAtIndex(0)
        th = frame0.GetThread()
        value = th.GetStopReturnValue()
        self.assertTrue(value.IsValid())
        self.assertEqual(value.GetChildAtIndex(0).GetName(), "father")
        child_value = value.GetChildAtIndex(0)
        self.assertEqual(child_value.GetChildAtIndex(0).GetName(), "a")
        self.assertEqual(int(child_value.GetChildAtIndex(0).GetValue()), 0)
        self.assertEqual(child_value.GetChildMemberWithName("s").GetSummary(), '"Hello"')
        child_vec = child_value.GetChildMemberWithName("vec")
        self.assertEqual(int(child_vec.GetChildAtIndex(0).GetValue()), 1)
        self.assertEqual(int(child_vec.GetChildAtIndex(1).GetValue()), 2)
        self.assertEqual(int(child_vec.GetChildAtIndex(2).GetValue()), 3)
        child_list = child_value.GetChildMemberWithName("l")
        self.assertEqual(int(child_list.GetChildAtIndex(0).GetValue()), 1)
        self.assertEqual(int(child_list.GetChildAtIndex(1).GetValue()), 1)
        self.assertEqual(int(child_list.GetChildAtIndex(2).GetValue()), 1)
