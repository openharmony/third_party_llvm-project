// REQUIRES: gwp_asan
// This test ensures that normal allocation/memory access/deallocation works
// as expected and we didn't accidentally break the supporting allocator.

// RUN: %clangxx_gwp_asan %s -o %t
// RUN: %env_scudo_options=GWP_ASAN_MaxSimultaneousAllocations=1 %run %t
// RUN: %env_scudo_options=GWP_ASAN_MaxSimultaneousAllocations=2 %run %t
// RUN: %env_scudo_options=GWP_ASAN_MaxSimultaneousAllocations=11 %run %t
// RUN: %env_scudo_options=GWP_ASAN_MaxSimultaneousAllocations=12 %run %t
// RUN: %env_scudo_options=GWP_ASAN_MaxSimultaneousAllocations=13 %run %t

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
using namespace std;

// perform many legitimate computations to make the program 'real'
// We will invoke many unique filler functions (they are real trivial computations).
int gwp_asan_fill_func_1(int a, int b) {
    int x = a * 6 + b * 8 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_1 alive
static volatile int keep_alive_1 = 3;
int gwp_asan_fill_func_2(int a, int b) {
    int x = a * 7 + b * 9 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_2 alive
static volatile int keep_alive_2 = 3;
int gwp_asan_fill_func_3(int a, int b) {
    int x = a * 8 + b * 10 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_3 alive
static volatile int keep_alive_3 = 3;
int gwp_asan_fill_func_4(int a, int b) {
    int x = a * 9 + b * 11 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_4 alive
static volatile int keep_alive_4 = 3;
int gwp_asan_fill_func_5(int a, int b) {
    int x = a * 10 + b * 12 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_5 alive
static volatile int keep_alive_5 = 3;
int gwp_asan_fill_func_6(int a, int b) {
    int x = a * 11 + b * 13 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_6 alive
static volatile int keep_alive_6 = 3;
int gwp_asan_fill_func_7(int a, int b) {
    int x = a * 12 + b * 14 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_7 alive
static volatile int keep_alive_7 = 3;
int gwp_asan_fill_func_8(int a, int b) {
    int x = a * 13 + b * 15 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_8 alive
static volatile int keep_alive_8 = 3;
int gwp_asan_fill_func_9(int a, int b) {
    int x = a * 14 + b * 16 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_9 alive
static volatile int keep_alive_9 = 3;
int gwp_asan_fill_func_10(int a, int b) {
    int x = a * 15 + b * 17 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_10 alive
static volatile int keep_alive_10 = 3;
int gwp_asan_fill_func_11(int a, int b) {
    int x = a * 16 + b * 18 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_11 alive
static volatile int keep_alive_11 = 3;
int gwp_asan_fill_func_12(int a, int b) {
    int x = a * 17 + b * 19 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_12 alive
static volatile int keep_alive_12 = 3;
int gwp_asan_fill_func_13(int a, int b) {
    int x = a * 18 + b * 20 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_13 alive
static volatile int keep_alive_13 = 3;
int gwp_asan_fill_func_14(int a, int b) {
    int x = a * 19 + b * 21 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_14 alive
static volatile int keep_alive_14 = 3;
int gwp_asan_fill_func_15(int a, int b) {
    int x = a * 20 + b * 22 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_15 alive
static volatile int keep_alive_15 = 3;
int gwp_asan_fill_func_16(int a, int b) {
    int x = a * 21 + b * 23 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_16 alive
static volatile int keep_alive_16 = 3;
int gwp_asan_fill_func_17(int a, int b) {
    int x = a * 22 + b * 7 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_17 alive
static volatile int keep_alive_17 = 3;
int gwp_asan_fill_func_18(int a, int b) {
    int x = a * 23 + b * 8 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_18 alive
static volatile int keep_alive_18 = 3;
int gwp_asan_fill_func_19(int a, int b) {
    int x = a * 24 + b * 9 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_19 alive
static volatile int keep_alive_19 = 3;
int gwp_asan_fill_func_20(int a, int b) {
    int x = a * 25 + b * 10 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_20 alive
static volatile int keep_alive_20 = 3;
int gwp_asan_fill_func_21(int a, int b) {
    int x = a * 26 + b * 11 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_21 alive
static volatile int keep_alive_21 = 3;
int gwp_asan_fill_func_22(int a, int b) {
    int x = a * 27 + b * 12 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_22 alive
static volatile int keep_alive_22 = 3;
int gwp_asan_fill_func_23(int a, int b) {
    int x = a * 5 + b * 13 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_23 alive
static volatile int keep_alive_23 = 3;
int gwp_asan_fill_func_24(int a, int b) {
    int x = a * 6 + b * 14 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_24 alive
static volatile int keep_alive_24 = 3;
int gwp_asan_fill_func_25(int a, int b) {
    int x = a * 7 + b * 15 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_25 alive
static volatile int keep_alive_25 = 3;
int gwp_asan_fill_func_26(int a, int b) {
    int x = a * 8 + b * 16 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_26 alive
static volatile int keep_alive_26 = 3;
int gwp_asan_fill_func_27(int a, int b) {
    int x = a * 9 + b * 17 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_27 alive
static volatile int keep_alive_27 = 3;
int gwp_asan_fill_func_28(int a, int b) {
    int x = a * 10 + b * 18 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_28 alive
static volatile int keep_alive_28 = 3;
int gwp_asan_fill_func_29(int a, int b) {
    int x = a * 11 + b * 19 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_29 alive
static volatile int keep_alive_29 = 3;
int gwp_asan_fill_func_30(int a, int b) {
    int x = a * 12 + b * 20 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_30 alive
static volatile int keep_alive_30 = 3;
int gwp_asan_fill_func_31(int a, int b) {
    int x = a * 13 + b * 21 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_31 alive
static volatile int keep_alive_31 = 3;
int gwp_asan_fill_func_32(int a, int b) {
    int x = a * 14 + b * 22 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_32 alive
static volatile int keep_alive_32 = 3;
int gwp_asan_fill_func_33(int a, int b) {
    int x = a * 15 + b * 23 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_33 alive
static volatile int keep_alive_33 = 3;
int gwp_asan_fill_func_34(int a, int b) {
    int x = a * 16 + b * 7 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_34 alive
static volatile int keep_alive_34 = 3;
int gwp_asan_fill_func_35(int a, int b) {
    int x = a * 17 + b * 8 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_35 alive
static volatile int keep_alive_35 = 3;
int gwp_asan_fill_func_36(int a, int b) {
    int x = a * 18 + b * 9 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_36 alive
static volatile int keep_alive_36 = 3;
int gwp_asan_fill_func_37(int a, int b) {
    int x = a * 19 + b * 10 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_37 alive
static volatile int keep_alive_37 = 3;
int gwp_asan_fill_func_38(int a, int b) {
    int x = a * 20 + b * 11 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_38 alive
static volatile int keep_alive_38 = 3;
int gwp_asan_fill_func_39(int a, int b) {
    int x = a * 21 + b * 12 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_39 alive
static volatile int keep_alive_39 = 3;
int gwp_asan_fill_func_40(int a, int b) {
    int x = a * 22 + b * 13 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_40 alive
static volatile int keep_alive_40 = 3;
int gwp_asan_fill_func_41(int a, int b) {
    int x = a * 23 + b * 14 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_41 alive
static volatile int keep_alive_41 = 3;
int gwp_asan_fill_func_42(int a, int b) {
    int x = a * 24 + b * 15 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_42 alive
static volatile int keep_alive_42 = 3;
int gwp_asan_fill_func_43(int a, int b) {
    int x = a * 25 + b * 16 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_43 alive
static volatile int keep_alive_43 = 3;
int gwp_asan_fill_func_44(int a, int b) {
    int x = a * 26 + b * 17 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_44 alive
static volatile int keep_alive_44 = 3;
int gwp_asan_fill_func_45(int a, int b) {
    int x = a * 27 + b * 18 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_45 alive
static volatile int keep_alive_45 = 3;
int gwp_asan_fill_func_46(int a, int b) {
    int x = a * 5 + b * 19 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_46 alive
static volatile int keep_alive_46 = 3;
int gwp_asan_fill_func_47(int a, int b) {
    int x = a * 6 + b * 20 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_47 alive
static volatile int keep_alive_47 = 3;
int gwp_asan_fill_func_48(int a, int b) {
    int x = a * 7 + b * 21 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_48 alive
static volatile int keep_alive_48 = 3;
int gwp_asan_fill_func_49(int a, int b) {
    int x = a * 8 + b * 22 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_49 alive
static volatile int keep_alive_49 = 3;
int gwp_asan_fill_func_50(int a, int b) {
    int x = a * 9 + b * 23 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_50 alive
static volatile int keep_alive_50 = 3;
int gwp_asan_fill_func_51(int a, int b) {
    int x = a * 10 + b * 7 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_51 alive
static volatile int keep_alive_51 = 3;
int gwp_asan_fill_func_52(int a, int b) {
    int x = a * 11 + b * 8 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_52 alive
static volatile int keep_alive_52 = 3;
int gwp_asan_fill_func_53(int a, int b) {
    int x = a * 12 + b * 9 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_53 alive
static volatile int keep_alive_53 = 3;
int gwp_asan_fill_func_54(int a, int b) {
    int x = a * 13 + b * 10 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_54 alive
static volatile int keep_alive_54 = 3;
int gwp_asan_fill_func_55(int a, int b) {
    int x = a * 14 + b * 11 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_55 alive
static volatile int keep_alive_55 = 3;
int gwp_asan_fill_func_56(int a, int b) {
    int x = a * 15 + b * 12 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_56 alive
static volatile int keep_alive_56 = 3;
int gwp_asan_fill_func_57(int a, int b) {
    int x = a * 16 + b * 13 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_57 alive
static volatile int keep_alive_57 = 3;
int gwp_asan_fill_func_58(int a, int b) {
    int x = a * 17 + b * 14 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_58 alive
static volatile int keep_alive_58 = 3;
int gwp_asan_fill_func_59(int a, int b) {
    int x = a * 18 + b * 15 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_59 alive
static volatile int keep_alive_59 = 3;
int gwp_asan_fill_func_60(int a, int b) {
    int x = a * 19 + b * 16 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_60 alive
static volatile int keep_alive_60 = 3;
int gwp_asan_fill_func_61(int a, int b) {
    int x = a * 20 + b * 17 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_61 alive
static volatile int keep_alive_61 = 3;
int gwp_asan_fill_func_62(int a, int b) {
    int x = a * 21 + b * 18 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_62 alive
static volatile int keep_alive_62 = 3;
int gwp_asan_fill_func_63(int a, int b) {
    int x = a * 22 + b * 19 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_63 alive
static volatile int keep_alive_63 = 3;
int gwp_asan_fill_func_64(int a, int b) {
    int x = a * 23 + b * 20 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_64 alive
static volatile int keep_alive_64 = 3;
int gwp_asan_fill_func_65(int a, int b) {
    int x = a * 24 + b * 21 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_65 alive
static volatile int keep_alive_65 = 3;
int gwp_asan_fill_func_66(int a, int b) {
    int x = a * 25 + b * 22 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_66 alive
static volatile int keep_alive_66 = 3;
int gwp_asan_fill_func_67(int a, int b) {
    int x = a * 26 + b * 23 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_67 alive
static volatile int keep_alive_67 = 3;
int gwp_asan_fill_func_68(int a, int b) {
    int x = a * 27 + b * 7 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_68 alive
static volatile int keep_alive_68 = 3;
int gwp_asan_fill_func_69(int a, int b) {
    int x = a * 5 + b * 8 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_69 alive
static volatile int keep_alive_69 = 3;
int gwp_asan_fill_func_70(int a, int b) {
    int x = a * 6 + b * 9 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_70 alive
static volatile int keep_alive_70 = 3;
int gwp_asan_fill_func_71(int a, int b) {
    int x = a * 7 + b * 10 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_71 alive
static volatile int keep_alive_71 = 3;
int gwp_asan_fill_func_72(int a, int b) {
    int x = a * 8 + b * 11 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_72 alive
static volatile int keep_alive_72 = 3;
int gwp_asan_fill_func_73(int a, int b) {
    int x = a * 9 + b * 12 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_73 alive
static volatile int keep_alive_73 = 3;
int gwp_asan_fill_func_74(int a, int b) {
    int x = a * 10 + b * 13 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_74 alive
static volatile int keep_alive_74 = 3;
int gwp_asan_fill_func_75(int a, int b) {
    int x = a * 11 + b * 14 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_75 alive
static volatile int keep_alive_75 = 3;
int gwp_asan_fill_func_76(int a, int b) {
    int x = a * 12 + b * 15 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_76 alive
static volatile int keep_alive_76 = 3;
int gwp_asan_fill_func_77(int a, int b) {
    int x = a * 13 + b * 16 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_77 alive
static volatile int keep_alive_77 = 3;
int gwp_asan_fill_func_78(int a, int b) {
    int x = a * 14 + b * 17 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_78 alive
static volatile int keep_alive_78 = 3;
int gwp_asan_fill_func_79(int a, int b) {
    int x = a * 15 + b * 18 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_79 alive
static volatile int keep_alive_79 = 3;
int gwp_asan_fill_func_80(int a, int b) {
    int x = a * 16 + b * 19 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_80 alive
static volatile int keep_alive_80 = 3;
int gwp_asan_fill_func_81(int a, int b) {
    int x = a * 17 + b * 20 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_81 alive
static volatile int keep_alive_81 = 3;
int gwp_asan_fill_func_82(int a, int b) {
    int x = a * 18 + b * 21 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_82 alive
static volatile int keep_alive_82 = 3;
int gwp_asan_fill_func_83(int a, int b) {
    int x = a * 19 + b * 22 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_83 alive
static volatile int keep_alive_83 = 3;
int gwp_asan_fill_func_84(int a, int b) {
    int x = a * 20 + b * 23 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_84 alive
static volatile int keep_alive_84 = 3;
int gwp_asan_fill_func_85(int a, int b) {
    int x = a * 21 + b * 7 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_85 alive
static volatile int keep_alive_85 = 3;
int gwp_asan_fill_func_86(int a, int b) {
    int x = a * 22 + b * 8 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_86 alive
static volatile int keep_alive_86 = 3;
int gwp_asan_fill_func_87(int a, int b) {
    int x = a * 23 + b * 9 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_87 alive
static volatile int keep_alive_87 = 3;
int gwp_asan_fill_func_88(int a, int b) {
    int x = a * 24 + b * 10 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_88 alive
static volatile int keep_alive_88 = 3;
int gwp_asan_fill_func_89(int a, int b) {
    int x = a * 25 + b * 11 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_89 alive
static volatile int keep_alive_89 = 3;
int gwp_asan_fill_func_90(int a, int b) {
    int x = a * 26 + b * 12 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_90 alive
static volatile int keep_alive_90 = 3;
int gwp_asan_fill_func_91(int a, int b) {
    int x = a * 27 + b * 13 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_91 alive
static volatile int keep_alive_91 = 3;
int gwp_asan_fill_func_92(int a, int b) {
    int x = a * 5 + b * 14 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_92 alive
static volatile int keep_alive_92 = 3;
int gwp_asan_fill_func_93(int a, int b) {
    int x = a * 6 + b * 15 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_93 alive
static volatile int keep_alive_93 = 3;
int gwp_asan_fill_func_94(int a, int b) {
    int x = a * 7 + b * 16 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_94 alive
static volatile int keep_alive_94 = 3;
int gwp_asan_fill_func_95(int a, int b) {
    int x = a * 8 + b * 17 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_95 alive
static volatile int keep_alive_95 = 3;
int gwp_asan_fill_func_96(int a, int b) {
    int x = a * 9 + b * 18 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_96 alive
static volatile int keep_alive_96 = 3;
int gwp_asan_fill_func_97(int a, int b) {
    int x = a * 10 + b * 19 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_97 alive
static volatile int keep_alive_97 = 3;
int gwp_asan_fill_func_98(int a, int b) {
    int x = a * 11 + b * 20 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_98 alive
static volatile int keep_alive_98 = 3;
int gwp_asan_fill_func_99(int a, int b) {
    int x = a * 12 + b * 21 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_99 alive
static volatile int keep_alive_99 = 3;
int gwp_asan_fill_func_100(int a, int b) {
    int x = a * 13 + b * 22 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_100 alive
static volatile int keep_alive_100 = 3;
int gwp_asan_fill_func_101(int a, int b) {
    int x = a * 14 + b * 23 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_101 alive
static volatile int keep_alive_101 = 3;
int gwp_asan_fill_func_102(int a, int b) {
    int x = a * 15 + b * 7 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_102 alive
static volatile int keep_alive_102 = 3;
int gwp_asan_fill_func_103(int a, int b) {
    int x = a * 16 + b * 8 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_103 alive
static volatile int keep_alive_103 = 3;
int gwp_asan_fill_func_104(int a, int b) {
    int x = a * 17 + b * 9 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_104 alive
static volatile int keep_alive_104 = 3;
int gwp_asan_fill_func_105(int a, int b) {
    int x = a * 18 + b * 10 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_105 alive
static volatile int keep_alive_105 = 3;
int gwp_asan_fill_func_106(int a, int b) {
    int x = a * 19 + b * 11 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_106 alive
static volatile int keep_alive_106 = 3;
int gwp_asan_fill_func_107(int a, int b) {
    int x = a * 20 + b * 12 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_107 alive
static volatile int keep_alive_107 = 3;
int gwp_asan_fill_func_108(int a, int b) {
    int x = a * 21 + b * 13 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_108 alive
static volatile int keep_alive_108 = 3;
int gwp_asan_fill_func_109(int a, int b) {
    int x = a * 22 + b * 14 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_109 alive
static volatile int keep_alive_109 = 3;
int gwp_asan_fill_func_110(int a, int b) {
    int x = a * 23 + b * 15 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_110 alive
static volatile int keep_alive_110 = 3;
int gwp_asan_fill_func_111(int a, int b) {
    int x = a * 24 + b * 16 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_111 alive
static volatile int keep_alive_111 = 3;
int gwp_asan_fill_func_112(int a, int b) {
    int x = a * 25 + b * 17 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_112 alive
static volatile int keep_alive_112 = 3;
int gwp_asan_fill_func_113(int a, int b) {
    int x = a * 26 + b * 18 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_113 alive
static volatile int keep_alive_113 = 3;
int gwp_asan_fill_func_114(int a, int b) {
    int x = a * 27 + b * 19 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_114 alive
static volatile int keep_alive_114 = 3;
int gwp_asan_fill_func_115(int a, int b) {
    int x = a * 5 + b * 20 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_115 alive
static volatile int keep_alive_115 = 3;
int gwp_asan_fill_func_116(int a, int b) {
    int x = a * 6 + b * 21 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_116 alive
static volatile int keep_alive_116 = 3;
int gwp_asan_fill_func_117(int a, int b) {
    int x = a * 7 + b * 22 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_117 alive
static volatile int keep_alive_117 = 3;
int gwp_asan_fill_func_118(int a, int b) {
    int x = a * 8 + b * 23 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_118 alive
static volatile int keep_alive_118 = 3;
int gwp_asan_fill_func_119(int a, int b) {
    int x = a * 9 + b * 7 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_119 alive
static volatile int keep_alive_119 = 3;
int gwp_asan_fill_func_120(int a, int b) {
    int x = a * 10 + b * 8 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_120 alive
static volatile int keep_alive_120 = 3;
int gwp_asan_fill_func_121(int a, int b) {
    int x = a * 11 + b * 9 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_121 alive
static volatile int keep_alive_121 = 3;
int gwp_asan_fill_func_122(int a, int b) {
    int x = a * 12 + b * 10 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_122 alive
static volatile int keep_alive_122 = 3;
int gwp_asan_fill_func_123(int a, int b) {
    int x = a * 13 + b * 11 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_123 alive
static volatile int keep_alive_123 = 3;
int gwp_asan_fill_func_124(int a, int b) {
    int x = a * 14 + b * 12 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_124 alive
static volatile int keep_alive_124 = 3;
int gwp_asan_fill_func_125(int a, int b) {
    int x = a * 15 + b * 13 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_125 alive
static volatile int keep_alive_125 = 3;
int gwp_asan_fill_func_126(int a, int b) {
    int x = a * 16 + b * 14 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_126 alive
static volatile int keep_alive_126 = 3;
int gwp_asan_fill_func_127(int a, int b) {
    int x = a * 17 + b * 15 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_127 alive
static volatile int keep_alive_127 = 3;
int gwp_asan_fill_func_128(int a, int b) {
    int x = a * 18 + b * 16 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_128 alive
static volatile int keep_alive_128 = 3;
int gwp_asan_fill_func_129(int a, int b) {
    int x = a * 19 + b * 17 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_129 alive
static volatile int keep_alive_129 = 3;
int gwp_asan_fill_func_130(int a, int b) {
    int x = a * 20 + b * 18 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_130 alive
static volatile int keep_alive_130 = 3;
int gwp_asan_fill_func_131(int a, int b) {
    int x = a * 21 + b * 19 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_131 alive
static volatile int keep_alive_131 = 3;
int gwp_asan_fill_func_132(int a, int b) {
    int x = a * 22 + b * 20 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_132 alive
static volatile int keep_alive_132 = 3;
int gwp_asan_fill_func_133(int a, int b) {
    int x = a * 23 + b * 21 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_133 alive
static volatile int keep_alive_133 = 3;
int gwp_asan_fill_func_134(int a, int b) {
    int x = a * 24 + b * 22 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_134 alive
static volatile int keep_alive_134 = 3;
int gwp_asan_fill_func_135(int a, int b) {
    int x = a * 25 + b * 23 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_135 alive
static volatile int keep_alive_135 = 3;
int gwp_asan_fill_func_136(int a, int b) {
    int x = a * 26 + b * 7 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_136 alive
static volatile int keep_alive_136 = 3;
int gwp_asan_fill_func_137(int a, int b) {
    int x = a * 27 + b * 8 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_137 alive
static volatile int keep_alive_137 = 3;
int gwp_asan_fill_func_138(int a, int b) {
    int x = a * 5 + b * 9 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_138 alive
static volatile int keep_alive_138 = 3;
int gwp_asan_fill_func_139(int a, int b) {
    int x = a * 6 + b * 10 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_139 alive
static volatile int keep_alive_139 = 3;
int gwp_asan_fill_func_140(int a, int b) {
    int x = a * 7 + b * 11 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_140 alive
static volatile int keep_alive_140 = 3;
int gwp_asan_fill_func_141(int a, int b) {
    int x = a * 8 + b * 12 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_141 alive
static volatile int keep_alive_141 = 3;
int gwp_asan_fill_func_142(int a, int b) {
    int x = a * 9 + b * 13 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_142 alive
static volatile int keep_alive_142 = 3;
int gwp_asan_fill_func_143(int a, int b) {
    int x = a * 10 + b * 14 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_143 alive
static volatile int keep_alive_143 = 3;
int gwp_asan_fill_func_144(int a, int b) {
    int x = a * 11 + b * 15 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_144 alive
static volatile int keep_alive_144 = 3;
int gwp_asan_fill_func_145(int a, int b) {
    int x = a * 12 + b * 16 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_145 alive
static volatile int keep_alive_145 = 3;
int gwp_asan_fill_func_146(int a, int b) {
    int x = a * 13 + b * 17 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_146 alive
static volatile int keep_alive_146 = 3;
int gwp_asan_fill_func_147(int a, int b) {
    int x = a * 14 + b * 18 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_147 alive
static volatile int keep_alive_147 = 3;
int gwp_asan_fill_func_148(int a, int b) {
    int x = a * 15 + b * 19 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_148 alive
static volatile int keep_alive_148 = 3;
int gwp_asan_fill_func_149(int a, int b) {
    int x = a * 16 + b * 20 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_149 alive
static volatile int keep_alive_149 = 3;
int gwp_asan_fill_func_150(int a, int b) {
    int x = a * 17 + b * 21 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_150 alive
static volatile int keep_alive_150 = 3;
int gwp_asan_fill_func_151(int a, int b) {
    int x = a * 18 + b * 22 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_151 alive
static volatile int keep_alive_151 = 3;
int gwp_asan_fill_func_152(int a, int b) {
    int x = a * 19 + b * 23 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_152 alive
static volatile int keep_alive_152 = 3;
int gwp_asan_fill_func_153(int a, int b) {
    int x = a * 20 + b * 7 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_153 alive
static volatile int keep_alive_153 = 3;
int gwp_asan_fill_func_154(int a, int b) {
    int x = a * 21 + b * 8 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_154 alive
static volatile int keep_alive_154 = 3;
int gwp_asan_fill_func_155(int a, int b) {
    int x = a * 22 + b * 9 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_155 alive
static volatile int keep_alive_155 = 3;
int gwp_asan_fill_func_156(int a, int b) {
    int x = a * 23 + b * 10 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_156 alive
static volatile int keep_alive_156 = 3;
int gwp_asan_fill_func_157(int a, int b) {
    int x = a * 24 + b * 11 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_157 alive
static volatile int keep_alive_157 = 3;
int gwp_asan_fill_func_158(int a, int b) {
    int x = a * 25 + b * 12 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_158 alive
static volatile int keep_alive_158 = 3;
int gwp_asan_fill_func_159(int a, int b) {
    int x = a * 26 + b * 13 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_159 alive
static volatile int keep_alive_159 = 3;
int gwp_asan_fill_func_160(int a, int b) {
    int x = a * 27 + b * 14 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_160 alive
static volatile int keep_alive_160 = 3;
int gwp_asan_fill_func_161(int a, int b) {
    int x = a * 5 + b * 15 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_161 alive
static volatile int keep_alive_161 = 3;
int gwp_asan_fill_func_162(int a, int b) {
    int x = a * 6 + b * 16 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_162 alive
static volatile int keep_alive_162 = 3;
int gwp_asan_fill_func_163(int a, int b) {
    int x = a * 7 + b * 17 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_163 alive
static volatile int keep_alive_163 = 3;
int gwp_asan_fill_func_164(int a, int b) {
    int x = a * 8 + b * 18 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_164 alive
static volatile int keep_alive_164 = 3;
int gwp_asan_fill_func_165(int a, int b) {
    int x = a * 9 + b * 19 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_165 alive
static volatile int keep_alive_165 = 3;
int gwp_asan_fill_func_166(int a, int b) {
    int x = a * 10 + b * 20 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_166 alive
static volatile int keep_alive_166 = 3;
int gwp_asan_fill_func_167(int a, int b) {
    int x = a * 11 + b * 21 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_167 alive
static volatile int keep_alive_167 = 3;
int gwp_asan_fill_func_168(int a, int b) {
    int x = a * 12 + b * 22 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_168 alive
static volatile int keep_alive_168 = 3;
int gwp_asan_fill_func_169(int a, int b) {
    int x = a * 13 + b * 23 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_169 alive
static volatile int keep_alive_169 = 3;
int gwp_asan_fill_func_170(int a, int b) {
    int x = a * 14 + b * 7 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_170 alive
static volatile int keep_alive_170 = 3;
int gwp_asan_fill_func_171(int a, int b) {
    int x = a * 15 + b * 8 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_171 alive
static volatile int keep_alive_171 = 3;
int gwp_asan_fill_func_172(int a, int b) {
    int x = a * 16 + b * 9 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_172 alive
static volatile int keep_alive_172 = 3;
int gwp_asan_fill_func_173(int a, int b) {
    int x = a * 17 + b * 10 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_173 alive
static volatile int keep_alive_173 = 3;
int gwp_asan_fill_func_174(int a, int b) {
    int x = a * 18 + b * 11 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_174 alive
static volatile int keep_alive_174 = 3;
int gwp_asan_fill_func_175(int a, int b) {
    int x = a * 19 + b * 12 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_175 alive
static volatile int keep_alive_175 = 3;
int gwp_asan_fill_func_176(int a, int b) {
    int x = a * 20 + b * 13 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_176 alive
static volatile int keep_alive_176 = 3;
int gwp_asan_fill_func_177(int a, int b) {
    int x = a * 21 + b * 14 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_177 alive
static volatile int keep_alive_177 = 3;
int gwp_asan_fill_func_178(int a, int b) {
    int x = a * 22 + b * 15 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_178 alive
static volatile int keep_alive_178 = 3;
int gwp_asan_fill_func_179(int a, int b) {
    int x = a * 23 + b * 16 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_179 alive
static volatile int keep_alive_179 = 3;
int gwp_asan_fill_func_180(int a, int b) {
    int x = a * 24 + b * 17 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_180 alive
static volatile int keep_alive_180 = 3;
int gwp_asan_fill_func_181(int a, int b) {
    int x = a * 25 + b * 18 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_181 alive
static volatile int keep_alive_181 = 3;
int gwp_asan_fill_func_182(int a, int b) {
    int x = a * 26 + b * 19 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_182 alive
static volatile int keep_alive_182 = 3;
int gwp_asan_fill_func_183(int a, int b) {
    int x = a * 27 + b * 20 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_183 alive
static volatile int keep_alive_183 = 3;
int gwp_asan_fill_func_184(int a, int b) {
    int x = a * 5 + b * 21 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_184 alive
static volatile int keep_alive_184 = 3;
int gwp_asan_fill_func_185(int a, int b) {
    int x = a * 6 + b * 22 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_185 alive
static volatile int keep_alive_185 = 3;
int gwp_asan_fill_func_186(int a, int b) {
    int x = a * 7 + b * 23 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_186 alive
static volatile int keep_alive_186 = 3;
int gwp_asan_fill_func_187(int a, int b) {
    int x = a * 8 + b * 7 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_187 alive
static volatile int keep_alive_187 = 3;
int gwp_asan_fill_func_188(int a, int b) {
    int x = a * 9 + b * 8 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_188 alive
static volatile int keep_alive_188 = 3;
int gwp_asan_fill_func_189(int a, int b) {
    int x = a * 10 + b * 9 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_189 alive
static volatile int keep_alive_189 = 3;
int gwp_asan_fill_func_190(int a, int b) {
    int x = a * 11 + b * 10 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_190 alive
static volatile int keep_alive_190 = 3;
int gwp_asan_fill_func_191(int a, int b) {
    int x = a * 12 + b * 11 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_191 alive
static volatile int keep_alive_191 = 3;
int gwp_asan_fill_func_192(int a, int b) {
    int x = a * 13 + b * 12 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_192 alive
static volatile int keep_alive_192 = 3;
int gwp_asan_fill_func_193(int a, int b) {
    int x = a * 14 + b * 13 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_193 alive
static volatile int keep_alive_193 = 3;
int gwp_asan_fill_func_194(int a, int b) {
    int x = a * 15 + b * 14 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_194 alive
static volatile int keep_alive_194 = 3;
int gwp_asan_fill_func_195(int a, int b) {
    int x = a * 16 + b * 15 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_195 alive
static volatile int keep_alive_195 = 3;
int gwp_asan_fill_func_196(int a, int b) {
    int x = a * 17 + b * 16 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_196 alive
static volatile int keep_alive_196 = 3;
int gwp_asan_fill_func_197(int a, int b) {
    int x = a * 18 + b * 17 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_197 alive
static volatile int keep_alive_197 = 3;
int gwp_asan_fill_func_198(int a, int b) {
    int x = a * 19 + b * 18 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_198 alive
static volatile int keep_alive_198 = 3;
int gwp_asan_fill_func_199(int a, int b) {
    int x = a * 20 + b * 19 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_199 alive
static volatile int keep_alive_199 = 3;
int gwp_asan_fill_func_200(int a, int b) {
    int x = a * 21 + b * 20 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_200 alive
static volatile int keep_alive_200 = 3;
int gwp_asan_fill_func_201(int a, int b) {
    int x = a * 22 + b * 21 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_201 alive
static volatile int keep_alive_201 = 3;
int gwp_asan_fill_func_202(int a, int b) {
    int x = a * 23 + b * 22 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_202 alive
static volatile int keep_alive_202 = 3;
int gwp_asan_fill_func_203(int a, int b) {
    int x = a * 24 + b * 23 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_203 alive
static volatile int keep_alive_203 = 3;
int gwp_asan_fill_func_204(int a, int b) {
    int x = a * 25 + b * 7 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_204 alive
static volatile int keep_alive_204 = 3;
int gwp_asan_fill_func_205(int a, int b) {
    int x = a * 26 + b * 8 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_205 alive
static volatile int keep_alive_205 = 3;
int gwp_asan_fill_func_206(int a, int b) {
    int x = a * 27 + b * 9 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_206 alive
static volatile int keep_alive_206 = 3;
int gwp_asan_fill_func_207(int a, int b) {
    int x = a * 5 + b * 10 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_207 alive
static volatile int keep_alive_207 = 3;
int gwp_asan_fill_func_208(int a, int b) {
    int x = a * 6 + b * 11 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_208 alive
static volatile int keep_alive_208 = 3;
int gwp_asan_fill_func_209(int a, int b) {
    int x = a * 7 + b * 12 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_209 alive
static volatile int keep_alive_209 = 3;
int gwp_asan_fill_func_210(int a, int b) {
    int x = a * 8 + b * 13 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_210 alive
static volatile int keep_alive_210 = 3;
int gwp_asan_fill_func_211(int a, int b) {
    int x = a * 9 + b * 14 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_211 alive
static volatile int keep_alive_211 = 3;
int gwp_asan_fill_func_212(int a, int b) {
    int x = a * 10 + b * 15 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_212 alive
static volatile int keep_alive_212 = 3;
int gwp_asan_fill_func_213(int a, int b) {
    int x = a * 11 + b * 16 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_213 alive
static volatile int keep_alive_213 = 3;
int gwp_asan_fill_func_214(int a, int b) {
    int x = a * 12 + b * 17 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_214 alive
static volatile int keep_alive_214 = 3;
int gwp_asan_fill_func_215(int a, int b) {
    int x = a * 13 + b * 18 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_215 alive
static volatile int keep_alive_215 = 3;
int gwp_asan_fill_func_216(int a, int b) {
    int x = a * 14 + b * 19 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_216 alive
static volatile int keep_alive_216 = 3;
int gwp_asan_fill_func_217(int a, int b) {
    int x = a * 15 + b * 20 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_217 alive
static volatile int keep_alive_217 = 3;
int gwp_asan_fill_func_218(int a, int b) {
    int x = a * 16 + b * 21 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_218 alive
static volatile int keep_alive_218 = 3;
int gwp_asan_fill_func_219(int a, int b) {
    int x = a * 17 + b * 22 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_219 alive
static volatile int keep_alive_219 = 3;
int gwp_asan_fill_func_220(int a, int b) {
    int x = a * 18 + b * 23 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_220 alive
static volatile int keep_alive_220 = 3;
int gwp_asan_fill_func_221(int a, int b) {
    int x = a * 19 + b * 7 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_221 alive
static volatile int keep_alive_221 = 3;
int gwp_asan_fill_func_222(int a, int b) {
    int x = a * 20 + b * 8 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_222 alive
static volatile int keep_alive_222 = 3;
int gwp_asan_fill_func_223(int a, int b) {
    int x = a * 21 + b * 9 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_223 alive
static volatile int keep_alive_223 = 3;
int gwp_asan_fill_func_224(int a, int b) {
    int x = a * 22 + b * 10 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_224 alive
static volatile int keep_alive_224 = 3;
int gwp_asan_fill_func_225(int a, int b) {
    int x = a * 23 + b * 11 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_225 alive
static volatile int keep_alive_225 = 3;
int gwp_asan_fill_func_226(int a, int b) {
    int x = a * 24 + b * 12 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_226 alive
static volatile int keep_alive_226 = 3;
int gwp_asan_fill_func_227(int a, int b) {
    int x = a * 25 + b * 13 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_227 alive
static volatile int keep_alive_227 = 3;
int gwp_asan_fill_func_228(int a, int b) {
    int x = a * 26 + b * 14 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_228 alive
static volatile int keep_alive_228 = 3;
int gwp_asan_fill_func_229(int a, int b) {
    int x = a * 27 + b * 15 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_229 alive
static volatile int keep_alive_229 = 3;
int gwp_asan_fill_func_230(int a, int b) {
    int x = a * 5 + b * 16 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_230 alive
static volatile int keep_alive_230 = 3;
int gwp_asan_fill_func_231(int a, int b) {
    int x = a * 6 + b * 17 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_231 alive
static volatile int keep_alive_231 = 3;
int gwp_asan_fill_func_232(int a, int b) {
    int x = a * 7 + b * 18 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_232 alive
static volatile int keep_alive_232 = 3;
int gwp_asan_fill_func_233(int a, int b) {
    int x = a * 8 + b * 19 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_233 alive
static volatile int keep_alive_233 = 3;
int gwp_asan_fill_func_234(int a, int b) {
    int x = a * 9 + b * 20 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_234 alive
static volatile int keep_alive_234 = 3;
int gwp_asan_fill_func_235(int a, int b) {
    int x = a * 10 + b * 21 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_235 alive
static volatile int keep_alive_235 = 3;
int gwp_asan_fill_func_236(int a, int b) {
    int x = a * 11 + b * 22 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_236 alive
static volatile int keep_alive_236 = 3;
int gwp_asan_fill_func_237(int a, int b) {
    int x = a * 12 + b * 23 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_237 alive
static volatile int keep_alive_237 = 3;
int gwp_asan_fill_func_238(int a, int b) {
    int x = a * 13 + b * 7 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_238 alive
static volatile int keep_alive_238 = 3;
int gwp_asan_fill_func_239(int a, int b) {
    int x = a * 14 + b * 8 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_239 alive
static volatile int keep_alive_239 = 3;
int gwp_asan_fill_func_240(int a, int b) {
    int x = a * 15 + b * 9 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_240 alive
static volatile int keep_alive_240 = 3;
int gwp_asan_fill_func_241(int a, int b) {
    int x = a * 16 + b * 10 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_241 alive
static volatile int keep_alive_241 = 3;
int gwp_asan_fill_func_242(int a, int b) {
    int x = a * 17 + b * 11 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_242 alive
static volatile int keep_alive_242 = 3;
int gwp_asan_fill_func_243(int a, int b) {
    int x = a * 18 + b * 12 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_243 alive
static volatile int keep_alive_243 = 3;
int gwp_asan_fill_func_244(int a, int b) {
    int x = a * 19 + b * 13 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_244 alive
static volatile int keep_alive_244 = 3;
int gwp_asan_fill_func_245(int a, int b) {
    int x = a * 20 + b * 14 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_245 alive
static volatile int keep_alive_245 = 3;
int gwp_asan_fill_func_246(int a, int b) {
    int x = a * 21 + b * 15 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_246 alive
static volatile int keep_alive_246 = 3;
int gwp_asan_fill_func_247(int a, int b) {
    int x = a * 22 + b * 16 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_247 alive
static volatile int keep_alive_247 = 3;
int gwp_asan_fill_func_248(int a, int b) {
    int x = a * 23 + b * 17 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_248 alive
static volatile int keep_alive_248 = 3;
int gwp_asan_fill_func_249(int a, int b) {
    int x = a * 24 + b * 18 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_249 alive
static volatile int keep_alive_249 = 3;
int gwp_asan_fill_func_250(int a, int b) {
    int x = a * 25 + b * 19 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_250 alive
static volatile int keep_alive_250 = 3;
int gwp_asan_fill_func_251(int a, int b) {
    int x = a * 26 + b * 20 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_251 alive
static volatile int keep_alive_251 = 3;
int gwp_asan_fill_func_252(int a, int b) {
    int x = a * 27 + b * 21 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_252 alive
static volatile int keep_alive_252 = 3;
int gwp_asan_fill_func_253(int a, int b) {
    int x = a * 5 + b * 22 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_253 alive
static volatile int keep_alive_253 = 3;
int gwp_asan_fill_func_254(int a, int b) {
    int x = a * 6 + b * 23 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_254 alive
static volatile int keep_alive_254 = 3;
int gwp_asan_fill_func_255(int a, int b) {
    int x = a * 7 + b * 7 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_255 alive
static volatile int keep_alive_255 = 3;
int gwp_asan_fill_func_256(int a, int b) {
    int x = a * 8 + b * 8 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_256 alive
static volatile int keep_alive_256 = 3;
int gwp_asan_fill_func_257(int a, int b) {
    int x = a * 9 + b * 9 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_257 alive
static volatile int keep_alive_257 = 3;
int gwp_asan_fill_func_258(int a, int b) {
    int x = a * 10 + b * 10 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_258 alive
static volatile int keep_alive_258 = 3;
int gwp_asan_fill_func_259(int a, int b) {
    int x = a * 11 + b * 11 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_259 alive
static volatile int keep_alive_259 = 3;
int gwp_asan_fill_func_260(int a, int b) {
    int x = a * 12 + b * 12 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_260 alive
static volatile int keep_alive_260 = 3;
int gwp_asan_fill_func_261(int a, int b) {
    int x = a * 13 + b * 13 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_261 alive
static volatile int keep_alive_261 = 3;
int gwp_asan_fill_func_262(int a, int b) {
    int x = a * 14 + b * 14 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_262 alive
static volatile int keep_alive_262 = 3;
int gwp_asan_fill_func_263(int a, int b) {
    int x = a * 15 + b * 15 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_263 alive
static volatile int keep_alive_263 = 3;
int gwp_asan_fill_func_264(int a, int b) {
    int x = a * 16 + b * 16 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_264 alive
static volatile int keep_alive_264 = 3;
int gwp_asan_fill_func_265(int a, int b) {
    int x = a * 17 + b * 17 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_265 alive
static volatile int keep_alive_265 = 3;
int gwp_asan_fill_func_266(int a, int b) {
    int x = a * 18 + b * 18 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_266 alive
static volatile int keep_alive_266 = 3;
int gwp_asan_fill_func_267(int a, int b) {
    int x = a * 19 + b * 19 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_267 alive
static volatile int keep_alive_267 = 3;
int gwp_asan_fill_func_268(int a, int b) {
    int x = a * 20 + b * 20 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_268 alive
static volatile int keep_alive_268 = 3;
int gwp_asan_fill_func_269(int a, int b) {
    int x = a * 21 + b * 21 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_269 alive
static volatile int keep_alive_269 = 3;
int gwp_asan_fill_func_270(int a, int b) {
    int x = a * 22 + b * 22 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_270 alive
static volatile int keep_alive_270 = 3;
int gwp_asan_fill_func_271(int a, int b) {
    int x = a * 23 + b * 23 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_271 alive
static volatile int keep_alive_271 = 3;
int gwp_asan_fill_func_272(int a, int b) {
    int x = a * 24 + b * 7 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_272 alive
static volatile int keep_alive_272 = 3;
int gwp_asan_fill_func_273(int a, int b) {
    int x = a * 25 + b * 8 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_273 alive
static volatile int keep_alive_273 = 3;
int gwp_asan_fill_func_274(int a, int b) {
    int x = a * 26 + b * 9 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_274 alive
static volatile int keep_alive_274 = 3;
int gwp_asan_fill_func_275(int a, int b) {
    int x = a * 27 + b * 10 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_275 alive
static volatile int keep_alive_275 = 3;
int gwp_asan_fill_func_276(int a, int b) {
    int x = a * 5 + b * 11 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_276 alive
static volatile int keep_alive_276 = 3;
int gwp_asan_fill_func_277(int a, int b) {
    int x = a * 6 + b * 12 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_277 alive
static volatile int keep_alive_277 = 3;
int gwp_asan_fill_func_278(int a, int b) {
    int x = a * 7 + b * 13 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_278 alive
static volatile int keep_alive_278 = 3;
int gwp_asan_fill_func_279(int a, int b) {
    int x = a * 8 + b * 14 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_279 alive
static volatile int keep_alive_279 = 3;
int gwp_asan_fill_func_280(int a, int b) {
    int x = a * 9 + b * 15 + 5;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_280 alive
static volatile int keep_alive_280 = 3;
int gwp_asan_fill_func_281(int a, int b) {
    int x = a * 10 + b * 16 + 6;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_281 alive
static volatile int keep_alive_281 = 3;
int gwp_asan_fill_func_282(int a, int b) {
    int x = a * 11 + b * 17 + 7;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_282 alive
static volatile int keep_alive_282 = 3;
int gwp_asan_fill_func_283(int a, int b) {
    int x = a * 12 + b * 18 + 8;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_283 alive
static volatile int keep_alive_283 = 3;
int gwp_asan_fill_func_284(int a, int b) {
    int x = a * 13 + b * 19 + 9;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_284 alive
static volatile int keep_alive_284 = 3;
int gwp_asan_fill_func_285(int a, int b) {
    int x = a * 14 + b * 20 + 10;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_285 alive
static volatile int keep_alive_285 = 3;
int gwp_asan_fill_func_286(int a, int b) {
    int x = a * 15 + b * 21 + 0;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_286 alive
static volatile int keep_alive_286 = 3;
int gwp_asan_fill_func_287(int a, int b) {
    int x = a * 16 + b * 22 + 1;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_287 alive
static volatile int keep_alive_287 = 3;
int gwp_asan_fill_func_288(int a, int b) {
    int x = a * 17 + b * 23 + 2;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_288 alive
static volatile int keep_alive_288 = 3;
int gwp_asan_fill_func_289(int a, int b) {
    int x = a * 18 + b * 7 + 3;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_289 alive
static volatile int keep_alive_289 = 3;
int gwp_asan_fill_func_290(int a, int b) {
    int x = a * 19 + b * 8 + 4;
    x ^= (x << 9) | (x >> 7);
    x += (a ^ b) & 0x1ff;
    x = (int)((unsigned)x * 22695477u + 314159265u) ^ (x >> 13);
    return x;
}
// keep symbol filler_func_290 alive
static volatile int keep_alive_290 = 3;
// use many filler functions in a loop by indexing into an array of function pointers
void fill_func_in_array() {
    using fptr_t = int(*)(int,int);
    static fptr_t fptrs[290];
    // populate function pointer table (unique functions)
    fptrs[0] = &gwp_asan_fill_func_1;
    fptrs[1] = &gwp_asan_fill_func_2;
    fptrs[2] = &gwp_asan_fill_func_3;
    fptrs[3] = &gwp_asan_fill_func_4;
    fptrs[4] = &gwp_asan_fill_func_5;
    fptrs[5] = &gwp_asan_fill_func_6;
    fptrs[6] = &gwp_asan_fill_func_7;
    fptrs[7] = &gwp_asan_fill_func_8;
    fptrs[8] = &gwp_asan_fill_func_9;
    fptrs[9] = &gwp_asan_fill_func_10;
    fptrs[10] = &gwp_asan_fill_func_11;
    fptrs[11] = &gwp_asan_fill_func_12;
    fptrs[12] = &gwp_asan_fill_func_13;
    fptrs[13] = &gwp_asan_fill_func_14;
    fptrs[14] = &gwp_asan_fill_func_15;
    fptrs[15] = &gwp_asan_fill_func_16;
    fptrs[16] = &gwp_asan_fill_func_17;
    fptrs[17] = &gwp_asan_fill_func_18;
    fptrs[18] = &gwp_asan_fill_func_19;
    fptrs[19] = &gwp_asan_fill_func_20;
    fptrs[20] = &gwp_asan_fill_func_21;
    fptrs[21] = &gwp_asan_fill_func_22;
    fptrs[22] = &gwp_asan_fill_func_23;
    fptrs[23] = &gwp_asan_fill_func_24;
    fptrs[24] = &gwp_asan_fill_func_25;
    fptrs[25] = &gwp_asan_fill_func_26;
    fptrs[26] = &gwp_asan_fill_func_27;
    fptrs[27] = &gwp_asan_fill_func_28;
    fptrs[28] = &gwp_asan_fill_func_29;
    fptrs[29] = &gwp_asan_fill_func_30;
    fptrs[30] = &gwp_asan_fill_func_31;
    fptrs[31] = &gwp_asan_fill_func_32;
    fptrs[32] = &gwp_asan_fill_func_33;
    fptrs[33] = &gwp_asan_fill_func_34;
    fptrs[34] = &gwp_asan_fill_func_35;
    fptrs[35] = &gwp_asan_fill_func_36;
    fptrs[36] = &gwp_asan_fill_func_37;
    fptrs[37] = &gwp_asan_fill_func_38;
    fptrs[38] = &gwp_asan_fill_func_39;
    fptrs[39] = &gwp_asan_fill_func_40;
    fptrs[40] = &gwp_asan_fill_func_41;
    fptrs[41] = &gwp_asan_fill_func_42;
    fptrs[42] = &gwp_asan_fill_func_43;
    fptrs[43] = &gwp_asan_fill_func_44;
    fptrs[44] = &gwp_asan_fill_func_45;
    fptrs[45] = &gwp_asan_fill_func_46;
    fptrs[46] = &gwp_asan_fill_func_47;
    fptrs[47] = &gwp_asan_fill_func_48;
    fptrs[48] = &gwp_asan_fill_func_49;
    fptrs[49] = &gwp_asan_fill_func_50;
    fptrs[50] = &gwp_asan_fill_func_51;
    fptrs[51] = &gwp_asan_fill_func_52;
    fptrs[52] = &gwp_asan_fill_func_53;
    fptrs[53] = &gwp_asan_fill_func_54;
    fptrs[54] = &gwp_asan_fill_func_55;
    fptrs[55] = &gwp_asan_fill_func_56;
    fptrs[56] = &gwp_asan_fill_func_57;
    fptrs[57] = &gwp_asan_fill_func_58;
    fptrs[58] = &gwp_asan_fill_func_59;
    fptrs[59] = &gwp_asan_fill_func_60;
    fptrs[60] = &gwp_asan_fill_func_61;
    fptrs[61] = &gwp_asan_fill_func_62;
    fptrs[62] = &gwp_asan_fill_func_63;
    fptrs[63] = &gwp_asan_fill_func_64;
    fptrs[64] = &gwp_asan_fill_func_65;
    fptrs[65] = &gwp_asan_fill_func_66;
    fptrs[66] = &gwp_asan_fill_func_67;
    fptrs[67] = &gwp_asan_fill_func_68;
    fptrs[68] = &gwp_asan_fill_func_69;
    fptrs[69] = &gwp_asan_fill_func_70;
    fptrs[70] = &gwp_asan_fill_func_71;
    fptrs[71] = &gwp_asan_fill_func_72;
    fptrs[72] = &gwp_asan_fill_func_73;
    fptrs[73] = &gwp_asan_fill_func_74;
    fptrs[74] = &gwp_asan_fill_func_75;
    fptrs[75] = &gwp_asan_fill_func_76;
    fptrs[76] = &gwp_asan_fill_func_77;
    fptrs[77] = &gwp_asan_fill_func_78;
    fptrs[78] = &gwp_asan_fill_func_79;
    fptrs[79] = &gwp_asan_fill_func_80;
    fptrs[80] = &gwp_asan_fill_func_81;
    fptrs[81] = &gwp_asan_fill_func_82;
    fptrs[82] = &gwp_asan_fill_func_83;
    fptrs[83] = &gwp_asan_fill_func_84;
    fptrs[84] = &gwp_asan_fill_func_85;
    fptrs[85] = &gwp_asan_fill_func_86;
    fptrs[86] = &gwp_asan_fill_func_87;
    fptrs[87] = &gwp_asan_fill_func_88;
    fptrs[88] = &gwp_asan_fill_func_89;
    fptrs[89] = &gwp_asan_fill_func_90;
    fptrs[90] = &gwp_asan_fill_func_91;
    fptrs[91] = &gwp_asan_fill_func_92;
    fptrs[92] = &gwp_asan_fill_func_93;
    fptrs[93] = &gwp_asan_fill_func_94;
    fptrs[94] = &gwp_asan_fill_func_95;
    fptrs[95] = &gwp_asan_fill_func_96;
    fptrs[96] = &gwp_asan_fill_func_97;
    fptrs[97] = &gwp_asan_fill_func_98;
    fptrs[98] = &gwp_asan_fill_func_99;
    fptrs[99] = &gwp_asan_fill_func_100;
    fptrs[100] = &gwp_asan_fill_func_101;
    fptrs[101] = &gwp_asan_fill_func_102;
    fptrs[102] = &gwp_asan_fill_func_103;
    fptrs[103] = &gwp_asan_fill_func_104;
    fptrs[104] = &gwp_asan_fill_func_105;
    fptrs[105] = &gwp_asan_fill_func_106;
    fptrs[106] = &gwp_asan_fill_func_107;
    fptrs[107] = &gwp_asan_fill_func_108;
    fptrs[108] = &gwp_asan_fill_func_109;
    fptrs[109] = &gwp_asan_fill_func_110;
    fptrs[110] = &gwp_asan_fill_func_111;
    fptrs[111] = &gwp_asan_fill_func_112;
    fptrs[112] = &gwp_asan_fill_func_113;
    fptrs[113] = &gwp_asan_fill_func_114;
    fptrs[114] = &gwp_asan_fill_func_115;
    fptrs[115] = &gwp_asan_fill_func_116;
    fptrs[116] = &gwp_asan_fill_func_117;
    fptrs[117] = &gwp_asan_fill_func_118;
    fptrs[118] = &gwp_asan_fill_func_119;
    fptrs[119] = &gwp_asan_fill_func_120;
    fptrs[120] = &gwp_asan_fill_func_121;
    fptrs[121] = &gwp_asan_fill_func_122;
    fptrs[122] = &gwp_asan_fill_func_123;
    fptrs[123] = &gwp_asan_fill_func_124;
    fptrs[124] = &gwp_asan_fill_func_125;
    fptrs[125] = &gwp_asan_fill_func_126;
    fptrs[126] = &gwp_asan_fill_func_127;
    fptrs[127] = &gwp_asan_fill_func_128;
    fptrs[128] = &gwp_asan_fill_func_129;
    fptrs[129] = &gwp_asan_fill_func_130;
    fptrs[130] = &gwp_asan_fill_func_131;
    fptrs[131] = &gwp_asan_fill_func_132;
    fptrs[132] = &gwp_asan_fill_func_133;
    fptrs[133] = &gwp_asan_fill_func_134;
    fptrs[134] = &gwp_asan_fill_func_135;
    fptrs[135] = &gwp_asan_fill_func_136;
    fptrs[136] = &gwp_asan_fill_func_137;
    fptrs[137] = &gwp_asan_fill_func_138;
    fptrs[138] = &gwp_asan_fill_func_139;
    fptrs[139] = &gwp_asan_fill_func_140;
    fptrs[140] = &gwp_asan_fill_func_141;
    fptrs[141] = &gwp_asan_fill_func_142;
    fptrs[142] = &gwp_asan_fill_func_143;
    fptrs[143] = &gwp_asan_fill_func_144;
    fptrs[144] = &gwp_asan_fill_func_145;
    fptrs[145] = &gwp_asan_fill_func_146;
    fptrs[146] = &gwp_asan_fill_func_147;
    fptrs[147] = &gwp_asan_fill_func_148;
    fptrs[148] = &gwp_asan_fill_func_149;
    fptrs[149] = &gwp_asan_fill_func_150;
    fptrs[150] = &gwp_asan_fill_func_151;
    fptrs[151] = &gwp_asan_fill_func_152;
    fptrs[152] = &gwp_asan_fill_func_153;
    fptrs[153] = &gwp_asan_fill_func_154;
    fptrs[154] = &gwp_asan_fill_func_155;
    fptrs[155] = &gwp_asan_fill_func_156;
    fptrs[156] = &gwp_asan_fill_func_157;
    fptrs[157] = &gwp_asan_fill_func_158;
    fptrs[158] = &gwp_asan_fill_func_159;
    fptrs[159] = &gwp_asan_fill_func_160;
    fptrs[160] = &gwp_asan_fill_func_161;
    fptrs[161] = &gwp_asan_fill_func_162;
    fptrs[162] = &gwp_asan_fill_func_163;
    fptrs[163] = &gwp_asan_fill_func_164;
    fptrs[164] = &gwp_asan_fill_func_165;
    fptrs[165] = &gwp_asan_fill_func_166;
    fptrs[166] = &gwp_asan_fill_func_167;
    fptrs[167] = &gwp_asan_fill_func_168;
    fptrs[168] = &gwp_asan_fill_func_169;
    fptrs[169] = &gwp_asan_fill_func_170;
    fptrs[170] = &gwp_asan_fill_func_171;
    fptrs[171] = &gwp_asan_fill_func_172;
    fptrs[172] = &gwp_asan_fill_func_173;
    fptrs[173] = &gwp_asan_fill_func_174;
    fptrs[174] = &gwp_asan_fill_func_175;
    fptrs[175] = &gwp_asan_fill_func_176;
    fptrs[176] = &gwp_asan_fill_func_177;
    fptrs[177] = &gwp_asan_fill_func_178;
    fptrs[178] = &gwp_asan_fill_func_179;
    fptrs[179] = &gwp_asan_fill_func_180;
    fptrs[180] = &gwp_asan_fill_func_181;
    fptrs[181] = &gwp_asan_fill_func_182;
    fptrs[182] = &gwp_asan_fill_func_183;
    fptrs[183] = &gwp_asan_fill_func_184;
    fptrs[184] = &gwp_asan_fill_func_185;
    fptrs[185] = &gwp_asan_fill_func_186;
    fptrs[186] = &gwp_asan_fill_func_187;
    fptrs[187] = &gwp_asan_fill_func_188;
    fptrs[188] = &gwp_asan_fill_func_189;
    fptrs[189] = &gwp_asan_fill_func_190;
    fptrs[190] = &gwp_asan_fill_func_191;
    fptrs[191] = &gwp_asan_fill_func_192;
    fptrs[192] = &gwp_asan_fill_func_193;
    fptrs[193] = &gwp_asan_fill_func_194;
    fptrs[194] = &gwp_asan_fill_func_195;
    fptrs[195] = &gwp_asan_fill_func_196;
    fptrs[196] = &gwp_asan_fill_func_197;
    fptrs[197] = &gwp_asan_fill_func_198;
    fptrs[198] = &gwp_asan_fill_func_199;
    fptrs[199] = &gwp_asan_fill_func_200;
    fptrs[200] = &gwp_asan_fill_func_201;
    fptrs[201] = &gwp_asan_fill_func_202;
    fptrs[202] = &gwp_asan_fill_func_203;
    fptrs[203] = &gwp_asan_fill_func_204;
    fptrs[204] = &gwp_asan_fill_func_205;
    fptrs[205] = &gwp_asan_fill_func_206;
    fptrs[206] = &gwp_asan_fill_func_207;
    fptrs[207] = &gwp_asan_fill_func_208;
    fptrs[208] = &gwp_asan_fill_func_209;
    fptrs[209] = &gwp_asan_fill_func_210;
    fptrs[210] = &gwp_asan_fill_func_211;
    fptrs[211] = &gwp_asan_fill_func_212;
    fptrs[212] = &gwp_asan_fill_func_213;
    fptrs[213] = &gwp_asan_fill_func_214;
    fptrs[214] = &gwp_asan_fill_func_215;
    fptrs[215] = &gwp_asan_fill_func_216;
    fptrs[216] = &gwp_asan_fill_func_217;
    fptrs[217] = &gwp_asan_fill_func_218;
    fptrs[218] = &gwp_asan_fill_func_219;
    fptrs[219] = &gwp_asan_fill_func_220;
    fptrs[220] = &gwp_asan_fill_func_221;
    fptrs[221] = &gwp_asan_fill_func_222;
    fptrs[222] = &gwp_asan_fill_func_223;
    fptrs[223] = &gwp_asan_fill_func_224;
    fptrs[224] = &gwp_asan_fill_func_225;
    fptrs[225] = &gwp_asan_fill_func_226;
    fptrs[226] = &gwp_asan_fill_func_227;
    fptrs[227] = &gwp_asan_fill_func_228;
    fptrs[228] = &gwp_asan_fill_func_229;
    fptrs[229] = &gwp_asan_fill_func_230;
    fptrs[230] = &gwp_asan_fill_func_231;
    fptrs[231] = &gwp_asan_fill_func_232;
    fptrs[232] = &gwp_asan_fill_func_233;
    fptrs[233] = &gwp_asan_fill_func_234;
    fptrs[234] = &gwp_asan_fill_func_235;
    fptrs[235] = &gwp_asan_fill_func_236;
    fptrs[236] = &gwp_asan_fill_func_237;
    fptrs[237] = &gwp_asan_fill_func_238;
    fptrs[238] = &gwp_asan_fill_func_239;
    fptrs[239] = &gwp_asan_fill_func_240;
    fptrs[240] = &gwp_asan_fill_func_241;
    fptrs[241] = &gwp_asan_fill_func_242;
    fptrs[242] = &gwp_asan_fill_func_243;
    fptrs[243] = &gwp_asan_fill_func_244;
    fptrs[244] = &gwp_asan_fill_func_245;
    fptrs[245] = &gwp_asan_fill_func_246;
    fptrs[246] = &gwp_asan_fill_func_247;
    fptrs[247] = &gwp_asan_fill_func_248;
    fptrs[248] = &gwp_asan_fill_func_249;
    fptrs[249] = &gwp_asan_fill_func_250;
    fptrs[250] = &gwp_asan_fill_func_251;
    fptrs[251] = &gwp_asan_fill_func_252;
    fptrs[252] = &gwp_asan_fill_func_253;
    fptrs[253] = &gwp_asan_fill_func_254;
    fptrs[254] = &gwp_asan_fill_func_255;
    fptrs[255] = &gwp_asan_fill_func_256;
    fptrs[256] = &gwp_asan_fill_func_257;
    fptrs[257] = &gwp_asan_fill_func_258;
    fptrs[258] = &gwp_asan_fill_func_259;
    fptrs[259] = &gwp_asan_fill_func_260;
    fptrs[260] = &gwp_asan_fill_func_261;
    fptrs[261] = &gwp_asan_fill_func_262;
    fptrs[262] = &gwp_asan_fill_func_263;
    fptrs[263] = &gwp_asan_fill_func_264;
    fptrs[264] = &gwp_asan_fill_func_265;
    fptrs[265] = &gwp_asan_fill_func_266;
    fptrs[266] = &gwp_asan_fill_func_267;
    fptrs[267] = &gwp_asan_fill_func_268;
    fptrs[268] = &gwp_asan_fill_func_269;
    fptrs[269] = &gwp_asan_fill_func_270;
    fptrs[270] = &gwp_asan_fill_func_271;
    fptrs[271] = &gwp_asan_fill_func_272;
    fptrs[272] = &gwp_asan_fill_func_273;
    fptrs[273] = &gwp_asan_fill_func_274;
    fptrs[274] = &gwp_asan_fill_func_275;
    fptrs[275] = &gwp_asan_fill_func_276;
    fptrs[276] = &gwp_asan_fill_func_277;
    fptrs[277] = &gwp_asan_fill_func_278;
    fptrs[278] = &gwp_asan_fill_func_279;
    fptrs[279] = &gwp_asan_fill_func_280;
    fptrs[280] = &gwp_asan_fill_func_281;
    fptrs[281] = &gwp_asan_fill_func_282;
    fptrs[282] = &gwp_asan_fill_func_283;
    fptrs[283] = &gwp_asan_fill_func_284;
    fptrs[284] = &gwp_asan_fill_func_285;
    fptrs[285] = &gwp_asan_fill_func_286;
    fptrs[286] = &gwp_asan_fill_func_287;
    fptrs[287] = &gwp_asan_fill_func_288;
    fptrs[288] = &gwp_asan_fill_func_289;
    fptrs[289] = &gwp_asan_fill_func_290;
    int acc = 0;
    for (int r = 0; r < 5; ++r) {
        for (int i = 0; i < (int)(sizeof(fptrs)/sizeof(fptrs[0])); ++i) {
            acc += fptrs[i](i, r);
            if ((i & 1023) == 0) cout << ".";
        }
        cout << endl;
    }
    // store into a few volatile variables to prevent elimination
    keep_alive_1 = acc + 1;
    keep_alive_2 = acc + 2;
    keep_alive_3 = acc + 3;
    keep_alive_4 = acc + 4;
    keep_alive_5 = acc + 5;
    keep_alive_6 = acc + 6;
    keep_alive_7 = acc + 7;
    keep_alive_8 = acc + 8;
    keep_alive_9 = acc + 9;
    keep_alive_10 = acc + 10;
    keep_alive_11 = acc + 11;
    keep_alive_12 = acc + 12;
    keep_alive_13 = acc + 13;
    keep_alive_14 = acc + 14;
    keep_alive_15 = acc + 15;
    keep_alive_16 = acc + 16;
    keep_alive_17 = acc + 17;
    keep_alive_18 = acc + 18;
    keep_alive_19 = acc + 19;
    keep_alive_20 = acc + 20;
}

int main() {
  void* Pointers[16];
  for (unsigned i = 0; i < 16; ++i) {
    char *Ptr = reinterpret_cast<char*>(malloc(1 << i));
    Pointers[i] = Ptr;
    *Ptr = 0;
    Ptr[(1 << i) - 1] = 0;
  }

  for (unsigned i = 0; i < 16; ++i) {
    free(Pointers[i]);
  }

  return 0;
}
