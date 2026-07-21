// Copyright (C) 2024 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

static int var = 5;
const char *bt = "This is a ArkTS backtrace";
const char *msg = "This is a ArkTS operate debug message result";

const char *GetJsBacktrace() {
    return bt;
}

const char *OperateJsDebugMessage(const char *message) {
    return msg;
}

int main ()
{
    printf ("%p is %d\n", &var, var); // break on this line
    return ++var;
}
