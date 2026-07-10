#!/usr/bin/env bash
CURDIR=$(cd $(dirname $0) && pwd)
export PYTHONPATH="$CURDIR/../script"
"$CURDIR/lldb" "$@"
