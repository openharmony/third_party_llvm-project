
import os

# TODO: move this to the cmake ?
HDC = os.environ.get('HDC', 'hdc')
# Please set "HDC_SERVER_IP_PORT" and "HDC_UTID" environment variables
# to pass options for connecting to remote device if needed
HDC_SERVER_IP_PORT = os.environ.get('HDC_SERVER_IP_PORT')
HDC_UTID = os.environ.get('HDC_UTID')
TMPDIR = os.environ.get('OHOS_REMOTE_TMP_DIR', '/data/local/tmp/Output')
DYN_LINKER = os.environ.get('OHOS_REMOTE_DYN_LINKER')

def get_hdc_cmd_prefix():
    server = ['-s', HDC_SERVER_IP_PORT] if HDC_SERVER_IP_PORT else []
    device = ['-t', HDC_UTID] if HDC_UTID else []
    return [HDC, *server, *device]
