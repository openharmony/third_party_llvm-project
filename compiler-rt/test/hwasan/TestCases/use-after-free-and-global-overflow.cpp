// RUN: %clangxx_hwasan -O0 %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s --check-prefix=CHECK
// REQUIRES: stable-runtime
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
using namespace std;

// Global dangling pointer that will be used for the single UAF access
static int *dangling_ptr = nullptr;

// Single function that will perform the UAF read exactly once
void UAF_func() {
    int val = dangling_ptr[0]; // USE-AFTER-FREE
}

// A small set of legitimate helper utilities used by real logic
int safe_sum(const vector<int>& v) { int s=0; for(int x:v) s+=x; return s; }
string repeat_char(char c, int n) { string s; s.reserve(n); for(int i=0;i<n;++i) s.push_back(c); return s; }
vector<int> make_sequence(int n) { vector<int> a(n); for(int i=0;i<n;++i) a[i]=i; return a; }

int filler_func_1(int a, int b) {
    int x = a * 2 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_1 alive
static volatile int keep_alive_1 = 0;
int filler_func_2(int a, int b) {
    int x = a * 3 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_2 alive
static volatile int keep_alive_2 = 0;
int filler_func_3(int a, int b) {
    int x = a * 4 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_3 alive
static volatile int keep_alive_3 = 0;
int filler_func_4(int a, int b) {
    int x = a * 5 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_4 alive
static volatile int keep_alive_4 = 0;
int filler_func_5(int a, int b) {
    int x = a * 6 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_5 alive
static volatile int keep_alive_5 = 0;
int filler_func_6(int a, int b) {
    int x = a * 7 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_6 alive
static volatile int keep_alive_6 = 0;
int filler_func_7(int a, int b) {
    int x = a * 8 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_7 alive
static volatile int keep_alive_7 = 0;
int filler_func_8(int a, int b) {
    int x = a * 9 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_8 alive
static volatile int keep_alive_8 = 0;
int filler_func_9(int a, int b) {
    int x = a * 10 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_9 alive
static volatile int keep_alive_9 = 0;
int filler_func_10(int a, int b) {
    int x = a * 11 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_10 alive
static volatile int keep_alive_10 = 0;
int filler_func_11(int a, int b) {
    int x = a * 12 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_11 alive
static volatile int keep_alive_11 = 0;
int filler_func_12(int a, int b) {
    int x = a * 13 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_12 alive
static volatile int keep_alive_12 = 0;
int filler_func_13(int a, int b) {
    int x = a * 1 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_13 alive
static volatile int keep_alive_13 = 0;
int filler_func_14(int a, int b) {
    int x = a * 2 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_14 alive
static volatile int keep_alive_14 = 0;
int filler_func_15(int a, int b) {
    int x = a * 3 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_15 alive
static volatile int keep_alive_15 = 0;
int filler_func_16(int a, int b) {
    int x = a * 4 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_16 alive
static volatile int keep_alive_16 = 0;
int filler_func_17(int a, int b) {
    int x = a * 5 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_17 alive
static volatile int keep_alive_17 = 0;
int filler_func_18(int a, int b) {
    int x = a * 6 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_18 alive
static volatile int keep_alive_18 = 0;
int filler_func_19(int a, int b) {
    int x = a * 7 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_19 alive
static volatile int keep_alive_19 = 0;
int filler_func_20(int a, int b) {
    int x = a * 8 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_20 alive
static volatile int keep_alive_20 = 0;
int filler_func_21(int a, int b) {
    int x = a * 9 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_21 alive
static volatile int keep_alive_21 = 0;
int filler_func_22(int a, int b) {
    int x = a * 10 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_22 alive
static volatile int keep_alive_22 = 0;
int filler_func_23(int a, int b) {
    int x = a * 11 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_23 alive
static volatile int keep_alive_23 = 0;
int filler_func_24(int a, int b) {
    int x = a * 12 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_24 alive
static volatile int keep_alive_24 = 0;
int filler_func_25(int a, int b) {
    int x = a * 13 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_25 alive
static volatile int keep_alive_25 = 0;
int filler_func_26(int a, int b) {
    int x = a * 1 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_26 alive
static volatile int keep_alive_26 = 0;
int filler_func_27(int a, int b) {
    int x = a * 2 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_27 alive
static volatile int keep_alive_27 = 0;
int filler_func_28(int a, int b) {
    int x = a * 3 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_28 alive
static volatile int keep_alive_28 = 0;
int filler_func_29(int a, int b) {
    int x = a * 4 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_29 alive
static volatile int keep_alive_29 = 0;
int filler_func_30(int a, int b) {
    int x = a * 5 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_30 alive
static volatile int keep_alive_30 = 0;
int filler_func_31(int a, int b) {
    int x = a * 6 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_31 alive
static volatile int keep_alive_31 = 0;
int filler_func_32(int a, int b) {
    int x = a * 7 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_32 alive
static volatile int keep_alive_32 = 0;
int filler_func_33(int a, int b) {
    int x = a * 8 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_33 alive
static volatile int keep_alive_33 = 0;
int filler_func_34(int a, int b) {
    int x = a * 9 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_34 alive
static volatile int keep_alive_34 = 0;
int filler_func_35(int a, int b) {
    int x = a * 10 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_35 alive
static volatile int keep_alive_35 = 0;
int filler_func_36(int a, int b) {
    int x = a * 11 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_36 alive
static volatile int keep_alive_36 = 0;
int filler_func_37(int a, int b) {
    int x = a * 12 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_37 alive
static volatile int keep_alive_37 = 0;
int filler_func_38(int a, int b) {
    int x = a * 13 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_38 alive
static volatile int keep_alive_38 = 0;
int filler_func_39(int a, int b) {
    int x = a * 1 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_39 alive
static volatile int keep_alive_39 = 0;
int filler_func_40(int a, int b) {
    int x = a * 2 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_40 alive
static volatile int keep_alive_40 = 0;
int filler_func_41(int a, int b) {
    int x = a * 3 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_41 alive
static volatile int keep_alive_41 = 0;
int filler_func_42(int a, int b) {
    int x = a * 4 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_42 alive
static volatile int keep_alive_42 = 0;
int filler_func_43(int a, int b) {
    int x = a * 5 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_43 alive
static volatile int keep_alive_43 = 0;
int filler_func_44(int a, int b) {
    int x = a * 6 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_44 alive
static volatile int keep_alive_44 = 0;
int filler_func_45(int a, int b) {
    int x = a * 7 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_45 alive
static volatile int keep_alive_45 = 0;
int filler_func_46(int a, int b) {
    int x = a * 8 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_46 alive
static volatile int keep_alive_46 = 0;
int filler_func_47(int a, int b) {
    int x = a * 9 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_47 alive
static volatile int keep_alive_47 = 0;
int filler_func_48(int a, int b) {
    int x = a * 10 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_48 alive
static volatile int keep_alive_48 = 0;
int filler_func_49(int a, int b) {
    int x = a * 11 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_49 alive
static volatile int keep_alive_49 = 0;
int filler_func_50(int a, int b) {
    int x = a * 12 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_50 alive
static volatile int keep_alive_50 = 0;
int filler_func_51(int a, int b) {
    int x = a * 13 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_51 alive
static volatile int keep_alive_51 = 0;
int filler_func_52(int a, int b) {
    int x = a * 1 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_52 alive
static volatile int keep_alive_52 = 0;
int filler_func_53(int a, int b) {
    int x = a * 2 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_53 alive
static volatile int keep_alive_53 = 0;
int filler_func_54(int a, int b) {
    int x = a * 3 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_54 alive
static volatile int keep_alive_54 = 0;
int filler_func_55(int a, int b) {
    int x = a * 4 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_55 alive
static volatile int keep_alive_55 = 0;
int filler_func_56(int a, int b) {
    int x = a * 5 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_56 alive
static volatile int keep_alive_56 = 0;
int filler_func_57(int a, int b) {
    int x = a * 6 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_57 alive
static volatile int keep_alive_57 = 0;
int filler_func_58(int a, int b) {
    int x = a * 7 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_58 alive
static volatile int keep_alive_58 = 0;
int filler_func_59(int a, int b) {
    int x = a * 8 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_59 alive
static volatile int keep_alive_59 = 0;
int filler_func_60(int a, int b) {
    int x = a * 9 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_60 alive
static volatile int keep_alive_60 = 0;
int filler_func_61(int a, int b) {
    int x = a * 10 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_61 alive
static volatile int keep_alive_61 = 0;
int filler_func_62(int a, int b) {
    int x = a * 11 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_62 alive
static volatile int keep_alive_62 = 0;
int filler_func_63(int a, int b) {
    int x = a * 12 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_63 alive
static volatile int keep_alive_63 = 0;
int filler_func_64(int a, int b) {
    int x = a * 13 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_64 alive
static volatile int keep_alive_64 = 0;
int filler_func_65(int a, int b) {
    int x = a * 1 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_65 alive
static volatile int keep_alive_65 = 0;
int filler_func_66(int a, int b) {
    int x = a * 2 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_66 alive
static volatile int keep_alive_66 = 0;
int filler_func_67(int a, int b) {
    int x = a * 3 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_67 alive
static volatile int keep_alive_67 = 0;
int filler_func_68(int a, int b) {
    int x = a * 4 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_68 alive
static volatile int keep_alive_68 = 0;
int filler_func_69(int a, int b) {
    int x = a * 5 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_69 alive
static volatile int keep_alive_69 = 0;
int filler_func_70(int a, int b) {
    int x = a * 6 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_70 alive
static volatile int keep_alive_70 = 0;
int filler_func_71(int a, int b) {
    int x = a * 7 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_71 alive
static volatile int keep_alive_71 = 0;
int filler_func_72(int a, int b) {
    int x = a * 8 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_72 alive
static volatile int keep_alive_72 = 0;
int filler_func_73(int a, int b) {
    int x = a * 9 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_73 alive
static volatile int keep_alive_73 = 0;
int filler_func_74(int a, int b) {
    int x = a * 10 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_74 alive
static volatile int keep_alive_74 = 0;
int filler_func_75(int a, int b) {
    int x = a * 11 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_75 alive
static volatile int keep_alive_75 = 0;
int filler_func_76(int a, int b) {
    int x = a * 12 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_76 alive
static volatile int keep_alive_76 = 0;
int filler_func_77(int a, int b) {
    int x = a * 13 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_77 alive
static volatile int keep_alive_77 = 0;
int filler_func_78(int a, int b) {
    int x = a * 1 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_78 alive
static volatile int keep_alive_78 = 0;
int filler_func_79(int a, int b) {
    int x = a * 2 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_79 alive
static volatile int keep_alive_79 = 0;
int filler_func_80(int a, int b) {
    int x = a * 3 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_80 alive
static volatile int keep_alive_80 = 0;
int filler_func_81(int a, int b) {
    int x = a * 4 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_81 alive
static volatile int keep_alive_81 = 0;
int filler_func_82(int a, int b) {
    int x = a * 5 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_82 alive
static volatile int keep_alive_82 = 0;
int filler_func_83(int a, int b) {
    int x = a * 6 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_83 alive
static volatile int keep_alive_83 = 0;
int filler_func_84(int a, int b) {
    int x = a * 7 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_84 alive
static volatile int keep_alive_84 = 0;
int filler_func_85(int a, int b) {
    int x = a * 8 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_85 alive
static volatile int keep_alive_85 = 0;
int filler_func_86(int a, int b) {
    int x = a * 9 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_86 alive
static volatile int keep_alive_86 = 0;
int filler_func_87(int a, int b) {
    int x = a * 10 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_87 alive
static volatile int keep_alive_87 = 0;
int filler_func_88(int a, int b) {
    int x = a * 11 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_88 alive
static volatile int keep_alive_88 = 0;
int filler_func_89(int a, int b) {
    int x = a * 12 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_89 alive
static volatile int keep_alive_89 = 0;
int filler_func_90(int a, int b) {
    int x = a * 13 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_90 alive
static volatile int keep_alive_90 = 0;
int filler_func_91(int a, int b) {
    int x = a * 1 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_91 alive
static volatile int keep_alive_91 = 0;
int filler_func_92(int a, int b) {
    int x = a * 2 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_92 alive
static volatile int keep_alive_92 = 0;
int filler_func_93(int a, int b) {
    int x = a * 3 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_93 alive
static volatile int keep_alive_93 = 0;
int filler_func_94(int a, int b) {
    int x = a * 4 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_94 alive
static volatile int keep_alive_94 = 0;
int filler_func_95(int a, int b) {
    int x = a * 5 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_95 alive
static volatile int keep_alive_95 = 0;
int filler_func_96(int a, int b) {
    int x = a * 6 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_96 alive
static volatile int keep_alive_96 = 0;
int filler_func_97(int a, int b) {
    int x = a * 7 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_97 alive
static volatile int keep_alive_97 = 0;
int filler_func_98(int a, int b) {
    int x = a * 8 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_98 alive
static volatile int keep_alive_98 = 0;
int filler_func_99(int a, int b) {
    int x = a * 9 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_99 alive
static volatile int keep_alive_99 = 0;
int filler_func_100(int a, int b) {
    int x = a * 10 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_100 alive
static volatile int keep_alive_100 = 0;
int filler_func_101(int a, int b) {
    int x = a * 11 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_101 alive
static volatile int keep_alive_101 = 0;
int filler_func_102(int a, int b) {
    int x = a * 12 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_102 alive
static volatile int keep_alive_102 = 0;
int filler_func_103(int a, int b) {
    int x = a * 13 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_103 alive
static volatile int keep_alive_103 = 0;
int filler_func_104(int a, int b) {
    int x = a * 1 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_104 alive
static volatile int keep_alive_104 = 0;
int filler_func_105(int a, int b) {
    int x = a * 2 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_105 alive
static volatile int keep_alive_105 = 0;
int filler_func_106(int a, int b) {
    int x = a * 3 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_106 alive
static volatile int keep_alive_106 = 0;
int filler_func_107(int a, int b) {
    int x = a * 4 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_107 alive
static volatile int keep_alive_107 = 0;
int filler_func_108(int a, int b) {
    int x = a * 5 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_108 alive
static volatile int keep_alive_108 = 0;
int filler_func_109(int a, int b) {
    int x = a * 6 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_109 alive
static volatile int keep_alive_109 = 0;
int filler_func_110(int a, int b) {
    int x = a * 7 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_110 alive
static volatile int keep_alive_110 = 0;
int filler_func_111(int a, int b) {
    int x = a * 8 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_111 alive
static volatile int keep_alive_111 = 0;
int filler_func_112(int a, int b) {
    int x = a * 9 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_112 alive
static volatile int keep_alive_112 = 0;
int filler_func_113(int a, int b) {
    int x = a * 10 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_113 alive
static volatile int keep_alive_113 = 0;
int filler_func_114(int a, int b) {
    int x = a * 11 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_114 alive
static volatile int keep_alive_114 = 0;
int filler_func_115(int a, int b) {
    int x = a * 12 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_115 alive
static volatile int keep_alive_115 = 0;
int filler_func_116(int a, int b) {
    int x = a * 13 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_116 alive
static volatile int keep_alive_116 = 0;
int filler_func_117(int a, int b) {
    int x = a * 1 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_117 alive
static volatile int keep_alive_117 = 0;
int filler_func_118(int a, int b) {
    int x = a * 2 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_118 alive
static volatile int keep_alive_118 = 0;
int filler_func_119(int a, int b) {
    int x = a * 3 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_119 alive
static volatile int keep_alive_119 = 0;
int filler_func_120(int a, int b) {
    int x = a * 4 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_120 alive
static volatile int keep_alive_120 = 0;
int filler_func_121(int a, int b) {
    int x = a * 5 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_121 alive
static volatile int keep_alive_121 = 0;
int filler_func_122(int a, int b) {
    int x = a * 6 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_122 alive
static volatile int keep_alive_122 = 0;
int filler_func_123(int a, int b) {
    int x = a * 7 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_123 alive
static volatile int keep_alive_123 = 0;
int filler_func_124(int a, int b) {
    int x = a * 8 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_124 alive
static volatile int keep_alive_124 = 0;
int filler_func_125(int a, int b) {
    int x = a * 9 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_125 alive
static volatile int keep_alive_125 = 0;
int filler_func_126(int a, int b) {
    int x = a * 10 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_126 alive
static volatile int keep_alive_126 = 0;
int filler_func_127(int a, int b) {
    int x = a * 11 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_127 alive
static volatile int keep_alive_127 = 0;
int filler_func_128(int a, int b) {
    int x = a * 12 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_128 alive
static volatile int keep_alive_128 = 0;
int filler_func_129(int a, int b) {
    int x = a * 13 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_129 alive
static volatile int keep_alive_129 = 0;
int filler_func_130(int a, int b) {
    int x = a * 1 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_130 alive
static volatile int keep_alive_130 = 0;
int filler_func_131(int a, int b) {
    int x = a * 2 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_131 alive
static volatile int keep_alive_131 = 0;
int filler_func_132(int a, int b) {
    int x = a * 3 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_132 alive
static volatile int keep_alive_132 = 0;
int filler_func_133(int a, int b) {
    int x = a * 4 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_133 alive
static volatile int keep_alive_133 = 0;
int filler_func_134(int a, int b) {
    int x = a * 5 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_134 alive
static volatile int keep_alive_134 = 0;
int filler_func_135(int a, int b) {
    int x = a * 6 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_135 alive
static volatile int keep_alive_135 = 0;
int filler_func_136(int a, int b) {
    int x = a * 7 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_136 alive
static volatile int keep_alive_136 = 0;
int filler_func_137(int a, int b) {
    int x = a * 8 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_137 alive
static volatile int keep_alive_137 = 0;
int filler_func_138(int a, int b) {
    int x = a * 9 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_138 alive
static volatile int keep_alive_138 = 0;
int filler_func_139(int a, int b) {
    int x = a * 10 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_139 alive
static volatile int keep_alive_139 = 0;
int filler_func_140(int a, int b) {
    int x = a * 11 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_140 alive
static volatile int keep_alive_140 = 0;
int filler_func_141(int a, int b) {
    int x = a * 12 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_141 alive
static volatile int keep_alive_141 = 0;
int filler_func_142(int a, int b) {
    int x = a * 13 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_142 alive
static volatile int keep_alive_142 = 0;
int filler_func_143(int a, int b) {
    int x = a * 1 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_143 alive
static volatile int keep_alive_143 = 0;
int filler_func_144(int a, int b) {
    int x = a * 2 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_144 alive
static volatile int keep_alive_144 = 0;
int filler_func_145(int a, int b) {
    int x = a * 3 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_145 alive
static volatile int keep_alive_145 = 0;
int filler_func_146(int a, int b) {
    int x = a * 4 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_146 alive
static volatile int keep_alive_146 = 0;
int filler_func_147(int a, int b) {
    int x = a * 5 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_147 alive
static volatile int keep_alive_147 = 0;
int filler_func_148(int a, int b) {
    int x = a * 6 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_148 alive
static volatile int keep_alive_148 = 0;
int filler_func_149(int a, int b) {
    int x = a * 7 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_149 alive
static volatile int keep_alive_149 = 0;
int filler_func_150(int a, int b) {
    int x = a * 8 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_150 alive
static volatile int keep_alive_150 = 0;
int filler_func_151(int a, int b) {
    int x = a * 9 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_151 alive
static volatile int keep_alive_151 = 0;
int filler_func_152(int a, int b) {
    int x = a * 10 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_152 alive
static volatile int keep_alive_152 = 0;
int filler_func_153(int a, int b) {
    int x = a * 11 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_153 alive
static volatile int keep_alive_153 = 0;
int filler_func_154(int a, int b) {
    int x = a * 12 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_154 alive
static volatile int keep_alive_154 = 0;
int filler_func_155(int a, int b) {
    int x = a * 13 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_155 alive
static volatile int keep_alive_155 = 0;
int filler_func_156(int a, int b) {
    int x = a * 1 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_156 alive
static volatile int keep_alive_156 = 0;
int filler_func_157(int a, int b) {
    int x = a * 2 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_157 alive
static volatile int keep_alive_157 = 0;
int filler_func_158(int a, int b) {
    int x = a * 3 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_158 alive
static volatile int keep_alive_158 = 0;
int filler_func_159(int a, int b) {
    int x = a * 4 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_159 alive
static volatile int keep_alive_159 = 0;
int filler_func_160(int a, int b) {
    int x = a * 5 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_160 alive
static volatile int keep_alive_160 = 0;
int filler_func_161(int a, int b) {
    int x = a * 6 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_161 alive
static volatile int keep_alive_161 = 0;
int filler_func_162(int a, int b) {
    int x = a * 7 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_162 alive
static volatile int keep_alive_162 = 0;
int filler_func_163(int a, int b) {
    int x = a * 8 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_163 alive
static volatile int keep_alive_163 = 0;
int filler_func_164(int a, int b) {
    int x = a * 9 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_164 alive
static volatile int keep_alive_164 = 0;
int filler_func_165(int a, int b) {
    int x = a * 10 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_165 alive
static volatile int keep_alive_165 = 0;
int filler_func_166(int a, int b) {
    int x = a * 11 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_166 alive
static volatile int keep_alive_166 = 0;
int filler_func_167(int a, int b) {
    int x = a * 12 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_167 alive
static volatile int keep_alive_167 = 0;
int filler_func_168(int a, int b) {
    int x = a * 13 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_168 alive
static volatile int keep_alive_168 = 0;
int filler_func_169(int a, int b) {
    int x = a * 1 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_169 alive
static volatile int keep_alive_169 = 0;
int filler_func_170(int a, int b) {
    int x = a * 2 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_170 alive
static volatile int keep_alive_170 = 0;
int filler_func_171(int a, int b) {
    int x = a * 3 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_171 alive
static volatile int keep_alive_171 = 0;
int filler_func_172(int a, int b) {
    int x = a * 4 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_172 alive
static volatile int keep_alive_172 = 0;
int filler_func_173(int a, int b) {
    int x = a * 5 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_173 alive
static volatile int keep_alive_173 = 0;
int filler_func_174(int a, int b) {
    int x = a * 6 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_174 alive
static volatile int keep_alive_174 = 0;
int filler_func_175(int a, int b) {
    int x = a * 7 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_175 alive
static volatile int keep_alive_175 = 0;
int filler_func_176(int a, int b) {
    int x = a * 8 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_176 alive
static volatile int keep_alive_176 = 0;
int filler_func_177(int a, int b) {
    int x = a * 9 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_177 alive
static volatile int keep_alive_177 = 0;
int filler_func_178(int a, int b) {
    int x = a * 10 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_178 alive
static volatile int keep_alive_178 = 0;
int filler_func_179(int a, int b) {
    int x = a * 11 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_179 alive
static volatile int keep_alive_179 = 0;
int filler_func_180(int a, int b) {
    int x = a * 12 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_180 alive
static volatile int keep_alive_180 = 0;
int filler_func_181(int a, int b) {
    int x = a * 13 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_181 alive
static volatile int keep_alive_181 = 0;
int filler_func_182(int a, int b) {
    int x = a * 1 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_182 alive
static volatile int keep_alive_182 = 0;
int filler_func_183(int a, int b) {
    int x = a * 2 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_183 alive
static volatile int keep_alive_183 = 0;
int filler_func_184(int a, int b) {
    int x = a * 3 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_184 alive
static volatile int keep_alive_184 = 0;
int filler_func_185(int a, int b) {
    int x = a * 4 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_185 alive
static volatile int keep_alive_185 = 0;
int filler_func_186(int a, int b) {
    int x = a * 5 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_186 alive
static volatile int keep_alive_186 = 0;
int filler_func_187(int a, int b) {
    int x = a * 6 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_187 alive
static volatile int keep_alive_187 = 0;
int filler_func_188(int a, int b) {
    int x = a * 7 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_188 alive
static volatile int keep_alive_188 = 0;
int filler_func_189(int a, int b) {
    int x = a * 8 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_189 alive
static volatile int keep_alive_189 = 0;
int filler_func_190(int a, int b) {
    int x = a * 9 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_190 alive
static volatile int keep_alive_190 = 0;
int filler_func_191(int a, int b) {
    int x = a * 10 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_191 alive
static volatile int keep_alive_191 = 0;
int filler_func_192(int a, int b) {
    int x = a * 11 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_192 alive
static volatile int keep_alive_192 = 0;
int filler_func_193(int a, int b) {
    int x = a * 12 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_193 alive
static volatile int keep_alive_193 = 0;
int filler_func_194(int a, int b) {
    int x = a * 13 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_194 alive
static volatile int keep_alive_194 = 0;
int filler_func_195(int a, int b) {
    int x = a * 1 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_195 alive
static volatile int keep_alive_195 = 0;
int filler_func_196(int a, int b) {
    int x = a * 2 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_196 alive
static volatile int keep_alive_196 = 0;
int filler_func_197(int a, int b) {
    int x = a * 3 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_197 alive
static volatile int keep_alive_197 = 0;
int filler_func_198(int a, int b) {
    int x = a * 4 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_198 alive
static volatile int keep_alive_198 = 0;
int filler_func_199(int a, int b) {
    int x = a * 5 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_199 alive
static volatile int keep_alive_199 = 0;
int filler_func_200(int a, int b) {
    int x = a * 6 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_200 alive
static volatile int keep_alive_200 = 0;
int filler_func_201(int a, int b) {
    int x = a * 7 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_201 alive
static volatile int keep_alive_201 = 0;
int filler_func_202(int a, int b) {
    int x = a * 8 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_202 alive
static volatile int keep_alive_202 = 0;
int filler_func_203(int a, int b) {
    int x = a * 9 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_203 alive
static volatile int keep_alive_203 = 0;
int filler_func_204(int a, int b) {
    int x = a * 10 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_204 alive
static volatile int keep_alive_204 = 0;
int filler_func_205(int a, int b) {
    int x = a * 11 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_205 alive
static volatile int keep_alive_205 = 0;
int filler_func_206(int a, int b) {
    int x = a * 12 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_206 alive
static volatile int keep_alive_206 = 0;
int filler_func_207(int a, int b) {
    int x = a * 13 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_207 alive
static volatile int keep_alive_207 = 0;
int filler_func_208(int a, int b) {
    int x = a * 1 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_208 alive
static volatile int keep_alive_208 = 0;
int filler_func_209(int a, int b) {
    int x = a * 2 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_209 alive
static volatile int keep_alive_209 = 0;
int filler_func_210(int a, int b) {
    int x = a * 3 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_210 alive
static volatile int keep_alive_210 = 0;
int filler_func_211(int a, int b) {
    int x = a * 4 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_211 alive
static volatile int keep_alive_211 = 0;
int filler_func_212(int a, int b) {
    int x = a * 5 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_212 alive
static volatile int keep_alive_212 = 0;
int filler_func_213(int a, int b) {
    int x = a * 6 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_213 alive
static volatile int keep_alive_213 = 0;
int filler_func_214(int a, int b) {
    int x = a * 7 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_214 alive
static volatile int keep_alive_214 = 0;
int filler_func_215(int a, int b) {
    int x = a * 8 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_215 alive
static volatile int keep_alive_215 = 0;
int filler_func_216(int a, int b) {
    int x = a * 9 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_216 alive
static volatile int keep_alive_216 = 0;
int filler_func_217(int a, int b) {
    int x = a * 10 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_217 alive
static volatile int keep_alive_217 = 0;
int filler_func_218(int a, int b) {
    int x = a * 11 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_218 alive
static volatile int keep_alive_218 = 0;
int filler_func_219(int a, int b) {
    int x = a * 12 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_219 alive
static volatile int keep_alive_219 = 0;
int filler_func_220(int a, int b) {
    int x = a * 13 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_220 alive
static volatile int keep_alive_220 = 0;
int filler_func_221(int a, int b) {
    int x = a * 1 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_221 alive
static volatile int keep_alive_221 = 0;
int filler_func_222(int a, int b) {
    int x = a * 2 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_222 alive
static volatile int keep_alive_222 = 0;
int filler_func_223(int a, int b) {
    int x = a * 3 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_223 alive
static volatile int keep_alive_223 = 0;
int filler_func_224(int a, int b) {
    int x = a * 4 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_224 alive
static volatile int keep_alive_224 = 0;
int filler_func_225(int a, int b) {
    int x = a * 5 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_225 alive
static volatile int keep_alive_225 = 0;
int filler_func_226(int a, int b) {
    int x = a * 6 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_226 alive
static volatile int keep_alive_226 = 0;
int filler_func_227(int a, int b) {
    int x = a * 7 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_227 alive
static volatile int keep_alive_227 = 0;
int filler_func_228(int a, int b) {
    int x = a * 8 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_228 alive
static volatile int keep_alive_228 = 0;
int filler_func_229(int a, int b) {
    int x = a * 9 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_229 alive
static volatile int keep_alive_229 = 0;
int filler_func_230(int a, int b) {
    int x = a * 10 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_230 alive
static volatile int keep_alive_230 = 0;
int filler_func_231(int a, int b) {
    int x = a * 11 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_231 alive
static volatile int keep_alive_231 = 0;
int filler_func_232(int a, int b) {
    int x = a * 12 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_232 alive
static volatile int keep_alive_232 = 0;
int filler_func_233(int a, int b) {
    int x = a * 13 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_233 alive
static volatile int keep_alive_233 = 0;
int filler_func_234(int a, int b) {
    int x = a * 1 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_234 alive
static volatile int keep_alive_234 = 0;
int filler_func_235(int a, int b) {
    int x = a * 2 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_235 alive
static volatile int keep_alive_235 = 0;
int filler_func_236(int a, int b) {
    int x = a * 3 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_236 alive
static volatile int keep_alive_236 = 0;
int filler_func_237(int a, int b) {
    int x = a * 4 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_237 alive
static volatile int keep_alive_237 = 0;
int filler_func_238(int a, int b) {
    int x = a * 5 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_238 alive
static volatile int keep_alive_238 = 0;
int filler_func_239(int a, int b) {
    int x = a * 6 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_239 alive
static volatile int keep_alive_239 = 0;
int filler_func_240(int a, int b) {
    int x = a * 7 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_240 alive
static volatile int keep_alive_240 = 0;
int filler_func_241(int a, int b) {
    int x = a * 8 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_241 alive
static volatile int keep_alive_241 = 0;
int filler_func_242(int a, int b) {
    int x = a * 9 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_242 alive
static volatile int keep_alive_242 = 0;
int filler_func_243(int a, int b) {
    int x = a * 10 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_243 alive
static volatile int keep_alive_243 = 0;
int filler_func_244(int a, int b) {
    int x = a * 11 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_244 alive
static volatile int keep_alive_244 = 0;
int filler_func_245(int a, int b) {
    int x = a * 12 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_245 alive
static volatile int keep_alive_245 = 0;
int filler_func_246(int a, int b) {
    int x = a * 13 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_246 alive
static volatile int keep_alive_246 = 0;
int filler_func_247(int a, int b) {
    int x = a * 1 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_247 alive
static volatile int keep_alive_247 = 0;
int filler_func_248(int a, int b) {
    int x = a * 2 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_248 alive
static volatile int keep_alive_248 = 0;
int filler_func_249(int a, int b) {
    int x = a * 3 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_249 alive
static volatile int keep_alive_249 = 0;
int filler_func_250(int a, int b) {
    int x = a * 4 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_250 alive
static volatile int keep_alive_250 = 0;
int filler_func_251(int a, int b) {
    int x = a * 5 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_251 alive
static volatile int keep_alive_251 = 0;
int filler_func_252(int a, int b) {
    int x = a * 6 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_252 alive
static volatile int keep_alive_252 = 0;
int filler_func_253(int a, int b) {
    int x = a * 7 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_253 alive
static volatile int keep_alive_253 = 0;
int filler_func_254(int a, int b) {
    int x = a * 8 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_254 alive
static volatile int keep_alive_254 = 0;
int filler_func_255(int a, int b) {
    int x = a * 9 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_255 alive
static volatile int keep_alive_255 = 0;
int filler_func_256(int a, int b) {
    int x = a * 10 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_256 alive
static volatile int keep_alive_256 = 0;
int filler_func_257(int a, int b) {
    int x = a * 11 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_257 alive
static volatile int keep_alive_257 = 0;
int filler_func_258(int a, int b) {
    int x = a * 12 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_258 alive
static volatile int keep_alive_258 = 0;
int filler_func_259(int a, int b) {
    int x = a * 13 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_259 alive
static volatile int keep_alive_259 = 0;
int filler_func_260(int a, int b) {
    int x = a * 1 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_260 alive
static volatile int keep_alive_260 = 0;
int filler_func_261(int a, int b) {
    int x = a * 2 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_261 alive
static volatile int keep_alive_261 = 0;
int filler_func_262(int a, int b) {
    int x = a * 3 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_262 alive
static volatile int keep_alive_262 = 0;
int filler_func_263(int a, int b) {
    int x = a * 4 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_263 alive
static volatile int keep_alive_263 = 0;
int filler_func_264(int a, int b) {
    int x = a * 5 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_264 alive
static volatile int keep_alive_264 = 0;
int filler_func_265(int a, int b) {
    int x = a * 6 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_265 alive
static volatile int keep_alive_265 = 0;
int filler_func_266(int a, int b) {
    int x = a * 7 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_266 alive
static volatile int keep_alive_266 = 0;
int filler_func_267(int a, int b) {
    int x = a * 8 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_267 alive
static volatile int keep_alive_267 = 0;
int filler_func_268(int a, int b) {
    int x = a * 9 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_268 alive
static volatile int keep_alive_268 = 0;
int filler_func_269(int a, int b) {
    int x = a * 10 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_269 alive
static volatile int keep_alive_269 = 0;
int filler_func_270(int a, int b) {
    int x = a * 11 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_270 alive
static volatile int keep_alive_270 = 0;
int filler_func_271(int a, int b) {
    int x = a * 12 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_271 alive
static volatile int keep_alive_271 = 0;
int filler_func_272(int a, int b) {
    int x = a * 13 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_272 alive
static volatile int keep_alive_272 = 0;
int filler_func_273(int a, int b) {
    int x = a * 1 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_273 alive
static volatile int keep_alive_273 = 0;
int filler_func_274(int a, int b) {
    int x = a * 2 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_274 alive
static volatile int keep_alive_274 = 0;
int filler_func_275(int a, int b) {
    int x = a * 3 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_275 alive
static volatile int keep_alive_275 = 0;
int filler_func_276(int a, int b) {
    int x = a * 4 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_276 alive
static volatile int keep_alive_276 = 0;
int filler_func_277(int a, int b) {
    int x = a * 5 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_277 alive
static volatile int keep_alive_277 = 0;
int filler_func_278(int a, int b) {
    int x = a * 6 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_278 alive
static volatile int keep_alive_278 = 0;
int filler_func_279(int a, int b) {
    int x = a * 7 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_279 alive
static volatile int keep_alive_279 = 0;
int filler_func_280(int a, int b) {
    int x = a * 8 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_280 alive
static volatile int keep_alive_280 = 0;
int filler_func_281(int a, int b) {
    int x = a * 9 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_281 alive
static volatile int keep_alive_281 = 0;
int filler_func_282(int a, int b) {
    int x = a * 10 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_282 alive
static volatile int keep_alive_282 = 0;
int filler_func_283(int a, int b) {
    int x = a * 11 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_283 alive
static volatile int keep_alive_283 = 0;
int filler_func_284(int a, int b) {
    int x = a * 12 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_284 alive
static volatile int keep_alive_284 = 0;
int filler_func_285(int a, int b) {
    int x = a * 13 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_285 alive
static volatile int keep_alive_285 = 0;
int filler_func_286(int a, int b) {
    int x = a * 1 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_286 alive
static volatile int keep_alive_286 = 0;
int filler_func_287(int a, int b) {
    int x = a * 2 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_287 alive
static volatile int keep_alive_287 = 0;
int filler_func_288(int a, int b) {
    int x = a * 3 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_288 alive
static volatile int keep_alive_288 = 0;
int filler_func_289(int a, int b) {
    int x = a * 4 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_289 alive
static volatile int keep_alive_289 = 0;
int filler_func_290(int a, int b) {
    int x = a * 5 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_290 alive
static volatile int keep_alive_290 = 0;
int filler_func_291(int a, int b) {
    int x = a * 6 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_291 alive
static volatile int keep_alive_291 = 0;
int filler_func_292(int a, int b) {
    int x = a * 7 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_292 alive
static volatile int keep_alive_292 = 0;
int filler_func_293(int a, int b) {
    int x = a * 8 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_293 alive
static volatile int keep_alive_293 = 0;
int filler_func_294(int a, int b) {
    int x = a * 9 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_294 alive
static volatile int keep_alive_294 = 0;
int filler_func_295(int a, int b) {
    int x = a * 10 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_295 alive
static volatile int keep_alive_295 = 0;
int filler_func_296(int a, int b) {
    int x = a * 11 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_296 alive
static volatile int keep_alive_296 = 0;
int filler_func_297(int a, int b) {
    int x = a * 12 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_297 alive
static volatile int keep_alive_297 = 0;
int filler_func_298(int a, int b) {
    int x = a * 13 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_298 alive
static volatile int keep_alive_298 = 0;
int filler_func_299(int a, int b) {
    int x = a * 1 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_299 alive
static volatile int keep_alive_299 = 0;
int filler_func_300(int a, int b) {
    int x = a * 2 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_300 alive
static volatile int keep_alive_300 = 0;
int filler_func_301(int a, int b) {
    int x = a * 3 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_301 alive
static volatile int keep_alive_301 = 0;
int filler_func_302(int a, int b) {
    int x = a * 4 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_302 alive
static volatile int keep_alive_302 = 0;
int filler_func_303(int a, int b) {
    int x = a * 5 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_303 alive
static volatile int keep_alive_303 = 0;
int filler_func_304(int a, int b) {
    int x = a * 6 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_304 alive
static volatile int keep_alive_304 = 0;
int filler_func_305(int a, int b) {
    int x = a * 7 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_305 alive
static volatile int keep_alive_305 = 0;
int filler_func_306(int a, int b) {
    int x = a * 8 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_306 alive
static volatile int keep_alive_306 = 0;
int filler_func_307(int a, int b) {
    int x = a * 9 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_307 alive
static volatile int keep_alive_307 = 0;
int filler_func_308(int a, int b) {
    int x = a * 10 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_308 alive
static volatile int keep_alive_308 = 0;
int filler_func_309(int a, int b) {
    int x = a * 11 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_309 alive
static volatile int keep_alive_309 = 0;
int filler_func_310(int a, int b) {
    int x = a * 12 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_310 alive
static volatile int keep_alive_310 = 0;
int filler_func_311(int a, int b) {
    int x = a * 13 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_311 alive
static volatile int keep_alive_311 = 0;
int filler_func_312(int a, int b) {
    int x = a * 1 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_312 alive
static volatile int keep_alive_312 = 0;
int filler_func_313(int a, int b) {
    int x = a * 2 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_313 alive
static volatile int keep_alive_313 = 0;
int filler_func_314(int a, int b) {
    int x = a * 3 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_314 alive
static volatile int keep_alive_314 = 0;
int filler_func_315(int a, int b) {
    int x = a * 4 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_315 alive
static volatile int keep_alive_315 = 0;
int filler_func_316(int a, int b) {
    int x = a * 5 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_316 alive
static volatile int keep_alive_316 = 0;
int filler_func_317(int a, int b) {
    int x = a * 6 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_317 alive
static volatile int keep_alive_317 = 0;
int filler_func_318(int a, int b) {
    int x = a * 7 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_318 alive
static volatile int keep_alive_318 = 0;
int filler_func_319(int a, int b) {
    int x = a * 8 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_319 alive
static volatile int keep_alive_319 = 0;
int filler_func_320(int a, int b) {
    int x = a * 9 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_320 alive
static volatile int keep_alive_320 = 0;
int filler_func_321(int a, int b) {
    int x = a * 10 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_321 alive
static volatile int keep_alive_321 = 0;
int filler_func_322(int a, int b) {
    int x = a * 11 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_322 alive
static volatile int keep_alive_322 = 0;
int filler_func_323(int a, int b) {
    int x = a * 12 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_323 alive
static volatile int keep_alive_323 = 0;
int filler_func_324(int a, int b) {
    int x = a * 13 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_324 alive
static volatile int keep_alive_324 = 0;
int filler_func_325(int a, int b) {
    int x = a * 1 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_325 alive
static volatile int keep_alive_325 = 0;
int filler_func_326(int a, int b) {
    int x = a * 2 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_326 alive
static volatile int keep_alive_326 = 0;
int filler_func_327(int a, int b) {
    int x = a * 3 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_327 alive
static volatile int keep_alive_327 = 0;
int filler_func_328(int a, int b) {
    int x = a * 4 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_328 alive
static volatile int keep_alive_328 = 0;
int filler_func_329(int a, int b) {
    int x = a * 5 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_329 alive
static volatile int keep_alive_329 = 0;
int filler_func_330(int a, int b) {
    int x = a * 6 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_330 alive
static volatile int keep_alive_330 = 0;
int filler_func_331(int a, int b) {
    int x = a * 7 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_331 alive
static volatile int keep_alive_331 = 0;
int filler_func_332(int a, int b) {
    int x = a * 8 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_332 alive
static volatile int keep_alive_332 = 0;
int filler_func_333(int a, int b) {
    int x = a * 9 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_333 alive
static volatile int keep_alive_333 = 0;
int filler_func_334(int a, int b) {
    int x = a * 10 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_334 alive
static volatile int keep_alive_334 = 0;
int filler_func_335(int a, int b) {
    int x = a * 11 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_335 alive
static volatile int keep_alive_335 = 0;
int filler_func_336(int a, int b) {
    int x = a * 12 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_336 alive
static volatile int keep_alive_336 = 0;
int filler_func_337(int a, int b) {
    int x = a * 13 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_337 alive
static volatile int keep_alive_337 = 0;
int filler_func_338(int a, int b) {
    int x = a * 1 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_338 alive
static volatile int keep_alive_338 = 0;
int filler_func_339(int a, int b) {
    int x = a * 2 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_339 alive
static volatile int keep_alive_339 = 0;
int filler_func_340(int a, int b) {
    int x = a * 3 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_340 alive
static volatile int keep_alive_340 = 0;
int filler_func_341(int a, int b) {
    int x = a * 4 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_341 alive
static volatile int keep_alive_341 = 0;
int filler_func_342(int a, int b) {
    int x = a * 5 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_342 alive
static volatile int keep_alive_342 = 0;
int filler_func_343(int a, int b) {
    int x = a * 6 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_343 alive
static volatile int keep_alive_343 = 0;
int filler_func_344(int a, int b) {
    int x = a * 7 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_344 alive
static volatile int keep_alive_344 = 0;
int filler_func_345(int a, int b) {
    int x = a * 8 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_345 alive
static volatile int keep_alive_345 = 0;
int filler_func_346(int a, int b) {
    int x = a * 9 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_346 alive
static volatile int keep_alive_346 = 0;
int filler_func_347(int a, int b) {
    int x = a * 10 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_347 alive
static volatile int keep_alive_347 = 0;
int filler_func_348(int a, int b) {
    int x = a * 11 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_348 alive
static volatile int keep_alive_348 = 0;
int filler_func_349(int a, int b) {
    int x = a * 12 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_349 alive
static volatile int keep_alive_349 = 0;
int filler_func_350(int a, int b) {
    int x = a * 13 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_350 alive
static volatile int keep_alive_350 = 0;
int filler_func_351(int a, int b) {
    int x = a * 1 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_351 alive
static volatile int keep_alive_351 = 0;
int filler_func_352(int a, int b) {
    int x = a * 2 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_352 alive
static volatile int keep_alive_352 = 0;
int filler_func_353(int a, int b) {
    int x = a * 3 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_353 alive
static volatile int keep_alive_353 = 0;
int filler_func_354(int a, int b) {
    int x = a * 4 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_354 alive
static volatile int keep_alive_354 = 0;
int filler_func_355(int a, int b) {
    int x = a * 5 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_355 alive
static volatile int keep_alive_355 = 0;
int filler_func_356(int a, int b) {
    int x = a * 6 + b * 7 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_356 alive
static volatile int keep_alive_356 = 0;
int filler_func_357(int a, int b) {
    int x = a * 7 + b * 1 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_357 alive
static volatile int keep_alive_357 = 0;
int filler_func_358(int a, int b) {
    int x = a * 8 + b * 2 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_358 alive
static volatile int keep_alive_358 = 0;
int filler_func_359(int a, int b) {
    int x = a * 9 + b * 3 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_359 alive
static volatile int keep_alive_359 = 0;
int filler_func_360(int a, int b) {
    int x = a * 10 + b * 4 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_360 alive
static volatile int keep_alive_360 = 0;
int filler_func_361(int a, int b) {
    int x = a * 11 + b * 5 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_361 alive
static volatile int keep_alive_361 = 0;
int filler_func_362(int a, int b) {
    int x = a * 12 + b * 6 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_362 alive
static volatile int keep_alive_362 = 0;
int filler_func_363(int a, int b) {
    int x = a * 13 + b * 7 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_363 alive
static volatile int keep_alive_363 = 0;
int filler_func_364(int a, int b) {
    int x = a * 1 + b * 1 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_364 alive
static volatile int keep_alive_364 = 0;
int filler_func_365(int a, int b) {
    int x = a * 2 + b * 2 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_365 alive
static volatile int keep_alive_365 = 0;
int filler_func_366(int a, int b) {
    int x = a * 3 + b * 3 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_366 alive
static volatile int keep_alive_366 = 0;
int filler_func_367(int a, int b) {
    int x = a * 4 + b * 4 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_367 alive
static volatile int keep_alive_367 = 0;
int filler_func_368(int a, int b) {
    int x = a * 5 + b * 5 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_368 alive
static volatile int keep_alive_368 = 0;
int filler_func_369(int a, int b) {
    int x = a * 6 + b * 6 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_369 alive
static volatile int keep_alive_369 = 0;
int filler_func_370(int a, int b) {
    int x = a * 7 + b * 7 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_370 alive
static volatile int keep_alive_370 = 0;
int filler_func_371(int a, int b) {
    int x = a * 8 + b * 1 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_371 alive
static volatile int keep_alive_371 = 0;
int filler_func_372(int a, int b) {
    int x = a * 9 + b * 2 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_372 alive
static volatile int keep_alive_372 = 0;
int filler_func_373(int a, int b) {
    int x = a * 10 + b * 3 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_373 alive
static volatile int keep_alive_373 = 0;
int filler_func_374(int a, int b) {
    int x = a * 11 + b * 4 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_374 alive
static volatile int keep_alive_374 = 0;
int filler_func_375(int a, int b) {
    int x = a * 12 + b * 5 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_375 alive
static volatile int keep_alive_375 = 0;
int filler_func_376(int a, int b) {
    int x = a * 13 + b * 6 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_376 alive
static volatile int keep_alive_376 = 0;
int filler_func_377(int a, int b) {
    int x = a * 1 + b * 7 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_377 alive
static volatile int keep_alive_377 = 0;
int filler_func_378(int a, int b) {
    int x = a * 2 + b * 1 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_378 alive
static volatile int keep_alive_378 = 0;
int filler_func_379(int a, int b) {
    int x = a * 3 + b * 2 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_379 alive
static volatile int keep_alive_379 = 0;
int filler_func_380(int a, int b) {
    int x = a * 4 + b * 3 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_380 alive
static volatile int keep_alive_380 = 0;
int filler_func_381(int a, int b) {
    int x = a * 5 + b * 4 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_381 alive
static volatile int keep_alive_381 = 0;
int filler_func_382(int a, int b) {
    int x = a * 6 + b * 5 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_382 alive
static volatile int keep_alive_382 = 0;
int filler_func_383(int a, int b) {
    int x = a * 7 + b * 6 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_383 alive
static volatile int keep_alive_383 = 0;
int filler_func_384(int a, int b) {
    int x = a * 8 + b * 7 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_384 alive
static volatile int keep_alive_384 = 0;
int filler_func_385(int a, int b) {
    int x = a * 9 + b * 1 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_385 alive
static volatile int keep_alive_385 = 0;
int filler_func_386(int a, int b) {
    int x = a * 10 + b * 2 + 1;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_386 alive
static volatile int keep_alive_386 = 0;
int filler_func_387(int a, int b) {
    int x = a * 11 + b * 3 + 2;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_387 alive
static volatile int keep_alive_387 = 0;
int filler_func_388(int a, int b) {
    int x = a * 12 + b * 4 + 3;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_388 alive
static volatile int keep_alive_388 = 0;
int filler_func_389(int a, int b) {
    int x = a * 13 + b * 5 + 4;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_389 alive
static volatile int keep_alive_389 = 0;
int filler_func_390(int a, int b) {
    int x = a * 1 + b * 6 + 0;
    x ^= (x << 5) | (x >> 3);
    x += (a ^ b) & 0xff;
    x = (int)((unsigned)x * 1664525u + 1013904223u) ^ (x >> 7);
    return x;
}
// keep symbol filler_func_390 alive
static volatile int keep_alive_390 = 0;

// use many filler functions in a loop by indexing into an array of function pointers
void fill_func_in_array() {
    using fptr_t = int(*)(int,int);
    static fptr_t fptrs[390];
    // populate function pointer table (unique functions)
    fptrs[0] = &filler_func_1;
    fptrs[1] = &filler_func_2;
    fptrs[2] = &filler_func_3;
    fptrs[3] = &filler_func_4;
    fptrs[4] = &filler_func_5;
    fptrs[5] = &filler_func_6;
    fptrs[6] = &filler_func_7;
    fptrs[7] = &filler_func_8;
    fptrs[8] = &filler_func_9;
    fptrs[9] = &filler_func_10;
    fptrs[10] = &filler_func_11;
    fptrs[11] = &filler_func_12;
    fptrs[12] = &filler_func_13;
    fptrs[13] = &filler_func_14;
    fptrs[14] = &filler_func_15;
    fptrs[15] = &filler_func_16;
    fptrs[16] = &filler_func_17;
    fptrs[17] = &filler_func_18;
    fptrs[18] = &filler_func_19;
    fptrs[19] = &filler_func_20;
    fptrs[20] = &filler_func_21;
    fptrs[21] = &filler_func_22;
    fptrs[22] = &filler_func_23;
    fptrs[23] = &filler_func_24;
    fptrs[24] = &filler_func_25;
    fptrs[25] = &filler_func_26;
    fptrs[26] = &filler_func_27;
    fptrs[27] = &filler_func_28;
    fptrs[28] = &filler_func_29;
    fptrs[29] = &filler_func_30;
    fptrs[30] = &filler_func_31;
    fptrs[31] = &filler_func_32;
    fptrs[32] = &filler_func_33;
    fptrs[33] = &filler_func_34;
    fptrs[34] = &filler_func_35;
    fptrs[35] = &filler_func_36;
    fptrs[36] = &filler_func_37;
    fptrs[37] = &filler_func_38;
    fptrs[38] = &filler_func_39;
    fptrs[39] = &filler_func_40;
    fptrs[40] = &filler_func_41;
    fptrs[41] = &filler_func_42;
    fptrs[42] = &filler_func_43;
    fptrs[43] = &filler_func_44;
    fptrs[44] = &filler_func_45;
    fptrs[45] = &filler_func_46;
    fptrs[46] = &filler_func_47;
    fptrs[47] = &filler_func_48;
    fptrs[48] = &filler_func_49;
    fptrs[49] = &filler_func_50;
    fptrs[50] = &filler_func_51;
    fptrs[51] = &filler_func_52;
    fptrs[52] = &filler_func_53;
    fptrs[53] = &filler_func_54;
    fptrs[54] = &filler_func_55;
    fptrs[55] = &filler_func_56;
    fptrs[56] = &filler_func_57;
    fptrs[57] = &filler_func_58;
    fptrs[58] = &filler_func_59;
    fptrs[59] = &filler_func_60;
    fptrs[60] = &filler_func_61;
    fptrs[61] = &filler_func_62;
    fptrs[62] = &filler_func_63;
    fptrs[63] = &filler_func_64;
    fptrs[64] = &filler_func_65;
    fptrs[65] = &filler_func_66;
    fptrs[66] = &filler_func_67;
    fptrs[67] = &filler_func_68;
    fptrs[68] = &filler_func_69;
    fptrs[69] = &filler_func_70;
    fptrs[70] = &filler_func_71;
    fptrs[71] = &filler_func_72;
    fptrs[72] = &filler_func_73;
    fptrs[73] = &filler_func_74;
    fptrs[74] = &filler_func_75;
    fptrs[75] = &filler_func_76;
    fptrs[76] = &filler_func_77;
    fptrs[77] = &filler_func_78;
    fptrs[78] = &filler_func_79;
    fptrs[79] = &filler_func_80;
    fptrs[80] = &filler_func_81;
    fptrs[81] = &filler_func_82;
    fptrs[82] = &filler_func_83;
    fptrs[83] = &filler_func_84;
    fptrs[84] = &filler_func_85;
    fptrs[85] = &filler_func_86;
    fptrs[86] = &filler_func_87;
    fptrs[87] = &filler_func_88;
    fptrs[88] = &filler_func_89;
    fptrs[89] = &filler_func_90;
    fptrs[90] = &filler_func_91;
    fptrs[91] = &filler_func_92;
    fptrs[92] = &filler_func_93;
    fptrs[93] = &filler_func_94;
    fptrs[94] = &filler_func_95;
    fptrs[95] = &filler_func_96;
    fptrs[96] = &filler_func_97;
    fptrs[97] = &filler_func_98;
    fptrs[98] = &filler_func_99;
    fptrs[99] = &filler_func_100;
    fptrs[100] = &filler_func_101;
    fptrs[101] = &filler_func_102;
    fptrs[102] = &filler_func_103;
    fptrs[103] = &filler_func_104;
    fptrs[104] = &filler_func_105;
    fptrs[105] = &filler_func_106;
    fptrs[106] = &filler_func_107;
    fptrs[107] = &filler_func_108;
    fptrs[108] = &filler_func_109;
    fptrs[109] = &filler_func_110;
    fptrs[110] = &filler_func_111;
    fptrs[111] = &filler_func_112;
    fptrs[112] = &filler_func_113;
    fptrs[113] = &filler_func_114;
    fptrs[114] = &filler_func_115;
    fptrs[115] = &filler_func_116;
    fptrs[116] = &filler_func_117;
    fptrs[117] = &filler_func_118;
    fptrs[118] = &filler_func_119;
    fptrs[119] = &filler_func_120;
    fptrs[120] = &filler_func_121;
    fptrs[121] = &filler_func_122;
    fptrs[122] = &filler_func_123;
    fptrs[123] = &filler_func_124;
    fptrs[124] = &filler_func_125;
    fptrs[125] = &filler_func_126;
    fptrs[126] = &filler_func_127;
    fptrs[127] = &filler_func_128;
    fptrs[128] = &filler_func_129;
    fptrs[129] = &filler_func_130;
    fptrs[130] = &filler_func_131;
    fptrs[131] = &filler_func_132;
    fptrs[132] = &filler_func_133;
    fptrs[133] = &filler_func_134;
    fptrs[134] = &filler_func_135;
    fptrs[135] = &filler_func_136;
    fptrs[136] = &filler_func_137;
    fptrs[137] = &filler_func_138;
    fptrs[138] = &filler_func_139;
    fptrs[139] = &filler_func_140;
    fptrs[140] = &filler_func_141;
    fptrs[141] = &filler_func_142;
    fptrs[142] = &filler_func_143;
    fptrs[143] = &filler_func_144;
    fptrs[144] = &filler_func_145;
    fptrs[145] = &filler_func_146;
    fptrs[146] = &filler_func_147;
    fptrs[147] = &filler_func_148;
    fptrs[148] = &filler_func_149;
    fptrs[149] = &filler_func_150;
    fptrs[150] = &filler_func_151;
    fptrs[151] = &filler_func_152;
    fptrs[152] = &filler_func_153;
    fptrs[153] = &filler_func_154;
    fptrs[154] = &filler_func_155;
    fptrs[155] = &filler_func_156;
    fptrs[156] = &filler_func_157;
    fptrs[157] = &filler_func_158;
    fptrs[158] = &filler_func_159;
    fptrs[159] = &filler_func_160;
    fptrs[160] = &filler_func_161;
    fptrs[161] = &filler_func_162;
    fptrs[162] = &filler_func_163;
    fptrs[163] = &filler_func_164;
    fptrs[164] = &filler_func_165;
    fptrs[165] = &filler_func_166;
    fptrs[166] = &filler_func_167;
    fptrs[167] = &filler_func_168;
    fptrs[168] = &filler_func_169;
    fptrs[169] = &filler_func_170;
    fptrs[170] = &filler_func_171;
    fptrs[171] = &filler_func_172;
    fptrs[172] = &filler_func_173;
    fptrs[173] = &filler_func_174;
    fptrs[174] = &filler_func_175;
    fptrs[175] = &filler_func_176;
    fptrs[176] = &filler_func_177;
    fptrs[177] = &filler_func_178;
    fptrs[178] = &filler_func_179;
    fptrs[179] = &filler_func_180;
    fptrs[180] = &filler_func_181;
    fptrs[181] = &filler_func_182;
    fptrs[182] = &filler_func_183;
    fptrs[183] = &filler_func_184;
    fptrs[184] = &filler_func_185;
    fptrs[185] = &filler_func_186;
    fptrs[186] = &filler_func_187;
    fptrs[187] = &filler_func_188;
    fptrs[188] = &filler_func_189;
    fptrs[189] = &filler_func_190;
    fptrs[190] = &filler_func_191;
    fptrs[191] = &filler_func_192;
    fptrs[192] = &filler_func_193;
    fptrs[193] = &filler_func_194;
    fptrs[194] = &filler_func_195;
    fptrs[195] = &filler_func_196;
    fptrs[196] = &filler_func_197;
    fptrs[197] = &filler_func_198;
    fptrs[198] = &filler_func_199;
    fptrs[199] = &filler_func_200;
    fptrs[200] = &filler_func_201;
    fptrs[201] = &filler_func_202;
    fptrs[202] = &filler_func_203;
    fptrs[203] = &filler_func_204;
    fptrs[204] = &filler_func_205;
    fptrs[205] = &filler_func_206;
    fptrs[206] = &filler_func_207;
    fptrs[207] = &filler_func_208;
    fptrs[208] = &filler_func_209;
    fptrs[209] = &filler_func_210;
    fptrs[210] = &filler_func_211;
    fptrs[211] = &filler_func_212;
    fptrs[212] = &filler_func_213;
    fptrs[213] = &filler_func_214;
    fptrs[214] = &filler_func_215;
    fptrs[215] = &filler_func_216;
    fptrs[216] = &filler_func_217;
    fptrs[217] = &filler_func_218;
    fptrs[218] = &filler_func_219;
    fptrs[219] = &filler_func_220;
    fptrs[220] = &filler_func_221;
    fptrs[221] = &filler_func_222;
    fptrs[222] = &filler_func_223;
    fptrs[223] = &filler_func_224;
    fptrs[224] = &filler_func_225;
    fptrs[225] = &filler_func_226;
    fptrs[226] = &filler_func_227;
    fptrs[227] = &filler_func_228;
    fptrs[228] = &filler_func_229;
    fptrs[229] = &filler_func_230;
    fptrs[230] = &filler_func_231;
    fptrs[231] = &filler_func_232;
    fptrs[232] = &filler_func_233;
    fptrs[233] = &filler_func_234;
    fptrs[234] = &filler_func_235;
    fptrs[235] = &filler_func_236;
    fptrs[236] = &filler_func_237;
    fptrs[237] = &filler_func_238;
    fptrs[238] = &filler_func_239;
    fptrs[239] = &filler_func_240;
    fptrs[240] = &filler_func_241;
    fptrs[241] = &filler_func_242;
    fptrs[242] = &filler_func_243;
    fptrs[243] = &filler_func_244;
    fptrs[244] = &filler_func_245;
    fptrs[245] = &filler_func_246;
    fptrs[246] = &filler_func_247;
    fptrs[247] = &filler_func_248;
    fptrs[248] = &filler_func_249;
    fptrs[249] = &filler_func_250;
    fptrs[250] = &filler_func_251;
    fptrs[251] = &filler_func_252;
    fptrs[252] = &filler_func_253;
    fptrs[253] = &filler_func_254;
    fptrs[254] = &filler_func_255;
    fptrs[255] = &filler_func_256;
    fptrs[256] = &filler_func_257;
    fptrs[257] = &filler_func_258;
    fptrs[258] = &filler_func_259;
    fptrs[259] = &filler_func_260;
    fptrs[260] = &filler_func_261;
    fptrs[261] = &filler_func_262;
    fptrs[262] = &filler_func_263;
    fptrs[263] = &filler_func_264;
    fptrs[264] = &filler_func_265;
    fptrs[265] = &filler_func_266;
    fptrs[266] = &filler_func_267;
    fptrs[267] = &filler_func_268;
    fptrs[268] = &filler_func_269;
    fptrs[269] = &filler_func_270;
    fptrs[270] = &filler_func_271;
    fptrs[271] = &filler_func_272;
    fptrs[272] = &filler_func_273;
    fptrs[273] = &filler_func_274;
    fptrs[274] = &filler_func_275;
    fptrs[275] = &filler_func_276;
    fptrs[276] = &filler_func_277;
    fptrs[277] = &filler_func_278;
    fptrs[278] = &filler_func_279;
    fptrs[279] = &filler_func_280;
    fptrs[280] = &filler_func_281;
    fptrs[281] = &filler_func_282;
    fptrs[282] = &filler_func_283;
    fptrs[283] = &filler_func_284;
    fptrs[284] = &filler_func_285;
    fptrs[285] = &filler_func_286;
    fptrs[286] = &filler_func_287;
    fptrs[287] = &filler_func_288;
    fptrs[288] = &filler_func_289;
    fptrs[289] = &filler_func_290;
    fptrs[290] = &filler_func_291;
    fptrs[291] = &filler_func_292;
    fptrs[292] = &filler_func_293;
    fptrs[293] = &filler_func_294;
    fptrs[294] = &filler_func_295;
    fptrs[295] = &filler_func_296;
    fptrs[296] = &filler_func_297;
    fptrs[297] = &filler_func_298;
    fptrs[298] = &filler_func_299;
    fptrs[299] = &filler_func_300;
    fptrs[300] = &filler_func_301;
    fptrs[301] = &filler_func_302;
    fptrs[302] = &filler_func_303;
    fptrs[303] = &filler_func_304;
    fptrs[304] = &filler_func_305;
    fptrs[305] = &filler_func_306;
    fptrs[306] = &filler_func_307;
    fptrs[307] = &filler_func_308;
    fptrs[308] = &filler_func_309;
    fptrs[309] = &filler_func_310;
    fptrs[310] = &filler_func_311;
    fptrs[311] = &filler_func_312;
    fptrs[312] = &filler_func_313;
    fptrs[313] = &filler_func_314;
    fptrs[314] = &filler_func_315;
    fptrs[315] = &filler_func_316;
    fptrs[316] = &filler_func_317;
    fptrs[317] = &filler_func_318;
    fptrs[318] = &filler_func_319;
    fptrs[319] = &filler_func_320;
    fptrs[320] = &filler_func_321;
    fptrs[321] = &filler_func_322;
    fptrs[322] = &filler_func_323;
    fptrs[323] = &filler_func_324;
    fptrs[324] = &filler_func_325;
    fptrs[325] = &filler_func_326;
    fptrs[326] = &filler_func_327;
    fptrs[327] = &filler_func_328;
    fptrs[328] = &filler_func_329;
    fptrs[329] = &filler_func_330;
    fptrs[330] = &filler_func_331;
    fptrs[331] = &filler_func_332;
    fptrs[332] = &filler_func_333;
    fptrs[333] = &filler_func_334;
    fptrs[334] = &filler_func_335;
    fptrs[335] = &filler_func_336;
    fptrs[336] = &filler_func_337;
    fptrs[337] = &filler_func_338;
    fptrs[338] = &filler_func_339;
    fptrs[339] = &filler_func_340;
    fptrs[340] = &filler_func_341;
    fptrs[341] = &filler_func_342;
    fptrs[342] = &filler_func_343;
    fptrs[343] = &filler_func_344;
    fptrs[344] = &filler_func_345;
    fptrs[345] = &filler_func_346;
    fptrs[346] = &filler_func_347;
    fptrs[347] = &filler_func_348;
    fptrs[348] = &filler_func_349;
    fptrs[349] = &filler_func_350;
    fptrs[350] = &filler_func_351;
    fptrs[351] = &filler_func_352;
    fptrs[352] = &filler_func_353;
    fptrs[353] = &filler_func_354;
    fptrs[354] = &filler_func_355;
    fptrs[355] = &filler_func_356;
    fptrs[356] = &filler_func_357;
    fptrs[357] = &filler_func_358;
    fptrs[358] = &filler_func_359;
    fptrs[359] = &filler_func_360;
    fptrs[360] = &filler_func_361;
    fptrs[361] = &filler_func_362;
    fptrs[362] = &filler_func_363;
    fptrs[363] = &filler_func_364;
    fptrs[364] = &filler_func_365;
    fptrs[365] = &filler_func_366;
    fptrs[366] = &filler_func_367;
    fptrs[367] = &filler_func_368;
    fptrs[368] = &filler_func_369;
    fptrs[369] = &filler_func_370;
    fptrs[370] = &filler_func_371;
    fptrs[371] = &filler_func_372;
    fptrs[372] = &filler_func_373;
    fptrs[373] = &filler_func_374;
    fptrs[374] = &filler_func_375;
    fptrs[375] = &filler_func_376;
    fptrs[376] = &filler_func_377;
    fptrs[377] = &filler_func_378;
    fptrs[378] = &filler_func_379;
    fptrs[379] = &filler_func_380;
    fptrs[380] = &filler_func_381;
    fptrs[381] = &filler_func_382;
    fptrs[382] = &filler_func_383;
    fptrs[383] = &filler_func_384;
    fptrs[384] = &filler_func_385;
    fptrs[385] = &filler_func_386;
    fptrs[386] = &filler_func_387;
    fptrs[387] = &filler_func_388;
    fptrs[388] = &filler_func_389;
    fptrs[389] = &filler_func_390;
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
    ios::sync_with_stdio(false); cin.tie(nullptr);

    // allocate and initialize a small buffer
    int *buf = new int[16];
    for (int i = 0; i < 16; ++i) buf[i] = i * 11 + 7;

    // free it to create a potential UAF scenario
    delete [] buf;

    // create a dangling address: allocate and free a small block to obtain an address likely reused by allocator
    int *tmp = new int[1];
    delete [] tmp;
    uintptr_t addr = reinterpret_cast<uintptr_t>(tmp);
    dangling_ptr = reinterpret_cast<int*>(addr);

    // Finally, call the single trigger that performs the UAF read exactly once.
    UAF_func();
    return 0;
}

// CHECK-NOT: Cause: heap-buffer-overflow
// CHECK: Cause: use-after-free
// OHOS_LOCAL
// CHECK: Cause: global-overflow