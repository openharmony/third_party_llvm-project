#include "napi_include.h"

void test_bad_case_1() {
    napi_env env = nullptr;
    napi_value thisArg = nullptr;
    void* cbInfo = nullptr;
    napi_ref* res = new napi_ref[5];
    napi_wrap(env, thisArg, cbInfo, [](napi_env env, void* data, void* hint) {}, nullptr, res);
}

void test_good_case_1() {
    napi_env env = nullptr;
    napi_value thisArg = nullptr;
    void* cbInfo = nullptr;
    napi_wrap(env, thisArg, cbInfo, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
}