//===-- XVMAsmPrinter.cpp - XVM LLVM assembly writer ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the XVM assembly language.
//
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE

#include "XVM_def.h"
#include "XVM.h"
#include "XVMInstrInfo.h"
#include "XVMTargetMachine.h"
#include "XVMMCInstLower.h"
#include "XVMErrorMsg.h"
#include "MCTargetDesc/XVMInstPrinter.h"
#include "TargetInfo/XVMTargetInfo.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace llvm;

#define DEBUG_TYPE "asm-printer"
#define CHAT_LEN_IN_HEX 2
#define REF_TYPE_LENGTH 8
#define REF_TYPE_HEX_LENGTH 16
#define NUM_BITS_PER_BYTE 8
#define MAX_PTR_SIZE 8
#define MAX_SIZE_CONSTANT_EXPRESSION 8
#define INIT_SMALL_STR_SIZE 128
#define NUM_MO_CTOR_DTOR 3
#define MAX_FUNC_SIZE 32767

static cl::opt<bool> XVMExportAll("xvm-export-all",
  cl::Hidden, cl::init(false),
  cl::desc("Exports all function"));

namespace {
class XVMAsmPrinter : public AsmPrinter {
public:
  explicit XVMAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "XVM Assembly Printer"; }
  bool doInitialization(Module &M) override;
  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O);
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                             const char *ExtraCode, raw_ostream &O) override;

  void emitInstruction(const MachineInstr *MI) override;
  void emitFunctionHeader() override;
  void emitFunctionParamList(const MachineFunction &MF, raw_ostream &O);
  void emitFunctionReturnVal(const MachineFunction &MF, raw_ostream &O);
  void emitFunctionBodyEnd() override;
  void emitStartOfAsmFile(Module &M) override;
  void emitEndOfAsmFile(Module &M) override;
  void emitBasicBlockStart(const MachineBasicBlock &MBB) override {};
  void emitGlobalVariable(const GlobalVariable *GV) override;
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  void emitDecls(const Module &M);
  void setFunctionCallInfo(MCInst *Inst);
  void setGlobalSymbolInfo(const MachineInstr *MI, MCInst* Inst);
  DenseMap<const MachineInstr *, unsigned> MIIndentMap;

  void GetMIIndent(MachineFunction &MF);

  void InitModuleMapFuncnameIndex(Module *M);
  void InitDataSectionGlobalConstant(Module *M);

  void emitDataSectionInfo(raw_svector_ostream &O);

  inline void emitFunctionParamList(const Function &F, raw_ostream &O);
  inline void emitFunctionReturnVal(const Function &F, raw_ostream &O);

  void InitGlobalConstantImpl(const DataLayout &DL, const Constant *CV,
                              const Constant *BaseCV,
                              uint64_t Offset, XVMSectionInfo* SInfo, Module *M);
  void InitGlobalConstantDataSequential(const DataLayout &DL,
                                        const ConstantDataSequential *CDS,
                                        XVMSectionInfo* SInfo);
  void InitGlobalConstantArray(const DataLayout &DL,
                               const ConstantArray *CA,
                               const Constant *BaseCV, uint64_t Offset,
                               XVMSectionInfo* SInfo, Module *M);
  void InitGlobalConstantStruct(const DataLayout &DL,
                                const ConstantStruct *CS,
                                const Constant *BaseCV, uint64_t Offset,
                                XVMSectionInfo* SInfo, Module *M);
};
} // namespace

typedef struct SecSubSecIndices {
  unsigned char SecNameIndex;
  unsigned int SubSecNameIndex;
} SecSubSecIndices;
static std::map<std::string, int> FunctionNameAndIndex;
static std::map<std::string, int> FunctionDefinitionMap;
static std::map<std::string, SecSubSecIndices> DataSectionNameIndexMap;
static std::map<int, XVMSectionInfo> DataSectionIndexInfoMap;

template <typename T>
inline std::string UnsignedIntTypeToHex(T V, size_t W = sizeof(T)*CHAT_LEN_IN_HEX) {
    std::stringstream SS;
    std::string RS;
    SS << std::setfill('0') << std::setw(W) << std::hex << (V|0);
    for (unsigned Index = 0; Index < SS.str().length(); Index = Index + CHAT_LEN_IN_HEX) {
        RS += "\\x" + SS.str().substr(Index, CHAT_LEN_IN_HEX);
    }
    return RS;
}

static inline std::string GetSymbolName(std::string InputSymName) {
  size_t Pos = InputSymName.find(".L");
  if (Pos == 0) {
    return InputSymName.substr(strlen(".L"));
  }
  return InputSymName;
}

static inline uint64_t ReverseBytes(uint64_t Input, unsigned int Width) {
  uint64_t Result = 0;
  for (unsigned int i = 0; i < Width; i++) {
    Result <<= NUM_BITS_PER_BYTE;
    Result |= (Input & 0xFF);
    Input >>= NUM_BITS_PER_BYTE;
  }
  return Result;
}
#define DATA_SECTION 0
#define DATA_SUB_SECTION 1

#define XVM_SD_SEG_START 4
#define XVM_REF_OFFSET_BITS 44
static uint64_t CreateRefContent(int ToDataSecID, uint64_t ToOffset) {
  uint64_t seg_index = XVM_SD_SEG_START + ToDataSecID;
  uint64_t ReTRefData = 0;
  ReTRefData = (seg_index & 0x00000000000FFFFF) << XVM_REF_OFFSET_BITS;
  ReTRefData = ReTRefData | (ToOffset & 0x00000FFFFFFFFFFF);
  return ReTRefData;
}

static int GetDataIndex(const char *name, const unsigned char SecOrSubSection) {
  std::map<std::string, SecSubSecIndices>::iterator I = DataSectionNameIndexMap.find(name);
  if (I == DataSectionNameIndexMap.end()) {
    return -1;
  } else {
    if (SecOrSubSection == DATA_SECTION)
      return I->second.SecNameIndex;
    else
      return I->second.SubSecNameIndex;
  }
}

int GetFuncIndex(const char *name) {
  std::map<std::string, int>::iterator I = FunctionNameAndIndex.find(name);
  if (I == FunctionNameAndIndex.end()) {
    return -1;
  } else {
    return I->second;
  }
}

