#!/usr/bin/env python3

import os, subprocess
import sys
from ohos_common import *

here = os.path.abspath(os.path.dirname(sys.argv[0]))
hos_run = os.path.join(here, 'ohos_run.py')

output, output_type = get_output_from_args(sys.argv[1:])

if output == None:
    print ("No output file name!")
    sys.exit(1)

append_args = []
if DYN_LINKER:
    append_args.append('-Wl,--dynamic-linker=' + DYN_LINKER)

ret = subprocess.call(sys.argv[1:] + append_args)

if ret != 0:
    sys.exit(ret)

if output_type in ['executable', 'shared']:
    push_to_device(output, TMPDIR)

if output_type == 'executable':
    os.rename(output, output + '.real')
    os.symlink(hos_run, output)
