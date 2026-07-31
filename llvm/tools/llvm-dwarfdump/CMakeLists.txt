set(LLVM_LINK_COMPONENTS
  DebugInfoDWARF
  DebugInfoDWARFLowLevel
  AllTargetsDescs
  AllTargetsInfos
  MC
  Object
  Support
  TargetParser
  )

# OHOS MemTracer in Dwarf.def changes DIDT_ID_Count; force rebuild with libLLVMDebugInfoDWARF.
set(_DWARFDUMP_DIDT_DEPS
  ${LLVM_MAIN_SRC_DIR}/include/llvm/BinaryFormat/Dwarf.def
  ${LLVM_MAIN_SRC_DIR}/include/llvm/DebugInfo/DIContext.h
  )

add_llvm_tool(llvm-dwarfdump
  SectionSizes.cpp
  Statistics.cpp
  llvm-dwarfdump.cpp
  )

set_source_files_properties(SectionSizes.cpp Statistics.cpp llvm-dwarfdump.cpp
  PROPERTIES OBJECT_DEPENDS "${_DWARFDUMP_DIDT_DEPS}")

add_subdirectory(fuzzer)
