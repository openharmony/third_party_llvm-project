#include "napi_include.h"
#include <cstddef>
#include <cstdlib>

void test_bad_case_1() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len);
  delete data;
}

void test_bad_case_2() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len);
  free(data);
}

void test_good_case_1() {
  napi_env env = nullptr;
  napi_value Args = nullptr;
  void *data = nullptr;
  size_t len = 0;
  napi_get_arraybuffer_info(env, Args, &data, &len);
}
