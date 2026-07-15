add_llvm_component_group(ARM HAS_JIT)

set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_TARGET_DEFINITIONS ARM.td)

tablegen(LLVM ARMGenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenCallingConv.inc -gen-callingconv ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenFastISel.inc -gen-fast-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenGlobalISel.inc -gen-global-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenMCPseudoLowering.inc -gen-pseudo-lowering ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenRegisterBank.inc -gen-register-bank ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})
tablegen(LLVM ARMGenSystemRegister.inc -gen-searchable-tables ${EXTRA_FEAT_LIST})

add_public_tablegen_target(ARMCommonTableGen)

add_llvm_target(ARMCodeGen
  A15SDOptimizer.cpp
  ARMAsmPrinter.cpp
  ARMBaseInstrInfo.cpp
  ARMBaseRegisterInfo.cpp
  ARMBasicBlockInfo.cpp
  ARMBranchTargets.cpp
  ARMCallingConv.cpp
  ARMCallLowering.cpp
  ARMConstantIslandPass.cpp
  ARMConstantPoolValue.cpp
  ARMExpandPseudoInsts.cpp
  ARMFastISel.cpp
  ARMFixCortexA57AES1742098Pass.cpp
  ARMFrameLowering.cpp
  ARMHazardRecognizer.cpp
  ARMInstructionSelector.cpp
  ARMISelDAGToDAG.cpp
  ARMISelLowering.cpp
  ARMInstrInfo.cpp
  ARMLegalizerInfo.cpp
  ARMParallelDSP.cpp
  ARMLoadStoreOptimizer.cpp
  ARMLowOverheadLoops.cpp
  ARMBlockPlacement.cpp
  ARMMCInstLower.cpp
  ARMMachineFunctionInfo.cpp
  ARMMacroFusion.cpp
  ARMRegisterInfo.cpp
  ARMOptimizeBarriersPass.cpp
  ARMRegisterBankInfo.cpp
  ARMSelectionDAGInfo.cpp
  ARMSLSHardening.cpp
  ARMSubtarget.cpp
  ARMTargetMachine.cpp
  ARMTargetObjectFile.cpp
  ARMTargetTransformInfo.cpp
  MLxExpansionPass.cpp
  MVEGatherScatterLowering.cpp
  MVELaneInterleavingPass.cpp
  MVETailPredication.cpp
  MVEVPTBlockPass.cpp
  MVETPAndVPTOptimisationsPass.cpp
  ARMLatencyMutations.cpp
  Thumb1FrameLowering.cpp
  Thumb1InstrInfo.cpp
  ThumbRegisterInfo.cpp
  Thumb2ITBlockPass.cpp
  Thumb2InstrInfo.cpp
  Thumb2SizeReduction.cpp

  LINK_COMPONENTS
  ARMDesc
  ARMInfo
  ARMUtils
  Analysis
  AsmPrinter
  CFGuard
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IPO
  MC
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  ARM
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Utils)
