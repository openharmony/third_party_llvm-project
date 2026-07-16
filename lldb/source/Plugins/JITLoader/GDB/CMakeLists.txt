set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

lldb_tablegen(JITLoaderGDBProperties.inc -gen-lldb-property-defs
  ${EXTRA_FEAT_LIST}
  SOURCE JITLoaderGDBProperties.td
  TARGET LLDBPluginJITLoaderGDBPropertiesGen)

lldb_tablegen(JITLoaderGDBPropertiesEnum.inc -gen-lldb-property-enum-defs
  ${EXTRA_FEAT_LIST}
  SOURCE JITLoaderGDBProperties.td
  TARGET LLDBPluginJITLoaderGDBPropertiesEnumGen)

add_lldb_library(lldbPluginJITLoaderGDB PLUGIN
  JITLoaderGDB.cpp

  LINK_COMPONENTS
    Support
  LINK_LIBS
    lldbBreakpoint
    lldbCore
    lldbInterpreter
    lldbSymbol
    lldbTarget
    lldbUtility
    lldbPluginObjectFileMachO
  )

add_dependencies(lldbPluginJITLoaderGDB
  LLDBPluginJITLoaderGDBPropertiesGen
  LLDBPluginJITLoaderGDBPropertiesEnumGen)
