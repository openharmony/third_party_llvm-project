import os, sys, subprocess, tempfile
import time

ANDROID_TMPDIR = '/data/local/tmp/Output'
ADB = os.environ.get('ADB', 'adb')
DYN_LINKER = os.environ.get('OHOS_REMOTE_DYN_LINKER') # OHOS_LOCAL
ANDROID_SERIAL = os.environ.get('ANDROID_SERIAL') # OHOS_LOCAL

verbose = False
if os.environ.get('ANDROID_RUN_VERBOSE') == '1':
    verbose = True

# OHOS_LOCAL begin
def get_adb_cmd_prefix():
    device = ['-s', ANDROID_SERIAL] if ANDROID_SERIAL else []
    return [ADB, *device]

def host_to_device_path(path, device_tmpdir=ANDROID_TMPDIR):
    rel = os.path.relpath(path, "/")
    dev = os.path.join(device_tmpdir, rel)
    return dev
# OHOS_LOCAL end

def adb(args, attempts = 1, timeout_sec = 600):
    if verbose:
        print(args)
    tmpname = tempfile.mktemp()
    out = open(tmpname, 'w')
    ret = 255
    while attempts > 0 and ret != 0:
      attempts -= 1
      # OHOS_LOCAL begin
      ret = subprocess.call(
            ['timeout', str(timeout_sec)] + get_adb_cmd_prefix() + args,
            stdout=out, stderr=subprocess.STDOUT)
      # OHOS_LOCAL end
    if ret != 0:
      print("adb command failed", args)
      print(tmpname)
      out.close()
      out = open(tmpname, 'r')
      print(out.read())
    out.close()
    os.unlink(tmpname)
    return ret

def pull_from_device(path):
    tmp = tempfile.mktemp()
    adb(['pull', path, tmp], 5, 60)
    text = open(tmp, 'r').read()
    os.unlink(tmp)
    return text

# OHOS_LOCAL begin
def push_to_device(path, device_tmpdir=ANDROID_TMPDIR):
    dst_path = host_to_device_path(path, device_tmpdir)
    adb(['push', path, dst_path], 5, 60)
# OHOS_LOCAL end
