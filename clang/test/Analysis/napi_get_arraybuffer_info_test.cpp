//===----------------------------------------------------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony %s

#include "napi_include.h"
#include <cstddef>
#include <cstdlib>

void test_bad_case_1() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len);
  delete data; // expected-warning{{The variable is not allowed to be released}}
}

void test_bad_case_2() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len);
  free(data); // expected-warning{{The variable is not allowed to be released}}
}

void test_good_case_1() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len); // no-warning
}
