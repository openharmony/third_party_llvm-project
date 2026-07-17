#!/usr/bin/env python3

import os
import re
import signal
import sys

import hdc_constants
from ohos_common import hdc, host_to_device_path, pull_from_device, push_to_device

device_binary = host_to_device_path(sys.argv[0])


def map_path(path, do_push):
    if os.path.exists(path):
        if do_push:
            push_to_device(path)
        return host_to_device_path(path)
    return path


def map_list(value, sep, regex, get_path_and_do_push):
    def repl(m):
        path, do_push = get_path_and_do_push(m)
        return map_path(path, do_push)

    opts = value.split(sep)
    return sep.join(re.sub(regex, repl, opt) for opt in opts)


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
    has_ld_library_path = False
    for key, value in os.environ.items():
        san_opt = key.endswith("SAN_OPTIONS")
        if san_opt and "abort_on_error=1" not in value:
            value += ":abort_on_error=0"
        if (
            key in ["ASAN_ACTIVATION_OPTIONS", "SCUDO_OPTIONS"]
            or san_opt
            or key == "LD_LIBRARY_PATH"
        ):
            if key in ["TSAN_OPTIONS", "UBSAN_OPTIONS"]:
                # Map sanitizer suppressions files to the device.
                value = map_list(
                    value,
                    ":",
                    r"(?<=suppressions=)(.+)",
                    lambda match: (match.group(1), True),
                )
            elif key == "LD_LIBRARY_PATH":
                # The OHOS linker ignores RPATH. Preserve the remote output
                # directory and map host library paths to their device paths.
                value = map_list(
                    value,
                    ":",
                    r"(.+)",
                    lambda match: (match.group(1), False),
                )
                value = ":".join(filter(None, [hdc_constants.TMPDIR, value]))
                has_ld_library_path = True

            args.append('%s="%s"' % (key, value))
    if not has_ld_library_path:
        args.append("LD_LIBRARY_PATH=%s" % hdc_constants.TMPDIR)
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
sys.stdout.flush()
sys.stderr.flush()
retcode = int(pull_from_device(device_exitcode))
if retcode > 128:
    os.kill(os.getpid(), signal.SIGABRT)
sys.exit(retcode)
