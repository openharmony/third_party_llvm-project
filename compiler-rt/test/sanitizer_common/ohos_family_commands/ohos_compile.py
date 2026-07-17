#!/usr/bin/env python3

import os
import subprocess
import sys

import hdc_constants
from ohos_common import push_to_device

here = os.path.abspath(os.path.dirname(sys.argv[0]))
ohos_run = os.path.join(here, "ohos_run.py")

output = None
output_type = "executable"
append_args = []
args = sys.argv[1:]
while args:
    arg = args.pop(0)
    if arg == "-shared":
        output_type = "shared"
    elif arg == "-c":
        output_type = "object"
    elif arg == "-o":
        output = args.pop(0)

if hdc_constants.DYN_LINKER:
    append_args.append("-Wl,--dynamic-linker=" + hdc_constants.DYN_LINKER)

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
