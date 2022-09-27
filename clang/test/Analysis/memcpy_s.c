//===----------------------------------------------------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

// RUN: %clang_analyze_cc1 -analyzer-checker=openharmony -verify %s

int memcpy_s(char *dst, int dstLen, char *src, int srcLen) {
    return dstLen;
}

void check1(int dstLen, int srcLen) {
    char dstStr[dstLen];
    char srcStr[srcLen];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, srcLen); // expected-warning{{memcpy_s(): src length may be larger than dst length}}
}

void check2() {
    char dstStr[20];
    char srcStr[10];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, 10); // no-warning
}

void check3() {
    char dstStr[10];
    char srcStr[20];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, 20); // expected-warning{{memcpy_s(): src length may be larger than dst length}}
}

void check4() {
    char dstStr[20];
    char srcStr[10];
    memcpy_s(&dstStr[15], 10, srcStr, 10); // expected-warning{{memcpy_s(): src length may be larger than dst length}}
    memcpy_s(&dstStr[5], 10, srcStr, 10); // no-warning
}
