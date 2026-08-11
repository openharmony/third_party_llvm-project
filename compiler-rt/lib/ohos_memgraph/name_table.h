//===-- name_table.h --------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Pointer-identity-based interning table for type and variable names.
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#ifndef OHOS_MEMGRAPH_NAME_TABLE_H
#define OHOS_MEMGRAPH_NAME_TABLE_H

#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_mutex.h"

namespace __ohos_memgraph {

using __sanitizer::u32;
using __sanitizer::u8;
using __sanitizer::uptr;

enum NameTableKind : u8 {
  kTypeNameTable = 0,
  kVarNameTable = 1,
};

// Intern frontend-provided string pointers into compact numeric ids.
//
// Deduplication is intentionally based on pointer identity, not string
// contents, so the frontend must provide stable static strings.
class NameTable {
public:
  NameTable();

  bool Init(NameTableKind kind, uptr initial_map_capacity);
  void Destroy();

  u32 Intern(const char *ptr);
  const char *Resolve(u32 id) const;

  uptr MapEntrySize() const;

private:
  struct Slot {
    const char *ptr;
    u32 id;
    u8 state;
    u8 pad[3];
  };

  void OnAlloc(uptr bytes);
  void OnFree(uptr bytes);
  uptr Hash(const char *ptr) const;
  bool Rehash(uptr new_cap);
  bool EnsureIdCapacity();

  NameTableKind kind_;
  mutable __sanitizer::StaticSpinMutex mu_;
  Slot *map_;
  uptr map_cap_;
  uptr map_used_;

  const char **id_to_ptr_;
  uptr id_cap_;
  uptr id_size_;
};

}  // namespace __ohos_memgraph
#endif  // OHOS_MEMGRAPH_NAME_TABLE_H
#endif /* OHOS_LLVM */
