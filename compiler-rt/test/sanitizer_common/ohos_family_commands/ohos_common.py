import os
import subprocess
import tempfile

HOS_TMPDIR = "/data/local/tmp/Output"
ADB = os.environ.get("ADB", "adb")

verbose = os.environ.get("HOS_RUN_VERBOSE") == "1"


def host_to_device_path(path):
    rel = os.path.relpath(path, "/")
    return os.path.join(HOS_TMPDIR, rel)


def adb(args, attempts=1):
    if verbose:
        print(args)
    tmpname = tempfile.mktemp()
    with open(tmpname, "w") as out:
        ret = 255
        while attempts > 0 and ret != 0:
            attempts -= 1
            ret = subprocess.call([ADB] + args, stdout=out, stderr=subprocess.STDOUT)
            if attempts != 0:
                ret = 5
        if ret != 0:
            print("adb command failed", args)
            print(tmpname)
            with open(tmpname, "r") as inf:
                print(inf.read())
    os.unlink(tmpname)
    return ret


def pull_from_device(path):
    tmp = tempfile.mktemp()
    adb(["pull", path, tmp], 5)
    with open(tmp, "r") as inf:
        text = inf.read()
    os.unlink(tmp)
    return text


def push_to_device(path):
    dst_path = host_to_device_path(path)
    adb(["push", path, dst_path], 5)
