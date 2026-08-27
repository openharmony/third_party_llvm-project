#!/usr/bin/env python3

import os
import shlex
import signal
import sys

from ohos_common import *


def _is_ubsan_suite_binary(host_path):
    """True when %t lives under the ubsan / ubsan_minimal lit suites."""
    parts = set(os.path.abspath(host_path).replace("\\", "/").split("/"))
    return bool(parts & {"ubsan", "ubsan_minimal"})


def _map_host_file_arg(arg):
    """Push host files named in argv and rewrite them to the device path.

    Tests such as dlopen-mixed-c-cxx.c pass %t.so without %run mapping, so the
    device otherwise dlopens a host path that does not exist.
    """
    if not arg or arg.startswith("-") or not os.path.isfile(arg):
        return arg
    push_to_device(arg, TMPDIR)
    return host_to_device_path(os.path.abspath(arg), TMPDIR)


device_binary = host_to_device_path(sys.argv[0], TMPDIR)
device_env = build_remote_env()
# Quote argv for hdc shell: unquoted [fourth] is a glob character class and
# expands against files in TMPDIR (print_cmdline.cpp CHECK-PRINT).
device_args = " ".join(
    shlex.quote(_map_host_file_arg(arg)) for arg in sys.argv[1:]
)
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
