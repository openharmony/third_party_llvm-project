
if(LLVM_SPLIT_LLVM_DYLIB_TARGETS)
ADD_DEFINITIONS("-DXVM_DYLIB_MODE=1")
endif()

add_llvm_component_library(LLVMXVMInfo
  XVMTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  XVM
  )
