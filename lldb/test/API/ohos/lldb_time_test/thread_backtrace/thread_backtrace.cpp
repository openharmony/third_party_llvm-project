/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

int a(int);
int b(int);
int c(int);

int a(int val) {
  if (val <= 1)
    return b(val);
  else if (val >= 3)
    return c(val);

  return val;
}

int b(int val) {
  int rc = c(val);
  void *ptr = malloc(1024); // breakpoint for thread backtrace.
  if (!ptr)
    return -1;
  else
    printf("ptr=%p\n", ptr);
  return rc;
}

int c(int val) { return val + 3; }

int main(int argc, char const *argv[]) {
  int A1 = a(1);
  printf("a(1) returns %d\n", A1);

  int B2 = b(2);
  printf("b(2) returns %d\n", B2);

  int A3 = a(3);
  printf("a(3) returns %d\n", A3);

  return 0;
}
