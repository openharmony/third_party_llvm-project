# This Cmake module is responsible for interpreting the user defined LLVM_
# options that toggle downstream OHOS features.

option(OHOS_LLVM "Enable OHOS features" OFF)
if(OHOS_LLVM)
  set(OHOS_LLVM 1)
  add_definitions( -DOHOS_LLVM )
else()
  set(OHOS_LLVM 0)
endif()
