#include <stdio.h>

volatile int value = VALUE;

__attribute__((noinline)) void marker() { printf("%d\n", value); }