static int GetDefFuncIndex(const char *name) {
  std::map<std::string, int>::iterator I = FunctionDefinitionMap.find(name);
  if (I == FunctionDefinitionMap.end())
    return -1;
  return I->second;
}

static inline unsigned int GetPtrLevel(short PtrSecIndex) {
  if (PtrSecIndex == -1)
    return 0;
  unsigned int ret = 0;
  std::map<int, XVMSectionInfo>::iterator I = DataSectionIndexInfoMap.find(PtrSecIndex);
  if (I == DataSectionIndexInfoMap.end()) {
    return 0;
  } else {
    if ((I->second.BufType & XVM_SECTION_DATA_TYPE_POINTER) != XVM_SECTION_DATA_TYPE_UNKNOWN) {
      return GetPtrLevel(I->second.PtrSecIndex) + ret + 1;
    }
  }
  return ret;
}

/* Checks the global variable's name */
static bool IsGlobalVariableType(const char *FuncName, const Module *M, const char *type) {
  GlobalVariable *GV = M->getGlobalVariable(type);
  if (GV) {
    const ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
    if (!InitList) {
      return false;
    }

    for (Value *P : InitList->operands()) {
      ConstantStruct *CS = dyn_cast<ConstantStruct>(P);
      if (!CS) continue;

      const char *Associated = CS->getOperand(1)->getName().data();
      if (strcmp(Associated, FuncName) == 0) {
        return true;
      }
    }
  }
  return false;
}

static bool IsConstructorDestructor(const char *FuncName, const Module *M) {
  return IsGlobalVariableType(FuncName, M, "llvm.global_dtors") ||
         IsGlobalVariableType(FuncName, M, "llvm.global_ctors");
}

unsigned int GetPtrRegisterLevelBasedOnName(const char *name) {
  int DataIndex = GetDataIndex(name, DATA_SUB_SECTION);
  if (DataIndex != -1) {
    std::map<int, XVMSectionInfo>::iterator I = DataSectionIndexInfoMap.find(DataIndex);
    if (I == DataSectionIndexInfoMap.end()) {
      return 0;
    }
    if ((I->second.BufType & XVM_SECTION_DATA_TYPE_POINTER) != XVM_SECTION_DATA_TYPE_UNKNOWN) {
      unsigned int ret = 1;
      return ret + GetPtrLevel(I->second.PtrSecIndex);
    }
  }
  return 0;
}

uint64_t GetSubSecOffsetForGlobal(const char *name) {
  std::map<std::string, SecSubSecIndices>::iterator I = DataSectionNameIndexMap.find(name);
  if (I == DataSectionNameIndexMap.end()) {
    return -1;
  } else {
    std::map<int, XVMSectionInfo>::iterator I1 = DataSectionIndexInfoMap.find(I->second.SubSecNameIndex);
    if (I1 == DataSectionIndexInfoMap.end())
      return -1;
    return I1->second.SubSecOffset;
  }
}

static int GetOffsetInDataSection(const char *name) {
  int DataIndex = GetDataIndex(name, DATA_SUB_SECTION);
  if (DataIndex != -1) {
    std::map<int, XVMSectionInfo>::iterator I = DataSectionIndexInfoMap.find(DataIndex);
    if (I == DataSectionIndexInfoMap.end()) {
      return -1;
    }
    return I->second.SubSecOffset;
  }
  return -1;
}

static int GetMergedSectionIndex(const std::string &SymName) {
  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    if (SInfo.SymName.compare(SymName) == 0) {
      return SInfo.MergedSecIndex;
    }
  }
  return -1;
}

static inline void PatchSectionInfo(void) {
  int MergedSectionIndex = 0;
  // Assign the merged section index to each section
  std::map<std::string, int> SectionNameIndexMap;
  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    std::map<std::string, int>::iterator It = SectionNameIndexMap.find(SInfo.SecName);
    if (It == SectionNameIndexMap.end()) {
      SectionNameIndexMap[SInfo.SecName] = MergedSectionIndex++;
    }
  }
  // patch merged section index
  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    SInfo.MergedSecIndex = SectionNameIndexMap[SInfo.SecName];
  }

  // patch offset
  std::map<unsigned int, uint64_t> SectionSizes;
  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    if (SectionSizes.find(SInfo.MergedSecIndex) == SectionSizes.end()) {
      SInfo.SubSecOffset = 0;
      SectionSizes[SInfo.MergedSecIndex] = SInfo.SecSize;
    } else {
      SInfo.SubSecOffset = SectionSizes[SInfo.MergedSecIndex];
      SectionSizes[SInfo.MergedSecIndex] += SInfo.SecSize;
    }
  }

  // patch the merged section index
  for (auto &E: DataSectionNameIndexMap) {
    int MergedSectionIndex = GetMergedSectionIndex(E.first);
    if (MergedSectionIndex != -1) {
      E.second.SecNameIndex = MergedSectionIndex;
    }
  }

  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    for (auto &EachPatch : SInfo.PatchListInfo) {
      uint64_t DataSectionOffset = GetOffsetInDataSection(EachPatch.SymName.data());
      DataSectionOffset += EachPatch.AddEnd;
      SInfo.PtrSecIndex = GetDataIndex(EachPatch.SymName.data(), DATA_SUB_SECTION);
      int DataSectionIndex = GetDataIndex(EachPatch.SymName.data(), DATA_SECTION);
      LLVM_DEBUG(dbgs() << "Add to Buf: "
                        << UnsignedIntTypeToHex(ReverseBytes(
                            CreateRefContent(DataSectionIndex, DataSectionOffset),
                            REF_TYPE_LENGTH), REF_TYPE_HEX_LENGTH).c_str()
                        << " size=" << REF_TYPE_LENGTH << "\n"
                        << " DataSectionOffset=" << DataSectionOffset
                        << " PtrSecIndex=" << SInfo.PtrSecIndex
                        << " patch loc="<< EachPatch.LocInByte << "\n");
      SInfo.SecBuf.replace(EachPatch.LocInByte, REF_TYPE_LENGTH * 4, UnsignedIntTypeToHex(
          ReverseBytes(CreateRefContent(DataSectionIndex, DataSectionOffset), REF_TYPE_LENGTH), REF_TYPE_HEX_LENGTH));
    }
  }
}

