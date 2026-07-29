#!/usr/bin/env python3

import os
import subprocess
import tempfile

# TODO: Move this to CMake.
HDC = os.environ.get("HDC", "hdc")
# Set HDC_SERVER_IP_PORT and HDC_UTID to pass remote connection options when
# needed.
HDC_SERVER_IP_PORT = os.environ.get("HDC_SERVER_IP_PORT")
HDC_UTID = os.environ.get("HDC_UTID")


def get_hdc_cmd_prefix():
    server = ["-s", HDC_SERVER_IP_PORT] if HDC_SERVER_IP_PORT else []
    device = ["-t", HDC_UTID] if HDC_UTID else []
    return [HDC, *server, *device]


def verbose():
    return os.environ.get("RUN_VERBOSE") == "1"


def host_to_device_path(path, device_tmpdir):
    rel = os.path.relpath(path, "/")
    return os.path.join(device_tmpdir, rel)


def hdc_output(args, timeout=300, env=None):
    command = get_hdc_cmd_prefix() + args
    if verbose():
        print("[CMD]:" + " ".join(command))
    return subprocess.check_output(
        command, stderr=subprocess.STDOUT, timeout=timeout, env=env
    )


def hdc(args, attempts=1, check_stdout="", env=None):
    ret = 255
    output = b""
    while attempts > 0 and ret != 0:
        attempts -= 1
        try:
            output = hdc_output(args, env=env)
            # Some hdc versions always return zero, so validate successful file
            # transfers using their stdout marker.
            ret = 0 if check_stdout in output.decode() else 255
        except subprocess.CalledProcessError as error:
            output = error.output
            ret = error.returncode or 255
        except subprocess.TimeoutExpired as error:
            output = error.output or b""
            ret = 255
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
    with open(tmp, "r", errors="replace") as inf:
        text = inf.read()
    os.unlink(tmp)
    return text


def _do_push(src, dst, env=None):
    hdc(
        ["file", "send", src, dst],
        attempts=5,
        check_stdout="FileTransfer finish",
        env=env,
    )


def push_to_device(path, device_tmpdir):
    dst_path = host_to_device_path(path, device_tmpdir)
    # hdc does not automatically create destination directories.
    hdc(["shell", "mkdir", "-p", os.path.dirname(dst_path)])
    _do_push(path, dst_path)
    hdc(["shell", "chmod", "+x", dst_path])


def connect():
    hdc(["tconn"])


remote = hdc
push = _do_push
command = HDC
hdc_str_prefix = " ".join(get_hdc_cmd_prefix())
config_push_str = f"{hdc_str_prefix} file send "
config_remove_str = f"{hdc_str_prefix} shell rm "
config_shell_str = f"{hdc_str_prefix} shell "
