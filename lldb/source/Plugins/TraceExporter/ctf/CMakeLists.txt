set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

lldb_tablegen(TraceExporterCTFCommandOptions.inc -gen-lldb-option-defs
  ${EXTRA_FEAT_LIST}
  SOURCE TraceExporterCTFOptions.td
  TARGET TraceExporterCTFOptionsGen)

add_lldb_library(lldbPluginTraceExporterCTF PLUGIN
CommandObjectThreadTraceExportCTF.cpp
  TraceExporterCTF.cpp

  LINK_COMPONENTS
    Support
  LINK_LIBS
    lldbCore
    lldbSymbol
    lldbTarget
    lldbPluginTraceExporterCommon
  )

add_dependencies(lldbPluginTraceExporterCTF TraceExporterCTFOptionsGen)
