import os
import re
import subprocess
import tempfile

import hdc_constants

verbose = os.environ.get("HOS_RUN_VERBOSE") == "1"


def host_to_device_path(path):
    rel = os.path.relpath(path, "/")
    return os.path.join(hdc_constants.TMPDIR, rel)


def hdc_output(args):
    command = hdc_constants.get_hdc_cmd_prefix() + args
    if verbose:
        print("[CMD]:" + " ".join(command))
    return subprocess.check_output(command, stderr=subprocess.STDOUT, timeout=300)


def hdc(args, attempts=1, check_stdout=""):
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
    hdc(["shell", "chmod", "+x", dst_path])


def map_path(path, do_push):
    if os.path.exists(path):
        if do_push:
            push_to_device(path)
        return host_to_device_path(path)
    return path


def map_list(value, sep, regex, get_path_and_do_push):
    def repl(match):
        path, do_push = get_path_and_do_push(match)
        return map_path(path, do_push)

    opts = value.split(sep)
    return sep.join(re.sub(regex, repl, opt) for opt in opts)


def _add_default_abort_on_error(value):
    if "abort_on_error=1" in value:
        return value
    return ":".join(filter(None, [value, "abort_on_error=0"]))


def build_remote_env():
    args = []
    environment = os.environ.copy()
    sanitizers = (
        "HWASAN",
        "ASAN",
        "LSAN",
        "MEMPROF",
        "MSAN",
        "TSAN",
        "UBSAN",
        "SCUDO",
    )
    for sanitizer in sanitizers:
        options_name = "%s_OPTIONS" % sanitizer
        environment.setdefault(options_name, "")
        symbolizer_name = "%s_SYMBOLIZER_PATH" % sanitizer
        args.append(
            "%s=%s"
            % (
                symbolizer_name,
                environment.get(
                    "LLVM_SYMBOLIZER_PATH",
                    os.path.join(hdc_constants.TMPDIR, "llvm-symbolizer-aarch64"),
                ),
            )
        )

    has_ld_library_path = False
    for key, value in environment.items():
        sanitizer_option = key.endswith("SAN_OPTIONS")
        if sanitizer_option:
            value = _add_default_abort_on_error(value)
        if (
            key in ["ASAN_ACTIVATION_OPTIONS", "SCUDO_OPTIONS"]
            or sanitizer_option
            or key == "LD_LIBRARY_PATH"
        ):
            if key in ["TSAN_OPTIONS", "UBSAN_OPTIONS"]:
                value = map_list(
                    value,
                    ":",
                    r"(?<=suppressions=)(.+)",
                    lambda match: (match.group(1), True),
                )
            elif key == "LD_LIBRARY_PATH":
                value = map_list(
                    value,
                    ":",
                    r"(.+)",
                    lambda match: (match.group(1), False),
                )
                value = ":".join(filter(None, [hdc_constants.TMPDIR, value]))
                has_ld_library_path = True
            args.append('%s="%s"' % (key, value))

    if not has_ld_library_path:
        args.append("LD_LIBRARY_PATH=%s" % hdc_constants.TMPDIR)
    return " ".join(args)


def build_host_env(runtime_dir):
    environment = os.environ.copy()
    sanitizers = (
        "HWASAN",
        "ASAN",
        "LSAN",
        "MEMPROF",
        "MSAN",
        "TSAN",
        "UBSAN",
        "SCUDO",
    )
    symbolizer_path = environment.get("LLVM_SYMBOLIZER_PATH")
    for sanitizer in sanitizers:
        options_name = "%s_OPTIONS" % sanitizer
        options_value = environment.get(options_name, "")
        if options_name.endswith("SAN_OPTIONS"):
            options_value = _add_default_abort_on_error(options_value)
        environment[options_name] = options_value
        symbolizer_name = "%s_SYMBOLIZER_PATH" % sanitizer
        if symbolizer_path and symbolizer_name not in environment:
            environment[symbolizer_name] = symbolizer_path

    library_path = environment.get("LD_LIBRARY_PATH", "")
    environment["LD_LIBRARY_PATH"] = os.pathsep.join(
        filter(None, [runtime_dir, library_path])
    )
    return environment


def get_output_from_args(args):
    output = None
    output_type = "executable"
    index = 0
    while index < len(args):
        arg = args[index]
        if arg == "-shared":
            output_type = "shared"
        elif arg == "-c":
            output_type = "object"
        elif arg == "-o" and index + 1 < len(args):
            index += 1
            output = args[index]
        index += 1
    return output, output_type
