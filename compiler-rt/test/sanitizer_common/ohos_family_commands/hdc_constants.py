
import os

# TODO: move this to the cmake ?
HDC = os.environ.get('HDC', 'hdc')
HDC_SERVER_IP_PORT = os.environ.get('HDC_SERVER_IP_PORT')
HDC_UTID = os.environ.get('HDC_UTID')
TMPDIR = os.environ.get('OHOS_REMOTE_TMP_DIR', '/data/local/tmp/Output')

# emit warning on import if some required constants are not set
if not HDC_SERVER_IP_PORT or not HDC_UTID:
    print('Please set "HDC_SERVER_IP_PORT" and "HDC_UTID" environment variables '
          'to be able to debug on remote device')

def get_hdc_cmd_prefix():
    server = ['-s', HDC_SERVER_IP_PORT] if HDC_SERVER_IP_PORT else []
    device = ['-t', HDC_UTID] if HDC_UTID else []
    return [HDC, *server, *device]
