#!/usr/bin/env python3

import os
import subprocess
import sys

import hdc_constants
from ohos_common import get_output_from_args

here = os.path.abspath(os.path.dirname(sys.argv[0]))
host_run = os.path.join(here, "ohos_host_run.py")

output, output_type = get_output_from_args(sys.argv[1:])
if output is None:
    print("No output file name!", file=sys.stderr)
    sys.exit(1)

append_args = []
if hdc_constants.DYN_LINKER:
    append_args.append("-Wl,--dynamic-linker=" + hdc_constants.DYN_LINKER)

sanitizer_include = os.path.abspath(os.path.join(here, "../../../include/"))
append_args.append("-I" + sanitizer_include)

ret = subprocess.call(sys.argv[1:] + append_args)
if ret != 0:
    sys.exit(ret)

if output_type == "executable":
    os.rename(output, output + ".real")
    os.symlink(host_run, output)
