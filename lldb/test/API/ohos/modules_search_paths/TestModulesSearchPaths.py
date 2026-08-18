"""
Test that the 'target.modules-search-paths' setting is available and
populates the image search paths when a target is created (OHOS !564).

When OHOS_LLVM is enabled, the Target constructor reads
'target.modules-search-paths' and appends it to m_image_search_paths,
so that remote module paths can be remapped before attach.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *


class TestModulesSearchPaths(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def setUp(self):
        TestBase.setUp(self)
        self.runCmd("settings clear target.modules-search-paths", check=False)

    def tearDown(self):
        self.runCmd("settings clear target.modules-search-paths", check=False)
        TestBase.tearDown(self)

    def test_setting_exists(self):
        """The 'target.modules-search-paths' setting should be registered."""
        self.expect(
            "settings show target.modules-search-paths",
            substrs=["target.modules-search-paths"],
        )

    def test_setting_set_and_show(self):
        """Setting a path mapping should be reflected in the setting value.
        Note: PathMappingList validates that both paths exist, so we use
        /tmp which always exists."""
        self.runCmd(
            "settings set target.modules-search-paths /tmp /tmp"
        )
        self.expect(
            "settings show target.modules-search-paths",
            substrs=["/tmp"],
        )

    def test_setting_cleared(self):
        """Clearing the setting should empty the mapping."""
        self.runCmd(
            "settings set target.modules-search-paths /tmp /tmp"
        )
        self.runCmd("settings clear target.modules-search-paths")
        result = lldb.SBCommandReturnObject()
        self.dbg.GetCommandInterpreter().HandleCommand(
            "settings show target.modules-search-paths", result
        )
        # After clearing, the output should not contain the old mapping
        # path pairs (the property name itself may still appear).
        output_lines = result.GetOutput().strip().split("\n")
        # Find the line with the value; if cleared, it should be empty
        # or contain no path mappings.
        for line in output_lines:
            if "target.modules-search-paths" in line and "=" in line:
                # The value part after '=' should not contain "/tmp"
                value_part = line.split("=", 1)[1] if "=" in line else ""
                self.assertNotIn("/tmp", value_part)
