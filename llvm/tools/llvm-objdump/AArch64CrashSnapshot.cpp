#ifdef OHOS_LLVM
//===-- AArch64CrashSnapshot.cpp - hwasan Log Parser & Context Builder ----===//
//
// This file implements the parsing logic for hwasan crash logs. It extracts
// fault information, register snapshots, stack frames, and memory mapping for
// binary, then builds ThreadContext objects and computes static PC addresses
// for subsequent DWARF-based analysis.
//
//===----------------------------------------------------------------------===//
#include "AArch64CrashSnapshot.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/LineIterator.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace crash_analyzer {

// JSON Configuration Parsing

llvm::Expected<CrashLogConfig>
CrashLogConfig::fromJSON(llvm::StringRef JsonText) {
  auto Val = llvm::json::parse(JsonText);
  if (!Val)
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid JSON config");
  auto *Root = Val->getAsObject();
  if (!Root)
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Config root is not an object");

  auto GetStr = [](llvm::json::Object *Obj,
                   llvm::StringRef Key) -> llvm::Expected<std::string> {
    auto Str = Obj->getString(Key);
    if (!Str)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing string field: " + Key);
    return Str->str();
  };
  auto GetObj = [](llvm::json::Object *Root,
                   llvm::StringRef Key) -> llvm::json::Object * {
    return Root->getObject(Key);
  };

  CrashLogConfig Cfg;

  // log_validation
  if (auto *LV = GetObj(Root, "log_validation"))
    Cfg.LogValidationRequiredString = cantFail(GetStr(LV, "required_string"));
  else
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing log_validation");

  // fault_line
  if (auto *FL = GetObj(Root, "fault_line")) {
    auto *APs = FL->getArray("address_patterns");
    if (!APs)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing fault_line.address_patterns");
    for (auto &El : *APs) {
      auto *Obj = El.getAsObject();
      if (!Obj)
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid address_pattern entry");
      auto Prefix = cantFail(GetStr(Obj, "prefix"));
      auto LengthOpt = Obj->getInteger("length");
      if (!LengthOpt)
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Missing length in address_pattern");
      Cfg.FaultLine.AddressPatterns.push_back(
          {Prefix, static_cast<size_t>(*LengthOpt)});
    }
    Cfg.FaultLine.TagsSuffix = cantFail(GetStr(FL, "tags_suffix"));
    Cfg.FaultLine.HexPrefix = cantFail(GetStr(FL, "hex_prefix"));
    auto *TPs = FL->getArray("thread_patterns");
    if (!TPs)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing fault_line.thread_patterns");
    for (auto &El : *TPs) {
      auto *Obj = El.getAsObject();
      if (!Obj)
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid thread_pattern entry");
      auto Prefix = cantFail(GetStr(Obj, "prefix"));
      auto LengthOpt = Obj->getInteger("length");
      if (!LengthOpt)
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Missing length in thread_pattern");
      Cfg.FaultLine.ThreadPatterns.push_back(
          {Prefix, static_cast<size_t>(*LengthOpt)});
    }
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing fault_line");
  }

  // stack_frame
  if (auto *SF = GetObj(Root, "stack_frame")) {
    Cfg.StackFrame.LinePrefix = cantFail(GetStr(SF, "line_prefix"));
    Cfg.StackFrame.BuildIDOpen = cantFail(GetStr(SF, "build_id_open"));
    Cfg.StackFrame.BuildIDClose = cantFail(GetStr(SF, "build_id_close"));
    Cfg.StackFrame.PCKeyword = cantFail(GetStr(SF, "pc_keyword"));
    Cfg.StackFrame.HexPrefix = cantFail(GetStr(SF, "hex_prefix"));
    if (auto *MS = GetObj(SF, "module_bracket_strip")) {
      Cfg.StackFrame.ModuleBracketStrip.Open = cantFail(GetStr(MS, "open"));
      Cfg.StackFrame.ModuleBracketStrip.Close = cantFail(GetStr(MS, "close"));
      Cfg.StackFrame.ModuleBracketStrip.OffsetSeparator =
          cantFail(GetStr(MS, "offset_separator"));
    } else {
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing module_bracket_strip");
    }
    Cfg.StackFrame.ModuleColonStrip =
        cantFail(GetStr(SF, "module_colon_strip"));
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing stack_frame");
  }

  // registers_section
  if (auto *RS = GetObj(Root, "registers_section")) {
    auto *SP = RS->getArray("skip_prefixes");
    if (!SP)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing registers_section.skip_prefixes");
    for (auto &El : *SP) {
      if (auto S = El.getAsString())
        Cfg.RegistersSection.SkipPrefixes.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid skip_prefix");
    }
    auto *VS = RS->getArray("value_separators");
    if (!VS)
      return llvm::createStringError(
          llvm::inconvertibleErrorCode(),
          "Missing registers_section.value_separators");
    for (auto &El : *VS) {
      if (auto S = El.getAsString())
        Cfg.RegistersSection.ValueSeparators.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid value_separator");
    }
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing registers_section");
  }

  // memory_map
  if (auto *MM = GetObj(Root, "memory_map")) {
    Cfg.MemoryMap.EndMarker = cantFail(GetStr(MM, "end_marker"));
    Cfg.MemoryMap.AddressSeparator = cantFail(GetStr(MM, "address_separator"));
    Cfg.MemoryMap.HexPrefix = cantFail(GetStr(MM, "hex_prefix"));
    Cfg.MemoryMap.PathIndicator = cantFail(GetStr(MM, "path_indicator"));
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing memory_map");
  }

  // state_machine
  if (auto *SM = GetObj(Root, "state_machine")) {
    auto *IST = SM->getArray("ignore_stack_triggers");
    if (!IST)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing ignore_stack_triggers");
    for (auto &El : *IST) {
      if (auto S = El.getAsString())
        Cfg.StateMachine.IgnoreStackTriggers.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid ignore_stack_trigger");
    }
    auto *RT = SM->getArray("registers_triggers");
    if (!RT)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing registers_triggers");
    for (auto &El : *RT) {
      if (auto S = El.getAsString())
        Cfg.StateMachine.RegistersTriggers.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid registers_trigger");
    }
    auto *MNRT = SM->getArray("memory_near_regs_triggers");
    if (!MNRT)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing memory_near_regs_triggers");
    for (auto &El : *MNRT) {
      if (auto S = El.getAsString())
        Cfg.StateMachine.MemoryNearRegsTriggers.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid memory_near_regs_trigger");
    }

    Cfg.StateMachine.MemoryMapTrigger =
        cantFail(GetStr(SM, "memory_map_trigger"));
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing state_machine");
  }

  // limits
  if (auto *Lim = GetObj(Root, "limits")) {
    auto MaxFrames = Lim->getInteger("max_frames_to_keep");
    if (!MaxFrames)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing max_frames_to_keep");
    Cfg.Limits.MaxFramesToKeep = static_cast<unsigned>(*MaxFrames);
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing limits");
  }

  // module_matching
  if (auto *MM = GetObj(Root, "module_matching")) {
    auto *Strats = MM->getArray("strategies");
    if (!Strats)
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                     "Missing strategies");
    for (auto &El : *Strats) {
      if (auto S = El.getAsString())
        Cfg.ModuleMatching.Strategies.push_back(S->str());
      else
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                       "Invalid strategy");
    }
  } else {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Missing module_matching");
  }

  return Cfg;
}

