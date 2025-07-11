// arkshim.c
#include <stddef.h>
#include <string.h>

struct DebugInput {
    size_t size;
    char *data;
};

static const char kBt[]  = "This is a ArkTS backtrace";
static const char kMsg[] = "This is a ArkTS operate debug message result";

__attribute__((visibility("default")))
struct DebugInput GetJsBacktrace(void) {
    struct DebugInput out;
    out.size = strlen(kBt) + 1;
    out.data = (char *)kBt;
    return out;
}

__attribute__((visibility("default")))
struct DebugInput OperateJsDebugMessage(const char *msg) {
    (void)msg;
    struct DebugInput out;
    out.size = strlen(kMsg) + 1;
    out.data = (char *)kMsg;
    return out;
}
