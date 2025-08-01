#!/usr/bin/env bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
#
# turn some bugs into errors
set -o errexit -o pipefail -o noclobber -o nounset

source "$(dirname $0)/logging.sh"

# The following code need to separate test binary and its argumetns with '--'
# The similar solution as was before in 'cmake/modules/TestFile.cmake',
# but now inside rununer.sh (previous solution broke lit 'RUN' generation logic)
script_args=
for (( i=$#;i>0;i-- ));do
    if [ "${!i:0:1}" = "/" ] && [ "${!i}" != "/dev/null" ]; then
        if file "${!i}" | grep -q 'ARM'; then
            # separate test binary args:
            script_args="\"${!i}\" -- ${script_args}"
            continue
        fi
    fi
    script_args="\"${!i}\" ${script_args}"
done
eval set -- ${script_args}

OPTIONS="dfo:vr:e:a:i"
LONGOPTS="help,redirect-output:,redirect-stdout:,redirect-input:,add-path:\
,ld-lib-path:,device-tmp:,device-serial:,isolate-cmd:\
,adb-client:,hdc-client:,cache-builder:,host-tmp:,rununderdebug,chdir:,summary:\
,connection:,ssh-user:,ssh-host:,ssh-port:,extra-data:,extra-link:,extra-directory:"

# -regarding ! and PIPESTATUS see above
# -temporarily store output to be able to check for errors
# -activate quoting/enhanced mode (e.g. by writing out “--options”)
# -pass arguments only via   -- "$@"   to separate them correctly
! PARSED=$(getopt --options=${OPTIONS} \
                  --longoptions=${LONGOPTS} --name "${0}" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    # return value is 1 - getopt has complained about wrong arguments to stdout
    exit 2
fi

eval set -- "${PARSED}"

# Default values for parameters
redirect_output=
redirect_stdout=
redirect_input="/dev/null"
chdir=
summary=
prog_to_test=n
DEVICE_TMP="/data/local/tmp"
EXPORT_LD_LIBRARY_PATH=
EXPORT_PATH=
ISOLATE_CMD=
RUNUNDERDEBUG="${RUNUNDERDEBUG-}" # get value from env variable(if any)
CACHE_BUILDER="${CACHE_BUILDER-}" # get value from env variable(if any)
SANDBOX_BUILD_DIR=
CONNECTION_TYPE=
SSH_USER=
SSH_HOST=
SSH_PORT=
ADB_SERVER_SOCKET=
HDC_SERVER_SOCKET=
device_serial=
extra_data=
extra_links=
extra_dirs=

function usage() {
    cat << EOF
Usage: ${0} <options> <program> <args>
Run tests on remote device.

Example of running a test on OHOS device:
./rununder.sh --connection=HDC --hdc-client="127.0.0.1:8710" --device-serial=XXXX1234 ./a.out

Common options:
    --connection HDC|ADB|SSH    Specify connection type
    --ssh-user <USER>           Specify ssh connection username
    --ssh-host <IP>             Specify ssh ip to connect
    --ssh-port <PORT>           Specify ssh port to connect
    --adb-client <CLIENT>       Set env variable ADB_SERVER_SOCKET for the
                                ADB connection.
                                For example, --adb-client="tcp:10.122.9.11:5037"
    --hdc-client <CLIENT>       Set env variable HDC_SERVER_SOCKET for the
                                HDC connection.
                                For example, --hdc-client="127.0.0.1:5039"
    --device-serial <SERIAL>    Set env variable ANDROID_SERIAL for the ADB
                                connection.
                                Set env variable HDC_DEVICE_SERIAL for the HDC
                                connection.
                                For example, --device-serial=
                                "a-provider.devops-b.huawei.com:23077"
    --device-tmp <PATH>         Specify path for test directory location on the
                                remote device. Default: ${DEVICE_TMP}
    --add-path <PATH>           Add specified path to the PATH variable.
    --ld-lib-path <PATH>        Add PATH to the LD_LIBRARY_PATH variable.
    --cache-builder <PATH>      Binary cache builder or its wrapper.
    --rununderdebug             Make output more verbose.
    --sandbox-build-dir <PATH>  Path to the host build directory for the tests.
                                It allows to use the same directories structure
                                on the remote device.
    --extra-data <PATH>         Extra directories/files to be pushed with the test
    --extra-link <SRC>:<DST>    Pair of link/target separated with `:` to be
                                created on the remote device
    --extra-directory <PATH>    Create remote directory on converted <PATH>
    --help                      Show this message.
Run options:
    --isolate-cmd               Command to run something on the isolated cores.
timeit options:
    --redirect-output <PATH>    Redirect stdout and stderr for the target to
                                PATH.
    --redirect-input <PATH>     Redirect stdin for the target to PATH.
                                Default: ${redirect_input}
    --summary <PATH>            Write monitored process summary (exit code and
                                time) to PATH.
EOF
}

# Processing parameters
while true; do
    case "${1}" in
        --adb-client)
            # Setup connection for adb command using
            # env variable, i.e. export ADB_SERVER_SOCKET="tcp:10.122.9.11:5037"
            export ADB_SERVER_SOCKET=${2}
            shift 2 ;;
        --hdc-client)
            # Setup connection for hdc command using
            # env variable, i.e. export HDC_SERVER_SOCKET="127.0.0.1:5039"
            export HDC_SERVER_SOCKET=${2}
            shift 2 ;;
        --rununderdebug)
            RUNUNDERDEBUG=1
            shift 1 ;;
        --cache-builder)
            CACHE_BUILDER=${2}
            shift 2 ;;
        --add-path)
            EXPORT_PATH="PATH=${2}:\${PATH}"
            shift 2 ;;
        --ld-lib-path)
            EXPORT_LD_LIBRARY_PATH="${EXPORT_LD_LIBRARY_PATH}:${2}"
            shift 2 ;;
        --device-tmp)
            DEVICE_TMP="${2}"
            shift 2 ;;
        --isolate-cmd)
            ISOLATE_CMD="${2}"
            shift 2 ;;
        --sandbox-build-dir)
            SANDBOX_BUILD_DIR="${2}"
            shift 2 ;;
        --device-serial)
            device_serial=${2}
            shift 2 ;;
        --do-not-push-clean-files)
            DO_NOT_PUSH_AND_CLEAN_BINARIES=1
            shift ;;
        --redirect-output)
            redirect_output="${2}"
            shift 2 ;;
        --redirect-stdout)
            redirect_stdout="${2}"
            shift 2 ;;
        --redirect-input)
            redirect_input="${2}"
            shift 2 ;;
        --chdir)
            chdir="${2}"
            cd "${chdir}"
            shift 2 ;;
        --summary)
            summary="${2}"
            shift 2 ;;
        --connection)
            CONNECTION_TYPE="${2}"
            shift 2;;
        --ssh-user)
            SSH_USER="${2}"
            shift 2;;
        --ssh-host)
            SSH_HOST="${2}"
            shift 2;;
        --ssh-port)
            SSH_PORT="${2}"
            shift 2;;
        --extra-data)
            extra_data="${extra_data} ${2}"
            shift 2;;
        --extra-link)
            extra_links="${extra_links} ${2}"
            shift 2;;
        --extra-directory)
            extra_dirs="${extra_dirs} ${2}"
            shift 2;;
        --help)
            usage
            exit 0 ;;
        --)
           shift
           break ;;
        *)
           error "Parameter ${1} cannot be parsed."
           usage
           exit 1 ;;
    esac
