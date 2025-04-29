//===-- SWIG Interface for SBMixedArkTSDebugger -----------------*- C++ -*-===//
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

namespace lldb {

%feature("docstring",
"Represents py:class:`SBMixedArkTSDebugger`.

SBMixedArkTSDebugger supports OHOS Applications to get ArkTS debug information after
calling napi and during debugging C/C++.

This class should not be used when ArkTS runtime running. If used, unexpected issue
may occur."
) SBMixedArkTSDebugger;
class SBMixedArkTSDebugger {
public:
  SBMixedArkTSDebugger();

  SBMixedArkTSDebugger(const lldb::SBTarget &rhs);

  SBMixedArkTSDebugger(const lldb::TargetSP &taraget_sp);

  lldb::SBData GetBackTrace(SBError &er);

  lldb::SBData OperateDebugMessage(const char *message, SBError &er);
};

} // namespace lldb
