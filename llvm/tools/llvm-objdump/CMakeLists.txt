set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_LINK_COMPONENTS
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  BinaryFormat
  DebugInfoBTF
  DebugInfoDWARF
  DebugInfoDWARFLowLevel
  Demangle
  MC
  MCDisassembler
  Object
  Option
  Support
  Symbolize
  TargetParser
  )

set(LLVM_TARGET_DEFINITIONS ObjdumpOpts.td)
tablegen(LLVM ObjdumpOpts.inc -gen-opt-parser-defs ${EXTRA_FEAT_LIST}
  )
add_public_tablegen_target(ObjdumpOptsTableGen)

set(LLVM_TARGET_DEFINITIONS OtoolOpts.td)
tablegen(LLVM OtoolOpts.inc -gen-opt-parser-defs ${EXTRA_FEAT_LIST}
  )
add_public_tablegen_target(OtoolOptsTableGen)

set(LLVM_OBJDUMP_OHOS_SOURCES
  AArch64CrashSnapshot.cpp
  AArch64BackwardSlicer.cpp
  DWARFLocator.cpp
  ValueEvaluator.cpp
  )

add_llvm_tool(llvm-objdump
  llvm-objdump.cpp
  SourcePrinter.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  OffloadDump.cpp
  WasmDump.cpp
  XCOFFDump.cpp
  ${LLVM_OBJDUMP_OHOS_SOURCES}
  DEPENDS
  ObjdumpOptsTableGen
  OtoolOptsTableGen
  GENERATE_DRIVER
  )

if(NOT LLVM_TOOL_LLVM_DRIVER_BUILD)
  target_link_libraries(llvm-objdump PRIVATE LLVMDebuginfod)
endif()

if(OHOS_LLVM)
  include_directories(
    ${LLVM_MAIN_SRC_DIR}/lib/Target/AArch64
    ${LLVM_BINARY_DIR}/lib/Target/AArch64
    )

  set(CRASH_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/crash_log_config.json")
  install(FILES ${CRASH_CONFIG_FILE}
          DESTINATION "${CMAKE_INSTALL_BINDIR}"
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
          COMPONENT llvm-objdump)
  add_custom_command(TARGET llvm-objdump POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CRASH_CONFIG_FILE}
          $<TARGET_FILE_DIR:llvm-objdump>/crash_log_config.json
      COMMENT "Copying crash_log_config.json next to llvm-objdump"
  )
endif()

add_llvm_tool_symlink(llvm-otool llvm-objdump)

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(objdump llvm-objdump)
endif()

if(LLVM_INSTALL_CCTOOLS_SYMLINKS)
  add_llvm_tool_symlink(otool llvm-otool)
endif()