done

if [[ "${SANDBOX_BUILD_DIR}" && "${SANDBOX_BUILD_DIR}" != /* ]]; then
    error "SANDBOX_BUILD_DIR should be absolute path, got: ${SANDBOX_BUILD_DIR}"
    exit 1
fi

if [[ "${CONNECTION_TYPE}" == "HDC" && ! ( "${HDC_SERVER_SOCKET}" && "${device_serial}" ) ]] ||
   [[ "${CONNECTION_TYPE}" == "ADB" && ! ( "${ADB_SERVER_SOCKET}" && "${device_serial}" ) ]] ||
   [[ "${CONNECTION_TYPE}" == "SSH" && ! ( "${SSH_USER}" && "${SSH_HOST}" && "${SSH_PORT}" ) ]]; then
    error "wrong connection type, or wrong connection type parameters!"
    error "CONNECTION_TYPE=${CONNECTION_TYPE}"
    error "SSH_USER=${SSH_USER}"
    error "SSH_HOST=${SSH_HOST}"
    error "SSH_PORT=${SSH_PORT}"
    error "ADB_SERVER_SOCKET=${ADB_SERVER_SOCKET}"
    error "HDC_SERVER_SOCKET=${HDC_SERVER_SOCKET}"
    error "DEVICE_SERIAL=${device_serial}"
    exit 1
fi

THIS_DIR="$(dirname $0)"
if [[ "${CONNECTION_TYPE}" == "ADB" ]]; then
    export ANDROID_SERIAL="${device_serial}"
    source ${THIS_DIR}/remote_interfaces/i_adb.sh
elif [[ "${CONNECTION_TYPE}" == "SSH" ]]; then
    source ${THIS_DIR}/remote_interfaces/i_ssh.sh
elif [[ "${CONNECTION_TYPE}" == "HDC" ]]; then
    export HDC_DEVICE_SERIAL="${device_serial}"
    source ${THIS_DIR}/remote_interfaces/i_hdc.sh
fi

function dbg_cat_file() {
    if [[ ${RUNUNDERDEBUG} -eq 1 || ${2-} -ne 0 ]]; then
        ls -l ${1}
        info "********************Content of ${1}********************"
        if ! file -L --mime "${1}" | grep -q "binary"; then
            cat_whole_if_less_than_ln_cnt=200
            if [[ $(wc -l < "${1}") -le ${cat_whole_if_less_than_ln_cnt} ]]; then
                cat "${1}"
            else
                # cat head and tail
                txt_ln_cnt=15
                (set -x; head -n ${txt_ln_cnt} "${1}")
                (set -x; tail -n ${txt_ln_cnt} "${1}")
            fi
        else
            if which xxd 1>/dev/null; then
                bin_ln_cnt=64
                xxd "${1}" | head -n ${bin_ln_cnt}
                xxd "${1}" | tail -n ${bin_ln_cnt}
            else
                info "<--binary file content(xxd utility was not found)-->"
            fi
        fi
        info "***********************************************************"
    fi
}

function convert_host_path_to_remote() {
    remote_path_converted="${1}"
    no_check="${2-}"
    if [[ "${SANDBOX_BUILD_DIR}" ]]; then
        if [[ ! "${1}" =~ ${SANDBOX_BUILD_DIR} && -z "${no_check}" ]]; then
            error "The host path not located under SANDBOX_BUILD_DIR"
            error "Host path(${1}) vs SANDBOX_BUILD_DIR(${SANDBOX_BUILD_DIR})"
            exit 1
        fi
        # replace host sandbox build dir on DEVICE_TMP
        remote_path_converted="${DEVICE_TMP}/${1##${SANDBOX_BUILD_DIR}/}"
    else
        # old fashion one level directory structure
        remote_path_converted="${DEVICE_TMP}/${$}/${1##*/}"
    fi
}

