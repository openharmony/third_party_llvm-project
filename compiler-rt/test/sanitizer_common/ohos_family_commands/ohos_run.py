#!/usr/bin/env python3

import os
import signal
import sys

from ohos_common import HOS_TMPDIR, adb, host_to_device_path, pull_from_device

device_binary = host_to_device_path(sys.argv[0])


def build_env():
    args = [f"LD_LIBRARY_PATH={HOS_TMPDIR}"]
    for key, value in os.environ.items():
        if key in ["ASAN_ACTIVATION_OPTIONS", "SCUDO_OPTIONS"] or key.endswith(
            "SAN_OPTIONS"
        ):
            args.append(f'{key}="{value}"')
    return " ".join(args)


device_env = build_env()
device_args = " ".join(sys.argv[1:])
device_stdout = device_binary + ".stdout"
device_stderr = device_binary + ".stderr"
device_exitcode = device_binary + ".exitcode"

ret = adb(
    [
        "shell",
        f"cd {HOS_TMPDIR} && {device_env} {device_binary} {device_args} "
        f">{device_stdout} 2>{device_stderr} ; echo $? >{device_exitcode}",
    ]
)
if ret != 0:
    sys.exit(ret)

sys.stdout.write(pull_from_device(device_stdout))
sys.stderr.write(pull_from_device(device_stderr))
retcode = int(pull_from_device(device_exitcode))
if retcode > 128:
    os.kill(os.getpid(), signal.SIGABRT)
sys.exit(retcode)
