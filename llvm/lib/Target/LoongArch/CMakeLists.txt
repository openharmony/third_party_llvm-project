add_llvm_component_group(LoongArch)

set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_TARGET_DEFINITIONS LoongArch.td)

tablegen(LLVM LoongArchGenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenMCPseudoLowering.inc -gen-pseudo-lowering ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM LoongArchGenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})

add_public_tablegen_target(LoongArchCommonTableGen)

add_llvm_target(LoongArchCodeGen
  LoongArchAsmPrinter.cpp
  LoongArchDeadRegisterDefinitions.cpp
  LoongArchExpandAtomicPseudoInsts.cpp
  LoongArchExpandPseudoInsts.cpp
  LoongArchFrameLowering.cpp
  LoongArchInstrInfo.cpp
  LoongArchISelDAGToDAG.cpp
  LoongArchISelLowering.cpp
  LoongArchMCInstLower.cpp
  LoongArchMergeBaseOffset.cpp
  LoongArchOptWInstrs.cpp
  LoongArchRegisterInfo.cpp
  LoongArchSubtarget.cpp
  LoongArchTargetMachine.cpp
  LoongArchTargetTransformInfo.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  LoongArchDesc
  LoongArchInfo
  MC
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  LoongArch
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
