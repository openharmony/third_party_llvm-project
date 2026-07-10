#include "napi_include.h"

void test_bad_case_1() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_value jsCallback = nullptr;
        jsCallback = 0;
        delete work;
      });
}

void test_bad_case_2() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_value args[1] = {nullptr};
        args[0] = 0;
        delete work;
      });
}

void test_bad_case_3() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value jsCallback = nullptr;
        jsCallback = 0;
        delete work;
      });
}

void test_bad_case_4() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value args[1] = {nullptr};
        args[0] = 0;
        delete work;
      });
}

void test_bad_case_5() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        delete work;
      });
}

void test_bad_case_6() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value jsCallback = nullptr;
        jsCallback = 0;
        napi_close_handle_scope(env_, scope);
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}

void test_bad_case_7() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value args[1] = {nullptr};
        args[0] = 0;
        napi_close_handle_scope(env_, scope);
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}

void test_good_case_1() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value jsCallback = nullptr;
        jsCallback = 0;
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}

void test_good_case_2() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        napi_value args[1] = {nullptr};
        args[0] = 0;
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}

void test_good_case_3() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_status stat = napi_open_handle_scope(env_, &scope);
        if (stat != napi_ok) {
          return;
        }
        napi_value args[1] = {nullptr};
        args[0] = 0;
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}

void test_good_case_4() {
  uv_loop_s *loop = nullptr;
  uv_work_t *work = new uv_work_t;
  uv_queue_work(
      loop, work, [](uv_work_t *work) {},
      [](uv_work_t *work, int status) {
        napi_env env_ = nullptr;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env_, &scope);
        if (scope == nullptr) {
          return;
        }
        napi_value args[1] = {nullptr};
        args[0] = 0;
        napi_close_handle_scope(env_, scope);
        delete work;
      });
}