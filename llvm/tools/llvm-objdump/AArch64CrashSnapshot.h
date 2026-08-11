#ifdef OHOS_LLVM
//===-- AArch64CrashSnapshot.h - Crash Snapshot & Thread Context -*- C++
//-*-===//
//
// This file defines data structures for managing crash snapshots extracted from
// HWASan logs, including thread contexts, register state, stack frames, and
// memory simulation utilities.
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/Error.h"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace crash_analyzer {

// ARM64 PAC signature stripping
static constexpr uint64_t PACMask = 0x0000007fffffffffULL;

inline uint64_t StripPAC(uint64_t Addr) { return Addr & PACMask; }

// ---- JSON configuration structures (all log format parameters) -------------
struct CrashLogConfig {
  struct Pattern {
    std::string Prefix;
    size_t Length;
  };
  struct FaultLineConfig {
    std::vector<Pattern> AddressPatterns;
    std::string TagsSuffix;
    std::string HexPrefix;
    std::vector<Pattern> ThreadPatterns;
  };
  struct StackFrameConfig {
    std::string LinePrefix;
    std::string BuildIDOpen;
    std::string BuildIDClose;
    std::string PCKeyword;
    std::string HexPrefix;
    struct ModuleStrip {
      std::string Open;
      std::string Close;
      std::string OffsetSeparator;
    } ModuleBracketStrip;
    std::string ModuleColonStrip;
  };
  struct RegistersSectionConfig {
    std::vector<std::string> SkipPrefixes;
    std::vector<std::string> ValueSeparators;
  };
  struct MemoryMapConfig {
    std::string EndMarker;
    std::string AddressSeparator;
    std::string HexPrefix;
    std::string PathIndicator;
  };
  struct StateMachineConfig {
    std::vector<std::string> IgnoreStackTriggers;
    std::vector<std::string> RegistersTriggers;
    std::vector<std::string> MemoryNearRegsTriggers;
    std::string MemoryMapTrigger;
  };
  struct LimitsConfig {
    unsigned MaxFramesToKeep;
  };
  struct ModuleMatchingConfig {
    std::vector<std::string> Strategies;
  };

  std::string LogValidationRequiredString;
  FaultLineConfig FaultLine;
  StackFrameConfig StackFrame;
  RegistersSectionConfig RegistersSection;
  MemoryMapConfig MemoryMap;
  StateMachineConfig StateMachine;
  LimitsConfig Limits;
  ModuleMatchingConfig ModuleMatching;

  /// Parses configuration from a JSON string. Returns an error if parsing
  /// fails.
  static llvm::Expected<CrashLogConfig> fromJSON(llvm::StringRef JsonText);
};

// Map the register string to the AArch64 DWARF number.
// In the AArch64 DWARF ABI, x0 to x31 correspond to 0 to 31 (x29 = fp, x30 =
// lr, x31 = sp).
inline std::optional<uint16_t>
MapRegNameToDwarfNumber(llvm::StringRef RegName) {
  if (RegName == "sp")
    return 31;
  if (RegName == "lr")
    return 30;
  if (RegName == "fp")
    return 29;
  if (RegName == "pc")
    return 32;

  if (RegName.consume_front("x")) {
    if (RegName == "zr")
      return std::nullopt;

    unsigned RegNum;
    if (!RegName.getAsInteger(10, RegNum))
      return RegNum;
  }
  return std::nullopt;
}

class StackFrame {
private:
  uint32_t FrameIndex;    // frame number
  uint64_t RuntimePC;     // running PC
  uint64_t StaticPC;      // static PC
  uint64_t LoadBias;      // Bias PC
  std::string ModuleName; // module
  std::string BuildID;    // buildID

public:
  StackFrame(uint32_t Index, uint64_t RtPc)
      : FrameIndex(Index), RuntimePC(RtPc), StaticPC(0), LoadBias(0) {}

  uint32_t getFrameIndex() const { return FrameIndex; }
  uint64_t getRuntimePC() const { return RuntimePC; }

  void setStaticPC(uint64_t StaticPc) { StaticPC = StaticPc; }
  uint64_t getStaticPC() const { return StaticPC; }

  void setLoadBias(uint64_t Bias) { LoadBias = Bias; }
  uint64_t getLoadBias() const { return LoadBias; }

  void setModuleName(const std::string &Name) { ModuleName = Name; }
  const std::string &getModuleName() const { return ModuleName; }

  void setBuildID(const std::string &ID) { BuildID = ID; }
  const std::string &getBuildID() const { return BuildID; }
};

// Thread context abstraction
class ThreadContext {
private:
  uint32_t Tid;
  bool IsCrashedThread;

  // Received parsed address information.
  uint64_t FaultAddress = 0;

  std::unordered_map<uint16_t, uint64_t> DwarfRegs;

  std::vector<StackFrame> Frames;

  std::map<uint64_t, uint64_t> MemoryDump;

public:
  ThreadContext(uint32_t Tid, bool IsCrashed)
      : Tid(Tid), IsCrashedThread(IsCrashed) {}

  void setFaultAddress(uint64_t Addr) { FaultAddress = Addr; }

