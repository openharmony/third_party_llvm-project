// RUN: %clangxx -O0 %s -fsanitize=address -o %t && %run %t 2>&1 | FileCheck %s
// RUN: %clangxx -O3 %s -fsanitize=address -o %t && %run %t 2>&1 | FileCheck %s

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_ring_buffer.h"
#include <stdio.h>

#define BLOCK_LEN 8
#define BLOCK_NUM 10
#define BLOCK_SIZE (BLOCK_LEN * BLOCK_NUM)

using namespace __asan;
int main()
{
  auto RBL = RingBufferLink<long long>::New(BLOCK_LEN, BLOCK_NUM);

  printf("#input:\n");
  for (int i = 0; i < BLOCK_LEN; i++) {
    RBL->push(i);
    printf(" %lu/%lu,", RBL->realsize(), RBL->size());
  }
// CHECK: 1/8, 2/8, 3/8, 4/8, 5/8, 6/8, 7/8, 8/8,

  printf("\n#input-expaned:\n");
  for (int i = BLOCK_LEN; i < BLOCK_SIZE; i++) {
    RBL->push(i);
    if (i % BLOCK_LEN == 0) {
        printf(" %lu/%lu,", RBL->realsize(),RBL->size());
    }
  }
// CHECK: 9/16, 17/32, 25/32, 33/64, 41/64, 49/64, 57/64, 65/80, 73/80,
  printf("\n#all:\n");
  for (int i = 0;i < BLOCK_SIZE; i++) {
    printf("%lld,", (*RBL)[i]);
  }
// CHECK: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  printf("\n#input-full:\n");
  for (int i = 0; i < BLOCK_SIZE/2; i++) {
    RBL->push(-i);
  }
  for(int i = 0; i < BLOCK_SIZE; i++) {
    printf("%lld,", (*RBL)[i]);
  }
// CHECK: 40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15,-16,-17,-18,-19,-20,-21,-22,-23,-24,-25,-26,-27,-28,-29,-30,-31,-32,-33,-34,-35,-36,-37,-38,-39,
  RBL->Delete();
  printf("ok\n");
// CHECK: ok
}