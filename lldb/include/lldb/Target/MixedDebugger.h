//===-- MixedDebugger.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_MIXEDDEBUGGER_H
#define LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_MIXEDDEBUGGER_H

#include "lldb/Utility/DataExtractor.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-forward.h"

using namespace lldb;

namespace lldb_private {

class MixedDebugger{
public:
  MixedDebugger(const TargetSP &target_sp);

  virtual ~MixedDebugger() {};

  /// Execute the C API provided by runtime to get debug information.
  ///
  /// \param [in] expr
  ///     A cstring of expression which start with "(const char*)".
  ///
  /// \param [out] error
  ///     The variable to record error reason.
  ///
  /// \return
  ///     A DataExtractorSP contain a cstring which length is less than
  ///     UINT32_MAX.
  DataExtractorSP ExecuteAction(const char* expr, Status &error);

  virtual DataExtractorSP GetCurrentThreadBackTrace(Status &error) = 0;

protected:
  TargetSP m_target_sp;

  MixedDebugger() = delete;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_MIXEDDEBUGGER_H
