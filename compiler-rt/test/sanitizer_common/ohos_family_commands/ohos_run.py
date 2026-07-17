#!/usr/bin/env python3

import os
import signal
import sys

import hdc_constants
from ohos_common import hdc, host_to_device_path, pull_from_device

device_binary = host_to_device_path(sys.argv[0])


def build_env():
    args = []
    sanitizers = (
        "HWASAN",
        "ASAN",
        "LSAN",
        "MEMPROF",
        "MSAN",
        "TSAN",
        "UBSAN",
        "SCUDO",
    )
    for san in sanitizers:
        # All sanitizers need abort_on_error=0 for remote test execution.
        opt_str = "%s_OPTIONS" % san
        if opt_str not in os.environ:
            os.environ[opt_str] = ""

        # Default to the symbolizer deployed in the remote temporary directory.
        symb_name = "%s_SYMBOLIZER_PATH" % san
        args.append(
            "%s=%s"
            % (
                symb_name,
                os.environ.get(
                    "LLVM_SYMBOLIZER_PATH",
                    os.path.join(hdc_constants.TMPDIR, "llvm-symbolizer-aarch64"),
                ),
            )
        )
    # HOS linker ignores RPATH. Set LD_LIBRARY_PATH to Output dir.
    args.append("LD_LIBRARY_PATH=%s" % hdc_constants.TMPDIR)
    for key, value in os.environ.items():
        san_opt = key.endswith("SAN_OPTIONS")
        if san_opt:
            value += ":abort_on_error=0"
        if key in ["ASAN_ACTIVATION_OPTIONS", "SCUDO_OPTIONS"] or san_opt:
            args.append('%s="%s"' % (key, value))
    return " ".join(args)

device_env = build_env()
device_args = " ".join(sys.argv[1:])
device_stdout = device_binary + ".stdout"
device_stderr = device_binary + ".stderr"
device_exitcode = device_binary + ".exitcode"

# OHOS may set log_path in UBSAN_OPTIONS, while the tests expect output in
# stdout/stderr. Unset it for the remote process.
ret = hdc(
    [
        "shell",
        f"unset UBSAN_OPTIONS && cd {hdc_constants.TMPDIR} && "
        f"{device_env} {device_binary} {device_args} "
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
