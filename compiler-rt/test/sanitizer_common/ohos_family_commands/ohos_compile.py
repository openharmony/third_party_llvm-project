#!/usr/bin/env python3

import os
import subprocess
import sys

from ohos_common import HOS_TMPDIR, push_to_device

here = os.path.abspath(os.path.dirname(sys.argv[0]))
ohos_run = os.path.join(here, "ohos_run.py")

output = None
output_type = "executable"
append_args = []
check_trgt = False

args = sys.argv[1:]
while args:
    arg = args.pop(0)
    if arg == "-shared":
        output_type = "shared"
    elif arg == "-c":
        output_type = "object"
    elif arg == "-o":
        output = args.pop(0)
    elif arg == "-target":
        check_trgt = True
    elif check_trgt or arg.startswith("--target="):
        check_trgt = False
        triple = arg.split("=")[-1]
        if triple.endswith("-linux-ohos"):
            dyld = "unknown_hos_dyld"
            if triple.startswith("arm"):
                dyld = "ld-musl-arm.so.1"
            elif triple.startswith("aarch64"):
                dyld = "ld-musl-aarch64.so.1"
            append_args.append(
                "-Wl,--dynamic-linker=" + os.path.join(HOS_TMPDIR, dyld)
            )

if output is None:
    print("No output file name!", file=sys.stderr)
    sys.exit(1)

ret = subprocess.call(sys.argv[1:] + append_args)
if ret != 0:
    sys.exit(ret)

if output_type in ["executable", "shared"]:
    push_to_device(output)

if output_type == "executable":
    os.rename(output, output + ".real")
    os.symlink(ohos_run, output)
