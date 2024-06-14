#!/bin/env python3

import os, signal, sys, subprocess
from ohos_common import *

device_binary = sys.argv[0] + '.real'
device_env = build_env(False)
device_args = ' '.join(sys.argv[1:]) # FIXME: escape?

# Currently OHOS set log_path in UBSAN_OPTIONS
# Tests expects to see output in stdout/stderr and fails when it is not there
# So unset UBSAN_OPTIONS before run tests.
result = subprocess.run(f'unset UBSAN_OPTIONS && {device_env} {device_binary} {device_args}',
                        shell=True, capture_output=True, text=True, timeout=30)

sys.stdout.write(result.stdout)
sys.stderr.write(result.stderr)
sys.stdout.flush()
sys.stderr.flush()

retcode = result.returncode

# If the device process died with a signal, do abort().
# Not exactly the same, but good enough to fool "not --crash".
if retcode < 0:
  os.kill(os.getpid(), signal.SIGABRT)
sys.exit(retcode)
