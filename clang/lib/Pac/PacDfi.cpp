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
std::map<RecordDecl*, std::vector<FieldDecl*>> PacFieldExclNameInfos;
std::map<RecordDecl*, std::vector<FieldDecl*>> PacFieldSeqlNameInfos;
std::map<const RecordDecl*, llvm::StructType*> RecordDecl2StructType;

bool IsBaseTypeNotArrStruct(RecordDecl *TagDecl, FieldDecl *Field, DiagnosticsEngine &Diags)
{
    if (Field->getType()->isConstantArrayType()) {
        Diags.Report(Field->getLocation(), diag::warn_unsupported_pac_dfi_type) << "array"
            << Field->getAttr<PacExclTagAttr>()->getSpelling();
        return false;
    }
    return true;
}

void AppendFieldDecl(RecordDecl *TagDecl, FieldDecl *TargetField, FieldDecl *NewField)
{
    const RecordDecl::field_iterator TargetFieldIt = std::find(TagDecl->field_begin(), TagDecl->field_end(), TargetField);
    if (TargetFieldIt != TagDecl->field_end()) {
        llvm::SmallVector<FieldDecl *, 16> Fields;

        for (FieldDecl *TmpFD :TagDecl->fields()) {
            Fields.push_back(TmpFD);
        }
        for (FieldDecl *TmpFD : Fields) {
            TagDecl->removeDecl(TmpFD);
        }
        for (FieldDecl *TmpFD : Fields) {
            TagDecl->addDecl(TmpFD);
            if (TmpFD == TargetField) {
                TagDecl->addDecl(NewField);
            }
        }
    }
}

void AppendPacFieldDecl(RecordDecl *TagDecl, FieldDecl *Field, ASTContext &Ctx)
{
    auto IntType = Ctx.getBitIntType(true, 64);
    FieldDecl *PacFD = FieldDecl::Create(Ctx, TagDecl, SourceLocation(), SourceLocation(),
        nullptr, IntType, nullptr, nullptr, true, ICIS_NoInit);
    llvm::APInt PacAlign(32, 16);
    Field->addAttr(AlignedAttr::CreateImplicit(
        Ctx,
        true,
        IntegerLiteral::Create(Ctx, PacAlign, Ctx.IntTy,
            SourceLocation()),
        {},
        AttributeCommonInfo::AS_GNU,
        AlignedAttr::GNU_aligned));
    AppendFieldDecl(TagDecl, Field, PacFD);
}

void AppendPacArrayFieldDecl(RecordDecl *TagDecl, FieldDecl *Field, unsigned ArySize, ASTContext &Ctx)
{
    auto Int64Ty = Ctx.getBitIntType(true, 64);
    auto Int64ArrayTy = Ctx.getConstantArrayType(Int64Ty, llvm::APInt(64, ArySize),
        nullptr, ArrayType::Normal, 0);
    FieldDecl *PacArrayFD = FieldDecl::Create(Ctx, TagDecl, SourceLocation(), SourceLocation(),
        nullptr, Int64ArrayTy, nullptr, nullptr, true, ICIS_NoInit);

    AppendFieldDecl(TagDecl, Field, PacArrayFD);
}

void PacDfiParseStruct(RecordDecl *TagDecl, ASTContext &Ctx, DiagnosticsEngine &Diags)
{
    if (!llvm::PARTS::useDataFieldTag()) {
        return;
    }

    // find pac_tag attr fields, and insert new fields
    std::vector<FieldDecl*> PacPtrNames;
    std::vector<FieldDecl*> PacFieldNames;
    std::vector<FieldDecl*> PacFieldExclNames;
    std::vector<FieldDecl*> PacFieldSeqlNames;
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
            if (Field->hasAttr<PacExclTagAttr>()) {
                if (!IsBaseTypeNotArrStruct(TagDecl, Field, Diags)) {
                    continue;
                }
                AppendPacFieldDecl(TagDecl, Field, Ctx);
                PacFieldExclNames.push_back(Field);
                continue;
            }
            if (Field->hasAttr<PacSeqlTagAttr>()) {
                AppendPacArrayFieldDecl(TagDecl, Field, Inc, Ctx);
                PacFieldSeqlNames.push_back(Field);
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
    if (!PacFieldExclNames.empty()) {
        PacFieldExclNameInfos.insert(std::make_pair(TagDecl, PacFieldExclNames));
    }
    if (!PacFieldSeqlNames.empty()) {
        PacFieldSeqlNameInfos.insert(std::make_pair(TagDecl, PacFieldSeqlNames));
    }
}

bool getStructNameFromRecord(const RecordDecl *Record, StringRef &Name) {
    auto Item = RecordDecl2StructName.find(Record);
    if (Item != RecordDecl2StructName.end()) {
        Name = Item->second;
        return true;
    }
    auto PreRecord = Record->getPreviousDecl();
    while (PreRecord) {
        auto PreItem = RecordDecl2StructName.find(PreRecord);
        if (PreItem != RecordDecl2StructName.end()) {
            Name = PreItem->second;
            return true;
        }
        PreRecord = PreRecord->getPreviousDecl();
    }
    return false;
}

void pacDfiCreateNameMetaData(
    std::map<RecordDecl *, std::vector<FieldDecl *>> &FieldInfos,
    StringRef MDName, llvm::Module &M, CodeGen::CodeGenModule *CGM,
    std::function<unsigned(CodeGen::CodeGenModule &, const RecordDecl *,
                           const FieldDecl *)>
        Func) {
    llvm::NamedMDNode *PacNMD = M.getOrInsertNamedMetadata(MDName);
    llvm::NamedMDNode *PacNMDName =
        M.getOrInsertNamedMetadata(MDName.str() + "name");
    for (auto Item : FieldInfos) {
        std::vector<llvm::Metadata *> PacFields;
        std::vector<llvm::Metadata *> PacFieldsName;
        StringRef StyName;
        if (!getStructNameFromRecord(Item.first, StyName)) {
            continue;
        }
        PacFields.push_back(llvm::MDString::get(M.getContext(), StyName));
        PacFieldsName.push_back(llvm::MDString::get(M.getContext(), StyName));
        for (auto *Field : Item.second) {
            PacFieldsName.push_back(
                llvm::MDString::get(M.getContext(), Field->getName()));
            unsigned Idx = Func(*CGM, Item.first, Field);
            PacFields.push_back(
                llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(M.getContext()), Idx)));
        }
        PacNMD->addOperand(llvm::MDNode::get(M.getContext(), PacFields));
        PacNMDName->addOperand(
            llvm::MDNode::get(M.getContext(), PacFieldsName));
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
        pacDfiCreateNameMetaData(PacFieldNameInfos, "pa_field_info", M, CGM,
                                 func);
    }
    if (!PacPtrNameInfos.empty()) {
        pacDfiCreateNameMetaData(PacPtrNameInfos, "pa_ptr_field_info", M, CGM,
                                 func);
    }
    if (!PacFieldExclNameInfos.empty()) {
        pacDfiCreateNameMetaData(PacFieldExclNameInfos, "pa_excl_field_info", M, CGM, func);
    }
    if (!PacFieldSeqlNameInfos.empty()) {
        pacDfiCreateNameMetaData(PacFieldSeqlNameInfos, "pa_seql_field_info", M, CGM, func);
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

