//===--- BackendUtil.cpp - LLVM Backend Utilities -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/CodeGen/BackendUtil.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/StackSafetyAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/LTO/LTOBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Object/OffloadBinary.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Coroutines/CoroCleanup.h"
#include "llvm/Transforms/Coroutines/CoroEarly.h"
#include "llvm/Transforms/Coroutines/CoroElide.h"
#include "llvm/Transforms/Coroutines/CoroSplit.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/LowerTypeTests.h"
#include "llvm/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Instrumentation/AddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/AddressSanitizerOptions.h"
#include "llvm/Transforms/Instrumentation/BoundsChecking.h"
#include "llvm/Transforms/Instrumentation/DataFlowSanitizer.h"
#include "llvm/Transforms/Instrumentation/GCOVProfiler.h"
#include "llvm/Transforms/Instrumentation/HWAddressSanitizer.h"
#include "llvm/Transforms/Instrumentation/InstrProfiling.h"
#include "llvm/Transforms/Instrumentation/MemProfiler.h"
#include "llvm/Transforms/Instrumentation/MemorySanitizer.h"
#include "llvm/Transforms/Instrumentation/SanitizerCoverage.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"
// OHOS_LOCAL begin
#include "llvm/Transforms/Instrumentation/GWPAddressSanitizer.h"
// OHOS_LOCAL end
#include "llvm/Transforms/ObjCARC.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/LowerMatrixIntrinsics.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/CanonicalizeAliases.h"
#include "llvm/Transforms/Utils/Debugify.h"
#include "llvm/Transforms/Utils/EntryExitInstrumenter.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/NameAnonGlobals.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include <memory>
using namespace clang;
using namespace llvm;

#define HANDLE_EXTENSION(Ext)                                                  \
  llvm::PassPluginLibraryInfo get##Ext##PluginInfo();
#include "llvm/Support/Extension.def"

namespace llvm {
extern cl::opt<bool> DebugInfoCorrelate;
}

namespace {

// Default filename used for profile generation.
std::string getDefaultProfileGenName() {
  return DebugInfoCorrelate ? "default_%p.proflite" : "default_%m.profraw";
}

class EmitAssemblyHelper {
  DiagnosticsEngine &Diags;
  const HeaderSearchOptions &HSOpts;
  const CodeGenOptions &CodeGenOpts;
  const clang::TargetOptions &TargetOpts;
  const LangOptions &LangOpts;
  Module *TheModule;

  Timer CodeGenerationTime;

  std::unique_ptr<raw_pwrite_stream> OS;

  Triple TargetTriple;

  TargetIRAnalysis getTargetIRAnalysis() const {
    if (TM)
      return TM->getTargetIRAnalysis();

    return TargetIRAnalysis();
  }

  /// Generates the TargetMachine.
  /// Leaves TM unchanged if it is unable to create the target machine.
  /// Some of our clang tests specify triples which are not built
  /// into clang. This is okay because these tests check the generated
  /// IR, and they require DataLayout which depends on the triple.
  /// In this case, we allow this method to fail and not report an error.
  /// When MustCreateTM is used, we print an error if we are unable to load
  /// the requested target.
  void CreateTargetMachine(bool MustCreateTM);

  /// Add passes necessary to emit assembly or LLVM IR.
  ///
  /// \return True on success.
  bool AddEmitPasses(legacy::PassManager &CodeGenPasses, BackendAction Action,
                     raw_pwrite_stream &OS, raw_pwrite_stream *DwoOS);

  std::unique_ptr<llvm::ToolOutputFile> openOutputFile(StringRef Path) {
    std::error_code EC;
    auto F = std::make_unique<llvm::ToolOutputFile>(Path, EC,
                                                     llvm::sys::fs::OF_None);
    if (EC) {
      Diags.Report(diag::err_fe_unable_to_open_output) << Path << EC.message();
      F.reset();
    }
    return F;
  }

  void
  RunOptimizationPipeline(BackendAction Action,
                          std::unique_ptr<raw_pwrite_stream> &OS,
                          std::unique_ptr<llvm::ToolOutputFile> &ThinLinkOS);
  void RunCodegenPipeline(BackendAction Action,
                          std::unique_ptr<raw_pwrite_stream> &OS,
                          std::unique_ptr<llvm::ToolOutputFile> &DwoOS);

  /// Check whether we should emit a module summary for regular LTO.
  /// The module summary should be emitted by default for regular LTO
  /// except for ld64 targets.
  ///
  /// \return True if the module summary should be emitted.
  bool shouldEmitRegularLTOSummary() const {
    return CodeGenOpts.PrepareForLTO && !CodeGenOpts.DisableLLVMPasses &&
           TargetTriple.getVendor() != llvm::Triple::Apple;
  }

public:
  EmitAssemblyHelper(DiagnosticsEngine &_Diags,
                     const HeaderSearchOptions &HeaderSearchOpts,
                     const CodeGenOptions &CGOpts,
                     const clang::TargetOptions &TOpts,
                     const LangOptions &LOpts, Module *M)
      : Diags(_Diags), HSOpts(HeaderSearchOpts), CodeGenOpts(CGOpts),
        TargetOpts(TOpts), LangOpts(LOpts), TheModule(M),
        CodeGenerationTime("codegen", "Code Generation Time"),
        TargetTriple(TheModule->getTargetTriple()) {}

  ~EmitAssemblyHelper() {
    if (CodeGenOpts.DisableFree)
      BuryPointer(std::move(TM));
  }

  std::unique_ptr<TargetMachine> TM;

  // Emit output using the new pass manager for the optimization pipeline.
  void EmitAssembly(BackendAction Action,
                    std::unique_ptr<raw_pwrite_stream> OS);
};
}

static SanitizerCoverageOptions
getSancovOptsFromCGOpts(const CodeGenOptions &CGOpts) {
  SanitizerCoverageOptions Opts;
  Opts.CoverageType =
      static_cast<SanitizerCoverageOptions::Type>(CGOpts.SanitizeCoverageType);
  Opts.IndirectCalls = CGOpts.SanitizeCoverageIndirectCalls;
  Opts.TraceBB = CGOpts.SanitizeCoverageTraceBB;
  Opts.TraceCmp = CGOpts.SanitizeCoverageTraceCmp;
  Opts.TraceDiv = CGOpts.SanitizeCoverageTraceDiv;
  Opts.TraceGep = CGOpts.SanitizeCoverageTraceGep;
  Opts.Use8bitCounters = CGOpts.SanitizeCoverage8bitCounters;
  Opts.TracePC = CGOpts.SanitizeCoverageTracePC;
  Opts.TracePCGuard = CGOpts.SanitizeCoverageTracePCGuard;
  Opts.NoPrune = CGOpts.SanitizeCoverageNoPrune;
  Opts.Inline8bitCounters = CGOpts.SanitizeCoverageInline8bitCounters;
  Opts.InlineBoolFlag = CGOpts.SanitizeCoverageInlineBoolFlag;
  Opts.PCTable = CGOpts.SanitizeCoveragePCTable;
  Opts.StackDepth = CGOpts.SanitizeCoverageStackDepth;
  Opts.TraceLoads = CGOpts.SanitizeCoverageTraceLoads;
  Opts.TraceStores = CGOpts.SanitizeCoverageTraceStores;
  return Opts;
}

