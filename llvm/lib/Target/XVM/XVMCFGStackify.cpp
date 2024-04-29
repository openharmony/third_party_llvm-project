//===-- XVMCFGStackify.cpp - CFG Stackification -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a CFG stacking pass.
///
/// This pass inserts BLOCK, LOOP, and TRY markers to mark the start of scopes,
/// since scope boundaries serve as the labels for XVM's control
/// transfers.
///
/// This is sufficient to convert arbitrary CFGs into a form that works on
/// XVM, provided that all loops are single-entry.
///
/// In case we use exceptions, this pass also fixes mismatches in unwind
/// destinations created during transforming CFG into xvm structured format.
///
//===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE
// Insert the XVM backend code here

#endif
