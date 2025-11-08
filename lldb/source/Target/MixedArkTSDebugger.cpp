//===-- MixedArkTSDebugger.cpp --------------------------------------------===//
//
// Copyright (C) 2024 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/MixedArkTSDebugger.h"
#include "lldb/Utility/LLDBLog.h"

using namespace lldb;
using namespace lldb_private;

// New ArkTS-side interface: returns DebugResponse{ size_t size; char *response; }.
static const char *kBacktraceNew =
    "struct DebugResponse{ size_t size; char *response; }; "
    "(DebugResponse)GetJsBacktraceV1()";

static const char *kDebugMsgNewFmt =
    "(DebugResponse)OperateJsDebugMessageV1(\"{0}\")";

// Legacy ArkTS-side interface: returns a null-terminated C string (const char*).
static const char *kBacktraceOld = "(const char*)GetJsBacktrace()";
static const char *kDebugMsgOldFmt =
    "(const char*)OperateJsDebugMessage(\"{0}\")";

MixedArkTSDebugger::MixedArkTSDebugger(const TargetSP &target_sp)
    : MixedDebugger(target_sp) {}

DataExtractorSP MixedArkTSDebugger::GetCurrentThreadBackTrace(Status &error) {
  Log *log = GetLog(LLDBLog::Process);

  // First try the new interface (DebugResponse).
  Status new_err;
  DataExtractorSP new_res = ExecuteAction(kBacktraceNew, new_err);
  if (new_err.Success()) {
    error = new_err;
    return new_res;
  }

  // Fallback to the legacy interface (const char*).
  Status old_err;
  DataExtractorSP old_res = ExecuteAction(kBacktraceOld, old_err);
  if (old_err.Success()) {
    error = old_err;
    LLDB_LOGF(log,
              "[MixedArkTSDebugger::GetBackTrace] new path failed, old path "
              "succeeded. new_err=%s",
              new_err.AsCString());
    return old_res;
  }

  // Both attempts failed: keep the new interface error for diagnosis.
  error = new_err;
  LLDB_LOGF(log,
            "[MixedArkTSDebugger::GetBackTrace] both paths failed. new_err=%s, "
            "old_err=%s",
            new_err.AsCString(), old_err.AsCString());
  return new_res; // Likely empty; 'error' already contains failure info.
}

DataExtractorSP MixedArkTSDebugger::GetCurrentThreadOperateDebugMessageResult(const char *message, Status &error) {
  Log *log = GetLog(LLDBLog::Process);

  // Build both expressions (new and legacy).
  std::string expr_new =
      "struct DebugResponse{ size_t size; char *response; }; " +
      llvm::formatv(kDebugMsgNewFmt, message).str();
  std::string expr_old = llvm::formatv(kDebugMsgOldFmt, message).str();

  // First try the new interface (DebugResponse).
  Status new_err;
  DataExtractorSP new_res = ExecuteAction(expr_new.c_str(), new_err);
  if (new_err.Success()) {
    error = new_err;
    return new_res;
  }

  // Fallback to the legacy interface (const char*).
  Status old_err;
  DataExtractorSP old_res = ExecuteAction(expr_old.c_str(), old_err);
  if (old_err.Success()) {
    error = old_err;
    LLDB_LOGF(log,
              "[MixedArkTSDebugger::OperateDebugMessage] new path failed, old "
              "path succeeded. new_err=%s",
              new_err.AsCString());
    return old_res;
  }

  // Both attempts failed: keep the new interface error for diagnosis.
  error = new_err;
  LLDB_LOGF(log,
            "[MixedArkTSDebugger::OperateDebugMessage] both paths failed. "
            "new_err=%s, old_err=%s",
            new_err.AsCString(), old_err.AsCString());
  return new_res;
}