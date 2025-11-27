// RUN: %clang_hwasan -mllvm -hwasan-instrument-without-TLS=true -mllvm -hwasan-use-after-scope -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

// REQUIRES: stable-runtime
// REQUIRES: ohos_family

// Test emutls stack allocation records printing functionality with deep call stack
// This test verifies that emutls frame records are properly recorded
// and displayed in error reports when there are multiple levels of function calls.
// The test simulates a realistic call stack with 30 levels of function calls.

#include <stdlib.h>
#include <sanitizer/hwasan_interface.h>

// External declaration for the emutls frame record function
extern void __hwasan_add_emutls_frame_record(unsigned long long frame_record_info);

// Base frame record info
#define BASE_FRAME_RECORD 0x123456789ABCDEF0ULL

// Forward declarations for all level functions
void level1_entry(void);
void level2_function_a(int param);
void level3_function_b(int param, char *buf);
void level4_function_c(int param, char *buf, double *arr);
void level5_function_d(int param, char *buf, int *arr);
void level6_function_e(int param, char *buf, long long *arr);
void level7_function_f(int param, char *buf, float *arr);
void level8_function_g(int param, char *buf, double *arr);
void level9_function_h(int param, char *buf, int *arr);
void level10_function_i(int param, char *buf, long long *arr);
void level11_function_j(int param, char *buf, float *arr);
void level12_function_k(int param, char *buf, double *arr);
void level13_function_l(int param, char *buf, int *arr);
void level14_function_m(int param, char *buf, long long *arr);
void level15_function_n(int param, char *buf, float *arr);
void level16_function_o(int param, char *buf, double *arr);
void level17_function_p(int param, char *buf, long long *arr);
void level18_function_q(int param, char *buf, int *arr);
void level19_function_r(int param, char *buf, float *arr);
void level20_function_s(int param, char *buf, double *arr);
void level21_function_t(int param, char *buf, long long *arr);
void level22_function_u(int param, char *buf, int *arr);
void level23_function_v(int param, char *buf, float *arr);
void level24_function_w(int param, char *buf, double *arr);
void level25_function_x(int param, char *buf, long long *arr);
void level26_function_y(int param, char *buf, int *arr);
void level27_function_z(int param, char *buf, float *arr);
void level28_function_aa(int param, char *buf, double *arr);
void level29_function_bb(int param, char *buf, long long *arr);
void level30_trigger_error();

// Forward declaration for helper function
void perform_memory_operations(char *buf, int size);

// Helper functions to simulate complex operations
__attribute__((noinline))
static int compute_hash(int value, int multiplier) {
  // Simple hash computation to add complexity
  int hash = value;
  hash = hash * multiplier + 0x5A5A5A5A;
  hash = hash ^ (hash >> 16);
  hash = hash * 0x9E3779B9;
  return hash;
}

__attribute__((noinline))
static void initialize_buffer(char *buf, int size, char base_char) {
  // Initialize buffer with pattern
  for (int i = 0; i < size; i++) {
    buf[i] = base_char + (i % 26);
  }
}

__attribute__((noinline))
static void process_array(int *arr, int size, int offset) {
  // Process array elements
  for (int i = 0; i < size; i++) {
    arr[i] = (arr[i] + offset) * 2;
  }
}

__attribute__((noinline))
static double compute_sum(double *arr, int size) {
  // Compute sum of array elements
  double sum = 0.0;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum;
}

__attribute__((noinline))
static long long compute_product(long long *arr, int size) {
  // Compute product of array elements
  long long product = 1;
  for (int i = 0; i < size && i < 100; i++) {  // Limit to avoid overflow
    product *= (arr[i] + 1);
  }
  return product;
}

// Level 1: Entry point function
__attribute__((noinline))
void level1_entry() {
  // Add emutls frame record for level 1
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x01);
  
  // Local variables to simulate stack usage
  int local_var1 = 0x11111111;
  char buffer1[20];
  int array1[10];
  double values1[5];
  
  // Initialize buffers and arrays
  initialize_buffer(buffer1, 20, 'A');
  for (int i = 0; i < 10; i++) {
    array1[i] = i * 2;
  }
  for (int i = 0; i < 5; i++) {
    values1[i] = i * 1.5;
  }
  
  // Perform some computations
  local_var1 = compute_hash(local_var1, 0x11);
  process_array(array1, 10, 0x10);
  double sum1 = compute_sum(values1, 5);
  (void)sum1;  // Suppress unused variable warning
  
  // Call next level
  level2_function_a(local_var1);
}