  uint64_t getFaultAddress() const { return FaultAddress; }

  void addMemoryDumpEntry(uint64_t Addr, uint64_t Val) {
    MemoryDump[Addr] = Val;
  }

  const std::map<uint64_t, uint64_t> &getMemoryDump() const {
    return MemoryDump;
  }

  std::optional<std::string> getMemoryValue(uint64_t Addr,
                                             uint64_t Size) const {
    if (Size == 0)
      return std::nullopt;

    std::string HexStr;
    std::string AsciiStr;
    bool allPrintable = true;
    uint64_t BytesRead = 0;
    uint64_t CurrentAddr = Addr;

    while (BytesRead < Size) {
      auto It = MemoryDump.upper_bound(CurrentAddr);
      if (It == MemoryDump.begin())
        return std::nullopt;
      --It;

      uint64_t BlockStart = It->first;
      uint64_t BlockVal = It->second;

      if (CurrentAddr < BlockStart || CurrentAddr >= BlockStart + 8)
        return std::nullopt;

      uint64_t OffsetInBlock = CurrentAddr - BlockStart;
      uint64_t BytesFromBlock = std::min(Size - BytesRead, 8 - OffsetInBlock);

      for (uint64_t i = 0; i < BytesFromBlock; ++i) {
        uint8_t ByteVal = (BlockVal >> ((OffsetInBlock + i) * 8)) & 0xFF;
        char Buf[3];
        snprintf(Buf, sizeof(Buf), "%02x", ByteVal);
        HexStr += Buf;
        if (ByteVal >= 32 && ByteVal <= 126)
          AsciiStr += static_cast<char>(ByteVal);
        else
          allPrintable = false;
      }

      BytesRead += BytesFromBlock;
      CurrentAddr += BytesFromBlock;
    }

    std::string Result = "0x" + HexStr;
    if (!AsciiStr.empty() && allPrintable && AsciiStr.size() == Size)
      Result += " (\"" + AsciiStr + "\")";
    else if (!AsciiStr.empty())
      Result += " (partial ASCII: \"" + AsciiStr + "\")";

    return Result;
  }

  void setRegister(uint16_t DwarfRegNum, uint64_t Value) {
    DwarfRegs[DwarfRegNum] = Value;
  }

  std::optional<uint64_t> getRegister(uint16_t DwarfRegNum) const {
    auto It = DwarfRegs.find(DwarfRegNum);
    if (It != DwarfRegs.end())
      return It->second;
    return std::nullopt;
  }

  std::optional<uint64_t> getCleanPC() const {
    auto Val = getRegister(32);
    return Val ? std::optional<uint64_t>(StripPAC(*Val)) : std::nullopt;
  }

  std::optional<uint64_t> getCleanSP() const {
    auto Val = getRegister(31);
    return Val ? std::optional<uint64_t>(StripPAC(*Val)) : std::nullopt;
  }

  uint32_t getTid() const { return Tid; }
  bool isCrashedThread() const { return IsCrashedThread; }

  void addFrame(StackFrame &Frame) { Frames.push_back(Frame); }

  std::vector<StackFrame> &getFrames() { return Frames; }

  StackFrame *getCrashFrame() {
    if (!Frames.empty())
      return &Frames[0];
    return nullptr;
  }
};

struct MemoryMapEntry {
  uint64_t Start;
  uint64_t End;
  std::string Path;
};

enum ParseState {
  INIT,
  FAULT_THREAD,
  IGNORE_STACK,
  REGISTERS,
  OTHER_THREADS,
  MEMORY_NEAR_REGS,
  MEMORY_MAP
};

// Global snapshot manager.
class CrashSnapshotManager {
private:
  CrashLogConfig Cfg;

  static constexpr uint64_t INVALID_BASE = static_cast<uint64_t>(-1);
  std::unordered_map<uint32_t, std::shared_ptr<ThreadContext>>
      ThreadsMap;      // ThreadsMap;
  uint32_t CrashedTid; // m_crashed_tid;

  bool parseFaultLine(llvm::StringRef Line, uint64_t &FaultAddress,
                      uint32_t &Tid);
  bool parseStackFrameLine(llvm::StringRef Line, uint32_t &FrameIdx,
                           uint64_t &PC, std::string &ModuleName,
                           std::string &BuildID);
  void addStackFrameToThread(ThreadContext &Thread, uint32_t FrameIdx,
                             uint64_t PC, const std::string &ModuleName,
                             const std::string &BuildID);
  void parseRegistersLine(llvm::StringRef Line, ThreadContext &Thread);
  bool parseMemoryMapLine(llvm::StringRef Line, MemoryMapEntry &Entry);
  bool fixupAddresses(const std::vector<MemoryMapEntry> &MemoryMaps);

public:
  CrashSnapshotManager() : CrashedTid(0) {}

  // Entry for parsing crash logs.
  bool loadFromLogAndDump(const std::string &LogFilePath);

  std::shared_ptr<ThreadContext> getThreadContext(uint32_t tid);
  std::shared_ptr<ThreadContext> getCrashedThreadContext();
};

} // namespace crash_analyzer
#endif // OHOS_LLVM
