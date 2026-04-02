//===-- memgraph_flags.inc --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Macro rows consumed by Flags. Each entry is:
//   OHOS_MEMGRAPH_FLAG(Type, Name, Default, Description)
//===----------------------------------------------------------------------===//

OHOS_MEMGRAPH_FLAG(bool, enabled, true,
                          "Enable OHOS memgraph runtime")
OHOS_MEMGRAPH_FLAG(bool, observability_enabled, false,
                          "Enable OHOS memgraph observability counters and interfaces")
OHOS_MEMGRAPH_FLAG(int, alloc_table_size, 1000000,
                          "Maximum number of live tracked allocations")
OHOS_MEMGRAPH_FLAG(int, store_table_size, 2000000,
                          "Maximum number of live tracked store records")
OHOS_MEMGRAPH_FLAG(bool, verbose, false,
                          "Reserved verbose switch; currently unused")
