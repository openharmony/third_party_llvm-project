#ifdef OHOS_LLVM

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
#include "lldb/ValueObject/ValueObject.h"
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

  if (!m_target_sp)
    return result;

  if (expr == nullptr || expr[0] == 0) {
    error = Status::FromErrorString("Expression is empty");
    return result;
  }

  std::lock_guard<std::recursive_mutex> guard(target_sp->GetAPIMutex());
  ExecutionContext exe_ctx(target_sp.get());
  StackFrame *frame = exe_ctx.GetFramePtr();
  Target *target = exe_ctx.GetTargetPtr();
  if (!target)
    return result;

  // Evaluate the expression in the target.
  lldb::ExpressionResults expr_re =
      target->EvaluateExpression(expr, frame, expr_value_sp);
  if (expr_re) {
    error = Status::FromErrorStringWithFormat(
        "[MixedDebugger::ExecuteAction] eval \"%s\" failed: %d", expr, expr_re);
    return result;
  }

  // Attempt path A: treat the result as
  //   struct DebugResponse { size_t size; char *response; }.
  auto try_struct_path = [&]() -> bool {
    // Expect child[0] = size, child[1] = response pointer.
    ValueObjectSP size_vo =
        expr_value_sp->GetChildAtIndex(0, /*can_create*/ true);
    ValueObjectSP data_ptr_vo =
        expr_value_sp->GetChildAtIndex(1, /*can_create*/ true);
    if (!size_vo || !data_ptr_vo)
      return false; // Not a DebugResponse-like aggregate.

    size_t payload_len = size_vo->GetValueAsUnsigned(0);
    if (payload_len == 0 || payload_len >= UINT32_MAX) {
      error = Status::FromErrorString("[ExecuteAction] Invalid payload size");
      LLDB_LOGF(log, "[ExecuteAction] invalid payload size: %zu", payload_len);
      return true; // Handled as struct path but invalid content.
    }

    DataExtractor data;
    size_t bytes_read = data_ptr_vo->GetPointeeData(data, 0, payload_len);
    if (bytes_read < payload_len) {
      error = Status::FromErrorString("[ExecuteAction] Failed to read response bytes");
      LLDB_LOGF(log, "[ExecuteAction] short read: %zu/%zu", bytes_read,
                payload_len);
      return true;
    }

    if (!result->Append(const_cast<uint8_t *>(data.GetDataStart()),
                        payload_len)) {
      error = Status::FromErrorString("[ExecuteAction] Failed to append result data");
      result->Clear();
      return true;
    }

    // Append a terminator for convenient C-string access (PeekCStr).
    char te = '\0';
    if (!result->Append(&te, 1)) {
      error = Status::FromErrorString("[ExecuteAction] append terminator failed");
      result->Clear();
      return true;
    }

    // ArkTS allocates the buffer with new[] in the target process.
    // Free it in the target after copying the bytes out.
    lldb::addr_t data_addr = data_ptr_vo->GetValueAsUnsigned(0);
    if (data_addr != 0) {
      ValueObjectSP free_result;
      std::string free_expr =
          llvm::formatv("delete[] (char*){0};", data_addr).str();
      lldb::ExpressionResults free_re =
          target->EvaluateExpression(free_expr.c_str(), frame, free_result);
      if (free_re) {
        LLDB_LOGF(log, "[ExecuteAction] delete[] 0x%llx failed: %d",
                  (unsigned long long)data_addr, (int)free_re);
      } else {
        LLDB_LOGF(log, "[ExecuteAction] freed target buffer at 0x%llx",
                  (unsigned long long)data_addr);
      }
    } else {
      LLDB_LOGF(log,
                "[ExecuteAction] data pointer is null/zero; skip free in target");
    }
    return true;
  };

  // Attempt path B: treat the result as a null-terminated C string (const char*).
  auto try_cstr_ptr_path = [&]() -> bool {
    const size_t kChunk = 64;       // Chunk size for incremental reads.
    size_t limit_left = UINT32_MAX; // Simple upper bound protection.
    size_t offset = 0;
    DataExtractor data;

    while (limit_left > 0) {
      size_t to_read = std::min(kChunk, limit_left);
      size_t bytes = expr_value_sp->GetPointeeData(data, offset, to_read);
      if (bytes == 0)
        break;

      const char *cstr = data.PeekCStr(0);
      size_t len = strnlen(cstr, bytes); // Scan for '\0' within this chunk.
      if (!result->Append(const_cast<char *>(cstr), len)) {
        error = Status::FromErrorString("[ExecuteAction] append data failed");
        result->Clear();
        return true;
      }

      offset += len;
      if (len < to_read) {
        // Found '\0' within this chunk. Append a terminator for PeekCStr.
        char te = '\0';
        if (!result->Append(&te, 1)) {
          error = Status::FromErrorString("[ExecuteAction] append terminator failed");
          result->Clear();
        }
        return true;
      }
      limit_left -= len;
    }

    // Defensive: ensure there is a terminator even if none was seen.
    char te = '\0';
    (void)result->Append(&te, 1);
    // Free it in the target after copying the bytes out.
    lldb::addr_t data_addr = expr_value_sp->GetValueAsUnsigned(0);
    if (data_addr != 0) {
      ValueObjectSP free_result;
      std::string free_expr =
          llvm::formatv("delete[] (char*){0};", data_addr).str();
      lldb::ExpressionResults free_re =
          target->EvaluateExpression(free_expr.c_str(), frame, free_result);
      if (free_re) {
        LLDB_LOGF(log, "[ExecuteAction] delete[] 0x%llx failed: %d",
                  (unsigned long long)data_addr, (int)free_re);
      } else {
        LLDB_LOGF(log, "[ExecuteAction] freed target buffer at 0x%llx",
                  (unsigned long long)data_addr);
      }
    } else {
      LLDB_LOGF(log,
                "[ExecuteAction] data pointer is null/zero; skip free in target");
    }
    return true;
  };

  bool handled = try_struct_path();
  if (!handled)
    handled = try_cstr_ptr_path();

  if (!handled)
    error = Status::FromErrorString("[ExecuteAction] Unsupported expression return type");

  LLDB_LOGF(log, "[MixedDebugger::ExecuteAction] result: %s",
            result->PeekCStr(0));
  return result;
}

#endif /* OHOS_LLVM */
