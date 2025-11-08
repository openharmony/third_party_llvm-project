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
#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

static int var = 5;

int main(void) {
    printf("%p is %d\n", (void*)&var, var); 

    char exe[PATH_MAX] = {0};
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe)-1);
    if (n > 0) exe[n] = '\0'; else return 1;

    char tmp[PATH_MAX] = {0};
    snprintf(tmp, sizeof(tmp), "%s", exe);
    char *dir = dirname(tmp);

    char so_path[PATH_MAX] = {0};
    snprintf(so_path, sizeof(so_path), "%s/libarkshim.so", dir);

    void *h = dlopen(so_path, RTLD_LAZY);
    if (!h) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
    } else {
        fprintf(stderr, "[DBG] dlopen OK\n"); // break on this line
    }

    for (volatile int i = 0; i < 1; ++i) {}
    return ++var;
}
