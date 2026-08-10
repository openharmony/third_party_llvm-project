add_lldb_unittest(OHOSTests
  ModuleCacheTest.cpp
  PlatformOHOSTest.cpp
  MakeUrlTest.cpp
  StringExtractorGDBRemoteQShowMemTest.cpp

  LINK_COMPONENTS
      Support
  LINK_LIBS
      lldbCore
      lldbHost
      lldbPluginObjectFileELF
      lldbPluginPlatformGDB
      lldbPluginPlatformOHOS
      lldbPluginSymbolFileSymtab
      lldbTarget
      lldbSymbol
      lldbUtility
      lldbUtilityHelpers
  )

set(test_inputs
  TestModule.so
  )
add_unittest_inputs(OHOSTests "${test_inputs}")