// Check if ASan should use GC-friendly instrumentation for globals.
// First of all, there is no point if -fdata-sections is off (expect for MachO,
// where this is not a factor). Also, on ELF this feature requires an assembler
// extension that only works with -integrated-as at the moment.
static bool asanUseGlobalsGC(const Triple &T, const CodeGenOptions &CGOpts) {
  if (!CGOpts.SanitizeAddressGlobalsDeadStripping)
    return false;
  switch (T.getObjectFormat()) {
  case Triple::MachO:
  case Triple::COFF:
    return true;
  case Triple::ELF:
    return !CGOpts.DisableIntegratedAS;
  case Triple::GOFF:
    llvm::report_fatal_error("ASan not implemented for GOFF");
  case Triple::XCOFF:
    llvm::report_fatal_error("ASan not implemented for XCOFF.");
  case Triple::Wasm:
  case Triple::DXContainer:
  case Triple::SPIRV:
  case Triple::UnknownObjectFormat:
    break;
  }
  return false;
}

static TargetLibraryInfoImpl *createTLII(llvm::Triple &TargetTriple,
                                         const CodeGenOptions &CodeGenOpts) {
  TargetLibraryInfoImpl *TLII = new TargetLibraryInfoImpl(TargetTriple);

  switch (CodeGenOpts.getVecLib()) {
  case CodeGenOptions::Accelerate:
    TLII->addVectorizableFunctionsFromVecLib(TargetLibraryInfoImpl::Accelerate);
    break;
  case CodeGenOptions::LIBMVEC:
    switch(TargetTriple.getArch()) {
      default:
        break;
      case llvm::Triple::x86_64:
        TLII->addVectorizableFunctionsFromVecLib
                (TargetLibraryInfoImpl::LIBMVEC_X86);
        break;
    }
    break;
  case CodeGenOptions::MASSV:
    TLII->addVectorizableFunctionsFromVecLib(TargetLibraryInfoImpl::MASSV);
    break;
  case CodeGenOptions::SVML:
    TLII->addVectorizableFunctionsFromVecLib(TargetLibraryInfoImpl::SVML);
    break;
  case CodeGenOptions::Darwin_libsystem_m:
    TLII->addVectorizableFunctionsFromVecLib(
        TargetLibraryInfoImpl::DarwinLibSystemM);
    break;
  default:
    break;
  }
  return TLII;
}

static CodeGenOpt::Level getCGOptLevel(const CodeGenOptions &CodeGenOpts) {
  switch (CodeGenOpts.OptimizationLevel) {
  default:
    llvm_unreachable("Invalid optimization level!");
  case 0:
    return CodeGenOpt::None;
  case 1:
    return CodeGenOpt::Less;
  case 2:
    return CodeGenOpt::Default; // O2/Os/Oz
  case 3:
    return CodeGenOpt::Aggressive;
  }
}

static Optional<llvm::CodeModel::Model>
getCodeModel(const CodeGenOptions &CodeGenOpts) {
  unsigned CodeModel = llvm::StringSwitch<unsigned>(CodeGenOpts.CodeModel)
                           .Case("tiny", llvm::CodeModel::Tiny)
                           .Case("small", llvm::CodeModel::Small)
                           .Case("kernel", llvm::CodeModel::Kernel)
                           .Case("medium", llvm::CodeModel::Medium)
                           .Case("large", llvm::CodeModel::Large)
                           .Case("default", ~1u)
                           .Default(~0u);
  assert(CodeModel != ~0u && "invalid code model!");
  if (CodeModel == ~1u)
    return None;
  return static_cast<llvm::CodeModel::Model>(CodeModel);
}

static CodeGenFileType getCodeGenFileType(BackendAction Action) {
  if (Action == Backend_EmitObj)
    return CGFT_ObjectFile;
  else if (Action == Backend_EmitMCNull)
    return CGFT_Null;
  else {
    assert(Action == Backend_EmitAssembly && "Invalid action!");
    return CGFT_AssemblyFile;
  }
}

static bool actionRequiresCodeGen(BackendAction Action) {
  return Action != Backend_EmitNothing && Action != Backend_EmitBC &&
         Action != Backend_EmitLL;
}

