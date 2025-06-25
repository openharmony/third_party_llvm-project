import lldb
import lldbsuite.test.lldbutil as lldbutil
import re
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *


class MemoryShowTestCase(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def build_run_test_stop(self):
        self.build()
        lldbutil.run_to_source_breakpoint(self, "// breakpoint",
                lldb.SBFileSpec("main.c"))

    def test_memory_show(self):
        """Test the 'memory read' and 'memory show' commands with a given address."""
        self.build_run_test_stop()

        # Find the starting address for variable 'argc' to use in the test.
        self.runCmd("p &argc")
        line = self.res.GetOutput().splitlines()[0]
        address = int(line.split('=')[1].strip(), 0)

        # Run 'memory read' command and capture the output.
        self.runCmd(f"memory read {address}")
        memory_read_output = self.res.GetOutput().strip()

        # Run 'memory show' command and capture the output.
        self.runCmd(f"memory show {address}")
        memory_show_output = self.res.GetOutput().strip()

        # Compare the outputs of 'memory read' and 'memory show'.
        self.assertEqual(memory_read_output, memory_show_output, "The output of 'memory read' and 'memory show' should be the same.")

    @skipUnlessArch("x86_64")
    def test_memory_show_breakpoint_x86(self):
        self.build_run_test_stop()
        self.runCmd("b 5")
        self.runCmd("b 7")
        self.runCmd("breakpoint list")
        line = self.res.GetOutput().splitlines()[5]
        pattern = r"0x[0-9a-fA-F]+"
        address = re.search(pattern, line).group(0)
        self.runCmd("memory show " + address)
        res = self.res.GetOutput()
        memory_data = res.split(':')[1][1:3]
        self.assertEqual(
            memory_data, "cc")
        self.runCmd("br del 2")
        self.runCmd("memory show " + address)
        res = self.res.GetOutput()
        memory_data = res.split(':')[1][1:3]
        self.assertNotEqual(
            memory_data, "cc")

    @skipUnlessArch("aarch64")
    def test_memory_show_breakpoint_aarch64(self):
        self.build_run_test_stop()
        self.runCmd("b 5")
        self.runCmd("b 7")
        self.runCmd("breakpoint list")
        pattern_line = r"2.1.*, resolved"
        line = re.search(pattern_line, self.res.GetOutput()).group(0)
        pattern = r"0x[0-9a-fA-F]+"
        address = re.search(pattern, line).group(0)
        self.runCmd("memory show " + address)
        res = self.res.GetOutput()
        memory_data = res.split(':')[1][1:12]
        self.assertEqual(
            memory_data, "00 00 20 d4")
        self.runCmd("br del 2")
        self.runCmd("memory show " + address)
        res = self.res.GetOutput()
        memory_data = res.split(':')[1][1:3]
        self.assertNotEqual(
            memory_data, "00 00 20 d4")
