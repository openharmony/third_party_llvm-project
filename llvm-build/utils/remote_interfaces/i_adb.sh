#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
#

source "$(dirname $0)/logging.sh"

function remote_shell() {
    info "executing on device: ${@}"
    adb shell ${@}
}

function remote_pull() {
    # pull with filter progress information
    local host="$(echo $@ | awk '{print $NF}')"
    local remote="${@:1:$#-1}"
    info "pulling files: ${remote}"
    info "to: ${host}"
    adb pull "${remote}" "${host}" 2>&1 | grep -v '\[\(1\| \)\([0-9]\| \)[0-9]%\]'
}

function remote_push() {
    # push with filter progress information
    local remote="$(echo $@ | awk '{print $NF}')"
    local host="${@:1:$#-1}"
    info "pushing files: ${host}"
    info "to: ${remote}"
    adb push ${host} ${remote} 2>&1 | grep -v '\[\(1\| \)\([0-9]\| \)[0-9]%\]'
}
