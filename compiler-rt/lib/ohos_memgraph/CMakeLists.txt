#===-- ohos_memgraph/CMakeLists.txt -----------------*- CMake -*-===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# OHOS memgraph runtime, published as libclang_rt.memgraph.so.
#===------------------------------------------------------------------------===#

add_compiler_rt_component(memgraph)

include_directories(..)

set(OHOS_MEMGRAPH_SOURCES
  alloc_table.cpp
  name_table.cpp
  memgraph.cpp
  memgraph_allocation_functions.cpp
  memgraph_interceptors.cpp
  memgraph_monitoring.cpp
  store_table.cpp
  )

set(OHOS_MEMGRAPH_HEADERS
  alloc_table.h
  name_table.h
  memgraph.h
  memgraph_flags.h
  memgraph_interface.h
  memgraph_stats_internal.h
  store_table.h
  )

set(OHOS_MEMGRAPH_CFLAGS ${SANITIZER_COMMON_CFLAGS})
append_list_if(COMPILER_RT_HAS_FPIC_FLAG -fPIC OHOS_MEMGRAPH_CFLAGS)
append_list_if(COMPILER_RT_HAS_WNO_FORMAT -Wno-format OHOS_MEMGRAPH_CFLAGS)

if(OHOS_LLVM AND OHOS)
  list(APPEND OHOS_MEMGRAPH_CFLAGS -fno-emulated-tls)
endif()

function(add_memgraph_runtime runtime_name arch)
  set(variant_cflags ${OHOS_MEMGRAPH_CFLAGS})

  add_compiler_rt_runtime(
    ${runtime_name}
    STATIC
    ARCHS ${arch}
    SOURCES ${OHOS_MEMGRAPH_SOURCES}
    ADDITIONAL_HEADERS ${OHOS_MEMGRAPH_HEADERS}
    CFLAGS ${variant_cflags}
    OBJECT_LIBS RTSanitizerCommon
                RTSanitizerCommonLibc
                RTInterception
    PARENT_TARGET memgraph
  )

  add_compiler_rt_runtime(
    ${runtime_name}
    SHARED
    ARCHS ${arch}
    SOURCES ${OHOS_MEMGRAPH_SOURCES}
    ADDITIONAL_HEADERS ${OHOS_MEMGRAPH_HEADERS}
    CFLAGS ${variant_cflags}
    OBJECT_LIBS RTSanitizerCommon
                RTSanitizerCommonLibc
                RTInterception
    PARENT_TARGET memgraph
  )
endfunction()

if(COMPILER_RT_HAS_OHOS_MEMGRAPH)
  foreach(arch ${OHOS_MEMGRAPH_SUPPORTED_ARCH})
    add_memgraph_runtime(
      clang_rt.memgraph ${arch})
  endforeach()
endif()