static bool initTargetOptions(DiagnosticsEngine &Diags,
                              llvm::TargetOptions &Options,
                              const CodeGenOptions &CodeGenOpts,
                              const clang::TargetOptions &TargetOpts,
                              const LangOptions &LangOpts,
                              const HeaderSearchOptions &HSOpts) {
  switch (LangOpts.getThreadModel()) {
  case LangOptions::ThreadModelKind::POSIX:
    Options.ThreadModel = llvm::ThreadModel::POSIX;
    break;
  case LangOptions::ThreadModelKind::Single:
    Options.ThreadModel = llvm::ThreadModel::Single;
    break;
  }

  // Set float ABI type.
  assert((CodeGenOpts.FloatABI == "soft" || CodeGenOpts.FloatABI == "softfp" ||
          CodeGenOpts.FloatABI == "hard" || CodeGenOpts.FloatABI.empty()) &&
         "Invalid Floating Point ABI!");
  Options.FloatABIType =
      llvm::StringSwitch<llvm::FloatABI::ABIType>(CodeGenOpts.FloatABI)
          .Case("soft", llvm::FloatABI::Soft)
          .Case("softfp", llvm::FloatABI::Soft)
          .Case("hard", llvm::FloatABI::Hard)
          .Default(llvm::FloatABI::Default);

  // Set FP fusion mode.
  switch (LangOpts.getDefaultFPContractMode()) {
  case LangOptions::FPM_Off:
    // Preserve any contraction performed by the front-end.  (Strict performs
    // splitting of the muladd intrinsic in the backend.)
    Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    break;
  case LangOptions::FPM_On:
  case LangOptions::FPM_FastHonorPragmas:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    break;
  case LangOptions::FPM_Fast:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    break;
  }

  Options.BinutilsVersion =
      llvm::TargetMachine::parseBinutilsVersion(CodeGenOpts.BinutilsVersion);
  Options.UseInitArray = CodeGenOpts.UseInitArray;
  Options.LowerGlobalDtorsViaCxaAtExit =
      CodeGenOpts.RegisterGlobalDtorsWithAtExit;
  Options.DisableIntegratedAS = CodeGenOpts.DisableIntegratedAS;
  Options.CompressDebugSections = CodeGenOpts.getCompressDebugSections();
  Options.RelaxELFRelocations = CodeGenOpts.RelaxELFRelocations;

  // Set EABI version.
  Options.EABIVersion = TargetOpts.EABIVersion;

  if (LangOpts.hasSjLjExceptions())
    Options.ExceptionModel = llvm::ExceptionHandling::SjLj;
  if (LangOpts.hasSEHExceptions())
    Options.ExceptionModel = llvm::ExceptionHandling::WinEH;
  if (LangOpts.hasDWARFExceptions())
    Options.ExceptionModel = llvm::ExceptionHandling::DwarfCFI;
  if (LangOpts.hasWasmExceptions())
    Options.ExceptionModel = llvm::ExceptionHandling::Wasm;

  Options.NoInfsFPMath = LangOpts.NoHonorInfs;
  Options.NoNaNsFPMath = LangOpts.NoHonorNaNs;
  Options.NoZerosInBSS = CodeGenOpts.NoZeroInitializedInBSS;
  Options.UnsafeFPMath = LangOpts.UnsafeFPMath;
  Options.ApproxFuncFPMath = LangOpts.ApproxFunc;

  Options.BBSections =
      llvm::StringSwitch<llvm::BasicBlockSection>(CodeGenOpts.BBSections)
          .Case("all", llvm::BasicBlockSection::All)
          .Case("labels", llvm::BasicBlockSection::Labels)
          .StartsWith("list=", llvm::BasicBlockSection::List)
          .Case("none", llvm::BasicBlockSection::None)
          .Default(llvm::BasicBlockSection::None);

  if (Options.BBSections == llvm::BasicBlockSection::List) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
        MemoryBuffer::getFile(CodeGenOpts.BBSections.substr(5));
    if (!MBOrErr) {
      Diags.Report(diag::err_fe_unable_to_load_basic_block_sections_file)
          << MBOrErr.getError().message();
      return false;
    }
    Options.BBSectionsFuncListBuf = std::move(*MBOrErr);
  }

  Options.EnableMachineFunctionSplitter = CodeGenOpts.SplitMachineFunctions;
  Options.FunctionSections = CodeGenOpts.FunctionSections;
  Options.DataSections = CodeGenOpts.DataSections;
  Options.IgnoreXCOFFVisibility = LangOpts.IgnoreXCOFFVisibility;
  Options.UniqueSectionNames = CodeGenOpts.UniqueSectionNames;
  Options.UniqueBasicBlockSectionNames =
      CodeGenOpts.UniqueBasicBlockSectionNames;
  Options.TLSSize = CodeGenOpts.TLSSize;
  Options.EmulatedTLS = CodeGenOpts.EmulatedTLS;
  Options.ExplicitEmulatedTLS = CodeGenOpts.ExplicitEmulatedTLS;
  Options.DebuggerTuning = CodeGenOpts.getDebuggerTuning();
  Options.EmitStackSizeSection = CodeGenOpts.StackSizeSection;
  Options.StackUsageOutput = CodeGenOpts.StackUsageOutput;
  Options.EmitAddrsig = CodeGenOpts.Addrsig;
  Options.ForceDwarfFrameSection = CodeGenOpts.ForceDwarfFrameSection;
  Options.EmitCallSiteInfo = CodeGenOpts.EmitCallSiteInfo;
  Options.EnableAIXExtendedAltivecABI = CodeGenOpts.EnableAIXExtendedAltivecABI;
  Options.XRayOmitFunctionIndex = CodeGenOpts.XRayOmitFunctionIndex;
  Options.LoopAlignment = CodeGenOpts.LoopAlignment;
  Options.DebugStrictDwarf = CodeGenOpts.DebugStrictDwarf;
  Options.ObjectFilenameForDebug = CodeGenOpts.ObjectFilenameForDebug;
  Options.Hotpatch = CodeGenOpts.HotPatch;
  Options.JMCInstrument = CodeGenOpts.JMCInstrument;

  switch (CodeGenOpts.getSwiftAsyncFramePointer()) {
  case CodeGenOptions::SwiftAsyncFramePointerKind::Auto:
    Options.SwiftAsyncFramePointer =
        SwiftAsyncFramePointerMode::DeploymentBased;
    break;

  case CodeGenOptions::SwiftAsyncFramePointerKind::Always:
    Options.SwiftAsyncFramePointer = SwiftAsyncFramePointerMode::Always;
    break;

  case CodeGenOptions::SwiftAsyncFramePointerKind::Never:
    Options.SwiftAsyncFramePointer = SwiftAsyncFramePointerMode::Never;
    break;
  }

  Options.MCOptions.SplitDwarfFile = CodeGenOpts.SplitDwarfFile;
  Options.MCOptions.EmitDwarfUnwind = CodeGenOpts.getEmitDwarfUnwind();
  Options.MCOptions.MCRelaxAll = CodeGenOpts.RelaxAll;
  Options.MCOptions.MCSaveTempLabels = CodeGenOpts.SaveTempLabels;
  Options.MCOptions.MCUseDwarfDirectory =
      CodeGenOpts.NoDwarfDirectoryAsm
          ? llvm::MCTargetOptions::DisableDwarfDirectory
          : llvm::MCTargetOptions::EnableDwarfDirectory;
  Options.MCOptions.MCNoExecStack = CodeGenOpts.NoExecStack;
  Options.MCOptions.MCIncrementalLinkerCompatible =
      CodeGenOpts.IncrementalLinkerCompatible;
  Options.MCOptions.MCFatalWarnings = CodeGenOpts.FatalWarnings;
  Options.MCOptions.MCNoWarn = CodeGenOpts.NoWarn;
  Options.MCOptions.AsmVerbose = CodeGenOpts.AsmVerbose;
  Options.MCOptions.Dwarf64 = CodeGenOpts.Dwarf64;
  Options.MCOptions.PreserveAsmComments = CodeGenOpts.PreserveAsmComments;
  Options.MCOptions.ABIName = TargetOpts.ABI;
  for (const auto &Entry : HSOpts.UserEntries)
    if (!Entry.IsFramework &&
        (Entry.Group == frontend::IncludeDirGroup::Quoted ||
         Entry.Group == frontend::IncludeDirGroup::Angled ||
         Entry.Group == frontend::IncludeDirGroup::System))
      Options.MCOptions.IASSearchPaths.push_back(
          Entry.IgnoreSysRoot ? Entry.Path : HSOpts.Sysroot + Entry.Path);
  Options.MCOptions.Argv0 = CodeGenOpts.Argv0;
  Options.MCOptions.CommandLineArgs = CodeGenOpts.CommandLineArgs;
  Options.MisExpect = CodeGenOpts.MisExpect;

  return true;
}