static inline std::string GetDataSectionPerm(XVMSectionInfo* SInfo) {
  assert(SInfo->Permission != XVM_SECTION_PERM_UNKNOWN && "Permission Unset");
  switch (SInfo->Permission) {
    case XVM_SECTION_PERM_RO:
      return "ro";
      break;
    case XVM_SECTION_PERM_WO:
      return "wo";
      break;
    case XVM_SECTION_PERM_XO:
      return "xo";
      break;
    case XVM_SECTION_PERM_RW:
      return "rw";
      break;
    case XVM_SECTION_PERM_RX:
      return "rx";
      break;
    case XVM_SECTION_PERM_WX:
      return "wx";
      break;
    case XVM_SECTION_PERM_RWX:
      return "rwx";
      break;
    default:
      llvm_unreachable("Unknown Data Permission Type");
  }
}

void XVMAsmPrinter::emitDataSectionInfo(raw_svector_ostream &O) {
  O << ";; \"data\" index name perm bytes init_data\n";
  // Merged section sizes
  int MaxMergedIndex = 0;
  std::map<unsigned int, uint64_t> SectionSizes;
  for (auto& I : DataSectionIndexInfoMap) {
    XVMSectionInfo &SInfo = I.second;
    if (SectionSizes.find(SInfo.MergedSecIndex) == SectionSizes.end()) {
      SectionSizes[SInfo.MergedSecIndex] = SInfo.SecSize;
    } else {
      SectionSizes[SInfo.MergedSecIndex] += SInfo.SecSize;
    }
    if (SInfo.MergedSecIndex > MaxMergedIndex)
      MaxMergedIndex = SInfo.MergedSecIndex;
  }

  if (DataSectionIndexInfoMap.size()>0) {
    for (int MergedSecIndex = 0; MergedSecIndex <= MaxMergedIndex; MergedSecIndex++)
    {
      // Merged section info
      std::string MergedSecBuf = "";
      std::string SecName = "";
      std::string SecPermit = "";
      for (auto& I : DataSectionIndexInfoMap) {
        XVMSectionInfo &SInfo = I.second;
        if (SInfo.MergedSecIndex == MergedSecIndex) {
          MergedSecBuf += SInfo.SecBuf;
          SecName = SInfo.SecName;
          SecPermit = GetDataSectionPerm(&SInfo);
        }
      }
      // The section data
      O << "(data " << MergedSecIndex << " $" << SecName << " " << SecPermit;
      O << " " << SectionSizes[MergedSecIndex];
      if (!MergedSecBuf.empty()) {
        O << " \"" << MergedSecBuf << "\"";
      }
      O << ")\n";
    }
  }
}

void XVMAsmPrinter::InitGlobalConstantDataSequential(
    const DataLayout &DL, const ConstantDataSequential *CDS, XVMSectionInfo* SInfo) {
  LLVM_DEBUG(dbgs() << "\n--------------------InitGlobalConstantDataSequential-------------------\n");

  unsigned ElementByteSize = CDS->getElementByteSize();
  if (isa<IntegerType>(CDS->getElementType())) {
    for (unsigned I = 0, E = CDS->getNumElements(); I != E; ++I) {
      LLVM_DEBUG(dbgs() << "Add to Buf: "
                        << UnsignedIntTypeToHex(ReverseBytes(
                            CDS->getElementAsInteger(I),
                            ElementByteSize), ElementByteSize*CHAT_LEN_IN_HEX).c_str()
                        << " size=" << ElementByteSize << "\n");
      SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(CDS->getElementAsInteger(I), ElementByteSize),
                                            ElementByteSize*CHAT_LEN_IN_HEX);
    }
  } else {
    llvm_unreachable("Should not have FP in sequential data");
  }
  unsigned Size = DL.getTypeAllocSize(CDS->getType());
  unsigned EmittedSize =
      DL.getTypeAllocSize(CDS->getElementType()) * CDS->getNumElements();
  assert(EmittedSize <= Size && "Size cannot be less than EmittedSize!");
  if (unsigned Padding = Size - EmittedSize) {
    LLVM_DEBUG(dbgs() << "\n------------Seq PADSIZE----------- " << Padding << "\n");
    LLVM_DEBUG(dbgs() << "Add to Buf: "
                      << UnsignedIntTypeToHex(ReverseBytes(0, Padding), Padding*CHAT_LEN_IN_HEX).c_str()
                      << " size=" << Padding << "\n");
    SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(0, Padding), Padding*CHAT_LEN_IN_HEX);
  }
}

void XVMAsmPrinter::InitGlobalConstantArray(const DataLayout &DL, const ConstantArray *CA,
                                            const Constant *BaseCV, uint64_t Offset,
                                            XVMSectionInfo* SInfo, Module *M) {
  LLVM_DEBUG(dbgs() << "\n--------------------InitGlobalConstantArray-------------------\n");
  for (unsigned I = 0, E = CA->getNumOperands(); I != E; ++I) {
    InitGlobalConstantImpl(DL, CA->getOperand(I), BaseCV, Offset, SInfo, M);
    Offset += DL.getTypeAllocSize(CA->getOperand(I)->getType());
  }
}

