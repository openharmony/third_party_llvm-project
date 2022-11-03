//===----------------------------------------------------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//===----------------------------------------------------------------------===//

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

int memcpy_s(char *dst, int dstLen, char *src, int srcLen) {
    return dstLen;
}

void check1(int dstLen, int srcLen) {
    char dstStr[dstLen];
    char srcStr[srcLen];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, srcLen); // expected-warning{{src length may be larger than dst length}}
}

void check2() {
    int dstLen = 20;
    int srcLen = 10;
    char dstStr[dstLen];
    char srcStr[srcLen];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, srcLen); // no-warning
}

void check3() {
    int dstLen = 10;
    int srcLen = 20;
    char dstStr[dstLen];
    char srcStr[srcLen];
    memcpy_s(dstStr, sizeof(dstStr), srcStr, srcLen); // expected-warning{{src length may be larger than dst length}}
}

void check4() {
    int dstLen = 20;
    int srcLen = 10;
    char dstStr[dstLen];
    char srcStr[srcLen];
    int offset = 15;
    // srcLen > dstStr[offset]'s length, bug reported
    memcpy_s(&dstStr[offset], srcLen, srcStr, srcLen); // expected-warning{{src length may be larger than dst length}}
    offset = 5;
    // srcLen < dstStr[offset]'s length, no bug reported
    memcpy_s(&dstStr[offset], srcLen, srcStr, srcLen); // no-warning
}
