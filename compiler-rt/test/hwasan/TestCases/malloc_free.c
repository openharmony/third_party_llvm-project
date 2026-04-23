// RUN: %clang_hwasan %s -o %t
// RUN: %env_hwasan_opts="heap_history_size_main_thread=128:heap_history_block_max_num_main_thread=200:heap_history_size=64:heap_history_block_max_num=100" %run %t | FileCheck %s
// CHECK: rb:({{[0-9]+}}/8192/25600)
// CHECK: rb:({{[0-9]+}}/64/6400)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define NUM_THREADS 4
#define ITERATIONS 16
#define ALLOC_SIZE 64

void* thread_task(void* arg) {
  int id = *(int*)arg;
  void* ptr;
  for (int i= 0; i < ITERATIONS; i++) {
    ptr = malloc(ALLOC_SIZE);
    if (!ptr) exit(1);
    *((volatile char*)ptr) = (char)id;
    free(ptr);
  }
  // UAF
  if (id == NUM_THREADS - 1)
    *((char*)ptr) = 1;
  return NULL;
}

int main() {
  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, thread_task, &thread_ids[i]);
  }
  void* p;
  for (int i = 0; i< ITERATIONS; i++) {
    p = malloc(ALLOC_SIZE);
    *((volatile char*)p) = 0xFF;
    free(p);
  }
  *((char*)p) = 1;

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}