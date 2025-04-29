//===-- SBMixedArkTSDebugger.cpp ------------------------------------------===//
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

#include "lldb/API/SBMixedArkTSDebugger.h"
#include "lldb/API/SBData.h"
#include "lldb/API/SBTarget.h"
#include "lldb/API/SBError.h"
#include "lldb/Utility/Instrumentation.h"
#include "lldb/Target/Target.h"
#include "lldb/Target/MixedArkTSDebugger.h"

using namespace lldb;
using namespace lldb_private;

SBMixedArkTSDebugger::SBMixedArkTSDebugger() {
  LLDB_INSTRUMENT_VA(this);
}

SBMixedArkTSDebugger::SBMixedArkTSDebugger(const lldb::SBTarget &rhs)
    : m_opaque_ptr(new MixedArkTSDebugger(rhs.GetSP())) {
  LLDB_INSTRUMENT_VA(this, rhs);
}

SBMixedArkTSDebugger::SBMixedArkTSDebugger(const lldb::TargetSP &target_sp)
    : m_opaque_ptr(new MixedArkTSDebugger(target_sp)) {
  LLDB_INSTRUMENT_VA(this, target_sp);
}

SBMixedArkTSDebugger::~SBMixedArkTSDebugger() {
  if (m_opaque_ptr) {
    delete m_opaque_ptr;
    m_opaque_ptr = nullptr;
  }
}

lldb::SBData SBMixedArkTSDebugger::GetBackTrace(SBError &er) {
  LLDB_INSTRUMENT_VA(this, er);

  return SBData(m_opaque_ptr->GetCurrentThreadBackTrace(er.ref()));
}

lldb::SBData SBMixedArkTSDebugger::OperateDebugMessage(const char *message, SBError &er) {
  LLDB_INSTRUMENT_VA(this, er);

  return SBData(m_opaque_ptr->GetCurrentThreadOperateDebugMessageResult(message, er.ref()));
}