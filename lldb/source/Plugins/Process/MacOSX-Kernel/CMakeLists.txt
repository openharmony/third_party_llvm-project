set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

lldb_tablegen(ProcessKDPProperties.inc -gen-lldb-property-defs
  ${EXTRA_FEAT_LIST}
  SOURCE ProcessKDPProperties.td
  TARGET LLDBPluginProcessMacOSXKernelPropertiesGen)

lldb_tablegen(ProcessKDPPropertiesEnum.inc -gen-lldb-property-enum-defs
  ${EXTRA_FEAT_LIST}
  SOURCE ProcessKDPProperties.td
  TARGET LLDBPluginProcessMacOSXKernelPropertiesEnumGen)

add_lldb_library(lldbPluginProcessMacOSXKernel PLUGIN
  CommunicationKDP.cpp
  ProcessKDP.cpp
  ProcessKDPLog.cpp
  RegisterContextKDP_arm.cpp
  RegisterContextKDP_arm64.cpp
  RegisterContextKDP_x86_64.cpp
  ThreadKDP.cpp

  LINK_LIBS
    lldbBreakpoint
    lldbCore
    lldbHost
    lldbInterpreter
    lldbSymbol
    lldbTarget
    lldbUtility
    lldbPluginDynamicLoaderDarwinKernel
    lldbPluginDynamicLoaderStatic
    lldbPluginProcessUtility
  )

add_dependencies(lldbPluginProcessMacOSXKernel
  LLDBPluginProcessMacOSXKernelPropertiesGen
  LLDBPluginProcessMacOSXKernelPropertiesEnumGen)
