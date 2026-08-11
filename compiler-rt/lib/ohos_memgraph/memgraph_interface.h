//===-- memgraph_interface.h ------------------------*- C++ -*-===//
//
// Compatibility forwarding header: existing internal code and older tests may
// continue to include `lib/ohos_memgraph/memgraph_interface.h`.
//
// The canonical public include path is now:
//   #include <sanitizer/memgraph_interface.h>
//===----------------------------------------------------------------------===//

#ifdef OHOS_LLVM
#ifndef OHOS_MEMGRAPH_INTERFACE_LEGACY_WRAPPER_H
#define OHOS_MEMGRAPH_INTERFACE_LEGACY_WRAPPER_H

#include "../../include/sanitizer/memgraph_interface.h"
#endif  // OHOS_MEMGRAPH_INTERFACE_LEGACY_WRAPPER_H
#endif /* OHOS_LLVM */
