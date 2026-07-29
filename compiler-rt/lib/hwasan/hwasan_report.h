//===-- hwasan_report.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file is a part of HWAddressSanitizer. HWASan-private header for error
/// reporting functions.
///
//===----------------------------------------------------------------------===//

#ifndef HWASAN_REPORT_H
#define HWASAN_REPORT_H

#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_stacktrace.h"
#if SANITIZER_OHOS
#include "sanitizer_common/sanitizer_procmaps.h"
#endif /* SANITIZER_OHOS */

namespace __hwasan {

void ReportStats();
void ReportTagMismatch(StackTrace *stack, uptr addr, uptr access_size,
                       bool is_store, bool fatal, uptr *registers_frame);
void ReportInvalidFree(StackTrace *stack, uptr addr);
void ReportTailOverwritten(StackTrace *stack, uptr addr, uptr orig_size,
                           const u8 *expected);
void ReportRegisters(const uptr *registers_frame, uptr pc);
#if SANITIZER_OHOS
void ReportMemoryNearRegisters(const uptr *registers_frame, uptr sp, uptr pc);
void PrintMemoryAroundAddress(__sanitizer::MemoryMappingLayout &proc_maps,
                              int reg_num, uptr addr, uptr len,
                              bool is_sp = false, bool is_pc = false);
#endif /* SANITIZER_OHOS */
void ReportAtExitStatistics();


}  // namespace __hwasan

#endif  // HWASAN_REPORT_H
