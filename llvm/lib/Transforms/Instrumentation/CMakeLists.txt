add_llvm_component_library(LLVMInstrumentation
  AddressSanitizer.cpp
  BoundsChecking.cpp
  CGProfile.cpp
  ControlHeightReduction.cpp
  DataFlowSanitizer.cpp
  GCOVProfiling.cpp
  MemProfiler.cpp
  MemorySanitizer.cpp
  IndirectCallPromotion.cpp
  Instrumentation.cpp
  InstrOrderFile.cpp
  InstrProfiling.cpp
  PGOInstrumentation.cpp
  PGOMemOPSizeOpt.cpp
  PoisonChecking.cpp
  SanitizerCoverage.cpp
  ValueProfileCollector.cpp
  ThreadSanitizer.cpp
  HWAddressSanitizer.cpp
  # OHOS_LOCAL begin
  GWPAddressSanitizer.cpp
  # OHOS_LOCAL end

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Demangle
  MC
  Support
  TransformUtils
  ProfileData
  )
