set(LLVM_LINK_COMPONENTS
  Support
  Core
  Parts
)
add_clang_library(pacDfi
  PacDfi.cpp

  LINK_LIBS
  clangAST
  clangBasic
)
