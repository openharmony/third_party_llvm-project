add_llvm_component_group(BPF)

set(EXTRA_FEAT_LIST)

if (OHOS_LLVM)
  list(APPEND EXTRA_FEAT_LIST "-DOHOS_LLVM")
endif()

set(LLVM_TARGET_DEFINITIONS BPF.td)

tablegen(LLVM BPFGenAsmMatcher.inc -gen-asm-matcher ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenAsmWriter.inc -gen-asm-writer ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenCallingConv.inc -gen-callingconv ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenDAGISel.inc -gen-dag-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenDisassemblerTables.inc -gen-disassembler ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenInstrInfo.inc -gen-instr-info ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenMCCodeEmitter.inc -gen-emitter ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenRegisterInfo.inc -gen-register-info ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenSubtargetInfo.inc -gen-subtarget ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenGlobalISel.inc -gen-global-isel ${EXTRA_FEAT_LIST})
tablegen(LLVM BPFGenRegisterBank.inc -gen-register-bank ${EXTRA_FEAT_LIST})

add_public_tablegen_target(BPFCommonTableGen)

add_llvm_target(BPFCodeGen
  GISel/BPFCallLowering.cpp
  GISel/BPFInstructionSelector.cpp
  GISel/BPFRegisterBankInfo.cpp
  GISel/BPFLegalizerInfo.cpp
  BPFAbstractMemberAccess.cpp
  BPFAdjustOpt.cpp
  BPFAsmPrinter.cpp
  BPFASpaceCastSimplifyPass.cpp
  BPFCheckAndAdjustIR.cpp
  BPFFrameLowering.cpp
  BPFInstrInfo.cpp
  BPFIRPeephole.cpp
  BPFISelDAGToDAG.cpp
  BPFISelLowering.cpp
  BPFMCInstLower.cpp
  BPFPreserveDIType.cpp
  BPFPreserveStaticOffset.cpp
  BPFRegisterInfo.cpp
  BPFSelectionDAGInfo.cpp
  BPFSubtarget.cpp
  BPFTargetMachine.cpp
  BPFMIPeephole.cpp
  BPFMIChecking.cpp
  BPFMISimplifyPatchable.cpp
  BTFDebug.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  BPFDesc
  BPFInfo
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IPO
  MC
  Passes
  Scalar
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  BPF
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
