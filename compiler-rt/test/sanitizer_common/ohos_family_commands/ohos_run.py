#!/usr/bin/env python3

import os
import signal
import sys

from ohos_common import *


def _is_ubsan_suite_binary(host_path):
    """True when %t lives under the ubsan / ubsan_minimal lit suites."""
    parts = set(os.path.abspath(host_path).replace("\\", "/").split("/"))
    return bool(parts & {"ubsan", "ubsan_minimal"})


device_binary = host_to_device_path(sys.argv[0], TMPDIR)
device_env = build_remote_env()
device_args = " ".join(sys.argv[1:])
device_stdout = device_binary + ".stdout"
device_stderr = device_binary + ".stderr"
device_exitcode = device_binary + ".exitcode"

# OHOS may set log_path in UBSAN_OPTIONS, while the tests expect output in
# stdout/stderr. Unset it for the remote process.
ret = remote(
    [
        "shell",
        f"unset UBSAN_OPTIONS && cd {TMPDIR} && "
        f"{device_env} {device_binary} {device_args} "
        f">{device_stdout} 2>{device_stderr} ; echo $? >{device_exitcode}",
    ]
)
if ret != 0:
    sys.exit(ret)

sys.stdout.write(pull_from_device(device_stdout))
sys.stderr.write(pull_from_device(device_stderr))
sys.stdout.flush()
sys.stderr.flush()
retcode = int(pull_from_device(device_exitcode))
# OHOS ubsan norecover uses __builtin_trap() → SIGTRAP (128+5 == 133). For
# ubsan suites only, map that to a normal non-zero exit so plain `not` works.
# Other sanitizers / other signals still raise SIGABRT for `not --crash`.
if retcode == 128 + signal.SIGTRAP and _is_ubsan_suite_binary(sys.argv[0]):
    sys.exit(1)
if retcode > 128:
    os.kill(os.getpid(), signal.SIGABRT)
sys.exit(retcode)
