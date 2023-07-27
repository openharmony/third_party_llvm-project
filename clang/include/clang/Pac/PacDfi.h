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
#ifndef PAC_DFI_H
#define PAC_DFI_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Decl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include <functional>
using namespace clang;

void PacDfiParseStruct(RecordDecl *TagDecl, ASTContext &Ctx, DiagnosticsEngine &Diags);
void PacDfiEmitStructFieldsMetadata(llvm::Module &M, CodeGen::CodeGenModule *CGM,
    std::function<unsigned(CodeGen::CodeGenModule&, const RecordDecl*, const FieldDecl*)> func);
void PacDfiRecordDecl2StructName(const RecordDecl *RD, llvm::StructType *Entry);

#endif

