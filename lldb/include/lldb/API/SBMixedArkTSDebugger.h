//===-- SBMixedArkTSDebugger.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBMIXEDARKTSDEBUGGER_H
#define LLDB_API_SBMIXEDARKTSDEBUGGER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBMixedArkTSDebugger {
public:
  SBMixedArkTSDebugger();

  SBMixedArkTSDebugger(const lldb::SBTarget &rhs);

  SBMixedArkTSDebugger(const lldb::TargetSP &target_sp);

  ~SBMixedArkTSDebugger();

  /// Get the backtrace of ArkTS for current thread. A cstring which contains
  /// the information of backtrace is saved in return.
  ///
  /// If the current thread does not have an ArkTS runtime, "\0" will be returned.
  ///
  /// \param [out] er
  ///     The variable to get error reason, when some error occurred.
  ///
  /// \return
  ///     An lldb::SBData object which contain the raw cstring of ArkTS backtrace.
  lldb::SBData GetBackTrace(SBError &er);

private:
  lldb_private::MixedArkTSDebugger* m_opaque_ptr;
};

} // namespace lldb

#endif // LLDB_API_SBMIXEDARKTSDEBUGGER_H