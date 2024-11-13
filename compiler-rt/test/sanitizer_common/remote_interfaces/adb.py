from android_commands.android_common import *

import subprocess

def adb_output(args, timeout_sec=300, env=None):
   return subprocess.check_output(get_adb_cmd_prefix() + args,
                                  stderr=subprocess.STDOUT,
                                  timeout=timeout_sec,
                                  env=env)

def connect():
    pass

remote = adb
push = push_to_device
command = ADB
adb_str_prefix = ' '.join(get_adb_cmd_prefix())
config_push_str = f'{adb_str_prefix} push '
config_remove_str = f'{adb_str_prefix} shell rm '
config_shell_str = f'{adb_str_prefix} shell '