static Optional<GCOVOptions> getGCOVOptions(const CodeGenOptions &CodeGenOpts,
                                            const LangOptions &LangOpts) {
  if (!CodeGenOpts.EmitGcovArcs && !CodeGenOpts.EmitGcovNotes)
    return None;
  // Not using 'GCOVOptions::getDefault' allows us to avoid exiting if
  // LLVM's -default-gcov-version flag is set to something invalid.
  GCOVOptions Options;
  Options.EmitNotes = CodeGenOpts.EmitGcovNotes;
  Options.EmitData = CodeGenOpts.EmitGcovArcs;
  llvm::copy(CodeGenOpts.CoverageVersion, std::begin(Options.Version));
  Options.NoRedZone = CodeGenOpts.DisableRedZone;
  Options.Filter = CodeGenOpts.ProfileFilterFiles;
  Options.Exclude = CodeGenOpts.ProfileExcludeFiles;
  Options.Atomic = CodeGenOpts.AtomicProfileUpdate;
  return Options;
}

static Optional<InstrProfOptions>
getInstrProfOptions(const CodeGenOptions &CodeGenOpts,
                    const LangOptions &LangOpts) {
  if (!CodeGenOpts.hasProfileClangInstr())
    return None;
  InstrProfOptions Options;
  Options.NoRedZone = CodeGenOpts.DisableRedZone;
  Options.InstrProfileOutput = CodeGenOpts.InstrProfileOutput;
  Options.Atomic = CodeGenOpts.AtomicProfileUpdate;
  return Options;
}

static void setCommandLineOpts(const CodeGenOptions &CodeGenOpts) {
  SmallVector<const char *, 16> BackendArgs;
  BackendArgs.push_back("clang"); // Fake program name.
  if (!CodeGenOpts.DebugPass.empty()) {
    BackendArgs.push_back("-debug-pass");
    BackendArgs.push_back(CodeGenOpts.DebugPass.c_str());
  }
  if (!CodeGenOpts.LimitFloatPrecision.empty()) {
    BackendArgs.push_back("-limit-float-precision");
    BackendArgs.push_back(CodeGenOpts.LimitFloatPrecision.c_str());
  }
  // Check for the default "clang" invocation that won't set any cl::opt values.
  // Skip trying to parse the command line invocation to avoid the issues
  // described below.
  if (BackendArgs.size() == 1)
    return;
  BackendArgs.push_back(nullptr);
  // FIXME: The command line parser below is not thread-safe and shares a global
  // state, so this call might crash or overwrite the options of another Clang
  // instance in the same process.
  llvm::cl::ParseCommandLineOptions(BackendArgs.size() - 1,
                                    BackendArgs.data());
}

void EmitAssemblyHelper::CreateTargetMachine(bool MustCreateTM) {
  // Create the TargetMachine for generating code.
  std::string Error;
  std::string Triple = TheModule->getTargetTriple();
  const llvm::Target *TheTarget = TargetRegistry::lookupTarget(Triple, Error);
  if (!TheTarget) {
    if (MustCreateTM)
      Diags.Report(diag::err_fe_unable_to_create_target) << Error;
    return;
  }

  Optional<llvm::CodeModel::Model> CM = getCodeModel(CodeGenOpts);
  std::string FeaturesStr =
      llvm::join(TargetOpts.Features.begin(), TargetOpts.Features.end(), ",");
  llvm::Reloc::Model RM = CodeGenOpts.RelocationModel;
  CodeGenOpt::Level OptLevel = getCGOptLevel(CodeGenOpts);

  llvm::TargetOptions Options;
  if (!initTargetOptions(Diags, Options, CodeGenOpts, TargetOpts, LangOpts,
                         HSOpts))
    return;
  TM.reset(TheTarget->createTargetMachine(Triple, TargetOpts.CPU, FeaturesStr,
                                          Options, RM, CM, OptLevel));
}

bool EmitAssemblyHelper::AddEmitPasses(legacy::PassManager &CodeGenPasses,
                                       BackendAction Action,
                                       raw_pwrite_stream &OS,
                                       raw_pwrite_stream *DwoOS) {
  // Add LibraryInfo.
  std::unique_ptr<TargetLibraryInfoImpl> TLII(
      createTLII(TargetTriple, CodeGenOpts));
  CodeGenPasses.add(new TargetLibraryInfoWrapperPass(*TLII));

  // Normal mode, emit a .s or .o file by running the code generator. Note,
  // this also adds codegenerator level optimization passes.
  CodeGenFileType CGFT = getCodeGenFileType(Action);

  // Add ObjC ARC final-cleanup optimizations. This is done as part of the
  // "codegen" passes so that it isn't run multiple times when there is
  // inlining happening.
  if (CodeGenOpts.OptimizationLevel > 0)
    CodeGenPasses.add(createObjCARCContractPass());

  if (TM->addPassesToEmitFile(CodeGenPasses, OS, DwoOS, CGFT,
                              /*DisableVerify=*/!CodeGenOpts.VerifyModule)) {
    Diags.Report(diag::err_fe_unable_to_interface_with_target);
    return false;
  }

  return true;
}

static OptimizationLevel mapToLevel(const CodeGenOptions &Opts) {
  switch (Opts.OptimizationLevel) {
  default:
    llvm_unreachable("Invalid optimization level!");

  case 0:
    return OptimizationLevel::O0;

  case 1:
    return OptimizationLevel::O1;

  case 2:
    switch (Opts.OptimizeSize) {
    default:
      llvm_unreachable("Invalid optimization level for size!");

    case 0:
      return OptimizationLevel::O2;

    case 1:
      return OptimizationLevel::Os;

    case 2:
      return OptimizationLevel::Oz;
    }

  case 3:
    return OptimizationLevel::O3;
  }
}

