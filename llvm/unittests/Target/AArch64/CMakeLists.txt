include_directories(
  ${CMAKE_SOURCE_DIR}/lib/Target/AArch64
  ${CMAKE_BINARY_DIR}/lib/Target/AArch64
  )

set(LLVM_LINK_COMPONENTS
  AArch64CodeGen
  AArch64Desc
  AArch64Info
  CodeGen
  Core
  GlobalISel
  MC
  MIRParser
  SelectionDAG
  Support
  Target
  )

add_llvm_target_unittest(AArch64Tests
  InstSizes.cpp
  DecomposeStackOffsetTest.cpp
  MatrixRegisterAliasing.cpp
  AArch64PARTS/AArch64EarlyPartsCpiPassTest.cpp
  )

set_property(TARGET AArch64Tests PROPERTY FOLDER "Tests/UnitTests/TargetTests")
