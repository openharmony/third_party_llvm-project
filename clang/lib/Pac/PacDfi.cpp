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
#include "llvm/IR/Metadata.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include "clang/Basic/DiagnosticParse.h"
using namespace clang;

std::map<const RecordDecl*, StringRef> RecordDecl2StructName;
std::map<RecordDecl*, std::vector<FieldDecl*>> PacPtrNameInfos;
std::map<RecordDecl*, std::vector<FieldDecl*>> PacFieldNameInfos;
std::map<const RecordDecl*, llvm::StructType*> RecordDecl2StructType;
void PacDfiParseStruct(RecordDecl *TagDecl, ASTContext &Ctx, DiagnosticsEngine &Diags)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    // find pac_tag attr fields, and insert new fields
    std::vector<FieldDecl*> PacPtrNames;
    std::vector<FieldDecl*> PacFieldNames;
    unsigned int ArraySize = 0;

    for (auto *Field : TagDecl->fields()) {
        auto ElemTy = Field->getType();
        if (Field->hasAttr<PacDataTagAttr>()) {
            unsigned Inc = 1;
            // if Field is array type ElemTy is equal to array element type.
            if (Field->getType()->isConstantArrayType()) {
                auto *ArrayTy = llvm::dyn_cast<ConstantArrayType>(ElemTy);
                ElemTy = ArrayTy->getElementType();
                Inc = ArrayTy->getSize().getZExtValue();
            }
            // pac_protected_data [not] support structure type or structure array.
            if (TagDecl->isUnion()) {
                Diags.Report(Field->getLocation(), diag::warn_unsupported_pac_dfi_type) << "union"
                    << Field->getAttr<PacDataTagAttr>()->getSpelling();
                continue;
            }
            if (ElemTy->isStructureOrClassType()) {
                Diags.Report(Field->getLocation(), diag::warn_unsupported_pac_dfi_type) << Field->getType()
                    << Field->getAttr<PacDataTagAttr>()->getSpelling();
                continue;
            }
            ArraySize += Inc;
            PacFieldNames.push_back(Field);
        } else if (Field->hasAttr<PacPtrTagAttr>()) {
            // pac_protected_ptr [only] support pointer type.
            if (!ElemTy->isAnyPointerType()) {
                Diags.Report(Field->getLocation(), diag::warn_unsupported_pac_dfi_type) << Field->getType()
                    << Field->getAttr<PacPtrTagAttr>()->getSpelling();
                continue;
            }
            PacPtrNames.push_back(Field);
        }
    }

    if (!PacFieldNames.empty()) {
        llvm::APInt ArraySizeInt(32, ArraySize);
        auto ArrayTy = Ctx.getConstantArrayType(Ctx.IntTy, ArraySizeInt, nullptr, ArrayType::Normal,
            /*IndexTypeQuals=*/ 0);
        FieldDecl *PacFD = FieldDecl::Create(Ctx, TagDecl, SourceLocation(), SourceLocation(), nullptr,
            ArrayTy, nullptr, nullptr, true, ICIS_NoInit);

        TagDecl->addDecl(PacFD);
        PacFieldNameInfos.insert(std::make_pair(TagDecl, PacFieldNames));
    }
    if (!PacPtrNames.empty()) {
        PacPtrNameInfos.insert(std::make_pair(TagDecl, PacPtrNames));
    }
}

void PacDfiCreateMetaData(std::map<RecordDecl*, std::vector<FieldDecl*>> &fieldInfos, StringRef mdName,
    llvm::Module &M, CodeGen::CodeGenModule *CGM,
    std::function<unsigned(CodeGen::CodeGenModule&, const RecordDecl*, const FieldDecl*)> func)
{
    llvm::NamedMDNode *PacNMD = M.getOrInsertNamedMetadata(mdName);
    llvm::NamedMDNode *PacNMDName = M.getOrInsertNamedMetadata(mdName.str() + "name");
    for (auto item : fieldInfos) {
        if (RecordDecl2StructName.find(item.first) == RecordDecl2StructName.end()) {
            continue;
        }
        std::vector<llvm::Metadata *> PacFields;
        std::vector<llvm::Metadata *> PacFieldsName;
        auto styName = RecordDecl2StructName.find(item.first)->second;
        PacFields.push_back(llvm::MDString::get(M.getContext(), styName));
        PacFieldsName.push_back(llvm::MDString::get(M.getContext(), styName));
        for (auto *Field : item.second) {
            PacFieldsName.push_back(llvm::MDString::get(M.getContext(), Field->getName()));
            unsigned idx = func(*CGM, item.first, Field);
            PacFields.push_back(llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(M.getContext()), idx)));
        }
        PacNMD->addOperand(llvm::MDNode::get(M.getContext(), PacFields));
        PacNMDName->addOperand(llvm::MDNode::get(M.getContext(), PacFieldsName));
    }
}

void PacDfiEmitStructFieldsMetadata(llvm::Module &M, CodeGen::CodeGenModule *CGM,
    std::function<unsigned(CodeGen::CodeGenModule&, const RecordDecl*, const FieldDecl*)> func)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    // emit struct fields that need to protect with PA
    if (!PacFieldNameInfos.empty()) {
        PacDfiCreateMetaData(PacFieldNameInfos, "pa_field_info", M, CGM, func);
    }
    if (!PacPtrNameInfos.empty()) {
        PacDfiCreateMetaData(PacPtrNameInfos, "pa_ptr_field_info", M, CGM, func);
    }
}

void PacDfiRecordDecl2StructName(const RecordDecl *RD, llvm::StructType *Entry)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }
    if (Entry->isLiteral()) {
        return;
    }
    RecordDecl2StructName.insert(std::make_pair(RD, Entry->getName()));
}