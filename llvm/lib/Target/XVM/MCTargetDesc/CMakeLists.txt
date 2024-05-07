
if(LLVM_BUILD_LLVM_DYLIB)
  ADD_DEFINITIONS("-DXVM_DYLIB_MODE=1")
endif()

add_llvm_component_library(LLVMXVMDesc
  XVMMCTargetDesc.cpp
  XVMInstPrinter.cpp
  XVMTargetStreamer.cpp

  LINK_COMPONENTS
  MC
  XVMInfo
  Support

  ADD_TO_COMPONENT
  XVM
  )