// Level 2: Function A
__attribute__((noinline))
void level2_function_a(int param) {
  // Add emutls frame record for level 2
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x02);
  
  // Local variables
  int local_var2 = param + 0x22222222;
  char buffer2[20];
  int array2[15];
  float values2[10];
  
  // Initialize data structures
  initialize_buffer(buffer2, 20, 'B');
  for (int i = 0; i < 15; i++) {
    array2[i] = i * 3;
  }
  for (int i = 0; i < 10; i++) {
    values2[i] = i * 2.5f;
  }
  
  // Perform computations
  local_var2 = compute_hash(local_var2, 0x22);
  process_array(array2, 15, 0x20);
  
  // Call next level
  level3_function_b(local_var2, buffer2);
}

// Level 3: Function B
__attribute__((noinline))
void level3_function_b(int param, char *buf) {
  // Add emutls frame record for level 3
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x03);
  
  // Local variables
  int local_var3 = param * 2;
  char buffer3[20];
  double array3[15];
  long long large_array3[20];
  
  // Initialize all data structures
  initialize_buffer(buffer3, 20, 'C');
  for (int i = 0; i < 15; i++) {
    array3[i] = i * 3.14159;
  }
  for (int i = 0; i < 20; i++) {
    large_array3[i] = i * 5LL;
  }
  
  // Perform complex computations
  local_var3 = compute_hash(local_var3, 0x33);
  double sum3 = compute_sum(array3, 15);
  long long prod3 = compute_product(large_array3, 20);
  (void)sum3;  // Suppress unused variable warning
  (void)prod3;  // Suppress unused variable warning
  
  // Call next level
  level4_function_c(local_var3, buffer3, array3);
}

// Level 4: Function C
__attribute__((noinline))
void level4_function_c(int param, char *buf, double *arr) {
  // Add emutls frame record for level 4
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x04);
  
  // Local variables
  int local_var4 = param + 0x44444444;
  char buffer4[20];
  int array4[20];
  float values4[20];
  double matrix4[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer4, 20, 'D');
  for (int i = 0; i < 20; i++) {
    array4[i] = i * 4;
  }
  for (int i = 0; i < 20; i++) {
    values4[i] = i * 4.0f;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix4[i][j] = i * 10.0 + j;
    }
  }
  
  // Perform computations
  local_var4 = compute_hash(local_var4, 0x44);
  process_array(array4, 20, 0x40);
  double sum4 = compute_sum((double*)matrix4, 100);
  (void)sum4;  // Suppress unused variable warning
  
  // Call next level
  level5_function_d(local_var4, buffer4, array4);
}

// Level 5: Function D
__attribute__((noinline))
void level5_function_d(int param, char *buf, int *arr) {
  // Add emutls frame record for level 5
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x05);
  
  // Local variables
  int local_var5 = param - 0x55555555;
  char buffer5[20];
  long long array5[20];
  int nested_array5[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer5, 20, 'E');
  for (int i = 0; i < 20; i++) {
    array5[i] = i * 5LL;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      nested_array5[i][j] = i * 10 + j;
    }
  }
  
  // Perform complex computations
  local_var5 = compute_hash(local_var5, 0x55);
  long long prod5 = compute_product(array5, 20);
  process_array((int*)nested_array5, 100, 0x50);
  (void)prod5;  // Suppress unused variable warning
  
  // Call next level
  level6_function_e(local_var5, buffer5, array5);
}

// Level 6: Function E
__attribute__((noinline))
void level6_function_e(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 6
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x06);
  
  // Local variables
  int local_var6 = param ^ 0x66666666;
  char buffer6[20];
  float array6[20];
  
  for (int i = 0; i < 20; i++) {
    buffer6[i] = 'F' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array6[i] = i * 6.0f;
  }
  
  // Call next level
  level7_function_f(local_var6, buffer6, array6);
}

// Level 7: Function F
__attribute__((noinline))
void level7_function_f(int param, char *buf, float *arr) {
  // Add emutls frame record for level 7
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x07);
  
  // Local variables
  int local_var7 = param | 0x77777777;
  char buffer7[20];
  double array7[20];
  
  for (int i = 0; i < 20; i++) {
    buffer7[i] = 'G' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array7[i] = i * 7.0;
  }
  
  // Call next level
  level8_function_g(local_var7, buffer7, array7);
}

