#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
#

cmd_tool="hdc"

source "$(dirname $0)/logging.sh"

function get_cmd_prefix() {
    cmd_prefix="${cmd_tool}"
    if [[ ! -z "${HDC_SERVER_SOCKET:-}" ]]; then
        cmd_prefix="${cmd_prefix} -s ${HDC_SERVER_SOCKET}"
    fi
    if [[ ! -z "${HDC_DEVICE_SERIAL:-}" ]]; then
        cmd_prefix="${cmd_prefix} -t ${HDC_DEVICE_SERIAL}"
    fi
}

function sync_files_mode() {

    function sync_file() {
        local local_file="${1}"
        local remote_file="${2}"
        local local_mode="$(stat -c "%a" "${local_file}")"
        remote_shell chmod "${local_mode}" "${remote_file}"
    }

    local remote="${1}"
    local host="${2}"

    get_cmd_prefix

    remote_type="$(remote_shell file "${remote}" | cut -f2 -d' ')"

    if [[ "${remote_type}" == "directory" && ! -d "${host}" ]]; then
        for f in ${host}; do
            sync_file "${f}" "${remote}/$(basename "${f}")"
        done
    else
        sync_file "${host}" "${remote}"
    fi

}

function remote_shell() {
    info "executing on device: ${@}"
    get_cmd_prefix
    # sed: remove only trailing \r and \n symbols
    $cmd_prefix shell ${@} | sed "s/\([\r\n]*\)$//"
}

function remote_pull() {
    # pull with filter progress information
    local host="$(echo $@ | awk '{print $NF}')"
    local remote="${@:1:$#-1}"
    info "pulling files: ${remote}"
    info "to: ${host}"
    get_cmd_prefix
    $cmd_prefix file recv ${remote} ${host} 2>&1 | grep -v '\[\(1\| \)\([0-9]\| \)[0-9]%\]'
}

function remote_push() {
    # push with filter progress information
    local remote="$(echo $@ | awk '{print $NF}')"
    local host="${@:1:$#-1}"
    info "pushing files: ${host}"
    info "to: ${remote}"
    get_cmd_prefix
    $cmd_prefix file send ${host} ${remote} 2>&1 | grep -v '\[\(1\| \)\([0-9]\| \)[0-9]%\]'

    sync_files_mode "${remote}" "${host}"

}
