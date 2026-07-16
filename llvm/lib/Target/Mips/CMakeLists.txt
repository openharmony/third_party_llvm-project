add_llvm_component_group(Mips HAS_JIT)

set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_TARGET_DEFINITIONS Mips.td)

tablegen(LLVM MipsGenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenCallingConv.inc -gen-callingconv ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenFastISel.inc -gen-fast-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenGlobalISel.inc -gen-global-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenPostLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="MipsPostLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenMCPseudoLowering.inc -gen-pseudo-lowering ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenRegisterBank.inc -gen-register-bank ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})
tablegen(LLVM MipsGenExegesis.inc -gen-exegesis ${EXTRA_FEAT_LIST})

add_public_tablegen_target(MipsCommonTableGen)

add_llvm_target(MipsCodeGen
  Mips16FrameLowering.cpp
  Mips16HardFloat.cpp
  Mips16HardFloatInfo.cpp
  Mips16InstrInfo.cpp
  Mips16ISelDAGToDAG.cpp
  Mips16ISelLowering.cpp
  Mips16RegisterInfo.cpp
  MipsAnalyzeImmediate.cpp
  MipsAsmPrinter.cpp
  MipsCallLowering.cpp
  MipsCCState.cpp
  MipsConstantIslandPass.cpp
  MipsDelaySlotFiller.cpp
  MipsExpandPseudo.cpp
  MipsFastISel.cpp
  MipsInstrInfo.cpp
  MipsInstructionSelector.cpp
  MipsISelDAGToDAG.cpp
  MipsISelLowering.cpp
  MipsFrameLowering.cpp
  MipsLegalizerInfo.cpp
  MipsBranchExpansion.cpp
  MipsMCInstLower.cpp
  MipsMachineFunction.cpp
  MipsModuleISelDAGToDAG.cpp
  MipsOptimizePICCall.cpp
  MipsOs16.cpp
  MipsPreLegalizerCombiner.cpp
  MipsPostLegalizerCombiner.cpp
  MipsRegisterBankInfo.cpp
  MipsRegisterInfo.cpp
  MipsSEFrameLowering.cpp
  MipsSEInstrInfo.cpp
  MipsSEISelDAGToDAG.cpp
  MipsSEISelLowering.cpp
  MipsSERegisterInfo.cpp
  MipsSelectionDAGInfo.cpp
  MipsSubtarget.cpp
  MipsTargetMachine.cpp
  MipsTargetObjectFile.cpp
  MipsTargetTransformInfo.cpp
  MicroMipsSizeReduction.cpp
  MipsMulMulBugPass.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  MipsDesc
  MipsInfo
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  Mips
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
