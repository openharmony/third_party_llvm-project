# -*- Python -*-

import os
import subprocess
import shlex
import sys

# Setup config name.
config.name = 'CRT' + config.name_suffix

# Setup source root.
config.test_source_root = os.path.dirname(__file__)

# Choose between lit's internal shell pipeline runner and a real shell.
use_lit_shell = os.environ.get("LIT_USE_INTERNAL_SHELL")
if use_lit_shell:
    execute_external = (use_lit_shell == "0")
else:
    execute_external = (not sys.platform in ['win32'])

def get_library_path(file):
    cmd = subprocess.Popen([config.clang.split()[-1].strip(), # OHOS_LOCAL
                            '-print-file-name=%s' % file] +
                           shlex.split(config.target_cflags),
                           stdout=subprocess.PIPE,
                           env=config.environment,
                           universal_newlines=True)
    if not cmd.stdout:
      lit_config.fatal("Couldn't find the library path for '%s'" % file)
    dir = cmd.stdout.read().strip()
    if sys.platform in ['win32'] and execute_external:
        dir = dir.replace('\\', '/')
    return dir


def get_libgcc_file_name():
    cmd = subprocess.Popen([config.clang.split()[-1].strip(), # OHOS_LOCAL
                            '-print-libgcc-file-name'] +
                           shlex.split(config.target_cflags),
                           stdout=subprocess.PIPE,
                           env=config.environment,
                           universal_newlines=True)
    if not cmd.stdout:
      lit_config.fatal("Couldn't find the library path for libgcc")
    dir = cmd.stdout.read().strip()
    if sys.platform in ['win32'] and execute_external:
        dir = dir.replace('\\', '/')
    return dir


def build_invocation(compile_flags):
    return ' ' + ' '.join([config.clang] + compile_flags) + ' '


config.substitutions.append(
    ('%clang ', build_invocation([config.target_cflags])))
config.substitutions.append(
    ('%clangxx ',
     build_invocation(config.cxx_mode_flags + [config.target_cflags])))

base_lib = os.path.join(
    config.compiler_rt_libdir, "clang_rt.%%s%s.o" % config.target_suffix)

if sys.platform in ['win32'] and execute_external:
    base_lib = base_lib.replace('\\', '/')

config.substitutions.append(('%crtbegin', base_lib % "crtbegin"))
config.substitutions.append(('%crtend', base_lib % "crtend"))

config.substitutions.append(
    ('%crt1', get_library_path('crt1.o')))
config.substitutions.append(
    ('%crti', get_library_path('crti.o')))
config.substitutions.append(
    ('%crtn', get_library_path('crtn.o')))

config.substitutions.append(
    ('%libgcc', get_libgcc_file_name()))

config.substitutions.append(
    ('%libstdcxx', '-l' + config.sanitizer_cxx_lib.lstrip('lib')))

# OHOS_LOCAL begin
config.substitutions.append(
    ('%shared_ubsan_standalone',
     get_library_path('libclang_rt.ubsan_standalone.so')))
# OHOS_LOCAL end

config.suffixes = ['.c', '.cpp']

if config.host_os not in ['Linux', 'OHOS']:
    config.unsupported = True
