#!/usr/bin/env python3

import os, subprocess, tempfile

# TODO: move this to the cmake ?
HDC = os.environ.get('HDC', 'hdc')
# Please set "HDC_SERVER_IP_PORT" and "HDC_UTID" environment variables
# to pass options for connecting to remote device if needed
HDC_SERVER_IP_PORT = os.environ.get('HDC_SERVER_IP_PORT')
HDC_UTID = os.environ.get('HDC_UTID')

def get_hdc_cmd_prefix():
    server = ['-s', HDC_SERVER_IP_PORT] if HDC_SERVER_IP_PORT else []
    device = ['-t', HDC_UTID] if HDC_UTID else []
    return [HDC, *server, *device]

def verbose():
    return os.environ.get('RUN_VERBOSE') == '1'

def host_to_device_path(path, device_tmpdir):
    rel = os.path.relpath(path, "/")
    dev = os.path.join(device_tmpdir, rel)
    return dev

def hdc_output(args, timeout=300, env=None):
    command = get_hdc_cmd_prefix() + args
    if verbose():
        print ("[CMD]:" + " ".join(command))
    return subprocess.check_output(
            command,
            stderr=subprocess.STDOUT,
            timeout=timeout,
            env=env)

def hdc(args, attempts=1, check_stdout='', env=None):
    tmpname = tempfile.mktemp()
    out = open(tmpname, 'w')
    ret = 255
    while attempts > 0 and ret != 0:
      attempts -= 1
      ret = 0
      output = hdc_output(args, env=env)
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

def _do_push(src, dst, env=None):
    hdc(['file', 'send', src, dst],
        attempts=5,
        check_stdout='FileTransfer finish',
        env=env)

def push_to_device(path, device_tmpdir):
    dst_path = host_to_device_path(path, device_tmpdir)
    # hdc do not auto create directories on device
    hdc(['shell', 'mkdir', '-p', os.path.dirname(dst_path)])
    _do_push(path, dst_path)
    hdc(['shell', 'chmod', '+x', dst_path])

def connect():
    hdc(['tconn'])

remote = hdc
push = _do_push
command = HDC
hdc_str_prefix = ' '.join(get_hdc_cmd_prefix())
config_push_str = f'{hdc_str_prefix} file send '
config_remove_str = f'{hdc_str_prefix} shell rm '
config_shell_str = f'{hdc_str_prefix} shell '
