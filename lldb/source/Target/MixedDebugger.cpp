//===-- MixedDebugger.cpp -------------------------------------------------===//
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

#include "lldb/Target/MixedDebugger.h"
#include "lldb/Core/ValueObject.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/DataExtractor.h"

using namespace lldb;
using namespace lldb_private;

MixedDebugger::MixedDebugger(const TargetSP &target_sp) : m_target_sp(target_sp) {}

DataExtractorSP MixedDebugger::ExecuteAction(const char* expr, Status &error) {
  Log *log = GetLog(LLDBLog::MixedDebugger);
  error.Clear();
  ValueObjectSP expr_value_sp;
  TargetSP target_sp(m_target_sp);
  DataExtractorSP result(new DataExtractor());
  
  if (m_target_sp) {
    if (expr == nullptr || expr[0] == 0) {
      error.SetErrorString("Expression is empty");
      return result;
    }

    std::lock_guard<std::recursive_mutex> guard(target_sp->GetAPIMutex());
    ExecutionContext exe_ctx(target_sp.get());

    StackFrame *frame = exe_ctx.GetFramePtr();
    Target *target = exe_ctx.GetTargetPtr();

    if (target) {
      lldb::ExpressionResults expr_re =
          target->EvaluateExpression(expr, frame, expr_value_sp);
      if (expr_re) {
        error.SetErrorStringWithFormat(
            "[MixedDebugger::ExecuteAction] exe %s failed: %d", expr, expr_re);
        return result;
      }

      const size_t k_max_buf_size = 64;
      size_t cstr_len = UINT32_MAX;
      size_t offset = 0;
      size_t bytes_read = 0;
      DataExtractor data;
      while ((bytes_read = expr_value_sp->GetPointeeData(data, offset, k_max_buf_size)) > 0) {
        const char *cstr = data.PeekCStr(0);
        size_t len = strnlen(cstr, k_max_buf_size);
        if (len >= cstr_len) {
          error.SetErrorString("[MixedDebugger::ExecuteAction] result over size");
          result->Clear();
          return result;
        }
        if (!result->Append(const_cast<char*>(cstr), len)) {
          error.SetErrorString("[MixedDebugger::ExecuteAction] result append data failed");
          result->Clear();
          return result;
        }
        if (len < k_max_buf_size) {
          break;
        }
        cstr_len -= len;
        offset += len;
      }

      // Add terminator to result data
      char te = '\0';
      if (!result->Append(&te, 1)) {
        error.SetErrorString("[MixedDebugger::ExecuteAction] result append terminator failed");
        result->Clear();
        return result;
      }
    }
  }
  LLDB_LOGF(log,
            "[MixedDebugger::ExecuteAction] result is "
            "%s", result->PeekCStr(0));
  return result;
}
