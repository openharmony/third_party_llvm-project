#!/usr/bin/env python3

import os, sys, subprocess
from ohos_common import *


here = os.path.abspath(os.path.dirname(sys.argv[0]))
hos_run = os.path.join(here, 'ohos_run.py')

output = None
output_type = 'executable'

args = sys.argv[1:]
append_args = []
check_trgt = False
while args:
    arg = args.pop(0)
    if arg == '-shared':
        output_type = 'shared'
    elif arg == '-c':
        output_type = 'object'
    elif arg == '-o':
        output = args.pop(0)
    elif arg == '-target':
        check_trgt = True
    elif check_trgt or arg.startswith('--target='):
        check_trgt = False
        if arg.endswith('-linux-ohos'):
            arg = arg.split('=')[-1]
            dyld = 'unknown_hos_dyld'
            # FIXME: Handle -mfloat-abi=hard for arm
            # TODO: Support x86_64
            if arg.startswith('arm'):
                dyld = 'ld-musl-arm.so.1'
            elif arg.startswith('aarch64'):
                dyld = 'ld-musl-aarch64.so.1'
            append_args += ['-Wl,--dynamic-linker=' + os.path.join(HOS_TMPDIR, dyld)]

if output == None:
    print ("No output file name!")
    sys.exit(1)

ret = subprocess.call(sys.argv[1:] + append_args)
if ret != 0:
    sys.exit(ret)

if output_type in ['executable', 'shared']:
    push_to_device(output)

if output_type == 'executable':
    os.rename(output, output + '.real')
    os.symlink(hos_run, output)