function make_remote_link() (
    # Note: we expect, that if link `l` point to local file `f`,
    # then `f` will be present on the remote device
    # on the `remote_path_converted` path.

    target="${1?"link target not set!"}"
    link="${2?"link path not set!"}"
    convert_host_path_to_remote "${target}" "nocheck"
    target_remote="${remote_path_converted}"
    convert_host_path_to_remote "${link}" "nocheck"
    link_remote="${remote_path_converted}"
    remote_shell "ln -s ${target_remote} ${link_remote}"
)

# push executable file and corresponding resources
function push_prog_data() {
    prog_to_test="${1}"
    shift 1
    prog_args="$@"

    # declare test_dir, test_remote_dir; create remote working directory
    test_dir=$(dirname "${prog_to_test}") # full path

    convert_host_path_to_remote "${test_dir}"
    test_remote_dir="${remote_path_converted}"
    if [[ "${DO_NOT_PUSH_AND_CLEAN_BINARIES-}" ]]; then
        # expect that same directory structure as in sandbox already present
        # on device, test_remote_dir point on it, so just return here
        return
    fi

    need_output_dir=
    if [[ -d "${test_dir}/Output" ]]; then
        need_output_dir="Output"
    fi
    remote_shell "mkdir -p ${test_remote_dir}/${need_output_dir}"
    remote_push "${prog_to_test}" "${test_remote_dir}/"

    # parse all args and push if any
    for arg in ${prog_args}; do
        if [[ "${arg}" == "." ]]; then
            continue
        fi
        if [[ -e "${test_dir}/${arg}" ]]; then
            remote_push "${test_dir}/${arg}" "${test_remote_dir}/${arg}"
        fi
    done

    # create extra directories
    for directory in ${extra_dirs}; do
        convert_host_path_to_remote "${directory}" "nocheck"
        remote_shell "mkdir -p ${remote_path_converted}"
    done

    # push all extra files
    for data in ${extra_data}; do

        all_files="$(find "${data}" -type f)"
        for file in ${all_files}; do
            convert_host_path_to_remote "${file}" "nocheck"
            remote_push "${file}" "${remote_path_converted}"
        done

        all_links="$(find "${data}" -type l)"
        for link in ${all_links}; do
            real_path="$(realpath "${link}")"
            make_remote_link "${real_path}" "${link}"
        done

    done

    # create extra links
    for target_link in ${extra_links}; do
        target="$(echo "${target_link}" | cut -d: -f1)"
        link="$(echo "${target_link}" | cut -d: -f2)"
        make_remote_link "${target}" "${link}"
    done
}

