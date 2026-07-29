//===-- utilities_posix.cpp -------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <alloca.h>
#include <features.h> // IWYU pragma: keep (for __BIONIC__ macro)
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#if defined(OHOS_LLVM) && defined(__OHOS__)
#include "gwp_asan/optional/printf.h"
#endif // defined(OHOS_LLVM) && defined(__OHOS__)

#ifdef __BIONIC__
#include "gwp_asan/definitions.h"
#include <stdlib.h>
extern "C" GWP_ASAN_WEAK void android_set_abort_message(const char *);
#else // __BIONIC__
#include <stdio.h>
#endif

namespace gwp_asan {
void die(const char *Message) {
#ifdef __BIONIC__
  if (&android_set_abort_message != nullptr)
    android_set_abort_message(Message);
  abort();
#else  // __BIONIC__
#if defined(OHOS_LLVM) && defined(__OHOS__)
  // Report check failure via allocator Printf (FaultLogger) before trap.
  Printf("GWP-ASan has a check error\n");
  Printf("%s\n", Message);
  Printf("*** End GWP-ASan report ***\n");
#endif // defined(OHOS_LLVM) && defined(__OHOS__)
  fprintf(stderr, "%s", Message);
  __builtin_trap();
#endif // __BIONIC__
}

void dieWithErrorCode(const char *Message, int64_t ErrorCode) {
#ifdef __BIONIC__
  if (&android_set_abort_message == nullptr)
    abort();

  size_t buffer_size = strlen(Message) + 48;
  char *buffer = static_cast<char *>(alloca(buffer_size));
  snprintf(buffer, buffer_size, "%s (Error Code: %" PRId64 ")", Message,
           ErrorCode);
  android_set_abort_message(buffer);
  abort();
#else  // __BIONIC__
#if defined(OHOS_LLVM) && defined(__OHOS__)
  // Symmetric with die(): FaultLogger via Printf before trap.
  Printf("GWP-ASan has a check error\n");
  Printf("%s (Error Code: %" PRId64 ")\n", Message, ErrorCode);
  Printf("*** End GWP-ASan report ***\n");
#endif // defined(OHOS_LLVM) && defined(__OHOS__)
  fprintf(stderr, "%s (Error Code: %" PRId64 ")", Message, ErrorCode);
  __builtin_trap();
#endif // __BIONIC__
}
} // namespace gwp_asan