void XVMAsmPrinter::InitGlobalConstantStruct(const DataLayout &DL, const ConstantStruct *CS,
                                             const Constant *BaseCV, uint64_t Offset,
                                             XVMSectionInfo* SInfo, Module *M) {
  LLVM_DEBUG(dbgs() << "\n--------------------InitGlobalConstantStruct-------------------\n");
  unsigned Size = DL.getTypeAllocSize(CS->getType());
  const StructLayout *Layout = DL.getStructLayout(CS->getType());
  uint64_t SizeSoFar = 0;
  for (unsigned I = 0, E = CS->getNumOperands(); I != E; ++I) {
    const Constant *Field = CS->getOperand(I);

    // Print the actual field value.
    InitGlobalConstantImpl(DL, Field, BaseCV, Offset + SizeSoFar, SInfo, M);

    // Check if padding is needed and insert one or more 0s.
    uint64_t FieldSize = DL.getTypeAllocSize(Field->getType());
    uint64_t PadSize = ((I == E - 1 ? Size : Layout->getElementOffset(I + 1)) -
                        Layout->getElementOffset(I)) -
                       FieldSize;
    // Insert padding - this may include padding to increase the size of the
    // current field up to the ABI size (if the struct is not packed) as well
    // as padding to ensure that the next field starts at the right offset.
    if (PadSize > 0) {
      LLVM_DEBUG(dbgs() << "\n------------Struct PADSIZE-----------" << PadSize << "\n");
      LLVM_DEBUG(dbgs() << "Add to Buf: "
                        << UnsignedIntTypeToHex(ReverseBytes(0, PadSize), PadSize*CHAT_LEN_IN_HEX).c_str()
                        << " size=" << PadSize << "\n");
      SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(0, PadSize), PadSize*CHAT_LEN_IN_HEX);
    }
    SizeSoFar += FieldSize + PadSize;
  }
  LLVM_DEBUG(dbgs() << "\n------------Total with Padding----------- " << SizeSoFar << "\n");
  assert(SizeSoFar == Layout->getSizeInBytes() &&
         "Layout of constant struct may be incorrect!");
}

void XVMAsmPrinter::InitGlobalConstantImpl(const DataLayout &DL, const Constant *CV,
                                           const Constant *BaseCV, uint64_t Offset,
                                           XVMSectionInfo* SInfo, Module *M) {
  LLVM_DEBUG(dbgs() << "\n--------------------InitGlobalConstantImpl "
                    << CV->getName().str().c_str()
                    << "-------------------\n");
  uint64_t Size = DL.getTypeAllocSize(CV->getType());

  // Globals with sub-elements such as combinations of arrays and structs
  // are handled recursively by InitGlobalConstantImpl. Keep track of the
  // constant symbol base and the current position with BaseCV and Offset.
  if (!BaseCV && CV->hasOneUse())
    BaseCV = dyn_cast<Constant>(CV->user_back());

  if (isa<ConstantAggregateZero>(CV) || isa<UndefValue>(CV)) {
    if (SInfo->BufType == XVM_SECTION_DATA_TYPE_UNKNOWN) {
      SInfo->BufType = XVM_SECTION_DATA_TYPE_BSS;
      SInfo->SecSize = Size;
      LLVM_DEBUG(dbgs() << "\nemit in InitGlobalConstantImpl bss "
                        << Size << " "
                        << DL.getTypeStoreSize(CV->getType()) << " "
                        << DL.getTypeAllocSize(CV->getType()).getFixedSize() << "\n");
      return;
    }
    LLVM_DEBUG(dbgs() << "\nemit in InitGlobalConstantImpl zero or undef "
                      << Size << " "
                      << DL.getTypeStoreSize(CV->getType()) << " "
                      << DL.getTypeAllocSize(CV->getType()).getFixedSize() << "\n");
    LLVM_DEBUG(dbgs() << "Add to Buf: "
                      << UnsignedIntTypeToHex(ReverseBytes(0, Size), Size*CHAT_LEN_IN_HEX).c_str()
                      << " size=" << Size << "\n");
    SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(0, Size), Size*CHAT_LEN_IN_HEX);
    return;
  }

  if (const ConstantInt *CI = dyn_cast<ConstantInt>(CV)) {
    const uint64_t StoreSize = DL.getTypeStoreSize(CV->getType());
    if (StoreSize <= MAX_PTR_SIZE) {
      SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_NUMERIC;
      LLVM_DEBUG(dbgs() << "\nemit in InitGlobalConstantImpl int\n");
      LLVM_DEBUG(dbgs() << "Add to Buf: "
                        << UnsignedIntTypeToHex(ReverseBytes(CI->getZExtValue(), StoreSize),
                                                StoreSize*CHAT_LEN_IN_HEX).c_str()
                        << " size=" << StoreSize << "\n");
      SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(CI->getZExtValue(), StoreSize), StoreSize*CHAT_LEN_IN_HEX);
    } else {
      for (Function &F1 : M->getFunctionList()) {
        for (BasicBlock &BB : F1) {
          for (Instruction &I : BB) {
            DebugLoc DL = I.getDebugLoc();
            ExportFailMsg(F1, DL, "Error: Large int globals are unsupported", (void*)&StoreSize);
            LLVM_DEBUG(dbgs() << "XVM Error: Should not have large int global value!");
            exit(1);
            break;
          }
          break;
        }
        break;
      }
    }
    return;
  }

  if (const ConstantFP *CFP = dyn_cast<ConstantFP>(CV))
    llvm_unreachable("Should not have floating point global value!");

  if (isa<ConstantPointerNull>(CV)) {
    SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_POINTER;
    LLVM_DEBUG(dbgs() << "\nemit in InitGlobalConstantImpl nullptr\n");
    LLVM_DEBUG(dbgs() << "Add to Buf: \\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00"
                      << " size=" << MAX_PTR_SIZE << "\n");
    SInfo->SecBuf += "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00";
    return;
  }

  if (const ConstantDataSequential *CDS = dyn_cast<ConstantDataSequential>(CV)) {
    SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_STRING;
    return InitGlobalConstantDataSequential(DL, CDS, SInfo);
  }

  if (const ConstantArray *CVA = dyn_cast<ConstantArray>(CV)) {
    SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_ARRAY;
    return InitGlobalConstantArray(DL, CVA, BaseCV, Offset, SInfo, M);
  }

  if (const ConstantStruct *CVS = dyn_cast<ConstantStruct>(CV)) {
    SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_STRUCT;
    return InitGlobalConstantStruct(DL, CVS,  BaseCV, Offset,  SInfo, M);
  }

  if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV)) {
    // Look through bitcasts, which might not be able to be MCExpr'ized (e.g. of
    // vectors).
    if (CE->getOpcode() == Instruction::BitCast)
      return InitGlobalConstantImpl(DL, CE->getOperand(0), BaseCV, Offset,  SInfo, M);

    if (Size > MAX_SIZE_CONSTANT_EXPRESSION) {
      // If the constant expression's size is greater than 64-bits, then we have
      // to emit the value in chunks. Try to constant fold the value and emit it
      // that way.
      Constant *New = ConstantFoldConstant(CE, DL);
      if (New != CE)
        llvm_unreachable("unimplemented after ConstantFoldConstant");
    }
  }

  if (const ConstantVector *V = dyn_cast<ConstantVector>(CV))
    llvm_unreachable("Should not have vector global value!");

  LLVM_DEBUG(dbgs() << "\nemit in InitGlobalConstantImpl ptr\n");

  int idx_func = GetFuncIndex(CV->getName().data());
  if (idx_func != -1) {
    SInfo->BufType = SInfo->BufType | XVM_SECTION_DATA_TYPE_POINTER;
    SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(idx_func, REF_TYPE_LENGTH), REF_TYPE_HEX_LENGTH);
    LLVM_DEBUG(dbgs() << "\nsuccess in function pointer "
                      << UnsignedIntTypeToHex(ReverseBytes(idx_func, REF_TYPE_LENGTH), REF_TYPE_HEX_LENGTH).c_str()
                      << "\n");
    return;
  }

  int DataSectionIndex = GetDataIndex(CV->getName().data(), DATA_SECTION);
  XVMGVPathInfo PatchInfo;
  if (DataSectionIndex != -1) {
    SInfo->BufType = XVM_SECTION_DATA_TYPE_POINTER;
    SInfo->SecComment += "=";
    SInfo->SecComment += CV->getName().data();

    PatchInfo.SymName = CV->getName().data();
    PatchInfo.AddEnd = 0;
    PatchInfo.LocInByte = SInfo->SecBuf.length();
    SInfo->SecBuf += "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00";
    SInfo->PatchListInfo.push_back(PatchInfo);
    return;
  }

  // Otherwise, it must be a ConstantExpr.  Lower it to an MCExpr, then emit it
  // thread the streamer with EmitValue.
  const MCExpr *ME = lowerConstant(CV);
  std::string StrME;
  llvm::raw_string_ostream rso(StrME);
  if (ME != NULL) {
    DataSectionIndex = 0;
    if (ME->getKind() == llvm::MCExpr::Binary) {
      const MCSymbolRefExpr *SRE;
      if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(ME)) {
        SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
        const auto *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
        assert(SRE && CE && "Binary expression must be sym+const.");
        // Here we need to handle the case such as .L.str+1: see sample source from pr53084.c
        std::string SymName = SRE->getSymbol().getName().str();
        SymName = GetSymbolName(SymName);
        DataSectionIndex = GetDataIndex(SymName.data(), DATA_SECTION);
        rso << *ME;
        SInfo->SecComment += "=";
        SInfo->SecComment += StrME.data();

        PatchInfo.SymName = SymName;
        PatchInfo.AddEnd = (int)CE->getValue();
        // Please note that if the variable is defined after, then
        // no DataSectionIndex
        SInfo->BufType = XVM_SECTION_DATA_TYPE_POINTER;
        PatchInfo.LocInByte = SInfo->SecBuf.length();
        SInfo->SecBuf += "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00";
        SInfo->PatchListInfo.push_back(PatchInfo);
      }
      else {
        SRE = dyn_cast<MCSymbolRefExpr>(ME);
        assert(SRE && "Unexpected MCExpr type.");
      }
    } else {
      if (ME->getKind() == llvm::MCExpr::Constant) {
        // Here we handle the case of constant cast to ptr
        const auto &ConstA = cast<MCConstantExpr>(ME);
        const ConstantExpr *CExprA = dyn_cast<ConstantExpr>(CV);
        assert(CExprA->getOpcode() == Instruction::IntToPtr);
        PatchInfo.LocInByte = SInfo->SecBuf.length();
        LLVM_DEBUG(dbgs() << "Add to Buf: "
                          << UnsignedIntTypeToHex(ReverseBytes(ConstA->getValue(), REF_TYPE_LENGTH),
                                                  REF_TYPE_HEX_LENGTH).c_str()
                          << " size=" << MAX_PTR_SIZE << "\n");
        SInfo->SecBuf += UnsignedIntTypeToHex(ReverseBytes(ConstA->getValue(), REF_TYPE_LENGTH), REF_TYPE_HEX_LENGTH);
        return;
      } else if (ME->getKind() == llvm::MCExpr::SymbolRef) {
        SInfo->BufType = XVM_SECTION_DATA_TYPE_POINTER;
        PatchInfo.LocInByte = SInfo->SecBuf.length();
        SInfo->SecBuf += "\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00";
      }
      rso << *ME;
      PatchInfo.SymName = StrME.data();
      PatchInfo.AddEnd = 0;
      SInfo->PatchListInfo.push_back(PatchInfo);
    }
    return;
  }
  llvm_unreachable("unhandled global type!!");
}