// Locate configuration file
static std::string findConfigFile() {
  // Infer the executable file path of the current process
  std::string ExePathStr = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
  if (ExePathStr.empty())
    return "";

  llvm::SmallString<256> ConfigPath(ExePathStr);

  // Prefer $prefix/share/clang/crash_log_config.json (installed layout).
  // Strip binary name then bin/ to reach the install prefix.
  llvm::sys::path::remove_filename(ConfigPath);
  llvm::sys::path::remove_filename(ConfigPath);
  llvm::sys::path::append(ConfigPath, "share", "clang", "crash_log_config.json");
  if (llvm::sys::fs::exists(ConfigPath))
    return ConfigPath.str().str();

  // Fallback: next to the executable (POST_BUILD / old bin layout).
  ConfigPath = ExePathStr;
  llvm::sys::path::remove_filename(ConfigPath);
  llvm::sys::path::append(ConfigPath, "crash_log_config.json");
  if (llvm::sys::fs::exists(ConfigPath))
    return ConfigPath.str().str();

  // can not find json file, return empty
  return "";
}

// Parses a fault line from sanitizer output (e.g., ASan, TSan) and extracts
// the fault address and thread ID.
// (see header for example)
bool CrashSnapshotManager::parseFaultLine(llvm::StringRef Line,
                                          uint64_t &FaultAddress,
                                          uint32_t &Tid) {
  const auto &FC = Cfg.FaultLine;

  // Try configured address patterns
  size_t AddrPos = llvm::StringRef::npos;
  size_t PrefixLen = 0;
  for (const auto &Pat : FC.AddressPatterns) {
    size_t P = Line.find(Pat.Prefix);
    if (P != llvm::StringRef::npos) {
      AddrPos = P;
      PrefixLen = Pat.Length;
      break;
    }
  }
  if (AddrPos == llvm::StringRef::npos)
    return false;

  llvm::StringRef AddrStr = Line.substr(AddrPos + PrefixLen).trim();

  // Remove trailing tags / extra tokens
  size_t TagsPos = AddrStr.find(FC.TagsSuffix);
  if (TagsPos != llvm::StringRef::npos) {
    AddrStr = AddrStr.substr(0, TagsPos).trim();
  } else {
    AddrStr = AddrStr.split(' ').first;
  }
  AddrStr.consume_front(FC.HexPrefix);
  if (AddrStr.getAsInteger(16, FaultAddress))
    return false;

  // Try configured thread patterns
  size_t ThreadPos = llvm::StringRef::npos;
  size_t TPrefixLen = 0;
  for (const auto &Pat : FC.ThreadPatterns) {
    ThreadPos = Line.find(Pat.Prefix);
    if (ThreadPos != llvm::StringRef::npos) {
      TPrefixLen = Pat.Length;
      break;
    }
  }
  if (ThreadPos == llvm::StringRef::npos)
    return false;

  llvm::StringRef TidStr =
      Line.substr(ThreadPos + TPrefixLen).trim().split(' ').first;
  // Remove any trailing non-numeric characters (e.g., punctuation)
  size_t NonDigit = TidStr.find_first_not_of("0123456789");
  if (NonDigit != llvm::StringRef::npos)
    TidStr = TidStr.substr(0, NonDigit);
  if (TidStr.empty() || TidStr.getAsInteger(10, Tid))
    return false;

  return true;
}

