//===-- XVMCFGSort.cpp - CFG Sorting ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements a CFG sorting pass.
///
/// This pass reorders the blocks in a function to put them into topological
/// order, ignoring loop backedges, and without any loop or exception being
/// interrupted by a block not dominated by the its header, with special care
/// to keep the order as similar as possible to the original order.
///
////===----------------------------------------------------------------------===//
#ifdef XVM_DYLIB_MODE
// Insert the XVM backend code here

#endif