function remote_clean_up() {
    if [[ "${SANDBOX_BUILD_DIR}" || "${DO_NOT_PUSH_AND_CLEAN_BINARIES-}" ]]
    then
        # do not delete in case of SANDBOX_BUILD_DIR because some other tests
        # can be in that dir - LNT should delete whole tmp directory at the end
        return
    fi
    one_str="${@}"
    remote_shell "rm -rf ${one_str}"
}

function check_wrapper_file_on_device() {
    if [[ "${DO_NOT_PUSH_AND_CLEAN_BINARIES-}" ]]; then
        return
    fi
    # check present, print exit code (remote hdc/adb run may not return
    # exit code)
    if [[ $(remote_shell 'test -f '"${2}"'; echo $?') -eq 1 ]]; then
        # there is no wrapper file on device yet -> push it
        remote_push "${1}" "${2}"
        remote_shell "chmod +x ${2}"
    fi
}

function device_shell_in_dir() {
    local cmd=
    if [[ ${2} ]]; then
        cmd="cd ${2} && "
    fi

    cmd+="${1:?Command is not specified}"
    remote_shell "${cmd}"
}

function run_on_device_wrapper() {
    device_shell_in_dir "${ISOLATE_CMD} ${1}" "${2}"
}

tmp=
for path in $(echo "${EXPORT_LD_LIBRARY_PATH}" | tr ':' ' '); do
    if [[ ! "${path}" =~ "${DEVICE_TMP}" ]]; then
        convert_host_path_to_remote "${path}"
        path="${remote_path_converted}"
    fi
    tmp="${tmp}:${path}"