// Parses a stack frame line like "#0 pc 0x102a4b3d0  libfoo.so".
bool CrashSnapshotManager::parseStackFrameLine(llvm::StringRef Line,
                                               uint32_t &FrameIdx, uint64_t &PC,
                                               std::string &ModuleName,
                                               std::string &BuildID) {
  const auto &SF = Cfg.StackFrame;
  if (!Line.starts_with(SF.LinePrefix))
    return false;

  BuildID = "";
  size_t BuildIdPos = Line.find(SF.BuildIDOpen);
  if (BuildIdPos != llvm::StringRef::npos) {
    size_t BuildIdEnd = Line.find(SF.BuildIDClose, BuildIdPos);
    if (BuildIdEnd != llvm::StringRef::npos) {
      BuildID = Line.substr(BuildIdPos + SF.BuildIDOpen.size(),
                            BuildIdEnd - (BuildIdPos + SF.BuildIDOpen.size()))
                    .str();
      Line = Line.substr(0, BuildIdPos).trim();
    }
  }

  llvm::SmallVector<llvm::StringRef, 8> Tokens;
  Line.split(Tokens, ' ', -1, false);
  if (Tokens.size() < 2)
    return false;

  llvm::StringRef FrameToken = Tokens[0];
  llvm::StringRef PCToken = Tokens[1];
  if (PCToken == SF.PCKeyword && Tokens.size() >= 3) {
    PCToken = Tokens[2];
  }

  if (!FrameToken.consume_front(SF.LinePrefix))
    return false;
  if (FrameToken.getAsInteger(10, FrameIdx))
    return false;

  PCToken.consume_front(SF.HexPrefix);
  if (PCToken.getAsInteger(16, PC))
    return false;

  llvm::StringRef LastToken = Tokens.back();
  const auto &Strip = SF.ModuleBracketStrip;
  if (LastToken.starts_with(Strip.Open) && LastToken.ends_with(Strip.Close)) {
    // Remove wrapping brackets and optional offset
    LastToken =
        LastToken.drop_front(Strip.Open.size()).drop_back(Strip.Close.size());
    size_t PlusPos = LastToken.find(Strip.OffsetSeparator);
    if (PlusPos != llvm::StringRef::npos) {
      // remove '+0x...'
      LastToken = LastToken.substr(0, PlusPos);
    }
  }

  size_t ColonPos = LastToken.find(SF.ModuleColonStrip);
  if (ColonPos != llvm::StringRef::npos)
    LastToken = LastToken.substr(0, ColonPos);
  ModuleName = LastToken.str();

  return true;
}

