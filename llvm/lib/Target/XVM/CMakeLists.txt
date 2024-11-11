
if(XVM IN_LIST LLVM_TARGETS_CALLED_VIA_DYLIB AND LLVM_SPLIT_LLVM_DYLIB_TARGETS)
  ADD_DEFINITIONS("-DXVM_DYLIB_MODE=1")
  file(GLOB MCInstrAnalysisFN "${CMAKE_CURRENT_SOURCE_DIR}/../../MC/MCInstrAnalysis.cpp")
endif()

add_llvm_component_group(XVM)

set(LLVM_TARGET_DEFINITIONS XVM.td)

tablegen(LLVM XVMGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM XVMGenCallingConv.inc -gen-callingconv)
tablegen(LLVM XVMGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM XVMGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM XVMGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM XVMGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM XVMGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(XVMCommonTableGen)

add_llvm_target(XVMCodeGen
  XVMAsmPrinter.cpp
  XVMFrameLowering.cpp
  XVMInstrInfo.cpp
  XVMISelDAGToDAG.cpp
  XVMISelLowering.cpp
  XVMRegisterInfo.cpp
  XVMSubtarget.cpp
  XVMTargetMachine.cpp
  XVMMCInstLower.cpp
  XVMCFGSort.cpp
  XVMSortRegion.cpp
  XVMCFGStackify.cpp
  XVMCFGStructure.cpp
  XVMExpandPseudoInsts.cpp
  XVMSelectionDAGInfo.cpp
  XVMUpdateRefInstrForMI.cpp
  ${MCInstrAnalysisFN}

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  XVMDesc
  XVMInfo
  IPO
  Scalar
  SelectionDAG
  Support
  Target
  TransformUtils

  ADD_TO_COMPONENT
  XVM
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