done
EXPORT_LD_LIBRARY_PATH="LD_LIBRARY_PATH=${tmp}"

if [[ ${RUNUNDERDEBUG} -eq 1 ]]; then
    info "rununder: CONNECTION_TYPE : ${CONNECTION_TYPE}"
    info "rununder: SSH_USER        : ${SSH_USER}"
    info "rununder: SSH_HOST        : ${SSH_HOST}"
    info "rununder: SSH_PORT        : ${SSH_PORT}"
    info "rununder: DEVICE TMP      : ${DEVICE_TMP}"
    info "rununder: CACHE_BUILDER   : ${CACHE_BUILDER}"
    info "rununder: ADB CLIENT      : ${ADB_SERVER_SOCKET}"
    info "rununder: HDC CLIENT      : ${HDC_SERVER_SOCKET}"
    info "rununder: LD_PATH         : ${EXPORT_LD_LIBRARY_PATH}"
    info "rununder: ENV VARS:"
    set
fi

function run_on_device() {
    prog_to_test="$(realpath ${1})"
    shift 1
    prog_args="$@"

    push_prog_data "${prog_to_test}" "${prog_args}"
    prog_to_test_on_device="${test_remote_dir}/$(basename ${prog_to_test})"

    cmd_input_redirect=""
    if [[ "${redirect_input}" != "/dev/null" ]]; then
        convert_host_path_to_remote "${redirect_input}"
        redirect_input_remote="${remote_path_converted}"
        remote_push "${redirect_input}" "${redirect_input_remote}"
        cmd_input_redirect=" < ${redirect_input_remote} "
    fi
    cmd_output_redirect=""
    if [[ "${redirect_output}" ]]; then
        convert_host_path_to_remote "${redirect_output}"
        redirect_output_remote="${remote_path_converted}"
        cmd_output_redirect="> ${redirect_output_remote} 2>&1 "
    fi
    cmd_stdout_redirect=""
    if [[ "${redirect_stdout}" ]]; then
        convert_host_path_to_remote "${redirect_stdout}"
        redirect_stdout_remote="${remote_path_converted}"
        cmd_stdout_redirect="> ${redirect_stdout_remote} "
    fi
    cmd_summary=""
    if [[ "${summary}" ]]; then
        convert_host_path_to_remote "${summary}"
        summary_remote="${remote_path_converted}"
        cmd_summary="; echo $? >${summary_remote} "
    fi
    cmd_chdir=""
    if [[ "${chdir}" ]]; then
        convert_host_path_to_remote "${chdir}"
        chdir_remote=${remote_path_converted}
        cmd_chdir="cd ${chdir_remote} && "
    fi

    cmd_str="${cmd_chdir}"
    cmd_str+="env ${EXPORT_LD_LIBRARY_PATH} ${EXPORT_PATH} "
    cmd_str+="${prog_to_test_on_device} ${prog_args}"
    cmd_str+="${cmd_input_redirect} ${cmd_output_redirect} ${cmd_stdout_redirect}"
    cmd_str+="${cmd_summary}"

    info "${cmd_str}"
    run_on_device_wrapper "${cmd_str}" "${test_remote_dir}"

    if [[ "${summary}" ]]; then
        remote_pull "${summary_remote}" "${summary}"
        dbg_cat_file "${summary}"
    fi
    if [[ "${redirect_output}" ]]; then
        rm -f "${redirect_output}"
        remote_pull "${redirect_output_remote}" "${redirect_output}"
        dbg_cat_file "${redirect_output}"
    fi
    if [[ "${redirect_stdout}" ]]; then
        rm -f "${redirect_stdout}"
        remote_pull "${redirect_stdout_remote}" "${redirect_stdout}"
        dbg_cat_file "${redirect_stdout}"
    fi
    remote_clean_up "${test_remote_dir}"
}

run_on_device "$@"
