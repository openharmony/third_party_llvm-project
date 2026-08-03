//===-- memgraph_flags.h ----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Runtime flags for the OHOS memgraph runtime.
//
// These settings control whether the runtime is enabled and how much metadata
// it retains. They do not define the shape of query results; they only affect
// runtime behavior and capacity limits.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#ifndef OHOS_MEMGRAPH_FLAGS_H
#define OHOS_MEMGRAPH_FLAGS_H

namespace __ohos_memgraph {

struct Flags {
  // Keep the field list and default-value descriptions in a single .inc file
  // so declarations and defaults cannot drift apart.
#define OHOS_MEMGRAPH_FLAG(Type, Name, Default, Description) Type Name;
#include "memgraph_flags.inc"
#undef OHOS_MEMGRAPH_FLAG

  void SetDefaults();
};

Flags *flags();

} // namespace __ohos_memgraph
#endif // OHOS_MEMGRAPH_FLAGS_H
#endif /* OHOS_LLVM */