// Level 8: Function G
__attribute__((noinline))
void level8_function_g(int param, char *buf, double *arr) {
  // Add emutls frame record for level 8
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x08);
  
  // Local variables
  int local_var8 = param & 0x88888888;
  char buffer8[20];
  int array8[20];
  
  for (int i = 0; i < 20; i++) {
    buffer8[i] = 'H' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array8[i] = i * 8;
  }
  
  // Call next level
  level9_function_h(local_var8, buffer8, array8);
}

// Level 9: Function H
__attribute__((noinline))
void level9_function_h(int param, char *buf, int *arr) {
  // Add emutls frame record for level 9
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x09);
  
  // Local variables
  int local_var9 = param << 1;
  char buffer9[20];
  long long array9[20];
  
  for (int i = 0; i < 20; i++) {
    buffer9[i] = 'I' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array9[i] = i * 9LL;
  }
  
  // Call next level
  level10_function_i(local_var9, buffer9, array9);
}

// Level 10: Function I
__attribute__((noinline))
void level10_function_i(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 10
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0A);
  
  // Local variables
  int local_var10 = param >> 1;
  char buffer10[20];
  float array10[20];
  
  for (int i = 0; i < 20; i++) {
    buffer10[i] = 'J' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array10[i] = i * 10.0f;
  }
  
  // Call next level
  level11_function_j(local_var10, buffer10, array10);
}

// Level 11: Function J
__attribute__((noinline))
void level11_function_j(int param, char *buf, float *arr) {
  // Add emutls frame record for level 11
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0B);
  
  // Local variables
  int local_var11 = param + 0x0B0B0B0B;
  char buffer11[20];
  double array11[20];
  
  for (int i = 0; i < 20; i++) {
    buffer11[i] = 'K' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array11[i] = i * 11.0;
  }
  
  // Call next level
  level12_function_k(local_var11, buffer11, array11);
}

// Level 12: Function K
__attribute__((noinline))
void level12_function_k(int param, char *buf, double *arr) {
  // Add emutls frame record for level 12
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0C);
  
  // Local variables
  int local_var12 = param - 0x0C0C0C0C;
  char buffer12[20];
  int array12[20];
  
  for (int i = 0; i < 20; i++) {
    buffer12[i] = 'L' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array12[i] = i * 12;
  }
  
  // Call next level
  level13_function_l(local_var12, buffer12, array12);
}

// Level 13: Function L
__attribute__((noinline))
void level13_function_l(int param, char *buf, int *arr) {
  // Add emutls frame record for level 13
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0D);
  
  // Local variables
  int local_var13 = param ^ 0x0D0D0D0D;
  char buffer13[20];
  long long array13[20];
  
  for (int i = 0; i < 20; i++) {
    buffer13[i] = 'M' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array13[i] = i * 13LL;
  }
  
  // Call next level
  level14_function_m(local_var13, buffer13, array13);
}

// Level 14: Function M
__attribute__((noinline))
void level14_function_m(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 14
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0E);
  
  // Local variables
  int local_var14 = param | 0x0E0E0E0E;
  char buffer14[20];
  float array14[20];
  
  for (int i = 0; i < 20; i++) {
    buffer14[i] = 'N' + (i % 26);
  }
  for (int i = 0; i < 20; i++) {
    array14[i] = i * 14.0f;
  }
  
  // Call next level
  level15_function_n(local_var14, buffer14, array14);
}

// Level 15: Function N
__attribute__((noinline))
void level15_function_n(int param, char *buf, float *arr) {
  // Add emutls frame record for level 15
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x0F);
  
  // Local variables
  int local_var15 = param & 0x0F0F0F0F;
  char buffer15[20];
  double array15[20];
  int matrix15[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer15, 20, 'O');
  for (int i = 0; i < 20; i++) {
    array15[i] = i * 15.0;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix15[i][j] = i * 10 + j;
    }
  }
  
  // Perform computations
  local_var15 = compute_hash(local_var15, 0x0F);
  double sum15 = compute_sum(array15, 20);
  process_array((int*)matrix15, 100, 0xF0);
  (void)sum15;  // Suppress unused variable warning
  
  // Call next level
  level16_function_o(local_var15, buffer15, array15);
}

