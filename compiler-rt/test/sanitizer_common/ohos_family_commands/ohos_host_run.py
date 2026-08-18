#!/usr/bin/env python3

import os
import signal
import subprocess
import sys

from ohos_common import build_host_env


def _is_ubsan_suite_binary(host_path):
    """True when %t lives under the ubsan / ubsan_minimal lit suites."""
    parts = set(os.path.abspath(host_path).replace("\\", "/").split("/"))
    return bool(parts & {"ubsan", "ubsan_minimal"})


host_binary = os.path.abspath(sys.argv[0] + ".real")
host_env = build_host_env(os.path.dirname(host_binary))

# OHOS may set log_path in UBSAN_OPTIONS, while tests expect diagnostics on
# stdout/stderr. Do not pass the platform default to the test process.
host_env.pop("UBSAN_OPTIONS", None)
result = subprocess.run(
    [host_binary, *sys.argv[1:]],
    env=host_env,
    capture_output=True,
    text=True,
    timeout=30,
)

sys.stdout.write(result.stdout)
sys.stderr.write(result.stderr)
sys.stdout.flush()
sys.stderr.flush()

# Match ohos_run.py: ubsan-suite SIGTRAP → exit(1); else host SIGABRT.
if result.returncode == -signal.SIGTRAP and _is_ubsan_suite_binary(sys.argv[0]):
    sys.exit(1)
if result.returncode < 0:
    os.kill(os.getpid(), signal.SIGABRT)
sys.exit(result.returncode)
