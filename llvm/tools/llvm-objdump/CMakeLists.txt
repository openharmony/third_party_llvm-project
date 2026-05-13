include_directories(
  ${LLVM_MAIN_SRC_DIR}/lib/Target/AArch64
  ${LLVM_BINARY_DIR}/lib/Target/AArch64
)

set(LLVM_LINK_COMPONENTS
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  BinaryFormat
  DebugInfoDWARF
  DebugInfoPDB
  Demangle
  MC
  MCDisassembler
  Object
  Option
  Support
  Symbolize
  )

set(LLVM_TARGET_DEFINITIONS ObjdumpOpts.td)
tablegen(LLVM ObjdumpOpts.inc -gen-opt-parser-defs)
add_public_tablegen_target(ObjdumpOptsTableGen)

set(LLVM_TARGET_DEFINITIONS OtoolOpts.td)
tablegen(LLVM OtoolOpts.inc -gen-opt-parser-defs)
add_public_tablegen_target(OtoolOptsTableGen)

add_llvm_tool(llvm-objdump
  llvm-objdump.cpp
  SourcePrinter.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  OffloadDump.cpp
  WasmDump.cpp
  XCOFFDump.cpp
  AArch64CrashSnapshot.cpp
  AArch64BackwardSlicer.cpp
  DWARFLocator.cpp
  ValueEvaluator.cpp
  DEPENDS
  ObjdumpOptsTableGen
  OtoolOptsTableGen
  )

if(LLVM_HAVE_LIBXAR)
  target_link_libraries(llvm-objdump PRIVATE ${XAR_LIB})
endif()

# ---- Install and deploy crash_log_config.json ----
set(CRASH_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/crash_log_config.json")

# Install the config file to the same directory as the llvm-objdump binary
install(FILES ${CRASH_CONFIG_FILE}
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        COMPONENT llvm-objdump)

# After build, copy the config file next to the executable for development convenience
add_custom_command(TARGET llvm-objdump POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CRASH_CONFIG_FILE}
        $<TARGET_FILE_DIR:llvm-objdump>/crash_log_config.json
    COMMENT "Copying crash_log_config.json next to llvm-objdump"
)
# ---------------------------------------------------


add_llvm_tool_symlink(llvm-otool llvm-objdump)

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(objdump llvm-objdump)
endif()

if(LLVM_INSTALL_CCTOOLS_SYMLINKS)
  add_llvm_tool_symlink(otool llvm-otool)
endif()