// Level 16: Function O
__attribute__((noinline))
void level16_function_o(int param, char *buf, double *arr) {
  // Add emutls frame record for level 16
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x10);
  
  // Local variables
  int local_var16 = param + 0x10101010;
  char buffer16[20];
  long long array16[20];
  float values16[20];
  
  // Initialize all data structures
  initialize_buffer(buffer16, 20, 'P');
  for (int i = 0; i < 20; i++) {
    array16[i] = i * 16LL;
  }
  for (int i = 0; i < 20; i++) {
    values16[i] = i * 16.0f;
  }
  
  // Perform computations
  local_var16 = compute_hash(local_var16, 0x10);
  long long prod16 = compute_product(array16, 20);
  perform_memory_operations(buffer16, 20);
  (void)prod16;  // Suppress unused variable warning
  
  // Call next level
  level17_function_p(local_var16, buffer16, array16);
}

// Level 17: Function P
__attribute__((noinline))
void level17_function_p(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 17
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x11);
  
  // Local variables
  int local_var17 = param - 0x11111111;
  char buffer17[20];
  int array17[20];
  double matrix17[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer17, 20, 'Q');
  for (int i = 0; i < 20; i++) {
    array17[i] = i * 17;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix17[i][j] = i * 10.0 + j;
    }
  }
  
  // Perform computations
  local_var17 = compute_hash(local_var17, 0x11);
  process_array(array17, 20, 0x110);
  double sum17 = compute_sum((double*)matrix17, 100);
  (void)sum17;  // Suppress unused variable warning
  
  // Call next level
  level18_function_q(local_var17, buffer17, array17);
}

// Level 18: Function Q
__attribute__((noinline))
void level18_function_q(int param, char *buf, int *arr) {
  // Add emutls frame record for level 18
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x12);
  
  // Local variables
  int local_var18 = param ^ 0x12121212;
  char buffer18[20];
  float array18[20];
  long long nested18[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer18, 20, 'R');
  for (int i = 0; i < 20; i++) {
    array18[i] = i * 18.0f;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      nested18[i][j] = i * 10LL + j;
    }
  }
  
  // Perform computations
  local_var18 = compute_hash(local_var18, 0x12);
  long long prod18 = compute_product((long long*)nested18, 100);
  perform_memory_operations(buffer18, 20);
  (void)prod18;  // Suppress unused variable warning
  
  // Call next level
  level19_function_r(local_var18, buffer18, array18);
}

// Level 19: Function R
__attribute__((noinline))
void level19_function_r(int param, char *buf, float *arr) {
  // Add emutls frame record for level 19
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x13);
  
  // Local variables
  int local_var19 = param | 0x13131313;
  char buffer19[20];
  double array19[20];
  int matrix19[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer19, 20, 'S');
  for (int i = 0; i < 20; i++) {
    array19[i] = i * 19.0;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix19[i][j] = i * 10 + j;
    }
  }
  
  // Perform computations
  local_var19 = compute_hash(local_var19, 0x13);
  double sum19 = compute_sum(array19, 20);
  process_array((int*)matrix19, 100, 0x130);
  (void)sum19;  // Suppress unused variable warning
  
  // Call next level
  level20_function_s(local_var19, buffer19, array19);
}

// Level 20: Function S
__attribute__((noinline))
void level20_function_s(int param, char *buf, double *arr) {
  // Add emutls frame record for level 20
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x14);
  
  // Local variables
  int local_var20 = param << 2;
  char buffer20[20];
  long long array20[20];
  float values20[20];
  
  // Initialize all data structures
  initialize_buffer(buffer20, 20, 'T');
  for (int i = 0; i < 20; i++) {
    array20[i] = i * 20LL;
  }
  for (int i = 0; i < 20; i++) {
    values20[i] = i * 20.0f;
  }
  
  // Perform computations
  local_var20 = compute_hash(local_var20, 0x14);
  long long prod20 = compute_product(array20, 20);
  perform_memory_operations(buffer20, 20);
  (void)prod20;  // Suppress unused variable warning
  
  // Call next level
  level21_function_t(local_var20, buffer20, array20);
}

