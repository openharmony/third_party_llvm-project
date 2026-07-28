#ifdef OHOS_LLVM
//===-- ValueEvaluator.cpp - dwarf Expression Evaluation ------------------===//
//
// This file implements the core variable evaluation logic for the
// crash_analyzer tool, including:
//   - Interpretation of DWARF location expressions (DW_OP_*)
//   - Resolution of variable values from registers, stack offsets, global
//     addresses, and inline constants
//   - Delegating backward instruction slicing to TargetSlicer to recover
//     entry values for variables marked with DW_OP_entry_value
//
//===----------------------------------------------------------------------===//
#include "ValueEvaluator.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/LowLevel/DWARFExpression.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/DebugInfo/DWARF/DWARFUnit.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <optional>

using namespace llvm;

namespace crash_analyzer {
namespace evaluator {

ValueEvaluator::ValueEvaluator(std::shared_ptr<ThreadContext> ThreadCtx,
                               BackwardSlicer *Slicer)
    : ThreadCtx(ThreadCtx), Slicer(Slicer) {}

std::string ValueEvaluator::formatHex(uint64_t Val) const {
  std::string Res;
  llvm::raw_string_ostream Os(Res);
  Os << "0x" << llvm::format_hex_no_prefix(Val, 16);
  return Os.str();
}

EvaluatedVariable
ValueEvaluator::evaluate(DWARFDie VarDie, ArrayRef<uint8_t> ExprData,
                         uint64_t CrashPC, uint64_t FuncLowPC,
                         const std::vector<SimplifiedInst> &InstStream) {
  EvaluatedVariable Result;
  Result.Name = dwarf::toString(VarDie.find(dwarf::DW_AT_name), "<anonymous>");
  Result.TypeName = "unknown_type";

  // DW_AT_const_value is already saved by DWARF
  if (auto ConstForm = VarDie.find(dwarf::DW_AT_const_value)) {
    Result.Status = EvalStatus::Resolved;
    Result.LocationType = "Hardcoded Constant";
    if (auto UnsignedVal = dwarf::toUnsigned(ConstForm)) {
      Result.FormattedValue = std::to_string(*UnsignedVal);
    } else if (auto SignedVal = dwarf::toSigned(ConstForm)) {
      Result.FormattedValue = std::to_string(*SignedVal);
    } else if (auto BlockVal = dwarf::toBlock(ConstForm)) {
      std::string HexStr = "0x";
      std::string AsciiStr = "";
      bool IsPrintable = true, IsAllZero = true;
      for (uint8_t Byte : *BlockVal) {
        char Buf[8];
        snprintf(Buf, sizeof(Buf), "%02x", Byte);
        HexStr += Buf;
        if (Byte != 0)
          IsAllZero = false;
        if (Byte >= 32 && Byte <= 126)
          AsciiStr += static_cast<char>(Byte);
        else if (Byte != 0)
          IsPrintable = false;
      }
      if (IsAllZero)
        Result.FormattedValue = "0x0 (NULL Block)";
      else if (IsPrintable && !AsciiStr.empty())
        Result.FormattedValue = HexStr + " (\"" + AsciiStr + "\")";
      else
        Result.FormattedValue = HexStr + " (Raw Block Data)";
    } else {
      Result.FormattedValue = "<unknown constant>";
    }
    return Result;
  }

  if (ExprData.empty()) {
    Result.Status = EvalStatus::OptimizedOut;
    Result.FormattedValue = "<optimized out>";
    return Result;
  }

  DataExtractor Data(ExprData,
                     VarDie.getDwarfUnit()->getContext().isLittleEndian(), 8);
  DWARFExpression Expr(Data, VarDie.getDwarfUnit()->getAddressByteSize());

  std::optional<uint64_t> PendingValue = std::nullopt;
  std::string PendingLocation = "";
  EvalStatus PendingStatus = EvalStatus::OptimizedOut;

  for (auto &Op : Expr) {
    uint8_t Opcode = Op.getCode();

    // 1. Direct register reading (State Machine Step 1)
    if (Opcode >= dwarf::DW_OP_reg0 && Opcode <= dwarf::DW_OP_reg31) {
      uint32_t RegNum = Opcode - dwarf::DW_OP_reg0;
      PendingLocation = "Register (x" + std::to_string(RegNum) + ")";
      if (ThreadCtx && (PendingValue = ThreadCtx->getRegister(RegNum))) {
        PendingStatus = EvalStatus::Resolved;
      }
      continue;
    }

    // 2. Handling DWARF Truncated Instructions (State Machine Step 2)
    if (Opcode == dwarf::DW_OP_piece && PendingValue) {
      uint64_t PieceBytes = Op.getRawOperand(0);
      if (PieceBytes < 8)
        *PendingValue &= ((1ULL << (PieceBytes * 8)) - 1);
      PendingLocation += " [Piece: " + std::to_string(PieceBytes) + " bytes]";
      continue;
    }

    // 3. CFA Instruction - Stack frame base parsing
    if (Opcode == dwarf::DW_OP_call_frame_cfa) {
      Result.LocationType = "Stack Memory (CFA)";
      if (ThreadCtx && ThreadCtx->getCleanSP()) {
        Result.Status = EvalStatus::MemoryFault;
        Result.FormattedValue = "need dump memory, can load [ " +
                                formatHex(*(ThreadCtx->getCleanSP())) +
                                " + offset ]";
      } else {
        Result.Status = EvalStatus::MemoryFault;
        Result.FormattedValue = "need dump memory, can load [ invalid CFA ]";
      }
      return Result; // Memory pointer calculation is complete, return directly
    }

    // 4. Read Stack offset
    if (Opcode == dwarf::DW_OP_fbreg) {
      int64_t Offset = Op.getRawOperand(0);
      Result.LocationType = "Stack Memory";
      if (ThreadCtx) {
        // Stack base reg
        uint16_t FrameBaseReg = 29;
        if (auto RegOpt = getFrameBaseRegister(VarDie)) {
          FrameBaseReg = *RegOpt;
        }

        auto FbVal = ThreadCtx->getRegister(FrameBaseReg);
        if (!FbVal) {
          // not find base seg，back to SP
          FbVal = ThreadCtx->getCleanSP();
        }

        if (FbVal) {
          uint64_t TargetAddr = *FbVal + Offset;
          uint64_t ByteSize = getVariableByteSize(VarDie);
          if (auto MemVal = ThreadCtx->getMemoryValue(TargetAddr, ByteSize)) {
            Result.Status = EvalStatus::Resolved;
            Result.FormattedValue = *MemVal;
          } else {
            Result.Status = EvalStatus::MemoryFault;
            Result.FormattedValue =
                "need dump memory, can load [ " + formatHex(TargetAddr) + " ]";
          }
        } else {
          Result.Status = EvalStatus::MemoryFault;
          Result.FormattedValue =
              "need dump memory, can load [ invalid FrameBase ]";
        }
      }
      return Result;
    }

    // 5. Read global address
    if (Opcode == dwarf::DW_OP_addr || Opcode == dwarf::DW_OP_addrx) {
      uint64_t StaticAddr = 0;
      if (Opcode == dwarf::DW_OP_addr) {
        // DWARF 4 and earlier directly fetch static addresses from the operand.
        StaticAddr = Op.getRawOperand(0);
      } else {
        // DWARF 5 obtains the static address from the.debug_addr section based
        // on the index.
        uint64_t Index = Op.getRawOperand(0);
        if (auto AddrOpt =
                VarDie.getDwarfUnit()->getAddrOffsetSectionItem(Index)) {
          StaticAddr = AddrOpt->Address;
        } else {
          Result.Status = EvalStatus::MemoryFault;
          Result.FormattedValue = "<Invalid DW_OP_addrx Index>";
          return Result;
        }
      }

      uint64_t LoadBias = 0;
      if (ThreadCtx) {
        LoadBias = ThreadCtx->getCrashFrame() != nullptr
                       ? ThreadCtx->getCrashFrame()->getLoadBias()
                       : 0;
      }

      // runtime address = Static address recorded in the DWARF + Module load
      // base address
      uint64_t RuntimeAddr = StaticAddr + LoadBias;
      Result.LocationType =
          "Global Memory (0x" + llvm::Twine::utohexstr(RuntimeAddr).str() + ")";

      // memory load is not absolutely right.so not dump memory to load val
      Result.Status = EvalStatus::MemoryFault;
      Result.FormattedValue =
          "need dump memory, can load [ " + formatHex(RuntimeAddr) + " ]";
      return Result;
    }

    // 6. DW_OP_entry_value need backtrace (Modified to use BackwardSlicer)
    if (Opcode == dwarf::DW_OP_entry_value ||
        Opcode == dwarf::DW_OP_GNU_entry_value) {
      Result.LocationType = "Register (Recovered By BackTrace)";

      uint64_t SubExprLen = Op.getRawOperand(0);

      // get the start pointer of the sub-expression.
      const uint8_t *SubExprPtr = ExprData.data() + Op.getOperandEndOffset(0);
      ArrayRef<uint8_t> SubExprData(SubExprPtr, SubExprLen);

      // Parse the subexpression and search for the reg num
      DataExtractor SubData(
          SubExprData, VarDie.getDwarfUnit()->getContext().isLittleEndian(), 8);
      DWARFExpression SubExpr(SubData,
                              VarDie.getDwarfUnit()->getAddressByteSize());

      std::optional<uint16_t> TargetReg;
      for (auto &SubOp : SubExpr) {
        uint8_t SubCode = SubOp.getCode();
        // entry_value:DW_OP_reg0 - DW_OP_reg31
        if (SubCode >= dwarf::DW_OP_reg0 && SubCode <= dwarf::DW_OP_reg31) {
          TargetReg = SubCode - dwarf::DW_OP_reg0;
          break;
        }
      }

      if (TargetReg) {
        Result.LocationType = "Register x" + std::to_string(*TargetReg);
      } else {
        Result.LocationType = "Unknown Register";
        return Result;
      }

      std::optional<uint64_t> RecoveredVal = std::nullopt;

      if (ThreadCtx) {
        if (auto InitialValOpt = ThreadCtx->getRegister(*TargetReg)) {
          if (Slicer) {
            RecoveredVal = Slicer->sliceAndRecover(
                *TargetReg, CrashPC, FuncLowPC, InstStream, *InitialValOpt);
          }
        }
      }

      if (RecoveredVal) {
        Result.Status = EvalStatus::Resolved;
        Result.FormattedValue = formatHex(*RecoveredVal);
      } else {
        Result.Status = EvalStatus::NeedsMCAnalysis;
        Result.FormattedValue = "Recovery Failed: Target Lost";
      }
      return Result;
    }

    // 7. Extracting inline integer constants (DW_OP_const* )
    if ((Opcode >= dwarf::DW_OP_lit0 && Opcode <= dwarf::DW_OP_lit31) ||
        Opcode == dwarf::DW_OP_const1u || Opcode == dwarf::DW_OP_const1s ||
        Opcode == dwarf::DW_OP_const2u || Opcode == dwarf::DW_OP_const2s ||
        Opcode == dwarf::DW_OP_const4u || Opcode == dwarf::DW_OP_const4s ||
        Opcode == dwarf::DW_OP_const8u || Opcode == dwarf::DW_OP_const8s ||
        Opcode == dwarf::DW_OP_constu || Opcode == dwarf::DW_OP_consts) {

      Result.Status = EvalStatus::Resolved;
      Result.LocationType = "Inline Constant";

      if (Opcode >= dwarf::DW_OP_lit0 && Opcode <= dwarf::DW_OP_lit31) {
        // DW_OP_litX is directly encoded in the opcode (0-31).
        Result.FormattedValue = std::to_string(Opcode - dwarf::DW_OP_lit0);
      } else {
        // Other const operation codes, which have been parsed by LLVM and
        // stored in operand 0.
        uint64_t Val = Op.getRawOperand(0);
        Result.FormattedValue =
            std::to_string(Val) + " (" + formatHex(Val) + ")";
      }
      return Result;
    }

    // 8. Extract implicit values
    if (Opcode == dwarf::DW_OP_implicit_value) {
      Result.Status = EvalStatus::Resolved;
      Result.LocationType = "Implicit Block Constant";

      // Operand 0 is the length of the data block.
      uint64_t DataLen = Op.getRawOperand(0);

      // Get the start pointer in ExprData.
      const uint8_t *DataPtr = ExprData.data() + Op.getOperandEndOffset(0);

      if (DataLen > 0 && DataLen <= 8) {
        uint64_t Val = 0;

        // If the length is shorter than 8 bytes, it is most likely a basic num
        // type (int, float, or double).
        for (uint64_t I = 0; I < DataLen; ++I) {
          Val |= static_cast<uint64_t>(DataPtr[I]) << (I * 8);
        }

        // Using the length to deduce the specific data type.
        if (DataLen == 4) {
          float FVal;
          // Copy the assembled memory bits directly to float.
          std::memcpy(&FVal, &Val, 4);
          Result.FormattedValue =
              formatHex(Val) + " (float: " + std::to_string(FVal) + ")";
        } else if (DataLen == 8) {
          double DVal;
          std::memcpy(&DVal, &Val, 8);
          Result.FormattedValue =
              formatHex(Val) + " (double: " + std::to_string(DVal) + ")";
        } else {
          Result.FormattedValue = formatHex(Val);
        }
      } else {
        // if longer than 8 bytes, print in hex
        std::string HexStr = "0x";
        for (uint64_t I = 0; I < DataLen; ++I) {
          char Buf[8];
          // reverse order
          snprintf(Buf, sizeof(Buf), "%02x", DataPtr[DataLen - 1 - I]);
          HexStr += Buf;
        }
        Result.FormattedValue = HexStr;
      }

      return Result;
    }
  }

  // Processes the register data that is temporarily stored after being parsed
  // by DW_OP_reg0 and DW_OP_piece.
  if (PendingStatus == EvalStatus::Resolved && PendingValue) {
    Result.Status = PendingStatus;
    Result.LocationType = PendingLocation;
    Result.FormattedValue = formatHex(*PendingValue);
    return Result;
  }

  Result.Status = EvalStatus::OptimizedOut;
  Result.FormattedValue = "<unsupported dwarf opcode>";
  return Result;
}

uint64_t ValueEvaluator::getVariableByteSize(DWARFDie VarDie) {
  // Default size: 8 bytes
  uint64_t DefaultSize = 8;

  // Use the variable's own byte size if present
  if (auto SizeOpt = dwarf::toUnsigned(VarDie.find(dwarf::DW_AT_byte_size))) {
    return *SizeOpt;
  }

  // Follow the DW_AT_type chain
  DWARFDie TypeDie = VarDie.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
  int MaxDepth = 10;

  while (TypeDie.isValid() && MaxDepth-- > 0) {
    if (auto SizeOpt =
            dwarf::toUnsigned(TypeDie.find(dwarf::DW_AT_byte_size))) {
      return *SizeOpt;
    }

    auto Tag = TypeDie.getTag();

    // Handle array types: size = element count * element size
    if (Tag == dwarf::DW_TAG_array_type) {
      uint64_t TotalElements = 1;
      bool HasSubrange = false;

      // Traverse all children (DW_TAG_subrange_type) to support
      // multi-dimensional arrays
      for (DWARFDie Child = TypeDie.getFirstChild(); Child.isValid();
           Child = Child.getSibling()) {
        if (Child.getTag() == dwarf::DW_TAG_subrange_type) {
          HasSubrange = true;
          uint64_t DimCount = 0;

          // Prefer DW_AT_count (Fortran-style)
          if (auto CountOpt =
                  dwarf::toUnsigned(Child.find(dwarf::DW_AT_count))) {
            DimCount = *CountOpt;
          }
          // Fall back to DW_AT_upper_bound (C/C++ style)
          else if (auto UBOpt = dwarf::toUnsigned(
                       Child.find(dwarf::DW_AT_upper_bound))) {
            DimCount = *UBOpt + 1;
          }

          TotalElements *= DimCount;
        }
      }

      if (!HasSubrange || TotalElements == 0) {
        break; // incomplete array type
      }

      // Get the element type
      DWARFDie ElemType =
          TypeDie.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
      if (!ElemType.isValid()) {
        break;
      }

      // Recursively compute the element size
      uint64_t ElemSize = getVariableByteSize(ElemType);
      if (ElemSize == 0) {
        break;
      }

      return TotalElements * ElemSize;
    }

    // Base types (int, char, float, double, etc.) – return their byte size
    // directly
    if (Tag == dwarf::DW_TAG_base_type) {
      if (auto SizeOpt =
              dwarf::toUnsigned(TypeDie.find(dwarf::DW_AT_byte_size))) {
        return *SizeOpt;
      }
      // As a last resort, deduce size from type name (unlikely with modern
      // compilers)
      if (auto TypeNameOpt = dwarf::toString(TypeDie.find(dwarf::DW_AT_name))) {
        llvm::StringRef TypeName(*TypeNameOpt);
        if (TypeName == "char" || TypeName == "unsigned char" ||
            TypeName == "signed char")
          return 1;
        if (TypeName == "short" || TypeName == "unsigned short")
          return 2;
        if (TypeName == "int" || TypeName == "unsigned int" ||
            TypeName == "long")
          return 4;
        if (TypeName == "float")
          return 4;
        if (TypeName == "double" || TypeName == "long long")
          return 8;
      }
      break; // unable to determine size
    }

    // Pointer / reference types are fixed to 8 bytes on 64-bit
    if (Tag == dwarf::DW_TAG_pointer_type ||
        Tag == dwarf::DW_TAG_reference_type ||
        Tag == dwarf::DW_TAG_rvalue_reference_type) {
      return 8;
    }

    // For typedef, const, volatile – continue to the underlying type
    if (Tag == dwarf::DW_TAG_typedef || Tag == dwarf::DW_TAG_const_type ||
        Tag == dwarf::DW_TAG_volatile_type) {
      TypeDie = TypeDie.getAttributeValueAsReferencedDie(dwarf::DW_AT_type);
      continue;
    }

    break; // unhandled type tag
  }

  // Return default size if nothing found
  return DefaultSize;
}

// Go to the program segment of the subroutine and extract the frame base
// register from DW_AT_frame_base. Return the DWARF register number. If the
// operation fails, std::nullopt returned.
std::optional<uint16_t>
ValueEvaluator::getFrameBaseRegister(llvm::DWARFDie Die) {
  if (!Die.isValid())
    return std::nullopt;

  // Walk up to the owning subprogram
  llvm::DWARFDie FuncDie = Die;
  while (FuncDie.isValid() &&
         FuncDie.getTag() != llvm::dwarf::DW_TAG_subprogram) {
    FuncDie = FuncDie.getParent();
  }
  if (!FuncDie.isValid())
    return std::nullopt;

  auto FrameBaseAttr = FuncDie.find(llvm::dwarf::DW_AT_frame_base);
  if (!FrameBaseAttr)
    return std::nullopt;

  auto BlockOpt = llvm::dwarf::toBlock(FrameBaseAttr);
  if (!BlockOpt)
    return std::nullopt;

  llvm::ArrayRef<uint8_t> ExprBytes = *BlockOpt;
  llvm::DataExtractor Data(
      ExprBytes, FuncDie.getDwarfUnit()->getContext().isLittleEndian(),
      FuncDie.getDwarfUnit()->getAddressByteSize());
  llvm::DWARFExpression Expr(Data,
                             FuncDie.getDwarfUnit()->getAddressByteSize());

  for (auto &Op : Expr) {
    uint8_t Opcode = Op.getCode();
    if (Opcode >= llvm::dwarf::DW_OP_reg0 &&
        Opcode <= llvm::dwarf::DW_OP_reg31) {
      return Opcode - llvm::dwarf::DW_OP_reg0;
    }
    // For more complex forms like DW_OP_call_frame_cfa, we don't handle them
    // here.
    break;
  }
  return std::nullopt;
}

} // namespace evaluator
} // namespace crash_analyzer
#endif // OHOS_LLVM