static void addSanitizers(const Triple &TargetTriple,
                          const CodeGenOptions &CodeGenOpts,
                          const LangOptions &LangOpts, PassBuilder &PB) {
  PB.registerOptimizerLastEPCallback([&](ModulePassManager &MPM,
                                         OptimizationLevel Level) {
    if (CodeGenOpts.hasSanitizeCoverage()) {
      auto SancovOpts = getSancovOptsFromCGOpts(CodeGenOpts);
      MPM.addPass(ModuleSanitizerCoveragePass(
          SancovOpts, CodeGenOpts.SanitizeCoverageAllowlistFiles,
          CodeGenOpts.SanitizeCoverageIgnorelistFiles));
    }

    auto MSanPass = [&](SanitizerMask Mask, bool CompileKernel) {
      if (LangOpts.Sanitize.has(Mask)) {
        int TrackOrigins = CodeGenOpts.SanitizeMemoryTrackOrigins;
        bool Recover = CodeGenOpts.SanitizeRecover.has(Mask);

        MemorySanitizerOptions options(TrackOrigins, Recover, CompileKernel,
                                       CodeGenOpts.SanitizeMemoryParamRetval);
        MPM.addPass(ModuleMemorySanitizerPass(options));
        FunctionPassManager FPM;
        FPM.addPass(MemorySanitizerPass(options));
        if (Level != OptimizationLevel::O0) {
          // MemorySanitizer inserts complex instrumentation that mostly
          // follows the logic of the original code, but operates on
          // "shadow" values. It can benefit from re-running some
          // general purpose optimization passes.
          FPM.addPass(EarlyCSEPass());
          // TODO: Consider add more passes like in
          // addGeneralOptsForMemorySanitizer. EarlyCSEPass makes visible
          // difference on size. It's not clear if the rest is still
          // usefull. InstCombinePass breakes
          // compiler-rt/test/msan/select_origin.cpp.
        }
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
      }
    };
    MSanPass(SanitizerKind::Memory, false);
    MSanPass(SanitizerKind::KernelMemory, true);

    if (LangOpts.Sanitize.has(SanitizerKind::Thread)) {
      MPM.addPass(ModuleThreadSanitizerPass());
      MPM.addPass(createModuleToFunctionPassAdaptor(ThreadSanitizerPass()));
    }

    // OHOS_LOCAL begin
    if (LangOpts.Sanitize.has(SanitizerKind::GWPAsan)) {
      MPM.addPass(ModuleGWPAddressSanitizerPass());
      MPM.addPass(createModuleToFunctionPassAdaptor(GWPAddressSanitizerPass()));
    }
    // OHOS_LOCAL end

    auto ASanPass = [&](SanitizerMask Mask, bool CompileKernel) {
      if (LangOpts.Sanitize.has(Mask)) {
        bool UseGlobalGC = asanUseGlobalsGC(TargetTriple, CodeGenOpts);
        bool UseOdrIndicator = CodeGenOpts.SanitizeAddressUseOdrIndicator;
        llvm::AsanDtorKind DestructorKind =
            CodeGenOpts.getSanitizeAddressDtor();
        AddressSanitizerOptions Opts;
        Opts.CompileKernel = CompileKernel;
        Opts.Recover = CodeGenOpts.SanitizeRecover.has(Mask);
        Opts.UseAfterScope = CodeGenOpts.SanitizeAddressUseAfterScope;
        Opts.UseAfterReturn = CodeGenOpts.getSanitizeAddressUseAfterReturn();
        MPM.addPass(ModuleAddressSanitizerPass(
            Opts, UseGlobalGC, UseOdrIndicator, DestructorKind));
      }
    };
    ASanPass(SanitizerKind::Address, false);
    ASanPass(SanitizerKind::KernelAddress, true);

    auto HWASanPass = [&](SanitizerMask Mask, bool CompileKernel) {
      if (LangOpts.Sanitize.has(Mask)) {
        bool Recover = CodeGenOpts.SanitizeRecover.has(Mask);
        MPM.addPass(HWAddressSanitizerPass(
            {CompileKernel, Recover,
             /*DisableOptimization=*/CodeGenOpts.OptimizationLevel == 0}));
      }
    };
    HWASanPass(SanitizerKind::HWAddress, false);
    HWASanPass(SanitizerKind::KernelHWAddress, true);

    if (LangOpts.Sanitize.has(SanitizerKind::DataFlow)) {
      MPM.addPass(DataFlowSanitizerPass(LangOpts.NoSanitizeFiles));
    }
  });
}

