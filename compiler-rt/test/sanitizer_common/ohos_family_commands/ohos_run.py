#!/usr/bin/env python3

import os, signal, sys, subprocess
import re
from ohos_common import *

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
        'HWASAN', 'ASAN', 'LSAN', 'MEMPROF', 'MSAN', 'TSAN', 'UBSAN', 'SCUDO'
    )
    for san in sanitizers:
        # for all sanitizers we need 'abort_on_error=0',
        # so prepare key for them, to set value later
        opt_str = '%s_OPTIONS' % san
        if opt_str not in os.environ:
            os.environ[opt_str] = ''

        # All sanitizers need external symbolizers for some tests
        # set them by default to llvm-symbolizer
        symb_name = '%s_SYMBOLIZER_PATH' % san
        args.append('%s=%s' % (symb_name,  os.environ.get('LLVM_SYMBOLIZER_PATH',
                os.path.join(hdc_constants.TMPDIR,'llvm-symbolizer-aarch64'))))
    # HOS linker ignores RPATH. Set LD_LIBRARY_PATH to Output dir.
    args.append('LD_LIBRARY_PATH=%s' % ( hdc_constants.TMPDIR,))
    for (key, value) in os.environ.items():
        san_opt = key.endswith('SAN_OPTIONS')
        if san_opt:
            value += ':abort_on_error=0'
        if key in ['ASAN_ACTIVATION_OPTIONS', 'SCUDO_OPTIONS'] or san_opt or key == 'LD_LIBRARY_PATH':
            if key in ['TSAN_OPTIONS', 'UBSAN_OPTIONS']:
                # Map TSan or UBSan suppressions file to device
                value = map_list(value, ':', r'(?<=suppressions=)(.+)', lambda m: (m.group(1), True))
            elif key == 'LD_LIBRARY_PATH':
                # Map LD_LIBRARY_PATH to device
                value = map_list(value, ':', r'(.+)', lambda m: (m.group(1), False))

            args.append('%s="%s"' % (key, value))
    return ' '.join(args)

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