// Adds a parsed stack frame to a ThreadContext object.
// For frame 0, also sets the "pc" register value.
void CrashSnapshotManager::addStackFrameToThread(ThreadContext &Thread,
                                                 uint32_t FrameIdx, uint64_t PC,
                                                 const std::string &ModuleName,
                                                 const std::string &BuildID) {
  StackFrame SF(FrameIdx, PC);
  SF.setBuildID(BuildID);
  SF.setModuleName(ModuleName);
  Thread.addFrame(SF);

  if (FrameIdx == 0) {
    auto DwarfOpt = MapRegNameToDwarfNumber("pc");
    if (DwarfOpt)
      Thread.setRegister(*DwarfOpt, PC);
  }
}

// Parses a line containing register values, e.g. "pc: 0x1000  sp: 0x2000".
void CrashSnapshotManager::parseRegistersLine(llvm::StringRef Line,
                                              ThreadContext &Thread) {
  const auto &RS = Cfg.RegistersSection;
  // Skip lines that start with any of the configured prefixes
  for (const auto &Prefix : RS.SkipPrefixes) {
    if (Line.starts_with(Prefix))
      return;
  }

  // Use the first value separator to split tokens
  llvm::SmallVector<llvm::StringRef, 8> Tokens;
  Line.split(Tokens, RS.ValueSeparators.front(), -1, false);

  for (size_t I = 0; I < Tokens.size();) {
    llvm::StringRef Token = Tokens[I];
    llvm::StringRef RegName, RegValStr;

    size_t ColonPos = Token.find(':');
    if (ColonPos != llvm::StringRef::npos) {
      RegName = Token.substr(0, ColonPos);
      RegValStr = Token.substr(ColonPos + 1);
      if (RegValStr.empty()) {
        ++I;
        if (I < Tokens.size())
          RegValStr = Tokens[I];
        else
          break;
      }
      ++I;
    } else {
      RegName = Token;
      ++I;
      if (I < Tokens.size()) {
        RegValStr = Tokens[I];
        ++I;
      } else {
        break;
      }
    }

    auto DwarfOpt = MapRegNameToDwarfNumber(RegName.str());
    if (DwarfOpt) {
      uint64_t RegVal;
      if (!RegValStr.getAsInteger(16, RegVal))
        Thread.setRegister(*DwarfOpt, RegVal);
    }
  }
}

// Parses a memory map line like "0x100000000-0x100004000  /path/to/binary".
bool CrashSnapshotManager::parseMemoryMapLine(llvm::StringRef Line,
                                              MemoryMapEntry &Entry) {
  const auto &MM = Cfg.MemoryMap;
  // Skip lines that indicate end of memory map section
  if (Line.contains(MM.EndMarker))
    return false;

  std::string MapLine = Line.str();

  // Replace only the dash between start and end addresses, not dashes in the
  // path. The first '-' in a /proc/self/maps line is always the address
  // separator.
  size_t dashPos = MapLine.find(MM.AddressSeparator);
  if (dashPos != std::string::npos) {
    // Make sure we are not breaking a path that might start with '-'
    // The address part is always at the
    // beginning, so the first '-' divides the two hex address strings.
    MapLine[dashPos] = ' ';
  }

  llvm::StringRef MapRef(MapLine);
  llvm::SmallVector<llvm::StringRef, 8> Tokens;
  MapRef.split(Tokens, ' ', -1, false);
  if (Tokens.size() < 2)
    return false;

  uint64_t Start, End;
  llvm::StringRef StartStr = Tokens[0];
  llvm::StringRef EndStr = Tokens[1];
  StartStr.consume_front(MM.HexPrefix);
  EndStr.consume_front(MM.HexPrefix);
  if (StartStr.getAsInteger(16, Start) || EndStr.getAsInteger(16, End))
    return false;

  // Extract path: scan tokens from index 2 onward until we find one containing
  // path indicator.
  std::string Path;
  for (size_t I = 2; I < Tokens.size(); ++I) {
    if (Tokens[I].contains(MM.PathIndicator)) {
      Path = Tokens[I].str();
      break;
    }
  }

  // Allow empty path for anonymous memory regions
  Entry.Start = Start;
  Entry.End = End;
  Entry.Path = Path;
  return true;
}

