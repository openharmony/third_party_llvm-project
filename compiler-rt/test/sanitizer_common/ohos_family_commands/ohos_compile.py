#!/usr/bin/env python3

import os
import subprocess
import sys

from ohos_common import hdc, host_to_device_path, push_to_device

here = os.path.abspath(os.path.dirname(sys.argv[0]))
ohos_run = os.path.join(here, "ohos_run.py")

output = None
output_type = "executable"
args = sys.argv[1:]
while args:
    arg = args.pop(0)
    if arg == "-shared":
        output_type = "shared"
    elif arg == "-c":
        output_type = "object"
    elif arg == "-o":
        output = args.pop(0)

if output is None:
    print("No output file name!", file=sys.stderr)
    sys.exit(1)

with open(f"{output}.stderr", "w") as stderr:
    ret = subprocess.call(sys.argv[1:], stderr=stderr)

if ret != 0:
    sys.exit(ret)

if output_type in ["executable", "shared"]:
    push_to_device(output)
    hdc(["shell", "chmod", "+x", host_to_device_path(output)])

if output_type == "executable":
    os.rename(output, output + ".real")
    os.symlink(ohos_run, output)
