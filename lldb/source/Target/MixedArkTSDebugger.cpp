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

static const char* BackTrace = "(const char*)GetJsBacktrace()";
static const char* DebugMessage = "(const char*)OperateJsDebugMessage(\"{0}\")";

MixedArkTSDebugger::MixedArkTSDebugger(const TargetSP &target_sp)
    : MixedDebugger(target_sp) {}

DataExtractorSP MixedArkTSDebugger::GetCurrentThreadBackTrace(Status &error) {
  DataExtractorSP result = ExecuteAction(BackTrace, error);
  if (!error.Success()) {
    Log *log = GetLog(LLDBLog::MixedDebugger);
    LLDB_LOGF(log, "[MixedArkTSDebugger::GetBackTrace] failed for %s",
              error.AsCString());
  }
  return result;
}

DataExtractorSP MixedArkTSDebugger::GetCurrentThreadOperateDebugMessageResult(const char *message, Status &error) {
  std::string operateMessage = llvm::formatv(DebugMessage, message).str();
  DataExtractorSP result = ExecuteAction(operateMessage.c_str(), error);
  if (!error.Success()) {
    Log *log = GetLog(LLDBLog::MixedDebugger);
    LLDB_LOGF(log, "[MixedArkTSDebugger::OperateDebugMessage] failed for %s",
              error.AsCString());
  }
  return result;
}