void XVMAsmPrinter::InitDataSectionGlobalConstant(Module *M) {
  LLVM_DEBUG(dbgs() << "--------------------InitGlobalConstant-------------------\n");
  SecSubSecIndices SecIndices;
  SecIndices.SecNameIndex = -1;
  unsigned int SectionIndex = 0;

  for (const llvm::GlobalVariable &GV : M->getGlobalList()) {
    if (GV.getName().compare("llvm.global_ctors") == 0 ||
        GV.getName().compare("llvm.global_dtors") == 0) {
      continue;
    }
    if (GV.hasInitializer()) {
      LLVM_DEBUG(dbgs() << "\n\nvar: "
                        << GV.getName().str().c_str()
                        << "\n");
      const DataLayout &DL =  GV.getParent()->getDataLayout();
      const Constant *CV = GV.getInitializer();
      uint64_t Size = DL.getTypeAllocSize(CV->getType());
      XVMSectionInfo SInfo;
      SInfo.MergedSecIndex = -1;

      if (Size) {
        SInfo.SecName = "unknown"; // placeholder
        SInfo.Permission = XVM_SECTION_PERM_UNKNOWN; // placeholder
        SecIndices.SubSecNameIndex = SectionIndex;
        SInfo.SecIndex = SectionIndex;
        SInfo.PtrSecIndex = -1;
        SInfo.SecSize = Size;
        SInfo.BufType = XVM_SECTION_DATA_TYPE_UNKNOWN;
        SInfo.SecBuf = "";
        SInfo.SecComment = GV.getName().data();
        SInfo.SymName = GV.getName().data();
        InitGlobalConstantImpl(DL, CV, nullptr, 0, &SInfo, M);
      }
      LLVM_DEBUG(dbgs() << "buf size is " << Size << " buf="
                        << SInfo.SecBuf.c_str() << "\n");
      // ser permission
      if (GV.isConstant()) {
        if (SInfo.BufType == XVM_SECTION_DATA_TYPE_BSS) {
          // zero init for constant should be in the ro with init
          SInfo.SecBuf += UnsignedIntTypeToHex(ReverseBytes(0, SInfo.SecSize), SInfo.SecSize*2);
        }
        SInfo.Permission = XVM_SECTION_PERM_RO;
        SInfo.SecName = "rodata";
      } else {
        SInfo.Permission = XVM_SECTION_PERM_RW;
        if ((SInfo.BufType & XVM_SECTION_DATA_TYPE_BSS) != XVM_SECTION_DATA_TYPE_UNKNOWN) {
          SInfo.SecName = "bss";
        } else {
          SInfo.SecName = "data";
        }
      }
      SInfo.SymName = GV.getName().data();

      DataSectionNameIndexMap.insert(std::pair<std::string, SecSubSecIndices>(GV.getName().data(), SecIndices));
      DataSectionIndexInfoMap.insert(std::pair<int, XVMSectionInfo>(SectionIndex++, SInfo));
    }
    // Notes: extern case we may need to consider later
  }
  PatchSectionInfo();
}

