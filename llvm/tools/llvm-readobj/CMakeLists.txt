set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_LINK_COMPONENTS
  BinaryFormat
  DebugInfoCodeView
  DebugInfoDWARF
  Demangle
  Object
  Option
  Support
  TargetParser
  )

set(LLVM_TARGET_DEFINITIONS Opts.td)
tablegen(LLVM Opts.inc -gen-opt-parser-defs ${EXTRA_FEAT_LIST}
  )
add_public_tablegen_target(ReadobjOptsTableGen)

add_llvm_tool(llvm-readobj
  ARMWinEHPrinter.cpp
  COFFDumper.cpp
  COFFImportDumper.cpp
  ELFDumper.cpp
  llvm-readobj.cpp
  MachODumper.cpp
  ObjDumper.cpp
  WasmDumper.cpp
  Win64EHDumper.cpp
  WindowsResourceDumper.cpp
  XCOFFDumper.cpp
  DEPENDS
  ReadobjOptsTableGen
  GENERATE_DRIVER
  )

setup_host_tool(llvm-readobj LLVM_READOBJ llvm_readobj_exe llvm_readobj_target)

add_llvm_tool_symlink(llvm-readelf llvm-readobj)

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(readelf llvm-readobj)
endif()