// Processes stack frames for the crashed thread, validating each frame's
// module path against the provided memory maps, and computing the load bias
// and static (unslid) PC for every valid frame.
//
// Frames whose expected module name does not match the actual memory map path
// are removed. The module's base address is determined as the lowest mapping
// start address for that path.
//
// return true if the crashed thread was found and frames were processed,
//        false if CrashedTid is 0, MemoryMaps is empty, or the crashed
//        thread is not present in ThreadsMap.
bool CrashSnapshotManager::fixupAddresses(
    const std::vector<MemoryMapEntry> &MemoryMaps) {
  if (CrashedTid == 0 || MemoryMaps.empty()) {
    llvm::errs()
        << "Warning: Invalid input (CrashedTid is 0 or MemoryMaps is empty)\n";
    return false;
  }

  auto It = ThreadsMap.find(CrashedTid);
  if (It == ThreadsMap.end())
    return false;

  auto CrashedThread = It->second;

  // Module path matching based on active strategies
  auto isModuleMatch = [this](llvm::StringRef MapPath,
                              llvm::StringRef LocalPath) -> bool {
    for (const auto &Strategy : Cfg.ModuleMatching.Strategies) {
      if (Strategy == "exact" && MapPath == LocalPath)
        return true;
      if (Strategy == "parent_dir") {
        llvm::StringRef MapDir = llvm::sys::path::parent_path(MapPath);
        llvm::StringRef LocalDir = llvm::sys::path::parent_path(LocalPath);
        if (!MapDir.empty() && !LocalDir.empty() && MapDir == LocalDir)
          return true;
      }
    }
    return false;
  };

  std::unordered_map<std::string, uint64_t> ModuleBaseCache;

  // Obtain a reference to the frame container.
  auto &Frames = CrashedThread->getFrames();

  // Use an iterator loop to safely erase frames during traversal.
  for (auto it = Frames.begin(); it != Frames.end();) {
    auto &Frame = *it;
    uint64_t RuntimePC = Frame.getRuntimePC();
    uint64_t CleanPC = StripPAC(RuntimePC);

    std::string ExpectedPath = Frame.getModuleName();
    std::string CrashPath = "";

    // Look up the memory map that contains the cleaned PC.
    for (const auto &Map : MemoryMaps) {
      if (CleanPC >= Map.Start && CleanPC < Map.End) {
        CrashPath = Map.Path;
        break;
      }
    }

    if (!CrashPath.empty()) {
      // Drop the frame if the expected module path does not match the actual
      // memory map path.
      if (!ExpectedPath.empty() && !isModuleMatch(CrashPath, ExpectedPath)) {
        llvm::errs() << "[WARNING] Frame #" << Frame.getFrameIndex()
                     << " path mismatch. Frame expects: " << ExpectedPath
                     << ", but MemoryMap is: " << CrashPath
                     << ". Deleting frame!\n";
        it = Frames.erase(it);
        continue;
      }

      // Path matches – update the module name and compute load bias / static
      // PC.
      Frame.setModuleName(CrashPath);

      uint64_t BasePC = INVALID_BASE;
      if (ModuleBaseCache.count(CrashPath)) {
        BasePC = ModuleBaseCache[CrashPath];
      } else {
        // Find the base address (lowest mapping start) for this module.
        for (const auto &Map : MemoryMaps) {
          if (Map.Path == CrashPath && Map.Start < BasePC) {
            BasePC = Map.Start;
          }
        }
        ModuleBaseCache[CrashPath] = BasePC;
      }

      if (BasePC != INVALID_BASE) {
        Frame.setLoadBias(BasePC);
        Frame.setStaticPC(CleanPC - BasePC);
      } else {
        llvm::errs() << "[WARNING] Failed to determine load bias for module "
                     << CrashPath << "\n";
      }

    } else {
      llvm::errs() << "[WARNING] No memory map entry for frame #"
                   << Frame.getFrameIndex() << " PC 0x"
                   << llvm::Twine::utohexstr(CleanPC) << "\n";
    }

    // Advance to the next frame only if the current one was not erased.
    ++it;
  }

  return true;
}

