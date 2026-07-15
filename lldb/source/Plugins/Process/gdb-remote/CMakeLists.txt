set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

lldb_tablegen(ProcessGDBRemoteProperties.inc -gen-lldb-property-defs
  ${EXTRA_FEAT_LIST}
  SOURCE ProcessGDBRemoteProperties.td
  TARGET LLDBPluginProcessGDBRemotePropertiesGen)

lldb_tablegen(ProcessGDBRemotePropertiesEnum.inc -gen-lldb-property-enum-defs
  ${EXTRA_FEAT_LIST}
  SOURCE ProcessGDBRemoteProperties.td
  TARGET LLDBPluginProcessGDBRemotePropertiesEnumGen)

set(LLDB_PLUGINS
  lldbPluginProcessUtility
)

if(HAVE_LIBCOMPRESSION)
  set(LIBCOMPRESSION compression)
endif()

if(LLVM_ENABLE_ZLIB)
  list(APPEND LLDB_SYSTEM_LIBS ZLIB::ZLIB)
endif()

add_lldb_library(lldbPluginProcessGDBRemote PLUGIN
  GDBRemoteClientBase.cpp
  GDBRemoteCommunication.cpp
  GDBRemoteCommunicationClient.cpp
  GDBRemoteCommunicationHistory.cpp
  GDBRemoteCommunicationServer.cpp
  GDBRemoteCommunicationServerCommon.cpp
  GDBRemoteCommunicationServerLLGS.cpp
  GDBRemoteCommunicationServerPlatform.cpp
  GDBRemoteRegisterContext.cpp
  GDBRemoteRegisterFallback.cpp
  ProcessGDBRemote.cpp
  ProcessGDBRemoteLog.cpp
  ThreadGDBRemote.cpp

  LINK_COMPONENTS
    Support
    TargetParser
  LINK_LIBS
    lldbBreakpoint
    lldbCore
    lldbDataFormatters
    lldbHost
    lldbInterpreter
    lldbSymbol
    lldbTarget
    lldbUtility
    ${LLDB_SYSTEM_LIBS}
    ${LLDB_PLUGINS}
    ${LIBCOMPRESSION}
  )

add_dependencies(lldbPluginProcessGDBRemote
  LLDBPluginProcessGDBRemotePropertiesGen
  LLDBPluginProcessGDBRemotePropertiesEnumGen)