void EmitAssemblyHelper::RunOptimizationPipeline(
    BackendAction Action, std::unique_ptr<raw_pwrite_stream> &OS,
    std::unique_ptr<llvm::ToolOutputFile> &ThinLinkOS) {
  Optional<PGOOptions> PGOOpt;

  if (CodeGenOpts.hasProfileIRInstr())
    // -fprofile-generate.
    PGOOpt = PGOOptions(CodeGenOpts.InstrProfileOutput.empty()
                            ? getDefaultProfileGenName()
                            : CodeGenOpts.InstrProfileOutput,
                        "", "", PGOOptions::IRInstr, PGOOptions::NoCSAction,
                        CodeGenOpts.DebugInfoForProfiling);
  else if (CodeGenOpts.hasProfileIRUse()) {
    // -fprofile-use.
    auto CSAction = CodeGenOpts.hasProfileCSIRUse() ? PGOOptions::CSIRUse
                                                    : PGOOptions::NoCSAction;
    PGOOpt = PGOOptions(CodeGenOpts.ProfileInstrumentUsePath, "",
                        CodeGenOpts.ProfileRemappingFile, PGOOptions::IRUse,
                        CSAction, CodeGenOpts.DebugInfoForProfiling);
  } else if (!CodeGenOpts.SampleProfileFile.empty())
    // -fprofile-sample-use
    PGOOpt = PGOOptions(
        CodeGenOpts.SampleProfileFile, "", CodeGenOpts.ProfileRemappingFile,
        PGOOptions::SampleUse, PGOOptions::NoCSAction,
        CodeGenOpts.DebugInfoForProfiling, CodeGenOpts.PseudoProbeForProfiling);
  else if (CodeGenOpts.PseudoProbeForProfiling)
    // -fpseudo-probe-for-profiling
    PGOOpt =
        PGOOptions("", "", "", PGOOptions::NoAction, PGOOptions::NoCSAction,
                   CodeGenOpts.DebugInfoForProfiling, true);
  else if (CodeGenOpts.DebugInfoForProfiling)
    // -fdebug-info-for-profiling
    PGOOpt = PGOOptions("", "", "", PGOOptions::NoAction,
                        PGOOptions::NoCSAction, true);

  // Check to see if we want to generate a CS profile.
  if (CodeGenOpts.hasProfileCSIRInstr()) {
    assert(!CodeGenOpts.hasProfileCSIRUse() &&
           "Cannot have both CSProfileUse pass and CSProfileGen pass at "
           "the same time");
    if (PGOOpt) {
      assert(PGOOpt->Action != PGOOptions::IRInstr &&
             PGOOpt->Action != PGOOptions::SampleUse &&
             "Cannot run CSProfileGen pass with ProfileGen or SampleUse "
             " pass");
      PGOOpt->CSProfileGenFile = CodeGenOpts.InstrProfileOutput.empty()
                                     ? getDefaultProfileGenName()
                                     : CodeGenOpts.InstrProfileOutput;
      PGOOpt->CSAction = PGOOptions::CSIRInstr;
    } else
      PGOOpt = PGOOptions("",
                          CodeGenOpts.InstrProfileOutput.empty()
                              ? getDefaultProfileGenName()
                              : CodeGenOpts.InstrProfileOutput,
                          "", PGOOptions::NoAction, PGOOptions::CSIRInstr,
                          CodeGenOpts.DebugInfoForProfiling);
  }
  if (TM)
    TM->setPGOOption(PGOOpt);

  PipelineTuningOptions PTO;
  PTO.LoopUnrolling = CodeGenOpts.UnrollLoops;
  // For historical reasons, loop interleaving is set to mirror setting for loop
  // unrolling.
  PTO.LoopInterleaving = CodeGenOpts.UnrollLoops;
  PTO.LoopVectorization = CodeGenOpts.VectorizeLoop;
  PTO.SLPVectorization = CodeGenOpts.VectorizeSLP;
  PTO.MergeFunctions = CodeGenOpts.MergeFunctions;
  // Only enable CGProfilePass when using integrated assembler, since
  // non-integrated assemblers don't recognize .cgprofile section.
  PTO.CallGraphProfile = !CodeGenOpts.DisableIntegratedAS;

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  bool DebugPassStructure = CodeGenOpts.DebugPass == "Structure";
  PassInstrumentationCallbacks PIC;
  PrintPassOptions PrintPassOpts;
  PrintPassOpts.Indent = DebugPassStructure;
  PrintPassOpts.SkipAnalyses = DebugPassStructure;
  StandardInstrumentations SI(CodeGenOpts.DebugPassManager ||
                                  DebugPassStructure,
                              /*VerifyEach*/ false, PrintPassOpts);
  SI.registerCallbacks(PIC, &FAM);
  PassBuilder PB(TM.get(), PTO, PGOOpt, &PIC);

  // Enable verify-debuginfo-preserve-each for new PM.
  DebugifyEachInstrumentation Debugify;
  DebugInfoPerPass DebugInfoBeforePass;
  if (CodeGenOpts.EnableDIPreservationVerify) {
    Debugify.setDebugifyMode(DebugifyMode::OriginalDebugInfo);
    Debugify.setDebugInfoBeforePass(DebugInfoBeforePass);

    if (!CodeGenOpts.DIBugsReportFilePath.empty())
      Debugify.setOrigDIVerifyBugsReportFilePath(
          CodeGenOpts.DIBugsReportFilePath);
    Debugify.registerCallbacks(PIC);
  }
  // Attempt to load pass plugins and register their callbacks with PB.
  for (auto &PluginFN : CodeGenOpts.PassPlugins) {
    auto PassPlugin = PassPlugin::Load(PluginFN);
    if (PassPlugin) {
      PassPlugin->registerPassBuilderCallbacks(PB);
    } else {
      Diags.Report(diag::err_fe_unable_to_load_plugin)
          << PluginFN << toString(PassPlugin.takeError());
    }
  }
#define HANDLE_EXTENSION(Ext)                                                  \
  get##Ext##PluginInfo().RegisterPassBuilderCallbacks(PB);
#include "llvm/Support/Extension.def"

  // Register the target library analysis directly and give it a customized
  // preset TLI.
  std::unique_ptr<TargetLibraryInfoImpl> TLII(
      createTLII(TargetTriple, CodeGenOpts));
  FAM.registerPass([&] { return TargetLibraryAnalysis(*TLII); });

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  ModulePassManager MPM;

  if (!CodeGenOpts.DisableLLVMPasses) {
    // Map our optimization levels into one of the distinct levels used to
    // configure the pipeline.
    OptimizationLevel Level = mapToLevel(CodeGenOpts);

    bool IsThinLTO = CodeGenOpts.PrepareForThinLTO;
    bool IsLTO = CodeGenOpts.PrepareForLTO;

    if (LangOpts.ObjCAutoRefCount) {
      PB.registerPipelineStartEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel Level) {
            if (Level != OptimizationLevel::O0)
              MPM.addPass(
                  createModuleToFunctionPassAdaptor(ObjCARCExpandPass()));
          });
      PB.registerPipelineEarlySimplificationEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel Level) {
            if (Level != OptimizationLevel::O0)
              MPM.addPass(ObjCARCAPElimPass());
          });
      PB.registerScalarOptimizerLateEPCallback(
          [](FunctionPassManager &FPM, OptimizationLevel Level) {
            if (Level != OptimizationLevel::O0)
              FPM.addPass(ObjCARCOptPass());
          });
    }

    // If we reached here with a non-empty index file name, then the index
    // file was empty and we are not performing ThinLTO backend compilation
    // (used in testing in a distributed build environment).
    bool IsThinLTOPostLink = !CodeGenOpts.ThinLTOIndexFile.empty();
    // If so drop any the type test assume sequences inserted for whole program
    // vtables so that codegen doesn't complain.
    if (IsThinLTOPostLink)
      PB.registerPipelineStartEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel Level) {
            MPM.addPass(LowerTypeTestsPass(/*ExportSummary=*/nullptr,
                                           /*ImportSummary=*/nullptr,
                                           /*DropTypeTests=*/true));
          });

    if (CodeGenOpts.InstrumentFunctions ||
        CodeGenOpts.InstrumentFunctionEntryBare ||
        CodeGenOpts.InstrumentFunctionsAfterInlining ||
        CodeGenOpts.InstrumentForProfiling) {
      PB.registerPipelineStartEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel Level) {
            MPM.addPass(createModuleToFunctionPassAdaptor(
                EntryExitInstrumenterPass(/*PostInlining=*/false)));
          });
      PB.registerOptimizerLastEPCallback(
          [](ModulePassManager &MPM, OptimizationLevel Level) {
            MPM.addPass(createModuleToFunctionPassAdaptor(
                EntryExitInstrumenterPass(/*PostInlining=*/true)));
          });
    }

    // Register callbacks to schedule sanitizer passes at the appropriate part
    // of the pipeline.
    if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds))
      PB.registerScalarOptimizerLateEPCallback(
          [](FunctionPassManager &FPM, OptimizationLevel Level) {
            FPM.addPass(BoundsCheckingPass());
          });

    // Don't add sanitizers if we are here from ThinLTO PostLink. That already
    // done on PreLink stage.
    if (!IsThinLTOPostLink)
      addSanitizers(TargetTriple, CodeGenOpts, LangOpts, PB);

    if (Optional<GCOVOptions> Options = getGCOVOptions(CodeGenOpts, LangOpts))
      PB.registerPipelineStartEPCallback(
          [Options](ModulePassManager &MPM, OptimizationLevel Level) {
            MPM.addPass(GCOVProfilerPass(*Options));
          });
    if (Optional<InstrProfOptions> Options =
            getInstrProfOptions(CodeGenOpts, LangOpts))
      PB.registerPipelineStartEPCallback(
          [Options](ModulePassManager &MPM, OptimizationLevel Level) {
            MPM.addPass(InstrProfiling(*Options, false));
          });

    if (CodeGenOpts.OptimizationLevel == 0) {
      MPM = PB.buildO0DefaultPipeline(Level, IsLTO || IsThinLTO);
    } else if (IsThinLTO) {
      MPM = PB.buildThinLTOPreLinkDefaultPipeline(Level);
    } else if (IsLTO) {
      MPM = PB.buildLTOPreLinkDefaultPipeline(Level);
    } else {
      MPM = PB.buildPerModuleDefaultPipeline(Level);
    }

    if (!CodeGenOpts.MemoryProfileOutput.empty()) {
      MPM.addPass(createModuleToFunctionPassAdaptor(MemProfilerPass()));
      MPM.addPass(ModuleMemProfilerPass());
    }
  }

  // Add a verifier pass if requested. We don't have to do this if the action
  // requires code generation because there will already be a verifier pass in
  // the code-generation pipeline.
  if (!actionRequiresCodeGen(Action) && CodeGenOpts.VerifyModule)
    MPM.addPass(VerifierPass());

  switch (Action) {
  case Backend_EmitBC:
    if (CodeGenOpts.PrepareForThinLTO && !CodeGenOpts.DisableLLVMPasses) {
      if (!CodeGenOpts.ThinLinkBitcodeFile.empty()) {
        ThinLinkOS = openOutputFile(CodeGenOpts.ThinLinkBitcodeFile);
        if (!ThinLinkOS)
          return;
      }
      if (!TheModule->getModuleFlag("EnableSplitLTOUnit"))
        TheModule->addModuleFlag(Module::Error, "EnableSplitLTOUnit",
                                 CodeGenOpts.EnableSplitLTOUnit);
      MPM.addPass(ThinLTOBitcodeWriterPass(*OS, ThinLinkOS ? &ThinLinkOS->os()
                                                           : nullptr));
    } else {
      // Emit a module summary by default for Regular LTO except for ld64
      // targets
      bool EmitLTOSummary = shouldEmitRegularLTOSummary();
      if (EmitLTOSummary) {
        if (!TheModule->getModuleFlag("ThinLTO"))
          TheModule->addModuleFlag(Module::Error, "ThinLTO", uint32_t(0));
        if (!TheModule->getModuleFlag("EnableSplitLTOUnit"))
          TheModule->addModuleFlag(Module::Error, "EnableSplitLTOUnit",
                                   uint32_t(1));
      }
      MPM.addPass(
          BitcodeWriterPass(*OS, CodeGenOpts.EmitLLVMUseLists, EmitLTOSummary));
    }
    break;

  case Backend_EmitLL:
    MPM.addPass(PrintModulePass(*OS, "", CodeGenOpts.EmitLLVMUseLists));
    break;

  default:
    break;
  }

  // Now that we have all of the passes ready, run them.
  {
    PrettyStackTraceString CrashInfo("Optimizer");
    llvm::TimeTraceScope TimeScope("Optimizer");
    MPM.run(*TheModule, MAM);
  }
}