// ---- Main entry point ---------------------------------------------------

// Parses the HWASAN crash log file and extracts the context snapshot for
// reverse backtracing.  … (original doc remains)
bool CrashSnapshotManager::loadFromLogAndDump(const std::string &LogFilePath) {
  // Ensure configuration is loaded (once)
  if (Cfg.LogValidationRequiredString.empty()) {
    std::string ConfigPath = findConfigFile();
    auto Buf = llvm::MemoryBuffer::getFile(ConfigPath);
    if (!Buf) {
      llvm::errs() << "Failed to open config: " << ConfigPath << "\n";
      return false;
    }
    auto CfgOr = CrashLogConfig::fromJSON(Buf.get()->getBuffer());
    if (!CfgOr) {
      llvm::errs() << "Config error: " << llvm::toString(CfgOr.takeError())
                   << "\n";
      return false;
    }
    Cfg = std::move(*CfgOr);
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> BufferOrErr =
      llvm::MemoryBuffer::getFile(LogFilePath);
  if (!BufferOrErr) {
    llvm::errs() << "Failed to open log file: " << LogFilePath << "\n";
    return false;
  }

  llvm::StringRef BufferContent = (*BufferOrErr)->getBuffer();
  // Use configured validation string
  if (!BufferContent.contains(Cfg.LogValidationRequiredString)) {
    llvm::errs() << "Warning: Not a valid HWASAN log (missing '"
                 << Cfg.LogValidationRequiredString << "')\n";
    return false;
  }

  ParseState State = INIT;
  std::shared_ptr<ThreadContext> CurrentThread = nullptr;
  std::vector<MemoryMapEntry> MemoryMaps;

  for (llvm::line_iterator LineIt(*BufferOrErr.get(), true);
       !LineIt.is_at_eof(); ++LineIt) {

    llvm::StringRef RawLine = LineIt->trim();
    if (RawLine.empty())
      continue;

    std::string LineStr = RawLine.str();
    std::replace(LineStr.begin(), LineStr.end(), '\t', ' ');
    llvm::StringRef Line(LineStr);

    // State transitions driven by configuration
    bool toIgnore = false;
    for (const auto &Trigger : Cfg.StateMachine.IgnoreStackTriggers) {
      if (Line.contains(Trigger)) {
        State = IGNORE_STACK;
        toIgnore = true;
        break;
      }
    }
    if (toIgnore)
      continue;

    for (const auto &Trigger : Cfg.StateMachine.RegistersTriggers) {
      if (Line.starts_with(Trigger)) {
        State = REGISTERS;
        toIgnore = true;
        break;
      }
    }

    if (toIgnore)
      continue;

    for (const auto &Trigger : Cfg.StateMachine.MemoryNearRegsTriggers) {
      if (Line.contains(Trigger)) {
        State = MEMORY_NEAR_REGS;
        toIgnore = true;
        break;
      }
    }

    if (toIgnore)
      continue;

    if (Line.contains(Cfg.StateMachine.MemoryMapTrigger)) {
      State = MEMORY_MAP;
      continue;
    }

    // Parse based on current state
    if (State == FAULT_THREAD && Line.starts_with(Cfg.StackFrame.LinePrefix) &&
        CurrentThread) {
      uint32_t FrameIdx;
      uint64_t PC;
      std::string ModuleName;
      std::string BuildID;
      if (parseStackFrameLine(Line, FrameIdx, PC, ModuleName, BuildID)) {
        if (FrameIdx < Cfg.Limits.MaxFramesToKeep) { // use config limit
          addStackFrameToThread(*CurrentThread, FrameIdx, PC, ModuleName,
                                BuildID);
        }
      }
    } else if (State == REGISTERS && CurrentThread) {
      parseRegistersLine(Line, *CurrentThread);
    } else if (State == MEMORY_MAP) {
      if (Line.contains(Cfg.MemoryMap.EndMarker)) {
        State = INIT;
        continue;
      }
      MemoryMapEntry Entry;
      if (parseMemoryMapLine(Line, Entry)) {
        MemoryMaps.push_back(Entry);
      }
    } else if (State == MEMORY_NEAR_REGS && CurrentThread) {
      // end this state
      if (Line.contains("SUMMARY: HWAddressSanitizer:")) {
        State = INIT;
        continue;
      }

      Line.consume_front("==>");
      Line = Line.trim();
      if (!Line.starts_with("0x")) {
        continue;
      }

      llvm::SmallVector<llvm::StringRef, 4> Tokens;
      Line.split(Tokens, ' ', -1, false);
      if (Tokens.size() >= 2) {
        llvm::StringRef AddrStr = Tokens[0];
        llvm::StringRef ValStr = Tokens[1];

        // should consume "0x", only data can be recoreded
        AddrStr.consume_front("0x");
        ValStr.consume_front("0x");

        uint64_t Addr, Val;
        if (!AddrStr.getAsInteger(16, Addr) && !ValStr.getAsInteger(16, Val)) {
          CurrentThread->addMemoryDumpEntry(Addr, Val);
        }
      }
    } else {
      // Try to match a fault line (may appear in any state)
      uint64_t FaultAddress = 0;
      uint32_t Tid = 0;
      if (parseFaultLine(Line, FaultAddress, Tid)) {
        State = FAULT_THREAD;
        CurrentThread = std::make_shared<ThreadContext>(Tid, true);
        CurrentThread->setFaultAddress(FaultAddress);
        ThreadsMap[Tid] = CurrentThread;
        CrashedTid = Tid;
      }
    }
  }

  if (!fixupAddresses(MemoryMaps)) {
    llvm::errs()
        << "Failed to resolve stack frame addresses. Aborting analysis.\n";
    return false;
  }

  if (CrashedTid != 0) {
    auto CrashedThread = ThreadsMap[CrashedTid];
    llvm::outs() << "=== Crash Snapshot Target ===\n";
    llvm::outs() << "  Fault Address: 0x"
                 << llvm::Twine::utohexstr(CrashedThread->getFaultAddress())
                 << "\n";
    llvm::outs() << "  Thread ID: " << CrashedThread->getTid() << "\n";

    llvm::outs() << "=== Registers ===\n";
    // Print x0-x30
    for (uint16_t i = 0; i <= 30; ++i) {
      if (auto Val = CrashedThread->getRegister(i)) {
        llvm::outs() << "  x" << i << ": 0x" << llvm::Twine::utohexstr(*Val)
                     << "\n";
      }
    }
    // Print sp (DWARF 31)
    if (auto Val = CrashedThread->getRegister(31)) {
      llvm::outs() << "  sp: 0x" << llvm::Twine::utohexstr(*Val) << "\n";
    }
    // Print pc (DWARF 32)
    if (auto Val = CrashedThread->getRegister(32)) {
      llvm::outs() << "  pc: 0x" << llvm::Twine::utohexstr(*Val) << "\n";
    }

    llvm::outs() << "=== Stack Frames ===\n";
    for (auto &Frame : CrashedThread->getFrames()) {
      llvm::outs() << "  #" << Frame.getFrameIndex() << " runtime pc: 0x"
                   << llvm::Twine::utohexstr(Frame.getRuntimePC())
                   << "  module: " << Frame.getModuleName() << "  static pc: 0x"
                   << llvm::Twine::utohexstr(Frame.getStaticPC())
                   << "  load bias: 0x"
                   << llvm::Twine::utohexstr(Frame.getLoadBias());
      if (!Frame.getBuildID().empty()) {
        llvm::outs() << "  build id: " << Frame.getBuildID();
      }
      llvm::outs() << "\n";
    }
  }

  return CrashedTid != 0 && ThreadsMap.find(CrashedTid) != ThreadsMap.end();
}

std::shared_ptr<ThreadContext>
CrashSnapshotManager::getThreadContext(uint32_t Tid) {
  auto It = ThreadsMap.find(Tid);
  return It != ThreadsMap.end() ? It->second : nullptr;
}

std::shared_ptr<ThreadContext> CrashSnapshotManager::getCrashedThreadContext() {
  return getThreadContext(CrashedTid);
}

} // namespace crash_analyzer
#endif // OHOS_LLVM
