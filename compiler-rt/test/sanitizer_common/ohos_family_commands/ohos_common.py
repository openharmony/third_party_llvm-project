#!/usr/bin/env python3

import os, subprocess, tempfile
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