void EmitAssemblyHelper::RunCodegenPipeline(
    BackendAction Action, std::unique_ptr<raw_pwrite_stream> &OS,
    std::unique_ptr<llvm::ToolOutputFile> &DwoOS) {
  // We still use the legacy PM to run the codegen pipeline since the new PM
  // does not work with the codegen pipeline.
  // FIXME: make the new PM work with the codegen pipeline.
  legacy::PassManager CodeGenPasses;

  // Append any output we need to the pass manager.
  switch (Action) {
  case Backend_EmitAssembly:
  case Backend_EmitMCNull:
  case Backend_EmitObj:
    CodeGenPasses.add(
        createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));
    if (!CodeGenOpts.SplitDwarfOutput.empty()) {
      DwoOS = openOutputFile(CodeGenOpts.SplitDwarfOutput);
      if (!DwoOS)
        return;
    }
    if (!AddEmitPasses(CodeGenPasses, Action, *OS,
                       DwoOS ? &DwoOS->os() : nullptr))
      // FIXME: Should we handle this error differently?
      return;
    break;
  default:
    return;
  }

  {
    PrettyStackTraceString CrashInfo("Code generation");
    llvm::TimeTraceScope TimeScope("CodeGenPasses");
    CodeGenPasses.run(*TheModule);
  }
}

void EmitAssemblyHelper::EmitAssembly(BackendAction Action,
                                      std::unique_ptr<raw_pwrite_stream> OS) {
  TimeRegion Region(CodeGenOpts.TimePasses ? &CodeGenerationTime : nullptr);
  setCommandLineOpts(CodeGenOpts);

  bool RequiresCodeGen = actionRequiresCodeGen(Action);
  CreateTargetMachine(RequiresCodeGen);

  if (RequiresCodeGen && !TM)
    return;
  if (TM)
    TheModule->setDataLayout(TM->createDataLayout());

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  std::unique_ptr<llvm::ToolOutputFile> ThinLinkOS, DwoOS;
  RunOptimizationPipeline(Action, OS, ThinLinkOS);
  RunCodegenPipeline(Action, OS, DwoOS);

  if (ThinLinkOS)
    ThinLinkOS->keep();
  if (DwoOS)
    DwoOS->keep();
}

