set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

lldb_tablegen(DynamicLoaderDarwinKernelProperties.inc -gen-lldb-property-defs
  ${EXTRA_FEAT_LIST}
  SOURCE DynamicLoaderDarwinKernelProperties.td
  TARGET LLDBPluginDynamicLoaderDarwinKernelPropertiesGen)

lldb_tablegen(DynamicLoaderDarwinKernelPropertiesEnum.inc -gen-lldb-property-enum-defs
  ${EXTRA_FEAT_LIST}
  SOURCE DynamicLoaderDarwinKernelProperties.td
  TARGET LLDBPluginDynamicLoaderDarwinKernelPropertiesEnumGen)

add_lldb_library(lldbPluginDynamicLoaderDarwinKernel PLUGIN
  DynamicLoaderDarwinKernel.cpp

  LINK_LIBS
    lldbBreakpoint
    lldbCore
    lldbHost
    lldbInterpreter
    lldbSymbol
    lldbTarget
    lldbUtility
    lldbPluginObjectFileMachO
  )

add_dependencies(lldbPluginDynamicLoaderDarwinKernel
  LLDBPluginDynamicLoaderDarwinKernelPropertiesGen
  LLDBPluginDynamicLoaderDarwinKernelPropertiesEnumGen)
