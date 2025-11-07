// arkshim.c
#include <stddef.h>
#include <string.h>

// new interface check
struct DebugResponse {
    size_t size;
    char *response;
};

static const char kBt[]  = "This is a ArkTS backtrace";
static const char kMsg[] = "This is a ArkTS operate debug message result";

__attribute__((visibility("default")))
struct DebugResponse GetJsBacktraceV1(void) {
    struct DebugResponse out;
    out.size = strlen(kBt) + 1;
    out.response = (char *)kBt;
    return out;
}

__attribute__((visibility("default")))
struct DebugResponse OperateJsDebugMessageV1(const char *msg) {
    (void)msg;
    struct DebugResponse out;
    out.size = strlen(kMsg) + 1;
    out.response = (char *)kMsg;
    return out;
}
