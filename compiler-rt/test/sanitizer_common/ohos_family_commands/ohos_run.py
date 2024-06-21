#!/usr/bin/env python3

import os, signal, sys, subprocess
import re
from ohos_common import *

device_binary = host_to_device_path(sys.argv[0])
device_env = build_env()
device_args = ' '.join(sys.argv[1:]) # FIXME: escape?
device_stdout = device_binary + '.stdout'
device_stderr = device_binary + '.stderr'
device_exitcode = device_binary + '.exitcode'
device_linker = ''

# Currently OHOS set log_path in UBSAN_OPTIONS
# Tests expects to see output in stdout/stderr and fails when it is not there
# So unset UBSAN_OPTIONS before run tests.
hdc(['shell', 'unset UBSAN_OPTIONS && cd %s && %s %s %s %s >%s 2>%s ; echo $? >%s' %
    (hdc_constants.TMPDIR, device_env, device_linker, device_binary, device_args,
    device_stdout, device_stderr, device_exitcode)])

sys.stdout.write(pull_from_device(device_stdout))
sys.stderr.write(pull_from_device(device_stderr))
sys.stdout.flush()
sys.stderr.flush()
retcode = int(pull_from_device(device_exitcode))
# If the device process died with a signal, do abort().
# Not exactly the same, but good enough to fool "not --crash".
if retcode > 128:
  os.kill(os.getpid(), signal.SIGABRT)
sys.exit(retcode)
