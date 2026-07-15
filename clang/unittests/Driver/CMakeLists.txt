# OHOS_LOCAL begin
set(OHOS_DRIVER_TEST_SOURCES)
if(OHOS_LLVM)
  list(APPEND OHOS_DRIVER_TEST_SOURCES OHOSTCTest.cpp)
else()
  set(LLVM_OPTIONAL_SOURCES OHOSTCTest.cpp)
endif()
# OHOS_LOCAL end

add_clang_unittest(ClangDriverTests
  DistroTest.cpp
  DXCModeTest.cpp
  GCCVersionTest.cpp
  ${OHOS_DRIVER_TEST_SOURCES} # OHOS_LOCAL
  ToolChainTest.cpp
  ModuleCacheTest.cpp
  MultilibBuilderTest.cpp
  MultilibTest.cpp
  SanitizerArgsTest.cpp
  CLANG_LIBS
  clangDriver
  clangBasic
  clangFrontend # For TextDiagnosticPrinter.
  clangSerialization
  LLVM_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  MC
  Option
  Support
  TargetParser
  )
