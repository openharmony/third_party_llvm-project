#!/usr/bin/env python3

import os, sys, subprocess

def map_arg(arg):
    if not arg.startswith('-') and os.path.exists(arg) and os.path.exists(arg + '.real'):
        return arg + '.real'
    return arg

args = [map_arg(arg) for arg in sys.argv[2:]]
sys.exit(subprocess.call([sys.argv[1]] + args))
