#!/usr/bin/env python3

import os, subprocess, tempfile, re
import hdc_constants

verbose = False
if os.environ.get('HOS_RUN_VERBOSE') == '1':
    verbose = True

def host_to_device_path(path):
    rel = os.path.relpath(path, "/")
    dev = os.path.join(hdc_constants.TMPDIR, rel)
    return dev

def hdc_output(args):
    command = hdc_constants.get_hdc_cmd_prefix() + args
    if verbose:
        print ("[CMD]:" + " ".join(command))
    return subprocess.check_output(command, stderr=subprocess.STDOUT, timeout=300)

def hdc(args, attempts=1, check_stdout=''):
    tmpname = tempfile.mktemp()
    out = open(tmpname, 'w')
    ret = 255
    while attempts > 0 and ret != 0:
      attempts -= 1
      ret = 0
      output = hdc_output(args)
      # hdc exit code is always zero
      if check_stdout not in output.decode():
        ret = 255
    if ret != 0:
      print ("hdc command failed", args)
      print (tmpname)
      out.close()
      out = open(tmpname, 'r')
      print (out.read())
    out.close()
    os.unlink(tmpname)
    return ret

def pull_from_device(path):
    # hdc can't download empty files
    file_sz = hdc_output(['shell', 'du', path]).split()
    if file_sz and file_sz[0] == b'0':
        return ''

    tmp = tempfile.mktemp()
    hdc(['file', 'recv', path, tmp], attempts=5, check_stdout='FileTransfer finish')
    text = open(tmp, 'r').read()
    os.unlink(tmp)
    return text

def push_to_device(path):
    dst_path = host_to_device_path(path)
    # hdc do not auto create directories on device
    hdc(['shell', 'mkdir', '-p', os.path.dirname(dst_path)])
    hdc(['file', 'send', '-m', path, dst_path], attempts=5, check_stdout='FileTransfer finish')

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

def build_env(do_push=True):
    args = []
    sanitizers = (
        'HWASAN', 'ASAN', 'LSAN', 'MEMPROF', 'MSAN', 'TSAN', 'UBSAN', 'SCUDO'
    )
    set_abort_on_error=True
    for san in sanitizers:
        # for all sanitizers we need 'abort_on_error=0' if 'abort_on_error=1' is not set,
        # so prepare key for them, to set value later
        opt_str = '%s_OPTIONS' % san
        if opt_str not in os.environ:
            os.environ[opt_str] = ''
        elif 'abort_on_error=1' in os.environ[opt_str]:
            set_abort_on_error=False

        # All sanitizers need external symbolizers for some tests
        # set them by default to llvm-symbolizer
        symb_name = '%s_SYMBOLIZER_PATH' % san
        args.append('%s=%s' % (symb_name,  os.environ.get('LLVM_SYMBOLIZER_PATH',
                os.path.join(hdc_constants.TMPDIR,'llvm-symbolizer-aarch64'))))
    # HOS linker ignores RPATH. Set LD_LIBRARY_PATH to Output dir.
    args.append('LD_LIBRARY_PATH=%s' % ( hdc_constants.TMPDIR,))
    for (key, value) in os.environ.items():
        san_opt = key.endswith('SAN_OPTIONS')
        if san_opt and set_abort_on_error:
            value += ':abort_on_error=0'
        if key in ['ASAN_ACTIVATION_OPTIONS', 'SCUDO_OPTIONS'] or san_opt or key == 'LD_LIBRARY_PATH':
            if key in ['TSAN_OPTIONS', 'UBSAN_OPTIONS']:
                # Map TSan or UBSan suppressions file to device
                value = map_list(value, ':', r'(?<=suppressions=)(.+)', lambda m: (m.group(1), do_push))
            elif key == 'LD_LIBRARY_PATH':
                # Map LD_LIBRARY_PATH to device
                value = map_list(value, ':', r'(.+)', lambda m: (m.group(1), False))

            args.append('%s="%s"' % (key, value))
    return ' '.join(args)

def get_output_from_args(args):
    output = None
    output_type = 'executable'

    while args:
        arg = args.pop(0)
        if arg == '-shared':
            output_type = 'shared'
        elif arg == '-c':
            output_type = 'object'
        elif arg == '-o':
            output = args.pop(0)

    return output, output_type