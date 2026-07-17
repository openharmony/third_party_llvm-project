import os

# TODO: Move this to CMake.
HDC = os.environ.get("HDC", "hdc")
# Set HDC_SERVER_IP_PORT and HDC_UTID to pass remote connection options when
# needed.
HDC_SERVER_IP_PORT = os.environ.get("HDC_SERVER_IP_PORT")
HDC_UTID = os.environ.get("HDC_UTID")
TMPDIR = os.environ.get("OHOS_REMOTE_TMP_DIR", "/data/local/tmp/Output")
DYN_LINKER = os.environ.get("OHOS_REMOTE_DYN_LINKER")


def get_hdc_cmd_prefix():
    server = ["-s", HDC_SERVER_IP_PORT] if HDC_SERVER_IP_PORT else []
    device = ["-t", HDC_UTID] if HDC_UTID else []
    return [HDC, *server, *device]