static void runThinLTOBackend(
    DiagnosticsEngine &Diags, ModuleSummaryIndex *CombinedIndex, Module *M,
    const HeaderSearchOptions &HeaderOpts, const CodeGenOptions &CGOpts,
    const clang::TargetOptions &TOpts, const LangOptions &LOpts,
    std::unique_ptr<raw_pwrite_stream> OS, std::string SampleProfile,
    std::string ProfileRemapping, BackendAction Action) {
  StringMap<DenseMap<GlobalValue::GUID, GlobalValueSummary *>>
      ModuleToDefinedGVSummaries;
  CombinedIndex->collectDefinedGVSummariesPerModule(ModuleToDefinedGVSummaries);

  setCommandLineOpts(CGOpts);

  // We can simply import the values mentioned in the combined index, since
  // we should only invoke this using the individual indexes written out
  // via a WriteIndexesThinBackend.
  FunctionImporter::ImportMapTy ImportList;
  if (!lto::initImportList(*M, *CombinedIndex, ImportList))
    return;

  auto AddStream = [&](size_t Task) {
    return std::make_unique<CachedFileStream>(std::move(OS),
                                              CGOpts.ObjectFilenameForDebug);
  };
  lto::Config Conf;
  if (CGOpts.SaveTempsFilePrefix != "") {
    if (Error E = Conf.addSaveTemps(CGOpts.SaveTempsFilePrefix + ".",
                                    /* UseInputModulePath */ false)) {
      handleAllErrors(std::move(E), [&](ErrorInfoBase &EIB) {
        errs() << "Error setting up ThinLTO save-temps: " << EIB.message()
               << '\n';
      });
    }
  }
  Conf.CPU = TOpts.CPU;
  Conf.CodeModel = getCodeModel(CGOpts);
  Conf.MAttrs = TOpts.Features;
  Conf.RelocModel = CGOpts.RelocationModel;
  Conf.CGOptLevel = getCGOptLevel(CGOpts);
  Conf.OptLevel = CGOpts.OptimizationLevel;
  initTargetOptions(Diags, Conf.Options, CGOpts, TOpts, LOpts, HeaderOpts);
  Conf.SampleProfile = std::move(SampleProfile);
  Conf.PTO.LoopUnrolling = CGOpts.UnrollLoops;
  // For historical reasons, loop interleaving is set to mirror setting for loop
  // unrolling.
  Conf.PTO.LoopInterleaving = CGOpts.UnrollLoops;
  Conf.PTO.LoopVectorization = CGOpts.VectorizeLoop;
  Conf.PTO.SLPVectorization = CGOpts.VectorizeSLP;
  // Only enable CGProfilePass when using integrated assembler, since
  // non-integrated assemblers don't recognize .cgprofile section.
  Conf.PTO.CallGraphProfile = !CGOpts.DisableIntegratedAS;

  // Context sensitive profile.
  if (CGOpts.hasProfileCSIRInstr()) {
    Conf.RunCSIRInstr = true;
    Conf.CSIRProfile = std::move(CGOpts.InstrProfileOutput);
  } else if (CGOpts.hasProfileCSIRUse()) {
    Conf.RunCSIRInstr = false;
    Conf.CSIRProfile = std::move(CGOpts.ProfileInstrumentUsePath);
  }

  Conf.ProfileRemapping = std::move(ProfileRemapping);
  Conf.DebugPassManager = CGOpts.DebugPassManager;
  Conf.RemarksWithHotness = CGOpts.DiagnosticsWithHotness;
  Conf.RemarksFilename = CGOpts.OptRecordFile;
  Conf.RemarksPasses = CGOpts.OptRecordPasses;
  Conf.RemarksFormat = CGOpts.OptRecordFormat;
  Conf.SplitDwarfFile = CGOpts.SplitDwarfFile;
  Conf.SplitDwarfOutput = CGOpts.SplitDwarfOutput;
  switch (Action) {
  case Backend_EmitNothing:
    Conf.PreCodeGenModuleHook = [](size_t Task, const Module &Mod) {
      return false;
    };
    break;
  case Backend_EmitLL:
    Conf.PreCodeGenModuleHook = [&](size_t Task, const Module &Mod) {
      M->print(*OS, nullptr, CGOpts.EmitLLVMUseLists);
      return false;
    };
    break;
  case Backend_EmitBC:
    Conf.PreCodeGenModuleHook = [&](size_t Task, const Module &Mod) {
      WriteBitcodeToFile(*M, *OS, CGOpts.EmitLLVMUseLists);
      return false;
    };
    break;
  default:
    Conf.CGFileType = getCodeGenFileType(Action);
    break;
  }
  if (Error E =
          thinBackend(Conf, -1, AddStream, *M, *CombinedIndex, ImportList,
                      ModuleToDefinedGVSummaries[M->getModuleIdentifier()],
                      /* ModuleMap */ nullptr, CGOpts.CmdArgs)) {
    handleAllErrors(std::move(E), [&](ErrorInfoBase &EIB) {
      errs() << "Error running ThinLTO backend: " << EIB.message() << '\n';
    });
  }
}

void clang::EmitBackendOutput(DiagnosticsEngine &Diags,
                              const HeaderSearchOptions &HeaderOpts,
                              const CodeGenOptions &CGOpts,
                              const clang::TargetOptions &TOpts,
                              const LangOptions &LOpts,
                              StringRef TDesc, Module *M,
                              BackendAction Action,
                              std::unique_ptr<raw_pwrite_stream> OS) {

  llvm::TimeTraceScope TimeScope("Backend");

  std::unique_ptr<llvm::Module> EmptyModule;
  if (!CGOpts.ThinLTOIndexFile.empty()) {
    // If we are performing a ThinLTO importing compile, load the function index
    // into memory and pass it into runThinLTOBackend, which will run the
    // function importer and invoke LTO passes.
    std::unique_ptr<ModuleSummaryIndex> CombinedIndex;
    if (Error E = llvm::getModuleSummaryIndexForFile(
                      CGOpts.ThinLTOIndexFile,
                      /*IgnoreEmptyThinLTOIndexFile*/ true)
                      .moveInto(CombinedIndex)) {
      logAllUnhandledErrors(std::move(E), errs(),
                            "Error loading index file '" +
                            CGOpts.ThinLTOIndexFile + "': ");
      return;
    }

    // A null CombinedIndex means we should skip ThinLTO compilation
    // (LLVM will optionally ignore empty index files, returning null instead
    // of an error).
    if (CombinedIndex) {
      if (!CombinedIndex->skipModuleByDistributedBackend()) {
        runThinLTOBackend(Diags, CombinedIndex.get(), M, HeaderOpts, CGOpts,
                          TOpts, LOpts, std::move(OS), CGOpts.SampleProfileFile,
                          CGOpts.ProfileRemappingFile, Action);
        return;
      }
      // Distributed indexing detected that nothing from the module is needed
      // for the final linking. So we can skip the compilation. We sill need to
      // output an empty object file to make sure that a linker does not fail
      // trying to read it. Also for some features, like CFI, we must skip
      // the compilation as CombinedIndex does not contain all required
      // information.
      EmptyModule = std::make_unique<llvm::Module>("empty", M->getContext());
      EmptyModule->setTargetTriple(M->getTargetTriple());
      M = EmptyModule.get();
    }
  }

  EmitAssemblyHelper AsmHelper(Diags, HeaderOpts, CGOpts, TOpts, LOpts, M);
  AsmHelper.EmitAssembly(Action, std::move(OS));

  // Verify clang's TargetInfo DataLayout against the LLVM TargetMachine's
  // DataLayout.
  if (AsmHelper.TM) {
    std::string DLDesc = M->getDataLayout().getStringRepresentation();
    if (DLDesc != TDesc) {
      unsigned DiagID = Diags.getCustomDiagID(
          DiagnosticsEngine::Error, "backend data layout '%0' does not match "
                                    "expected target description '%1'");
      Diags.Report(DiagID) << DLDesc << TDesc;
    }
  }
}

// With -fembed-bitcode, save a copy of the llvm IR as data in the
// __LLVM,__bitcode section.
void clang::EmbedBitcode(llvm::Module *M, const CodeGenOptions &CGOpts,
                         llvm::MemoryBufferRef Buf) {
  if (CGOpts.getEmbedBitcode() == CodeGenOptions::Embed_Off)
    return;
  llvm::embedBitcodeInModule(
      *M, Buf, CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Marker,
      CGOpts.getEmbedBitcode() != CodeGenOptions::Embed_Bitcode,
      CGOpts.CmdArgs);
}

void clang::EmbedObject(llvm::Module *M, const CodeGenOptions &CGOpts,
                        DiagnosticsEngine &Diags) {
  if (CGOpts.OffloadObjects.empty())
    return;

  for (StringRef OffloadObject : CGOpts.OffloadObjects) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> ObjectOrErr =
        llvm::MemoryBuffer::getFileOrSTDIN(OffloadObject);
    if (std::error_code EC = ObjectOrErr.getError()) {
      auto DiagID = Diags.getCustomDiagID(DiagnosticsEngine::Error,
                                          "could not open '%0' for embedding");
      Diags.Report(DiagID) << OffloadObject;
      return;
    }

    llvm::embedBufferInModule(*M, **ObjectOrErr, ".llvm.offloading",
                              Align(object::OffloadBinary::getAlignment()));
  }
}
