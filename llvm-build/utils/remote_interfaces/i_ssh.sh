#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
#

source "$(dirname $0)/logging.sh"

function check_ssh_env() {
    msg=""
    [ -n "${SSH_USER}" ] || msg+="SSH_USER not set; "
    [ -n "${SSH_PORT}" ] || msg+="SSH_PORT not set; "
    [ -n "${SSH_HOST}" ] || msg+="SSH_HOST not set; "
    if [ -n "${msg}" ]; then
        error "${msg}"
        exit 1
    fi
}

function remote_shell() {
    check_ssh_env
    info "executing on device: $@"
    ssh -o "StrictHostKeyChecking no" -q -l ${SSH_USER} -p ${SSH_PORT} ${SSH_HOST} $@
}

function remote_pull() {
    check_ssh_env
    local host="$(echo $@ | awk '{print $NF}')"
    local remote="$(echo ${@:1:$#-1} | sed -E "s/([^ ]+)/${SSH_USER}@${SSH_HOST}:\1/g")"
    info "pulling files: ${remote}"
    info "to: ${host}"
    scp -o "StrictHostKeyChecking no" -r -q -P ${SSH_PORT} ${remote} ${host}
}

function remote_push() {
    # check_ssh_env - will be checked in remote_shell
    local remote="$(echo $@ | awk '{print $NF}')"
    local host="${@:1:$#-1}"
    remote_shell "mkdir -p $(dirname ${remote})"
    info "pushing files: ${host}"
    info "to: ${remote}"
    scp -o "StrictHostKeyChecking no" -r -q -P ${SSH_PORT} ${host} ${SSH_USER}@${SSH_HOST}:${remote}
}
