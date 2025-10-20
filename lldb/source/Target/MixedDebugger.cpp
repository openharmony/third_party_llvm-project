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
      // Evaluate expression in the target.
      lldb::ExpressionResults expr_re =
          target->EvaluateExpression(expr, frame, expr_value_sp);
      if (expr_re) {
        error.SetErrorStringWithFormat(
            "[MixedDebugger::ExecuteAction] exe %s failed: %d", expr, expr_re);
        return result;
      }

      ValueObjectSP size_vo = expr_value_sp->GetChildAtIndex(0, true);
      ValueObjectSP data_ptr_vo = expr_value_sp->GetChildAtIndex(1, true);

      if (!data_ptr_vo || !size_vo) {
        error.SetErrorString(
            "[ExecuteAction] Failed to get DebugResponse members");
        LLDB_LOGF(log,
                  "[ExecuteAction] struct members missing: data=%p, size=%p",
                  data_ptr_vo.get(), size_vo.get());
        return result;
      }
      // Validate payload length.
      size_t payload_len = size_vo->GetValueAsUnsigned(0);
      if (payload_len == 0 || payload_len >= UINT32_MAX) {
        error.SetErrorString("[ExecuteAction] Invalid payload size");
        LLDB_LOGF(log, "[ExecuteAction] Invalid payload size: %zu",
                  payload_len);
        return result;
      }
      // Read bytes from target memory pointed by `data`.
      DataExtractor data;
      size_t bytes_read = data_ptr_vo->GetPointeeData(data, 0, payload_len);
      if (bytes_read < payload_len) {
        error.SetErrorString(
            "[ExecuteAction] Failed to read data from pointer");
        LLDB_LOGF(log, "[ExecuteAction] Failed to read expected payload_len");
        return result;
      }

      if (!result->Append(const_cast<uint8_t *>(data.GetDataStart()),
                          payload_len)) {
        error.SetErrorString("[ExecuteAction] Failed to append result data");
        LLDB_LOGF(log, "[ExecuteAction] Failed to append result data");
        result->Clear();
        return result;
      }

      char terminator = '\0';
      if (!result->Append(&terminator, 1)) {
        error.SetErrorString(
            "[MixedDebugger::ExecuteAction] result append terminator failed");
        result->Clear();
        return result;
      }

      // ArkTS allocates 'data' with new[] in the target process.
      // Free it in the target (not this process) after we've copied the bytes.
      lldb::addr_t data_addr = data_ptr_vo->GetValueAsUnsigned(0);
      if (data_addr != 0) {
        ValueObjectSP free_result;
        std::string free_expr =
            llvm::formatv("delete[] (char*){0};", data_addr).str();
        lldb::ExpressionResults free_re = target->EvaluateExpression(
            free_expr.c_str(), frame, free_result);
        if (free_re) {
          LLDB_LOGF(
              log,
              "[MixedDebugger::ExecuteAction] delete[] at 0x%llx failed: %d",
              (unsigned long long)data_addr, (int)free_re);
        } else {
          LLDB_LOGF(
              log,
              "[MixedDebugger::ExecuteAction] freed target buffer at 0x%llx",
              (unsigned long long)data_addr);
        }
      } else {
        LLDB_LOGF(log, "[MixedDebugger::ExecuteAction] data pointer "
                       "invalid/zero or empty payload; skip free");
      }
    }
  }
  LLDB_LOGF(log,
            "[MixedDebugger::ExecuteAction] result is "
            "%s",
            result->PeekCStr(0));
  return result;
}
