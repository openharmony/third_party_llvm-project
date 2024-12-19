//===-- MixedArkTSDebugger.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_ARKTS_MIXEDARKTSDEBUGGER_H
#define LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_ARKTS_MIXEDARKTSDEBUGGER_H

#include "lldb/Target/MixedDebugger.h"
using namespace lldb;

namespace lldb_private {

class MixedArkTSDebugger : public MixedDebugger {
public:
  MixedArkTSDebugger(const lldb::TargetSP &target_sp);

  ~MixedArkTSDebugger() {};

  DataExtractorSP GetCurrentThreadBackTrace(Status &error) override;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_MIXEDDEBUGGER_MIXEDDEBUGGER_H