void XVMAsmPrinter::InitModuleMapFuncnameIndex(Module *M) {
  int Index = 0;
  std::vector<std::string> FuncDecl;

  /* F1 can't be const because if the function is a constructor
   * or destructor, we need to add the export attribute to it */
  for (Function &F1 : M->getFunctionList()) {
    if (F1.getInstructionCount() != 0) {
      FunctionDefinitionMap.insert(std::make_pair(F1.getName().data(), Index));
      FunctionNameAndIndex.insert(std::make_pair(F1.getName().data(), Index));
      if (IsConstructorDestructor(F1.getName().data(), M)) {
        F1.addFnAttr("xvm-export-name", F1.getName().data());
      }
      Index ++;
    }
    else if (!F1.isIntrinsic()) {
      FuncDecl.push_back(F1.getName().data());
    }
  }

  for (std::string FName : FuncDecl) {
    FunctionNameAndIndex.insert(std::make_pair(FName, Index));
    Index ++;
  }
}

bool XVMAsmPrinter::doInitialization(Module &M) {
  AsmPrinter::doInitialization(M);

  return false;
}

void XVMAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                 raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << XVMInstPrinter::getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    O << *getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress: {
    MCSymbol *BA = GetBlockAddressSymbol(MO.getBlockAddress());
    O << BA->getName();
    break;
  }

  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  default:
    llvm_unreachable("<unknown operand type>");
  }
}

bool XVMAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                    const char *ExtraCode, raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, O);

  printOperand(MI, OpNo, O);
  return false;
}

bool XVMAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                          unsigned OpNum, const char *ExtraCode,
                                          raw_ostream &O) {
  assert(OpNum + 1 < MI->getNumOperands() && "Insufficient operands");
  const MachineOperand &BaseMO = MI->getOperand(OpNum);
  const MachineOperand &OffsetMO = MI->getOperand(OpNum + 1);
  assert(BaseMO.isReg() && "Unexpected base pointer for inline asm memory operand.");
  assert(OffsetMO.isImm() && "Unexpected offset for inline asm memory operand.");
  int Offset = OffsetMO.getImm();
  if (ExtraCode)
    return true; // Unknown modifier.

  if (Offset < 0)
    O << "(" << XVMInstPrinter::getRegisterName(BaseMO.getReg()) << " - " << -Offset << ")";
  else
    O << "(" << XVMInstPrinter::getRegisterName(BaseMO.getReg()) << " + " << Offset << ")";

  return false;
}

void XVMAsmPrinter::setFunctionCallInfo(MCInst *Inst) {
  const char *FuncName = NULL;
  const MCExpr *TmpExpr;
  int64_t FuncIndex = -1;
  if (Inst == NULL)
    return;

  assert(Inst->getNumOperands()>0);
  MCOperand &FirstMO = Inst->getOperand(0);
  if (FirstMO.isExpr()) {
    Inst->setFlags(FUNC_CALL_FLAG_MC_INST_IMM);
    TmpExpr = Inst->getOperand(0).getExpr();
    FuncName = cast<MCSymbolRefExpr>(*TmpExpr).getSymbol().getName().data();
    assert(FuncName != NULL);
    assert(Inst->getNumOperands() == 1);
    FuncIndex = GetFuncIndex(FuncName);
    if (FuncIndex == -1) {
      std::string ErrorMesg("Error: function name ");
      ErrorMesg += FuncName;
      ErrorMesg += " is called; but could not be found\n";
      report_fatal_error(ErrorMesg.data());
    }
    MCOperand MCOp = MCOperand::createImm(FuncIndex);
    Inst->addOperand(MCOp);
  }
  if (FirstMO.isReg()) {
    Inst->setFlags(FUNC_CALL_FLAG_MC_INST_REG);
    int reg_num = FirstMO.getReg();
    MCOperand MCOp = MCOperand::createReg(reg_num);
    Inst->addOperand(MCOp);
  }
}

