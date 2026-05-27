/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #include <dlfcn.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 
 #define GWP_ASAN_FFRT_DLOPEN_ALLOC_SIZE 32
 #define GWP_ASAN_LOG_DIR "/data/local/tmp/"
 #define GWP_ASAN_LOG_TAG "gwp_asan"
 #define GWP_ASAN_TEST_MARKER __attribute__((noinline, used))
 
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
 #define GWP_ASAN_FFRT_DLOPEN_CASE_NAME "gwp_asan_ffrt_dlopen_recoverable_coroutine_uaf"
 #define GWP_ASAN_FFRT_DLOPEN_RECOVERABLE_PARAM \
     "gwp_asan.recoverable.app.gwp_asan_ffrt_dlopen_recoverable_unwind_marker_test"
 #else
 #define GWP_ASAN_FFRT_DLOPEN_CASE_NAME "gwp_asan_ffrt_dlopen_coroutine_uaf"
 #endif
 
 typedef void (*ffrt_function_t)(void *);
 typedef void *ffrt_queue_t;
 typedef void *ffrt_task_handle_t;
 typedef bool (*may_init_gwp_asan_fn)(bool);
 typedef bool (*libc_gwp_asan_ptr_is_mine_fn)(void *);
 
 typedef struct {
     uint32_t storage[(128 + sizeof(uint32_t) - 1) / sizeof(uint32_t)];
 } ffrt_queue_attr_t;
 
 typedef struct {
     uint32_t storage[(128 + sizeof(uint32_t) - 1) / sizeof(uint32_t)];
 } ffrt_task_attr_t;
 
 typedef enum {
     ffrt_queue_serial = 0,
     ffrt_queue_concurrent = 1,
     ffrt_queue_max = 2,
 } ffrt_queue_type_t;
 
 struct ffrt_api {
     int (*queue_attr_init)(ffrt_queue_attr_t *);
     void (*queue_attr_destroy)(ffrt_queue_attr_t *);
     ffrt_queue_t (*queue_create)(ffrt_queue_type_t, const char *, const ffrt_queue_attr_t *);
     void (*queue_destroy)(ffrt_queue_t);
     int (*task_attr_init)(ffrt_task_attr_t *);
     void (*task_attr_destroy)(ffrt_task_attr_t *);
     ffrt_task_handle_t (*queue_submit_h_f)(ffrt_queue_t, ffrt_function_t, void *, const ffrt_task_attr_t *);
     void (*queue_wait)(ffrt_task_handle_t);
     void (*task_handle_destroy)(ffrt_task_handle_t);
     bool (*get_current_coroutine_stack)(void **, size_t *);
 };
 
 struct gwp_asan_api {
     may_init_gwp_asan_fn may_init;
     libc_gwp_asan_ptr_is_mine_fn ptr_is_mine;
 };
 
 static struct ffrt_api g_ffrt;
 static struct gwp_asan_api g_gwp_asan;
 static volatile char g_gwp_asan_uaf_sink;
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
 static volatile int g_gwp_asan_recovered_after_fault;
 #endif
 
 static void gwp_asan_restore_params(void)
 {
     (void)system("param set gwp_asan.sample.all false");
     (void)system("param set gwp_asan.log.path default");
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
     (void)system("param set " GWP_ASAN_FFRT_DLOPEN_RECOVERABLE_PARAM " false");
 #endif
 }
 
 static int gwp_asan_load_libc_api(void)
 {
     g_gwp_asan.may_init = (may_init_gwp_asan_fn)dlsym(RTLD_DEFAULT, "may_init_gwp_asan");
     g_gwp_asan.ptr_is_mine = (libc_gwp_asan_ptr_is_mine_fn)dlsym(RTLD_DEFAULT, "libc_gwp_asan_ptr_is_mine");
     if (g_gwp_asan.may_init == NULL || g_gwp_asan.ptr_is_mine == NULL) {
         fprintf(stderr, "SKIP: current libc does not export required GWP-ASan test symbols.\n");
         return 0;
     }
     return 1;
 }
 
 static int gwp_asan_standalone_prepare_dynamic(const char *case_name)
 {
     (void)system("param set gwp_asan.sample.all true");
     (void)system("param set gwp_asan.log.path file");
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
     (void)system("param set " GWP_ASAN_FFRT_DLOPEN_RECOVERABLE_PARAM " true");
 #endif
 
     printf("[%s] pid=%d\n", case_name, getpid());
     printf("[%s] report path: %s%s.*.%d.log\n", case_name, GWP_ASAN_LOG_DIR, GWP_ASAN_LOG_TAG, getpid());
     fflush(stdout);
 
     if (!gwp_asan_load_libc_api()) {
         return 0;
     }
     if (!g_gwp_asan.may_init(true)) {
         fprintf(stderr, "[%s] SKIP: may_init_gwp_asan(true) failed, rerun this case.\n", case_name);
         gwp_asan_restore_params();
         return 0;
     }
     return 1;
 }
 
 static int gwp_asan_check_sampled_dynamic(void *ptr, const char *case_name)
 {
     if (ptr == NULL) {
         fprintf(stderr, "[%s] FAIL: malloc returned NULL.\n", case_name);
         return 0;
     }
     if (!g_gwp_asan.ptr_is_mine(ptr)) {
         fprintf(stderr, "[%s] FAIL: allocation is not owned by GWP-ASan.\n", case_name);
         return 0;
     }
     return 1;
 }
 
 static void *gwp_asan_must_dlsym(void *handle, const char *name)
 {
     void *symbol = dlsym(handle, name);
     if (symbol == NULL) {
         fprintf(stderr, "missing FFRT symbol: %s\n", name);
         exit(2);
     }
     return symbol;
 }
 
 static int gwp_asan_load_ffrt(void)
 {
     void *handle = dlopen("libffrt.so", RTLD_NOW);
     if (handle == NULL) {
         handle = dlopen("/system/lib64/ndk/libffrt.so", RTLD_NOW);
     }
     if (handle == NULL) {
         fprintf(stderr, "failed to load libffrt: %s\n", dlerror());
         return 0;
     }
 
     g_ffrt.queue_attr_init = (int (*)(ffrt_queue_attr_t *))gwp_asan_must_dlsym(handle, "ffrt_queue_attr_init");
     g_ffrt.queue_attr_destroy =
         (void (*)(ffrt_queue_attr_t *))gwp_asan_must_dlsym(handle, "ffrt_queue_attr_destroy");
     g_ffrt.queue_create = (ffrt_queue_t(*)(ffrt_queue_type_t, const char *, const ffrt_queue_attr_t *))
         gwp_asan_must_dlsym(handle, "ffrt_queue_create");
     g_ffrt.queue_destroy = (void (*)(ffrt_queue_t))gwp_asan_must_dlsym(handle, "ffrt_queue_destroy");
     g_ffrt.task_attr_init = (int (*)(ffrt_task_attr_t *))gwp_asan_must_dlsym(handle, "ffrt_task_attr_init");
     g_ffrt.task_attr_destroy =
         (void (*)(ffrt_task_attr_t *))gwp_asan_must_dlsym(handle, "ffrt_task_attr_destroy");
     g_ffrt.queue_submit_h_f =
         (ffrt_task_handle_t(*)(ffrt_queue_t, ffrt_function_t, void *, const ffrt_task_attr_t *))
         gwp_asan_must_dlsym(handle, "ffrt_queue_submit_h_f");
     g_ffrt.queue_wait = (void (*)(ffrt_task_handle_t))gwp_asan_must_dlsym(handle, "ffrt_queue_wait");
     g_ffrt.task_handle_destroy =
         (void (*)(ffrt_task_handle_t))gwp_asan_must_dlsym(handle, "ffrt_task_handle_destroy");
     g_ffrt.get_current_coroutine_stack =
         (bool (*)(void **, size_t *))gwp_asan_must_dlsym(handle, "ffrt_get_current_coroutine_stack");
     return 1;
 }
 
 GWP_ASAN_TEST_MARKER void *GwpAsanFfrtDlopenAllocMarker(void)
 {
     return malloc(GWP_ASAN_FFRT_DLOPEN_ALLOC_SIZE);
 }
 
 GWP_ASAN_TEST_MARKER void GwpAsanFfrtDlopenFreeMarker(void *ptr)
 {
     free(ptr);
 }
 
 GWP_ASAN_TEST_MARKER void GwpAsanFfrtDlopenFaultMarker(char *ptr)
 {
     g_gwp_asan_uaf_sink = *(volatile char *)ptr;
 }
 
 static int gwp_asan_current_frame_in_ffrt_stack(void)
 {
     void *stack_addr = NULL;
     size_t stack_size = 0;
     if (!g_ffrt.get_current_coroutine_stack(&stack_addr, &stack_size)) {
         fprintf(stderr, "[%s] FAIL: coroutine stack unavailable.\n", GWP_ASAN_FFRT_DLOPEN_CASE_NAME);
         return 0;
     }
     if (stack_addr == NULL || stack_size == 0) {
         fprintf(stderr, "[%s] FAIL: empty coroutine stack range.\n", GWP_ASAN_FFRT_DLOPEN_CASE_NAME);
         return 0;
     }
 
     uintptr_t stack_start = (uintptr_t)stack_addr;
     if (stack_start > UINTPTR_MAX - stack_size) {
         fprintf(stderr, "[%s] FAIL: coroutine stack range overflows.\n", GWP_ASAN_FFRT_DLOPEN_CASE_NAME);
         return 0;
     }
     uintptr_t stack_end = stack_start + stack_size;
     uintptr_t frame = (uintptr_t)__builtin_frame_address(0);
     if (frame < stack_start || frame >= stack_end) {
         fprintf(stderr, "[%s] FAIL: frame %p is not in FFRT stack [%p, %p).\n",
                 GWP_ASAN_FFRT_DLOPEN_CASE_NAME, (void *)frame, (void *)stack_start, (void *)stack_end);
         return 0;
     }
     return 1;
 }
 
 static void gwp_asan_ffrt_dlopen_coroutine_task(void *arg)
 {
     (void)arg;
     if (!gwp_asan_current_frame_in_ffrt_stack()) {
         exit(3);
     }
 
     char *ptr = (char *)GwpAsanFfrtDlopenAllocMarker();
     if (!gwp_asan_check_sampled_dynamic(ptr, GWP_ASAN_FFRT_DLOPEN_CASE_NAME)) {
         free(ptr);
         exit(4);
     }
 
     GwpAsanFfrtDlopenFreeMarker(ptr);
     GwpAsanFfrtDlopenFaultMarker(ptr);
 
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
     fprintf(stderr, "AFTER_FFRT_RECOVERABLE_UAF\n");
     g_gwp_asan_recovered_after_fault = 1;
     return;
 #else
     fprintf(stderr, "[gwp_asan_ffrt_dlopen_coroutine_uaf] FAIL: UAF read did not trigger GWP-ASan.\n");
     exit(5);
 #endif
 }
 
 int main(void)
 {
     const char *case_name = GWP_ASAN_FFRT_DLOPEN_CASE_NAME;
     if (!gwp_asan_load_ffrt()) {
         return 2;
     }
     if (!gwp_asan_standalone_prepare_dynamic(case_name)) {
         return 0;
     }
 
     ffrt_queue_attr_t queue_attr;
     ffrt_task_attr_t task_attr;
     if (g_ffrt.queue_attr_init(&queue_attr) != 0) {
         fprintf(stderr, "[%s] FAIL: ffrt_queue_attr_init failed.\n", case_name);
         return 1;
     }
     if (g_ffrt.task_attr_init(&task_attr) != 0) {
         fprintf(stderr, "[%s] FAIL: ffrt_task_attr_init failed.\n", case_name);
         g_ffrt.queue_attr_destroy(&queue_attr);
         return 1;
     }
 
     ffrt_queue_t queue = g_ffrt.queue_create(ffrt_queue_serial, case_name, &queue_attr);
     if (queue == NULL) {
         fprintf(stderr, "[%s] FAIL: ffrt_queue_create failed.\n", case_name);
         g_ffrt.task_attr_destroy(&task_attr);
         g_ffrt.queue_attr_destroy(&queue_attr);
         return 1;
     }
 
     ffrt_task_handle_t task = g_ffrt.queue_submit_h_f(queue, gwp_asan_ffrt_dlopen_coroutine_task, NULL, &task_attr);
     if (task == NULL) {
         fprintf(stderr, "[%s] FAIL: ffrt_queue_submit_h_f failed.\n", case_name);
         g_ffrt.queue_destroy(queue);
         g_ffrt.task_attr_destroy(&task_attr);
         g_ffrt.queue_attr_destroy(&queue_attr);
         return 1;
     }
 
     g_ffrt.queue_wait(task);
     g_ffrt.task_handle_destroy(task);
     g_ffrt.queue_destroy(queue);
     g_ffrt.task_attr_destroy(&task_attr);
     g_ffrt.queue_attr_destroy(&queue_attr);
 
 #ifdef GWP_ASAN_FFRT_DLOPEN_RECOVERABLE
     if (g_gwp_asan_recovered_after_fault) {
         gwp_asan_restore_params();
         return 0;
     }
     fprintf(stderr, "[%s] FAIL: FFRT task returned without recoverable GWP-ASan fault.\n", case_name);
     gwp_asan_restore_params();
     return 1;
 #else
     fprintf(stderr, "[%s] FAIL: FFRT task returned without GWP-ASan SIGSEGV.\n", case_name);
     return 1;
 #endif
 }
 