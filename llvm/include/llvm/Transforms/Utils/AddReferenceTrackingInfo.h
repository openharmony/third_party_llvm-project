#ifdef OHOS_LLVM
//===- AddReferenceTrackingInfo.h - Reference-tracking metadata on stores -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declares the reference-tracking pass and helpers that attach "memtracer"
// metadata (name, typename) to store instructions and related sites, using
// debug info when available.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_ADDREFERENCETRACKINGINFO_H
#define LLVM_TRANSFORMS_UTILS_ADDREFERENCETRACKINGINFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/PassManager.h"
#include <cstddef>
#include <string>

namespace llvm {

class Function;
class Instruction;
class StoreInst;

// Describes debug info for a value (from dbg.declare / dbg.addr): variable
// name, pointer depth, base type tag/name, and composite type members.
// Name and BasicTypeName use std::string for concatenation and storage.
struct ReferenceDbgInfo {
  std::string Name;
  size_t PtrDepth = 0;
  dwarf::Tag BasicTypeTag = dwarf::DW_TAG_null;
  size_t ArrayDims = 0;
  std::string BasicTypeName;
  MDNode *MemberElements = nullptr; // DINodeArray for struct/class members
};

// Attach (name, typename) as "memtracer" metadata to \p I.
// \p I may be a StoreInst or any Instruction (e.g. allocation call).
// If both Name and TypeName are empty, no metadata is set.
void setReferenceInfo(Instruction *I, const std::string &Name,
                      const std::string &TypeName);

// Read "memtracer" from \p SI into \p Name and \p TypeName.
// \return true if metadata was present and valid (two MDStrings).
bool getReferenceInfo(const StoreInst *SI, std::string &Name,
                      std::string &TypeName);

class AddReferenceTrackingInfoPass
    : public PassInfoMixin<AddReferenceTrackingInfoPass> {
public:
  static bool isRequired() { return true; }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_ADDREFERENCETRACKINGINFO_H
#endif // OHOS_LLVM
