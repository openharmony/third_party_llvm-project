//===----------------------------------------------------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony %s

#include "napi_include.h"

void test_bad_case_1() {
    napi_env env = nullptr;
    napi_value thisArg = nullptr;
    void* cbInfo = nullptr;
    napi_ref* res = new napi_ref[5];
    napi_wrap(env, thisArg, cbInfo, [](napi_env env, void* data, void* hint) {}, nullptr, res); // expected-warning{{The last parameter of napi_wrap is not null}}
}

void test_good_case_1() {
    napi_env env = nullptr;
    napi_value thisArg = nullptr;
    void* cbInfo = nullptr;
    napi_wrap(env, thisArg, cbInfo, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
}