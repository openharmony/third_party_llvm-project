// HWAddressSanitizer Test Cases - Platform-Specific Category
// ============================================================================
// RUN: %clang_hwasan %s -o %t
// RUN: %run %t
// ============================================================================
// Classification: Linux and POSIX-specific tests
// Source: toolchain/llvm-project/compiler-rt/test/hwasan/TestCases/

#include <stdlib.h>
#include <assert.h>
#include <sanitizer/hwasan_interface.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

// ============================================================================
// SECTION 1: Linux Allocation Tests
// ============================================================================

// Test: aligned_alloc (Linux)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_aligned_alloc_linux() {
  __hwasan_enable_allocator_tagging();
  for (int a = 16; a <= 256; a *= 2) {
    void *p = aligned_alloc(a, a * 2);
    if (p) { assert(((size_t)p % a) == 0); free(p); }
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// Test: pvalloc (Linux)
// RUN: %clang_hwasan %s -o %t && %run %t
int test_pvalloc_linux() {
  __hwasan_enable_allocator_tagging();
  void *p = pvalloc(4096);
  if (p) free(p);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 2: Fork Tests
// ============================================================================

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

// Test: vfork
// RUN: %clang_hwasan %s -o %t && %run %t
int test_vfork() {
  pid_t pid = vfork();
  if (pid == 0) _exit(0);
  else { int st; waitpid(pid, &st, 0); }
  return 0;
}

// ============================================================================
// SECTION 3: pthread Tests
// ============================================================================

// Test: pthread_create
// RUN: %clang_hwasan %s -o %t && %run %t
void *thread_func(void *arg) {
  int *p = (int*)arg;
  *p = 42;
  return NULL;
}
int test_pthread_create() {
  __hwasan_enable_allocator_tagging();
  int value = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, thread_func, &value);
  pthread_join(tid, NULL);
  __hwasan_disable_allocator_tagging();
  return value != 42;
}

// ============================================================================
// SECTION 4: Thread Stress
// ============================================================================

// Test: Thread stress
// RUN: %clang_hwasan %s -o %t && %run %t
void *stress_worker(void *arg) {
  char *p = (char*)malloc(64);
  p[0] = 'A';
  free(p);
  return NULL;
}
int test_thread_stress() {
  __hwasan_enable_allocator_tagging();
  pthread_t threads[10];
  for (int i = 0; i < 10; i++) pthread_create(&threads[i], NULL, stress_worker, NULL);
  for (int i = 0; i < 10; i++) pthread_join(threads[i], NULL);
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// SECTION 5: Process Maps
// ============================================================================

// Test: /proc/self/maps
// RUN: %clang_hwasan %s -o %t && %run %t
int test_proc_maps() {
  FILE *f = fopen("/proc/self/maps", "r");
  if (f) {
    char line[1024];
    while (fgets(line, sizeof(line), f)) {}
    fclose(f);
  }
  return 0;
}

// ============================================================================
// SECTION 6: POSIX Memalign
// ============================================================================

// Test: posix_memalign
// RUN: %clang_hwasan %s -o %t && %run %t
int test_posix_memalign() {
  __hwasan_enable_allocator_tagging();
  for (int a = 16; a <= 256; a *= 2) {
    void *p = NULL;
    int ret = posix_memalign(&p, a, a * 2);
    if (ret == 0 && p) free(p);
  }
  __hwasan_disable_allocator_tagging();
  return 0;
}

// ============================================================================
// Main
// ============================================================================

int main() {
  test_aligned_alloc_linux();
  test_pvalloc_linux();
  test_fork();
  test_vfork();
  test_pthread_create();
  test_thread_stress();
  test_proc_maps();
  test_posix_memalign();
  printf("Platform-specific tests passed\n");
  return 0;
}

// ============================================================================
// SECTION 7: More Linux Tests
// ============================================================================

// Test: Multiple fork
// RUN: %clang_hwasan %s -o %t && %run %t
int test_multi_fork() {
  for (int i = 0; i < 3; i++) {
    pid_t pid = fork();
    if (pid == 0) _exit(0);
    else { int st; waitpid(pid, &st, 0); }
  }
  return 0;
}

// Test: Fork with allocation
// RUN: %clang_hwasan %s -o %t && %run %t
int test_fork_alloc() {
  pid_t pid = fork();
  if (pid == 0) {
    char *p = (char*)malloc(64);
    p[0] = 'F';
    free(p);
    _exit(0);
  } else {
    int st;
    waitpid(pid, &st, 0);
  }
  return 0;
}

// Test: Pipe operations
// RUN: %clang_hwasan %s -o %t && %run %t
#include <fcntl.h>
int test_pipe() {
  int pipefd[2];
  if (pipe(pipefd) == -1) return 0; // Skip if no pipe support
  char buf[32] = "test";
  write(pipefd[1], buf, 32);
  read(pipefd[0], buf, 32);
  close(pipefd[0]);
  close(pipefd[1]);
  return 0;
}

// ============================================================================
// SECTION 8: More Thread Tests
// ============================================================================

// Test: Detached thread
// RUN: %clang_hwasan %s -o %t && %run %t
void *detached_worker(void *arg) {
  char *p = (char*)malloc(32);
  p[0] = 'D';
  free(p);
  return NULL;
}
int test_detached_thread() {
  pthread_t t;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&t, &attr, detached_worker, NULL);
  pthread_attr_destroy(&attr);
  sleep(1);
  return 0;
}

// Test: Thread cancellation
// RUN: %clang_hwasan %s -o %t && %run %t
void *cancel_worker(void *arg) {
  while(1) {
    char *p = (char*)malloc(16);
    free(p);
  }
  return NULL;
}
int test_cancel_thread() {
  pthread_t t;
  pthread_create(&t, NULL, cancel_worker, NULL);
  usleep(1000);
  pthread_cancel(t);
  pthread_join(t, NULL);
  return 0;
}
