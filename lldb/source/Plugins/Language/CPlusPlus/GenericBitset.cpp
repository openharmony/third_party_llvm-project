//===-- GenericBitset.cpp //-----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LibCxx.h"
#include "LibStdcpp.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "lldb/DataFormatters/FormattersHelpers.h"
#include "lldb/Target/Target.h"

using namespace lldb;
using namespace lldb_private;

namespace {

/// This class can be used for handling bitsets from both libcxx and libstdcpp.
class GenericBitsetFrontEnd : public SyntheticChildrenFrontEnd {
public:
  enum class StdLib {
    LibCxx,
    LibStdcpp,
  };

  GenericBitsetFrontEnd(ValueObject &valobj, StdLib stdlib);

  size_t GetIndexOfChildWithName(ConstString name) override {
    return formatters::ExtractIndexFromString(name.GetCString());
  }

  bool MightHaveChildren() override { return true; }
  bool Update() override;
  size_t CalculateNumChildren() override { return m_elements.size(); }
  ValueObjectSP GetChildAtIndex(size_t idx) override;

private:
  ConstString GetDataContainerMemberName();

  // The lifetime of a ValueObject and all its derivative ValueObjects
  // (children, clones, etc.) is managed by a ClusterManager. These
  // objects are only destroyed when every shared pointer to any of them
  // is destroyed, so we must not store a shared pointer to any ValueObject
  // derived from our backend ValueObject (since we're in the same cluster).
  // Value objects created from raw data (i.e. in a different cluster) must
  // be referenced via shared pointer to keep them alive, however.
  std::vector<ValueObjectSP> m_elements;
  ValueObject *m_first = nullptr;
  CompilerType m_bool_type;
  ByteOrder m_byte_order = eByteOrderInvalid;
  uint8_t m_byte_size = 0;
  StdLib m_stdlib;
};
} // namespace

GenericBitsetFrontEnd::GenericBitsetFrontEnd(ValueObject &valobj, StdLib stdlib)
    : SyntheticChildrenFrontEnd(valobj), m_stdlib(stdlib) {
  m_bool_type = valobj.GetCompilerType().GetBasicTypeFromAST(eBasicTypeBool);
  if (auto target_sp = m_backend.GetTargetSP()) {
    m_byte_order = target_sp->GetArchitecture().GetByteOrder();
    m_byte_size = target_sp->GetArchitecture().GetAddressByteSize();
    Update();
  }
}

ConstString GenericBitsetFrontEnd::GetDataContainerMemberName() {
  switch (m_stdlib) {
  case StdLib::LibCxx:
    return ConstString("__first_");
  case StdLib::LibStdcpp:
    return ConstString("_M_w");
  }
  llvm_unreachable("Unknown StdLib enum");
}

bool GenericBitsetFrontEnd::Update() {
  m_elements.clear();
  m_first = nullptr;

  TargetSP target_sp = m_backend.GetTargetSP();
  if (!target_sp)
    return false;

  size_t size = 0;

  // OHOS_LOCAL begin
  const size_t bit_in_byte_cnt = 8;
  const size_t sizeof_sizet_in_bits = sizeof(size_t) * bit_in_byte_cnt;

  if (auto arg = m_backend.GetCompilerType().GetIntegralTemplateArgument(0))
    size = (arg->value.getLimitedValue() + sizeof_sizet_in_bits - 1) / sizeof_sizet_in_bits;
  // OHOS_LOCAL end

  m_elements.assign(size, ValueObjectSP());
  m_first = m_backend.GetChildMemberWithName(GetDataContainerMemberName(), true)
                .get();
  return false;
}

ValueObjectSP GenericBitsetFrontEnd::GetChildAtIndex(size_t idx) {
  if (idx >= m_elements.size() || !m_first)
    return ValueObjectSP();

  if (m_elements[idx])
    return m_elements[idx];

  ExecutionContext ctx = m_backend.GetExecutionContextRef().Lock(false);
  ValueObjectSP chunk;
  // For small bitsets __first_ is not an array, but a plain size_t.
  // OHOS_LOCAL begin
  if (m_first->GetCompilerType().IsArrayType(nullptr)) {
    chunk = m_first->GetChildAtIndex(idx, true);
  // OHOS_LOCAL end
  } else {
    chunk = m_first->GetSP();
  }
  // OHOS_LOCAL begin
  if (!chunk)
    return {};

  StreamString name;
  name.Printf("[%" PRIu64 "]", (uint64_t)idx);
  m_elements[idx] = chunk->Clone(ConstString(name.GetString()));
  // OHOS_LOCAL end

  return m_elements[idx];
}

SyntheticChildrenFrontEnd *formatters::LibStdcppBitsetSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new GenericBitsetFrontEnd(*valobj_sp,
                                     GenericBitsetFrontEnd::StdLib::LibStdcpp);
  return nullptr;
}

SyntheticChildrenFrontEnd *formatters::LibcxxBitsetSyntheticFrontEndCreator(
    CXXSyntheticChildren *, lldb::ValueObjectSP valobj_sp) {
  if (valobj_sp)
    return new GenericBitsetFrontEnd(*valobj_sp,
                                     GenericBitsetFrontEnd::StdLib::LibCxx);
  return nullptr;
}
