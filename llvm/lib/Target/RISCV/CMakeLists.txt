add_llvm_component_group(RISCV)

set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_TARGET_DEFINITIONS RISCV.td)

tablegen(LLVM RISCVGenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenCompressInstEmitter.inc -gen-compress-inst-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenMacroFusion.inc -gen-macro-fusion-pred ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenMCPseudoLowering.inc -gen-pseudo-lowering ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenRegisterBank.inc -gen-register-bank ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenSearchableTables.inc -gen-searchable-tables ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenExegesis.inc -gen-exegesis ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenSDNodeInfo.inc -gen-sd-node-info ${EXTRA_FEAT_LIST})

set(LLVM_TARGET_DEFINITIONS RISCVGISel.td)
tablegen(LLVM RISCVGenGlobalISel.inc -gen-global-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenO0PreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="RISCVO0PreLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenPreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="RISCVPreLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM RISCVGenPostLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="RISCVPostLegalizerCombiner" ${EXTRA_FEAT_LIST})

add_public_tablegen_target(RISCVCommonTableGen)

add_llvm_target(RISCVCodeGen
  RISCVAsmPrinter.cpp
  RISCVCallingConv.cpp
  RISCVCodeGenPrepare.cpp
  RISCVConstantPoolValue.cpp
  RISCVDeadRegisterDefinitions.cpp
  RISCVExpandAtomicPseudoInsts.cpp
  RISCVExpandPseudoInsts.cpp
  RISCVFoldMemOffset.cpp
  RISCVFrameLowering.cpp
  RISCVGatherScatterLowering.cpp
  RISCVIndirectBranchTracking.cpp
  RISCVInsertReadWriteCSR.cpp
  RISCVInsertVSETVLI.cpp
  RISCVInsertWriteVXRM.cpp
  RISCVInstrInfo.cpp
  RISCVInterleavedAccess.cpp
  RISCVISelDAGToDAG.cpp
  RISCVISelLowering.cpp
  RISCVLandingPadSetup.cpp
  RISCVLateBranchOpt.cpp
  RISCVLoadStoreOptimizer.cpp
  RISCVMachineFunctionInfo.cpp
  RISCVMakeCompressible.cpp
  RISCVMergeBaseOffset.cpp
  RISCVMoveMerger.cpp
  RISCVOptWInstrs.cpp
  RISCVPostRAExpandPseudoInsts.cpp
  RISCVPushPopOptimizer.cpp
  RISCVRedundantCopyElimination.cpp
  RISCVRegisterInfo.cpp
  RISCVSelectionDAGInfo.cpp
  RISCVSubtarget.cpp
  RISCVTargetMachine.cpp
  RISCVTargetObjectFile.cpp
  RISCVTargetTransformInfo.cpp
  RISCVVectorMaskDAGMutation.cpp
  RISCVVectorPeephole.cpp
  RISCVVLOptimizer.cpp
  RISCVVMV0Elimination.cpp
  RISCVZacasABIFix.cpp
  GISel/RISCVCallLowering.cpp
  GISel/RISCVInstructionSelector.cpp
  GISel/RISCVLegalizerInfo.cpp
  GISel/RISCVPostLegalizerCombiner.cpp
  GISel/RISCVO0PreLegalizerCombiner.cpp
  GISel/RISCVPreLegalizerCombiner.cpp
  GISel/RISCVRegisterBankInfo.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IPO
  MC
  RISCVDesc
  RISCVInfo
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils
  Vectorize

  ADD_TO_COMPONENT
  RISCV
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(MCA)
add_subdirectory(TargetInfo)