void XVMAsmPrinter::setGlobalSymbolInfo(const MachineInstr *MI, MCInst* Inst) {
  unsigned int numOps = MI->getNumOperands();
  bool hasGlobalSymbol = false;
  for (unsigned int i = 0; i < numOps; i++)
  {
    const MachineOperand& tmp = MI->getOperand(i);
    if (tmp.isGlobal()) {
      hasGlobalSymbol = true;
    }
  }
  if (!hasGlobalSymbol)
    return;
  /* add operand and indicate the global symbol type*/
  numOps = Inst->getNumOperands();
  for (unsigned int i = 0; i < numOps; i++) {
    const MCOperand &tmp = Inst->getOperand(i);
    if (tmp.isExpr()) {
      const MCExpr *TmpExpr = tmp.getExpr();
      const MCSymbolRefExpr *SRE;
      if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(TmpExpr))
        SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
      else
        SRE = dyn_cast<MCSymbolRefExpr>(TmpExpr);
      std::string SymName = SRE->getSymbol().getName().str();
      /* the symbol may be function name, may be global varaibel name */
      int SymIndex = GetFuncIndex(SymName.c_str());
      if (SymIndex != -1) {
        /* it is a function name */
        Inst->setFlags(FUNC_ID_FLAG_MC_INST_REG);
        MCOperand MCOp = MCOperand::createImm(SymIndex);
        Inst->addOperand(MCOp);
      } else {
        /* it may be a global variable name */
        SymName = GetSymbolName(SymName);
        SymIndex = GetDataIndex(SymName.c_str(), DATA_SECTION);
        if (SymIndex == -1)  {
          const MachineFunction *F = MI->getParent()->getParent();
          ExportFailMsg(F->getFunction(), MI->getDebugLoc(), "Error: Externs aren't supported", NULL);
          LLVM_DEBUG(dbgs() << "XVM TODO: Add the support of non-func-global-var scenarios\n");
          exit(1);
        } else {
          Inst->setFlags(GLOBAL_DATAREF_FLAG_MC_INST);
          MCOperand MCOp = MCOperand::createImm(SymIndex);
          Inst->addOperand(MCOp);
        }
      }
    }
  }
  return;
}

void XVMAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  XVMMCInstLower MCInstLowering(OutContext, *this);
  MCInstLowering.Lower(MI, TmpInst);

  if (MI->isCall()) {
    setFunctionCallInfo(&TmpInst);
  } else {
    setGlobalSymbolInfo(MI, &TmpInst);
  }
  TmpInst.addOperand(MCOperand::createImm(MIIndentMap[MI]));

  EmitToStreamer(*OutStreamer, TmpInst);
}

void XVMAsmPrinter::emitFunctionHeader() {
  const Function &F = MF->getFunction();
  SmallString<INIT_SMALL_STR_SIZE> Str;
  raw_svector_ostream O(Str);
  int Index = GetDefFuncIndex(F.getName().data());
  assert(Index != -1);

  if (F.hasFnAttribute("xvm-export-name")) {
    StringRef ExportName = F.getFnAttribute("xvm-export-name").getValueAsString();
    O << "(export $" << ExportName << "\n";
  } else {
    if (XVMExportAll) {
      O << "\t(export " << " $" << F.getName() << " ";
    }
  }
  O << "\t(func " << Index << " $" << F.getName() << " ";

  emitFunctionParamList(*MF, O);
  emitFunctionReturnVal(*MF, O);

  OutStreamer->emitRawText(O.str());

  auto Section = getObjFileLowering().SectionForGlobal(&F, TM);
  MF->setSection(Section);
}

void XVMAsmPrinter::emitFunctionReturnVal(const MachineFunction &MF, raw_ostream &O) {
  emitFunctionReturnVal(MF.getFunction(), O);
}

void XVMAsmPrinter::emitFunctionReturnVal(const Function &F, raw_ostream &O) {
  O << "(result";
  Type *Ty = F.getReturnType();
  if (auto *PTy = dyn_cast<PointerType>(Ty)) {
    O << " ref";
  } else if (Ty->isIntegerTy()) {
    O << " i64";
  } else if (!Ty->isVoidTy()) {
    DebugLoc DL;
    ExportFailMsg(F, DL, "Invalid return type", NULL);
    LLVM_DEBUG(dbgs() << "XVM Error: Invalid return type");
    exit(1);
  }

  O << ")";
}

void XVMAsmPrinter::emitFunctionBodyEnd() {
  SmallString<INIT_SMALL_STR_SIZE> Str;
  raw_svector_ostream O(Str);
  O << "\t)";
  if (MF->getFunction().hasFnAttribute("xvm-export-name"))
    O << "\n)";
  else if (XVMExportAll) {
      O << ")";
  }

  OutStreamer->emitRawText(O.str());
}

void XVMAsmPrinter::emitFunctionParamList(const MachineFunction &MF, raw_ostream &O) {
  emitFunctionParamList(MF.getFunction(), O);
}

void XVMAsmPrinter::emitFunctionParamList(const Function &F, raw_ostream &O) {
  O << "(param";

  for (Function::const_arg_iterator I = F.arg_begin(), E = F.arg_end();
       I != E; ++I) {
    Type *Ty = I->getType();
    if (auto *PTy = dyn_cast<PointerType>(Ty)) {
      O << " ref";
    } else if (Ty->isIntegerTy()) {
      O << " i64";
    } else {
      DebugLoc DL;
      ExportFailMsg(F, DL, "Invalid (non ref or interger) param type", NULL);
      LLVM_DEBUG(dbgs() << "XVM Error: Invalid param type");
      exit(1);
    }
  }

  O << ")";
}

void XVMAsmPrinter::emitStartOfAsmFile(Module &M) {
  InitModuleMapFuncnameIndex(&M);
  InitDataSectionGlobalConstant(&M);
  SmallString<INIT_SMALL_STR_SIZE> Str1;
  raw_svector_ostream O(Str1);

  O << "(module";
  OutStreamer->emitRawText(O.str());
}

static void output_constructor_destructor_metadata(raw_svector_ostream &O,
                                                   std::map<uint16_t, std::vector<const char *>> priority_map,
                                                   const char *MetadataName) {
  O << "(metadata " << MetadataName << " \"";
  if (strcmp(MetadataName, "$init_array") == 0) {
    for (auto i = priority_map.begin(); i != priority_map.end(); i++) {
      for (unsigned j = 0; j < (i->second).size(); j++) {
        int FuncIndex = GetFuncIndex((i->second)[j]);
        std::string func_id_data = UnsignedIntTypeToHex(ReverseBytes(FuncIndex, 4), 8);
        O << func_id_data.data();
      }
    }
  } else {
    /* Destructors are ran from largest number to smallest */
    for (auto i = priority_map.rbegin(); i != priority_map.rend(); i++) {
      for (unsigned j = 0; j < (i->second).size(); j++) {
        int FuncIndex = GetFuncIndex((i->second)[j]);
        std::string func_id_data = UnsignedIntTypeToHex(ReverseBytes(FuncIndex, 4), 8);
        O << func_id_data.data();
      }
    }
  }
  O << "\")\n";
}