// Level 21: Function T
__attribute__((noinline))
void level21_function_t(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 21
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x15);
  
  // Local variables
  int local_var21 = param >> 2;
  char buffer21[20];
  int array21[20];
  double matrix21[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer21, 20, 'U');
  for (int i = 0; i < 20; i++) {
    array21[i] = i * 21;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix21[i][j] = i * 10.0 + j;
    }
  }
  
  // Perform computations
  local_var21 = compute_hash(local_var21, 0x15);
  process_array(array21, 20, 0x150);
  double sum21 = compute_sum((double*)matrix21, 100);
  (void)sum21;  // Suppress unused variable warning
  
  // Call next level
  level22_function_u(local_var21, buffer21, array21);
}

// Level 22: Function U
__attribute__((noinline))
void level22_function_u(int param, char *buf, int *arr) {
  // Add emutls frame record for level 22
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x16);
  
  // Local variables
  int local_var22 = param + 0x16161616;
  char buffer22[20];
  float array22[20];
  long long nested22[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer22, 20, 'V');
  for (int i = 0; i < 20; i++) {
    array22[i] = i * 22.0f;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      nested22[i][j] = i * 10LL + j;
    }
  }
  
  // Perform computations
  local_var22 = compute_hash(local_var22, 0x16);
  long long prod22 = compute_product((long long*)nested22, 100);
  perform_memory_operations(buffer22, 20);
  (void)prod22;  // Suppress unused variable warning
  
  // Call next level
  level23_function_v(local_var22, buffer22, array22);
}

// Level 23: Function V
__attribute__((noinline))
void level23_function_v(int param, char *buf, float *arr) {
  // Add emutls frame record for level 23
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x17);
  
  // Local variables
  int local_var23 = param - 0x17171717;
  char buffer23[20];
  double array23[20];
  int matrix23[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer23, 20, 'W');
  for (int i = 0; i < 20; i++) {
    array23[i] = i * 23.0;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix23[i][j] = i * 10 + j;
    }
  }
  
  // Perform computations
  local_var23 = compute_hash(local_var23, 0x17);
  double sum23 = compute_sum(array23, 20);
  process_array((int*)matrix23, 100, 0x170);
  (void)sum23;  // Suppress unused variable warning
  
  // Call next level
  level24_function_w(local_var23, buffer23, array23);
}

// Level 24: Function W
__attribute__((noinline))
void level24_function_w(int param, char *buf, double *arr) {
  // Add emutls frame record for level 24
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x18);
  
  // Local variables
  int local_var24 = param ^ 0x18181818;
  char buffer24[20];
  long long array24[20];
  float values24[20];
  
  // Initialize all data structures
  initialize_buffer(buffer24, 20, 'X');
  for (int i = 0; i < 20; i++) {
    array24[i] = i * 24LL;
  }
  for (int i = 0; i < 20; i++) {
    values24[i] = i * 24.0f;
  }
  
  // Perform computations
  local_var24 = compute_hash(local_var24, 0x18);
  long long prod24 = compute_product(array24, 20);
  perform_memory_operations(buffer24, 20);
  (void)prod24;  // Suppress unused variable warning
  
  // Call next level
  level25_function_x(local_var24, buffer24, array24);
}

// Level 25: Function X
__attribute__((noinline))
void level25_function_x(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 25
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x19);
  
  // Local variables
  int local_var25 = param | 0x19191919;
  char buffer25[20];
  int array25[20];
  double matrix25[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer25, 20, 'Y');
  for (int i = 0; i < 20; i++) {
    array25[i] = i * 25;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix25[i][j] = i * 10.0 + j;
    }
  }
  
  // Perform computations
  local_var25 = compute_hash(local_var25, 0x19);
  process_array(array25, 20, 0x190);
  double sum25 = compute_sum((double*)matrix25, 100);
  (void)sum25;  // Suppress unused variable warning
  
  // Call next level
  level26_function_y(local_var25, buffer25, array25);
}

// Level 26: Function Y
__attribute__((noinline))
void level26_function_y(int param, char *buf, int *arr) {
  // Add emutls frame record for level 26
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x1A);
  
  // Local variables
  int local_var26 = param & 0x1A1A1A1A;
  char buffer26[20];
  float array26[20];
  long long nested26[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer26, 20, 'Z');
  for (int i = 0; i < 20; i++) {
    array26[i] = i * 26.0f;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      nested26[i][j] = i * 10LL + j;
    }
  }
  
  // Perform computations
  local_var26 = compute_hash(local_var26, 0x1A);
  long long prod26 = compute_product((long long*)nested26, 100);
  perform_memory_operations(buffer26, 20);
  (void)prod26;  // Suppress unused variable warning
  
  // Call next level
  level27_function_z(local_var26, buffer26, array26);
}

