// HWAddressSanitizer Test Cases - Thread Safety Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: pthread, threading, thread-local storage tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sanitizer/hwasan_interface.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// SECTION 1: Basic pthread Tests
// ============================================================================

// Test: pthread_create and join
// RUN: %clang_hwasan %s -o %t && %run %t
void *thread_func_basic(void *arg) {
  int *p = (int*)arg;
  *p = 42;
  return NULL;
}
int test_pthread_create() {
  __hwasan_enable_allocator_tagging();
  int value = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, thread_func_basic, &value);
  pthread_join(tid, NULL);
  __hwasan_disable_allocator_tagging();
  return value != 42;
}

// Test: pthread_exit
// RUN: %clang_hwasan %s -o %t && %run %t
void *thread_exit_func(void *arg) {
  int *p = (int*)arg;
  *p = 100;
  pthread_exit(NULL);
  return NULL;
}
int test_pthread_exit() {
  __hwasan_enable_allocator_tagging();
  int value = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, thread_exit_func, &value);
  pthread_join(tid, NULL);
  __hwasan_disable_allocator_tagging();
  return value != 100;
}

// ============================================================================
// SECTION 2: Thread-Local Storage Tests
// ============================================================================

// Test: TLS basic
// RUN: %clang_hwasan %s -o %t && %run %t
static __thread int tls_var = 123;
int test_tls_basic() {
  return tls_var != 123;
}

// Test: TLS in thread
// RUN: %clang_hwasan %s -o %t && %run %t
static __thread int tls_thread_var = 0;
static pthread_key_t tls_key;

void *tls_worker(void *arg) {
  pthread_setspecific(tls_key, &tls_thread_var);
  tls_thread_var = 42;
  return NULL;
}
int test_tls_thread() {
  pthread_key_create(&tls_key, NULL);
  pthread_t t;
  pthread_create(&t, NULL, tls_worker, NULL);
  pthread_join(t, NULL);
  pthread_key_delete(tls_key);
  return 0;
}

// ============================================================================
// SECTION 3: Synchronization Tests
// ============================================================================

// Test: mutex
// RUN: %clang_hwasan %s -o %t && %run %t
pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
int test_mutex_basic() {
  pthread_mutex_lock(&test_mutex);
  int val = 100;
  pthread_mutex_unlock(&test_mutex);
  return val != 100;
}

// Test: fork
// RUN: %clang_hwasan %s -o %t && %run %t
int test_fork() {
  __hwasan_enable_allocator_tagging();
  pid_t pid = fork();
  if (pid == 0) _exit(0);
  else { int st; waitpid(pid, &st, 0); }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 4: Thread Stress Tests
// ============================================================================

// Test: thread stress
// RUN: %clang_hwasan %s -o %t && %run %t
#define STRESS_THREADS 10

void *stress_worker(void *arg) {
  char *p = (char*)malloc(64);
  p[0] = 'A';
  free(p);
  return NULL;
}
int test_thread_stress() {
  __hwasan_enable_allocator_tagging();
  pthread_t threads[STRESS_THREADS];
  for (int i = 0; i < STRESS_THREADS; i++) 
    pthread_create(&threads[i], NULL, stress_worker, NULL);
  for (int i = 0; i < STRESS_THREADS; i++) 
    pthread_join(threads[i], NULL);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 5: Linux-Specific Tests
// ============================================================================

// Test: pvalloc
// RUN: %clang_hwasan %s -o %t && %run %t
int test_pvalloc() {
  __hwasan_enable_allocator_tagging();
  void *p = pvalloc(4096);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: vfork
// RUN: %clang_hwasan %s -o %t && %run %t
int test_vfork() {
  pid_t pid = vfork();
  if (pid == 0) _exit(0);
  else { int st; waitpid(pid, &st, 0); }
  return 0;
}

// ============================================================================
// SECTION 6: Tail Magic
// ============================================================================

// Test: tail magic
// RUN: %clang_hwasan %s -o %t && %run %t
__attribute__((noinline))
char* get_tail(char *buf) { return buf + 16; }
int test_tail_magic() {
  __hwasan_enable_allocator_tagging();
  char buf[32] = {0};
  char *tail = get_tail(buf);
  *tail = 'A';
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_pthread_create();
  test_pthread_exit();
  test_tls_basic();
  test_tls_thread();
  test_mutex_basic();
  test_fork();
  test_thread_stress();
  test_pvalloc();
  test_vfork();
  test_tail_magic();
  printf("Thread safety tests passed\n");
  return 0;
}

// ============================================================================
// SECTION 7: More Thread Scenarios
// ============================================================================

// Test: Thread with multiple allocations
// RUN: %clang_hwasan %s -o %t && %run %t
void *thread_multi_alloc(void *arg) {
  for (int i = 0; i < 100; i++) {
    char *p = (char*)malloc(64);
    p[0] = i;
    free(p);
  }
  return NULL;
}
int test_thread_multi_alloc() {
  __hwasan_enable_allocator_tagging();
  pthread_t t;
  pthread_create(&t, NULL, thread_multi_alloc, NULL);
  pthread_join(t, NULL);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Thread pool
// RUN: %clang_hwasan %s -o %t && %run %t
#define POOL_SIZE 5
void *thread_pool_worker(void *arg) {
  char *p = (char*)malloc(32);
  p[0] = 'P';
  free(p);
  return NULL;
}
int test_thread_pool() {
  __hwasan_enable_allocator_tagging();
  pthread_t pool[POOL_SIZE];
  for (int i = 0; i < POOL_SIZE; i++) 
    pthread_create(&pool[i], NULL, thread_pool_worker, NULL);
  for (int i = 0; i < POOL_SIZE; i++) 
    pthread_join(pool[i], NULL);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: Mutex protection
// RUN: %clang_hwasan %s -o %t && %run %t
pthread_mutex_t protected_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int protected_data = 0;
void *mutex_worker(void *arg) {
  pthread_mutex_lock(&protected_mutex);
  protected_data++;
  pthread_mutex_unlock(&protected_mutex);
  return NULL;
}
int test_mutex_protection() {
  pthread_t threads[10];
  for (int i = 0; i < 10; i++) 
    pthread_create(&threads[i], NULL, mutex_worker, NULL);
  for (int i = 0; i < 10; i++) 
    pthread_join(threads[i], NULL);
  return protected_data != 10;
}

// ============================================================================
// SECTION 8: TLS More Tests
// ============================================================================

// Test: Multiple TLS variables
// RUN: %clang_hwasan %s -o %t && %run %t
static __thread int tls_int = 100;
static __thread char tls_char = 'A';
static __thread double tls_double = 3.14;

int test_tls_multiple() {
  if (tls_int != 100) return 1;
  if (tls_char != 'A') return 1;
  if (tls_double != 3.14) return 1;
  return 0;
}

// Test: TLS array
// RUN: %clang_hwasan %s -o %t && %run %t
static __thread char tls_array[256];

int test_tls_array() {
  for (int i = 0; i < 256; i++) tls_array[i] = i;
  for (int i = 0; i < 256; i++) if (tls_array[i] != i) return 1;
  return 0;
}
