get_property(LLDB_ALL_PLUGINS GLOBAL PROPERTY LLDB_PLUGINS)

add_lldb_tool(lldb-test
  FormatUtil.cpp
  lldb-test.cpp
  SystemInitializerTest.cpp

  LINK_LIBS
    lldbBreakpoint
    lldbCore
    lldbDataFormatters
    lldbExpression
    lldbHost
    lldbInitialization
    lldbInterpreter
    lldbSymbol
    lldbTarget
    lldbUtility
    lldbVersion
    ${LLDB_ALL_PLUGINS}

  LINK_COMPONENTS
    Support
  )

if(Python3_RPATH)
  set_property(TARGET lldb-test APPEND PROPERTY INSTALL_RPATH "${Python3_RPATH}")
  set_property(TARGET lldb-test APPEND PROPERTY BUILD_RPATH   "${Python3_RPATH}")
  # OHOS_LOCAL begin
  if(OHOS)
    set_property(TARGET lldb-test APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-rpath,\'${Python3_RPATH}\'")
  endif()
  # OHOS_LOCAL end
endif()

target_include_directories(lldb-test PRIVATE ${LLDB_SOURCE_DIR}/source)
target_include_directories(lldb-test PRIVATE ${LLDB_BINARY_DIR}/source)
