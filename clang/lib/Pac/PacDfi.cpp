//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Zaheer Gauhar <zaheer.gauhar@aalto.fi>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/PARTS/Parts.h"
#include "clang/Basic/AttributeCommonInfo.h"
#include "clang/Basic/Attributes.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "clang/Pac/PacDfi.h"

using namespace clang;

std::map<const RecordDecl*, StringRef> RecordDecl2StructName;
std::map<RecordDecl*, std::vector<unsigned int>> PacFieldInfos;
std::map<RecordDecl*, std::vector<unsigned int>> PacPtrInfos;

void PacDfiParseStruct(RecordDecl *TagDecl, ASTContext &Ctx)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    // find pac_tag attr fields, and insert new fields
    std::vector<unsigned int> PacFieldIdxs;
    std::vector<unsigned int> PacPtrIdxs;
    unsigned int FieldIdx = 0;
    unsigned int ArraySize = 0;

    for (auto Field : TagDecl->fields()) {
        if (Field->hasAttr<PacDataTagAttr>()) {
            PacFieldIdxs.push_back(FieldIdx);
            if (Field->getType()->isConstantArrayType()) {
                auto ArrayTy = dyn_cast<ConstantArrayType>(Field->getType());
                ArraySize += ArrayTy->getSize().getZExtValue();
            } else {
                ++ArraySize;
            }
        } else if (Field->hasAttr<PacPtrTagAttr>()) {
            PacPtrIdxs.push_back(FieldIdx);
        }
        ++FieldIdx;
    }

    if (!PacFieldIdxs.empty()) {
        llvm::APInt ArraySizeInt(32, ArraySize);
        auto ArrayTy = Ctx.getConstantArrayType(Ctx.IntTy, ArraySizeInt, nullptr, ArrayType::Normal,
            /*IndexTypeQuals=*/ 0);
        FieldDecl *PacFD = FieldDecl::Create(Ctx, TagDecl, SourceLocation(), SourceLocation(), nullptr,
            ArrayTy, nullptr, nullptr, true, ICIS_NoInit);

        TagDecl->addDecl(PacFD);
        PacFieldInfos.insert(std::make_pair(TagDecl, PacFieldIdxs));
    }
    if (!PacPtrIdxs.empty()) {
        PacPtrInfos.insert(std::make_pair(TagDecl, PacPtrIdxs));
    }
}

void PacDfiCreateMetaData(std::map<RecordDecl*, std::vector<unsigned int>> &fieldInfos, StringRef mdName,
    llvm::Module &M, llvm::LLVMContext &VMContext)
{
    llvm::NamedMDNode *PacNMD = M.getOrInsertNamedMetadata(mdName);
    for (auto item : fieldInfos) {
        std::vector<llvm::Metadata *> PacFields;
        auto styName = RecordDecl2StructName.find(item.first)->second;
        PacFields.push_back(llvm::MDString::get(VMContext, styName));
        for (auto idx : item.second) {
            PacFields.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(VMContext), idx)));
        }
        PacNMD->addOperand(llvm::MDNode::get(VMContext, PacFields));
    }
}

void PacDfiEmitStructFieldsMetadata(llvm::Module &M, llvm::LLVMContext &VMContext)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    // emit struct fields that need to protect with PA
    if (!PacFieldInfos.empty()) {
        PacDfiCreateMetaData(PacFieldInfos, "pa_field_info", M, VMContext);
    }
    if (!PacPtrInfos.empty()) {
        PacDfiCreateMetaData(PacPtrInfos, "pa_ptr_field_info", M, VMContext);
    }
}

void PacDfiRecordDecl2StructName(const RecordDecl *RD, llvm::StructType *Entry)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    RecordDecl2StructName.insert(std::make_pair(RD, Entry->getName()));
}