static void emitConstructorsDestructors(raw_svector_ostream &O, Module &M,
                                        const char *GVName, const char *MetadataName) {
  GlobalVariable *GV;
  std::map<uint16_t, std::vector<const char *>> priority_map;

  GV = M.getGlobalVariable(GVName);
  if (!GV) {
    return;
  }
  const ConstantArray *InitList = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!InitList) {
    return;
  }
  StructType *ETy = dyn_cast<StructType>(InitList->getType()->getElementType());
  if (!ETy || ETy->getNumElements() != NUM_MO_CTOR_DTOR ||
    !ETy->getTypeAtIndex(0U)->isIntegerTy() ||
    !ETy->getTypeAtIndex(1U)->isPointerTy() ||
    !ETy->getTypeAtIndex(2U)->isPointerTy()) {
      return;
  }

  for (Value *P : InitList->operands()) {
    ConstantStruct *CS = dyn_cast<ConstantStruct>(P);
    if (!CS) {
      continue;
    }
    ConstantInt *Priority = dyn_cast<ConstantInt>(CS->getOperand(0));
    if (!Priority) {
      continue;
    }
    uint16_t PriorityValue = Priority->getLimitedValue(UINT16_MAX);
    const char *FuncName = CS->getOperand(1)->getName().data();
    priority_map[PriorityValue].push_back(FuncName);
  }

  output_constructor_destructor_metadata(O, priority_map, MetadataName);
}

static void emitMetaDataSectionInfo(raw_svector_ostream &O, Module &M) {
  O << "\n;; \"metadata\" name init_data\n";
  emitConstructorsDestructors(O, M, "llvm.global_ctors", "$init_array");
  emitConstructorsDestructors(O, M, "llvm.global_dtors", "$fini_array");
}

void XVMAsmPrinter::emitEndOfAsmFile(Module &M) {
  SmallString<INIT_SMALL_STR_SIZE> Str1;
  raw_svector_ostream O(Str1);
  emitDecls(M);
  emitDataSectionInfo(O);
  emitMetaDataSectionInfo(O, M);
  O << ")";
  OutStreamer->emitRawText(O.str());
}

void XVMAsmPrinter::emitDecls(const Module &M) {
  SmallString<INIT_SMALL_STR_SIZE> Str1;
  raw_svector_ostream O(Str1);
  for (const Function &F : M.getFunctionList()) {
    if (GetDefFuncIndex(F.getName().data()) == -1 &&
        !F.isIntrinsic()) {
      O << "(import $" << F.getName() << "\n";
      O << "\t(func " << GetFuncIndex(F.getName().data()) << " $" << F.getName() << " ";
      emitFunctionParamList(F, O);
      emitFunctionReturnVal(F, O);
      O << ")\n)\n";
    }
  }
  OutStreamer->emitRawText(O.str());
}

static bool ShouldIncIndent(MachineInstr &MI, const XVMInstrInfo *TII) {
  if (TII->isCondBranch(&MI) || TII->isCondBranchProcessed(&MI) ||
      MI.getOpcode() == XVM::LOOP || MI.getOpcode() == XVM::BLOCK ||
      MI.getOpcode() == XVM::THEN || MI.getOpcode() == XVM::ELSE) {
    return true;
  } else {
    return false;
  }
}

static bool ShouldDecIndent(MachineInstr &MI) {
  if (MI.getOpcode() == XVM::END_LOOP || MI.getOpcode() == XVM::END_BLOCK ||
      MI.getOpcode() == XVM::END_THEN || MI.getOpcode() == XVM::END_IF ||
      MI.getOpcode() == XVM::END_ELSE) {
    return true;
  } else {
    return false;
  }
}

void XVMAsmPrinter::GetMIIndent(MachineFunction &MF) {
  unsigned CurrentIndent = 0;
  const XVMInstrInfo *TII = MF.getSubtarget<XVMSubtarget>().getInstrInfo();
  for (MachineFunction::iterator MBBI = MF.begin(), E = MF.end(); MBBI != E; ++MBBI) {
    for (MachineBasicBlock::iterator MII = MBBI->begin(), EI = MBBI->end(); MII != EI; ++MII) {
      if (ShouldIncIndent(*MII, TII)) {
        MIIndentMap[&*MII] = CurrentIndent++;
      } else if (ShouldDecIndent(*MII)) {
        MIIndentMap[&*MII] = --CurrentIndent;
      } else {
        MIIndentMap[&*MII] = CurrentIndent;
      }
    }
  }
  assert (CurrentIndent == 0 && "All the indents should be paired!");
}

static void checkFunctionSize(MachineFunction &MF) {
  int count = 0;

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      count++;
    }
  }

  if (count > MAX_FUNC_SIZE) {
    DebugLoc DL;
    Function &F = MF.getFunction();
    std::string ErrorMesg("Error: Function '");
    ErrorMesg += MF.getName().str().c_str();
    ErrorMesg += "' has ";
    ErrorMesg += std::to_string(count);
    ErrorMesg += " instructions. Max instructions is 32767.\n";
    ExportFailMsg(F, DL, ErrorMesg.data(), NULL);
    exit(1);
  } 
}

void XVMAsmPrinter::emitGlobalVariable(const GlobalVariable *GV) {}

bool XVMAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  SetupMachineFunction(MF);
  GetMIIndent(MF);
  emitFunctionBody();
  checkFunctionSize(MF);
  return false;
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMAsmPrinter() {
  RegisterAsmPrinter<XVMAsmPrinter> X(getTheXVMleTarget());
  RegisterAsmPrinter<XVMAsmPrinter> Y(getTheXVMTarget());
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeXVMAsmPrinterCalledInDylib() {
  LLVMInitializeXVMAsmPrinter();
}

#else

#include "XVMDylibHandler.h"
LLVM_INIT_XVM_COMP(AsmPrinter)

#endif
