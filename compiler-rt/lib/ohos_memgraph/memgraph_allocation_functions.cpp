//===-- memgraph_allocation_functions.cpp -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exported C API entry points for the OHOS memgraph runtime.
//===----------------------------------------------------------------------===//

#include "memgraph.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

extern "C" {

//===----------------------------------------------------------------------===//
// Exported core functional interfaces.
//===----------------------------------------------------------------------===//

SANITIZER_INTERFACE_ATTRIBUTE
void alloc_record(unsigned long malloc_addr, const char *type_name,
                  const char *var_name, unsigned long alloc_pc) {
  // Keep the exported C ABI stable and forward into the internal C++ runtime.
  __ohos_memgraph::RecordMallocMetadata((__sanitizer::uptr)malloc_addr,
                                        type_name, var_name,
                                        (__sanitizer::uptr)alloc_pc);
}

SANITIZER_INTERFACE_ATTRIBUTE
void store_record(unsigned long source_addr, unsigned long dst_ptr,
                  const char *type_name, const char *var_name,
                  unsigned long store_pc) {
  __ohos_memgraph::RecordStoreMetadata((__sanitizer::uptr)source_addr,
                                       (__sanitizer::uptr)dst_ptr, type_name,
                                       var_name,
                                       (__sanitizer::uptr)store_pc);
}

SANITIZER_INTERFACE_ATTRIBUTE
int get_block_info(unsigned long base, block_info_t *out) {
  return __ohos_memgraph::GetBlockInfo((__sanitizer::uptr)base, out) ? 1
                                                                      : 0;
}

SANITIZER_INTERFACE_ATTRIBUTE
int get_member_info(unsigned long addr, member_info_t *out) {
  return __ohos_memgraph::GetMemberInfo((__sanitizer::uptr)addr, out) ? 1 : 0;
}

SANITIZER_INTERFACE_ATTRIBUTE
int get_owner(unsigned long addr, owner_info_t *out) {
  return __ohos_memgraph::GetOwner((__sanitizer::uptr)addr, out) ? 1 : 0;
}

//===----------------------------------------------------------------------===//
// Exported observability and diagnostic interfaces.
//===----------------------------------------------------------------------===//

SANITIZER_INTERFACE_ATTRIBUTE
int get_info(unsigned long base, alloc_info_t *out) {
  return __ohos_memgraph::GetInfo((__sanitizer::uptr)base, out) ? 1 : 0;
}

SANITIZER_INTERFACE_ATTRIBUTE
unsigned long get_info_records(unsigned long base, info_record_t *out,
                               unsigned long capacity) {
  return (unsigned long)__ohos_memgraph::GetInfoRecords(
      (__sanitizer::uptr)base, out, (__sanitizer::uptr)capacity);
}

SANITIZER_INTERFACE_ATTRIBUTE
int get_runtime_stats(runtime_stats_t *out) {
  return __ohos_memgraph::GetRuntimeStats(out) ? 1 : 0;
}

SANITIZER_INTERFACE_ATTRIBUTE
int get_layout(unsigned long *alloc_row_bytes,
               unsigned long *store_row_bytes) {
  return __ohos_memgraph::GetLayout(alloc_row_bytes, store_row_bytes)
             ? 1
             : 0;
}

SANITIZER_INTERFACE_ATTRIBUTE
unsigned long get_live_allocs(unsigned long cursor, live_alloc_info_t *out,
                              unsigned long capacity) {
  return (unsigned long)__ohos_memgraph::GetLiveAllocs(
      (__sanitizer::uptr)cursor, out, (__sanitizer::uptr)capacity);
}

//===----------------------------------------------------------------------===//
// Exported explicit initialization interface.
//===----------------------------------------------------------------------===//

SANITIZER_INTERFACE_ATTRIBUTE
void memgraph_init(void) { __ohos_memgraph::Initialize(); }

} // extern "C"
