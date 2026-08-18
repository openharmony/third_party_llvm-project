"""
Test the build-id exec-search-path module cache setting (OHOS !932).

When OHOS_LLVM is enabled, the 'platform.use-exec-search-path-module-cache'
setting is available, allowing callers to enable caching of module lookups
resolved via exec-search-path and build-id.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *


class TestBuildIdModuleCache(TestBase):
    NO_DEBUG_INFO_TESTCASE = True

    def test_setting_exists(self):
        """The 'platform.use-exec-search-path-module-cache' setting should
        be registered when built with OHOS_LLVM=ON."""
        self.expect(
            "settings show platform.use-exec-search-path-module-cache",
            substrs=["use-exec-search-path-module-cache"],
        )

    def test_setting_default(self):
        """The default value should be False (opt-in)."""
        self.expect(
            "settings show platform.use-exec-search-path-module-cache",
            substrs=["false"],
        )

    def test_setting_toggle(self):
        """The setting should be toggleable."""
        self.runCmd(
            "settings set platform.use-exec-search-path-module-cache true"
        )
        self.expect(
            "settings show platform.use-exec-search-path-module-cache",
            substrs=["true"],
        )
        self.runCmd(
            "settings set platform.use-exec-search-path-module-cache false"
        )
        self.expect(
            "settings show platform.use-exec-search-path-module-cache",
            substrs=["false"],
        )
