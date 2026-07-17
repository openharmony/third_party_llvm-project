import os
import subprocess
import tempfile

import hdc_constants

verbose = os.environ.get("HOS_RUN_VERBOSE") == "1"


def host_to_device_path(path):
    rel = os.path.relpath(path, "/")
    return os.path.join(hdc_constants.TMPDIR, rel)


def hdc_output(args):
    command = hdc_constants.get_hdc_cmd_prefix() + args
    return subprocess.check_output(command, stderr=subprocess.STDOUT)


def hdc(args, attempts=1, check_stdout=""):
    if verbose:
        print(args)
    ret = 255
    output = b""
    while attempts > 0 and ret != 0:
        attempts -= 1
        try:
            output = hdc_output(args)
            # Some hdc versions always return zero, so validate successful file
            # transfers using their stdout marker.
            ret = 0 if check_stdout in output.decode() else 255
        except subprocess.CalledProcessError as error:
            output = error.output
            ret = error.returncode or 255
    if ret != 0:
        print("hdc command failed", args)
        print(output.decode(errors="replace"))
    return ret


def pull_from_device(path):
    # hdc can't download empty files
    file_sz = hdc_output(["shell", "du", path]).split()
    if file_sz and file_sz[0] == b"0":
        return ""

    tmp = tempfile.mktemp()
    hdc(
        ["file", "recv", path, tmp],
        attempts=5,
        check_stdout="FileTransfer finish",
    )
    with open(tmp, "r") as inf:
        text = inf.read()
    os.unlink(tmp)
    return text


def push_to_device(path):
    dst_path = host_to_device_path(path)
    # hdc does not automatically create destination directories.
    hdc(["shell", "mkdir", "-p", os.path.dirname(dst_path)])
    hdc(
        ["file", "send", path, dst_path],
        attempts=5,
        check_stdout="FileTransfer finish",
    )
