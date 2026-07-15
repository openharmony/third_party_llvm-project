add_llvm_component_group(AArch64 HAS_JIT)
set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

if(OHOS_LLVM AND BUILD_ARK_GC_SUPPORT)
    set(LLVM_TARGET_DEFINITIONS AArch64ArkGc.td)
else()
    set(LLVM_TARGET_DEFINITIONS AArch64.td)
endif()
tablegen(LLVM AArch64GenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenAsmWriter1.inc -gen-asm-writer -asmwriternum=1 ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenCallingConv.inc -gen-callingconv ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenFastISel.inc -gen-fast-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenGlobalISel.inc -gen-global-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenO0PreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="AArch64O0PreLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenPreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="AArch64PreLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenPostLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="AArch64PostLegalizerCombiner" ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenPostLegalizeGILowering.inc -gen-global-isel-combiner
              -combiners="AArch64PostLegalizerLowering" ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenMCPseudoLowering.inc -gen-pseudo-lowering ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenRegisterBank.inc -gen-register-bank ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenSDNodeInfo.inc -gen-sd-node-info ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenSystemOperands.inc -gen-searchable-tables ${EXTRA_FEAT_LIST})
tablegen(LLVM AArch64GenExegesis.inc -gen-exegesis ${EXTRA_FEAT_LIST})

add_public_tablegen_target(AArch64CommonTableGen)

# OHOS_LOCAL begin
set(OHOS_AARCH64_SOURCES)
if(OHOS_LLVM)
  list(APPEND OHOS_AARCH64_SOURCES AArch64StackProtectorRetLowering.cpp)
else()
  set(LLVM_OPTIONAL_SOURCES AArch64StackProtectorRetLowering.cpp)
endif()
# OHOS_LOCAL end

add_llvm_target(AArch64CodeGen
  GISel/AArch64CallLowering.cpp
  GISel/AArch64GlobalISelUtils.cpp
  GISel/AArch64InstructionSelector.cpp
  GISel/AArch64LegalizerInfo.cpp
  GISel/AArch64O0PreLegalizerCombiner.cpp
  GISel/AArch64PreLegalizerCombiner.cpp
  GISel/AArch64PostLegalizerCombiner.cpp
  GISel/AArch64PostLegalizerLowering.cpp
  GISel/AArch64PostSelectOptimize.cpp
  GISel/AArch64RegisterBankInfo.cpp
  AArch64A57FPLoadBalancing.cpp
  AArch64AdvSIMDScalarPass.cpp
  AArch64Arm64ECCallLowering.cpp
  AArch64AsmPrinter.cpp
  AArch64BranchTargets.cpp
  AArch64CallingConvention.cpp
  AArch64CleanupLocalDynamicTLSPass.cpp
  AArch64CollectLOH.cpp
  AArch64CondBrTuning.cpp
  AArch64ConditionalCompares.cpp
  AArch64DeadRegisterDefinitionsPass.cpp
  AArch64ExpandImm.cpp
  AArch64ExpandPseudoInsts.cpp
  AArch64FalkorHWPFFix.cpp
  AArch64FastISel.cpp
  AArch64A53Fix835769.cpp
  AArch64FrameLowering.cpp
  AArch64CompressJumpTables.cpp
  AArch64ConditionOptimizer.cpp
  AArch64RedundantCopyElimination.cpp
  AArch64ISelDAGToDAG.cpp
  AArch64ISelLowering.cpp
  AArch64InstrInfo.cpp
  AArch64LoadStoreOptimizer.cpp
  AArch64LowerHomogeneousPrologEpilog.cpp
  AArch64MachineFunctionInfo.cpp
  AArch64MachineScheduler.cpp
  AArch64MacroFusion.cpp
  AArch64MIPeepholeOpt.cpp
  AArch64MCInstLower.cpp
  AArch64PointerAuth.cpp
  AArch64PostCoalescerPass.cpp
  AArch64PromoteConstant.cpp
  AArch64PBQPRegAlloc.cpp
  AArch64RegisterInfo.cpp
  AArch64SLSHardening.cpp
  AArch64SelectionDAGInfo.cpp
  AArch64SpeculationHardening.cpp
  ${OHOS_AARCH64_SOURCES} # OHOS_LOCAL
  AArch64StackTagging.cpp
  AArch64StackTaggingPreRA.cpp
  AArch64StorePairSuppress.cpp
  AArch64Subtarget.cpp
  AArch64TargetMachine.cpp
  AArch64TargetObjectFile.cpp
  AArch64TargetTransformInfo.cpp
  SMEABIPass.cpp
  SMEPeepholeOpt.cpp
  SVEIntrinsicOpts.cpp
  AArch64SIMDInstrOpt.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  AArch64Desc
  AArch64Info
  AArch64Utils
  Analysis
  AsmPrinter
  CFGuard
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils
  Vectorize

  ADD_TO_COMPONENT
  AArch64
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Utils)