// Level 27: Function Z
__attribute__((noinline))
void level27_function_z(int param, char *buf, float *arr) {
  // Add emutls frame record for level 27
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x1B);
  
  // Local variables
  int local_var27 = param << 3;
  char buffer27[20];
  double array27[20];
  int matrix27[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer27, 20, 'a');
  for (int i = 0; i < 20; i++) {
    array27[i] = i * 27.0;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix27[i][j] = i * 10 + j;
    }
  }
  
  // Perform computations
  local_var27 = compute_hash(local_var27, 0x1B);
  double sum27 = compute_sum(array27, 20);
  process_array((int*)matrix27, 100, 0x1B0);
  (void)sum27;  // Suppress unused variable warning
  
  // Call next level
  level28_function_aa(local_var27, buffer27, array27);
}

// Level 28: Function AA
__attribute__((noinline))
void level28_function_aa(int param, char *buf, double *arr) {
  // Add emutls frame record for level 28
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x1C);
  
  // Local variables
  int local_var28 = param >> 3;
  char buffer28[20];
  long long array28[20];
  float values28[20];
  
  // Initialize all data structures
  initialize_buffer(buffer28, 20, 'b');
  for (int i = 0; i < 20; i++) {
    array28[i] = i * 28LL;
  }
  for (int i = 0; i < 20; i++) {
    values28[i] = i * 28.0f;
  }
  
  // Perform computations
  local_var28 = compute_hash(local_var28, 0x1C);
  long long prod28 = compute_product(array28, 20);
  perform_memory_operations(buffer28, 20);
  (void)prod28;  // Suppress unused variable warning
  
  // Call next level
  level29_function_bb(local_var28, buffer28, array28);
}

// Level 29: Function BB
__attribute__((noinline))
void level29_function_bb(int param, char *buf, long long *arr) {
  // Add emutls frame record for level 29
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x1D);
  
  // Local variables
  int local_var29 = param + 0x1D1D1D1D;
  char buffer29[20];
  int array29[20];
  double matrix29[10][10];
  
  // Initialize all data structures
  initialize_buffer(buffer29, 20, 'c');
  for (int i = 0; i < 20; i++) {
    array29[i] = i * 29;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      matrix29[i][j] = i * 10.0 + j;
    }
  }
  
  // Perform computations
  local_var29 = compute_hash(local_var29, 0x1D);
  process_array(array29, 20, 0x1D0);
  double sum29 = compute_sum((double*)matrix29, 100);
  (void)sum29;  // Suppress unused variable warning
  
  // Call next level - this will trigger the error
  level30_trigger_error();
}

// Level 30: Function CC - This function triggers the error
__attribute__((noinline))
void level30_trigger_error() {
  // Add emutls frame record for level 30
  __hwasan_add_emutls_frame_record(BASE_FRAME_RECORD + 0x1E);
  
  int* arr;
  {
    int local_arr[10];
    arr = local_arr;
  }
  arr[0] = 1;
}

// Additional utility functions for testing
__attribute__((noinline))
static void validate_stack_depth(int expected_depth) {
  // This function can be used to validate stack depth if needed
  volatile int depth = expected_depth;
  (void)depth;  // Suppress unused variable warning
}

__attribute__((noinline))
void perform_memory_operations(char *buf, int size) {
  // Perform various memory operations to test stack tracking
  for (int i = 0; i < size; i++) {
    buf[i] = (char)(buf[i] ^ 0xAA);
  }
  for (int i = 0; i < size / 2; i++) {
    char temp = buf[i];
    buf[i] = buf[size - 1 - i];
    buf[size - 1 - i] = temp;
  }
}

__attribute__((noinline))
static int calculate_fibonacci(int n) {
  // Calculate Fibonacci number recursively (inefficient but tests stack)
  if (n <= 1) return n;
  return calculate_fibonacci(n - 1) + calculate_fibonacci(n - 2);
}

int main() {
  // Initialize test environment
  validate_stack_depth(30);
  
  // Start the deep call stack
  // This will create a 30-level deep call stack, each level adding
  // emutls frame records and performing various stack operations.
  // The final level will trigger a buffer overflow error to test
  // the error reporting with deep stack traces.
  level1_entry();
  
  // CHECK: Previously allocated frames with emutls:
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}}  {{.*}}
  return 0;  
}  