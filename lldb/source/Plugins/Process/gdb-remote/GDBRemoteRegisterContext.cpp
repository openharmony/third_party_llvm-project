//===-- GDBRemoteRegisterContext.cpp --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "GDBRemoteRegisterContext.h"

#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/DataBufferHeap.h"
#include "lldb/Utility/DataExtractor.h"
#include "lldb/Utility/RegisterValue.h"
#include "lldb/Utility/Scalar.h"
#include "lldb/Utility/StreamString.h"
#include "lldb/Utility/Timer.h"   // OHOS_LOCAL
#include "ProcessGDBRemote.h"
#include "ProcessGDBRemoteLog.h"
#include "ThreadGDBRemote.h"
#include "Utility/ARM_DWARF_Registers.h"
#include "Utility/ARM_ehframe_Registers.h"
#include "lldb/Utility/StringExtractorGDBRemote.h"

#include <memory>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_gdb_remote;

// GDBRemoteRegisterContext constructor
GDBRemoteRegisterContext::GDBRemoteRegisterContext(
    ThreadGDBRemote &thread, uint32_t concrete_frame_idx,
    GDBRemoteDynamicRegisterInfoSP reg_info_sp, bool read_all_at_once,
    bool write_all_at_once)
    : RegisterContext(thread, concrete_frame_idx),
      m_reg_info_sp(std::move(reg_info_sp)), m_reg_valid(), m_reg_data(),
      m_read_all_at_once(read_all_at_once),
      m_write_all_at_once(write_all_at_once), m_gpacket_cached(false) {
  // Resize our vector of bools to contain one bool for every register. We will
  // use these boolean values to know when a register value is valid in
  // m_reg_data.
  m_reg_valid.resize(m_reg_info_sp->GetNumRegisters());

  // Make a heap based buffer that is big enough to store all registers
  DataBufferSP reg_data_sp(
      new DataBufferHeap(m_reg_info_sp->GetRegisterDataByteSize(), 0));
  m_reg_data.SetData(reg_data_sp);
  m_reg_data.SetByteOrder(thread.GetProcess()->GetByteOrder());
}

// Destructor
GDBRemoteRegisterContext::~GDBRemoteRegisterContext() = default;

void GDBRemoteRegisterContext::InvalidateAllRegisters() {
  SetAllRegisterValid(false);
}

void GDBRemoteRegisterContext::SetAllRegisterValid(bool b) {
  m_gpacket_cached = b;
  std::vector<bool>::iterator pos, end = m_reg_valid.end();
  for (pos = m_reg_valid.begin(); pos != end; ++pos)
    *pos = b;
}

size_t GDBRemoteRegisterContext::GetRegisterCount() {
  return m_reg_info_sp->GetNumRegisters();
}

const RegisterInfo *
GDBRemoteRegisterContext::GetRegisterInfoAtIndex(size_t reg) {
  return m_reg_info_sp->GetRegisterInfoAtIndex(reg);
}

size_t GDBRemoteRegisterContext::GetRegisterSetCount() {
  return m_reg_info_sp->GetNumRegisterSets();
}

const RegisterSet *GDBRemoteRegisterContext::GetRegisterSet(size_t reg_set) {
  return m_reg_info_sp->GetRegisterSet(reg_set);
}

bool GDBRemoteRegisterContext::ReadRegister(const RegisterInfo *reg_info,
                                            RegisterValue &value) {
  // Read the register
  LLDB_MODULE_TIMER(LLDBPerformanceTagName::TAG_GDBREMOTE);   // OHOS_LOCAL
  if (ReadRegisterBytes(reg_info)) {
    const uint32_t reg = reg_info->kinds[eRegisterKindLLDB];
    if (m_reg_valid[reg] == false)
      return false;
    if (reg_info->value_regs &&
        reg_info->value_regs[0] != LLDB_INVALID_REGNUM &&
        reg_info->value_regs[1] != LLDB_INVALID_REGNUM) {
      std::vector<char> combined_data;
      uint32_t offset = 0;
      for (int i = 0; reg_info->value_regs[i] != LLDB_INVALID_REGNUM; i++) {
        const RegisterInfo *parent_reg = GetRegisterInfo(
            eRegisterKindLLDB, reg_info->value_regs[i]);
        if (!parent_reg)
          return false;
        combined_data.resize(offset + parent_reg->byte_size);
        if (m_reg_data.CopyData(parent_reg->byte_offset, parent_reg->byte_size,
                                combined_data.data() + offset) !=
            parent_reg->byte_size)
          return false;
        offset += parent_reg->byte_size;
      }

      Status error;
      return value.SetFromMemoryData(
                 reg_info, combined_data.data(), combined_data.size(),
                 m_reg_data.GetByteOrder(), error) == combined_data.size();
    } else {
      const bool partial_data_ok = false;
      Status error(value.SetValueFromData(
          reg_info, m_reg_data, reg_info->byte_offset, partial_data_ok));
      return error.Success();
    }
  }
  return false;
}

bool GDBRemoteRegisterContext::PrivateSetRegisterValue(
    uint32_t reg, llvm::ArrayRef<uint8_t> data) {
  const RegisterInfo *reg_info = GetRegisterInfoAtIndex(reg);
  if (reg_info == nullptr)
    return false;

  // Invalidate if needed
  InvalidateIfNeeded(false);

  const size_t reg_byte_size = reg_info->byte_size;
  memcpy(const_cast<uint8_t *>(
             m_reg_data.PeekData(reg_info->byte_offset, reg_byte_size)),
         data.data(), std::min(data.size(), reg_byte_size));
  bool success = data.size() >= reg_byte_size;
  if (success) {
    SetRegisterIsValid(reg, true);
  } else if (data.size() > 0) {
    // Only set register is valid to false if we copied some bytes, else leave
    // it as it was.
    SetRegisterIsValid(reg, false);
  }
  return success;
}

bool GDBRemoteRegisterContext::PrivateSetRegisterValue(uint32_t reg,
                                                       uint64_t new_reg_val) {
  const RegisterInfo *reg_info = GetRegisterInfoAtIndex(reg);
  if (reg_info == nullptr)
    return false;

  // Early in process startup, we can get a thread that has an invalid byte
  // order because the process hasn't been completely set up yet (see the ctor
  // where the byte order is setfrom the process).  If that's the case, we
  // can't set the value here.
  if (m_reg_data.GetByteOrder() == eByteOrderInvalid) {
    return false;
  }

  // Invalidate if needed
  InvalidateIfNeeded(false);

  DataBufferSP buffer_sp(new DataBufferHeap(&new_reg_val, sizeof(new_reg_val)));
  DataExtractor data(buffer_sp, endian::InlHostByteOrder(), sizeof(void *));

  // If our register context and our register info disagree, which should never
  // happen, don't overwrite past the end of the buffer.
  if (m_reg_data.GetByteSize() < reg_info->byte_offset + reg_info->byte_size)
    return false;

  // Grab a pointer to where we are going to put this register
  uint8_t *dst = const_cast<uint8_t *>(
      m_reg_data.PeekData(reg_info->byte_offset, reg_info->byte_size));

  if (dst == nullptr)
    return false;

  if (data.CopyByteOrderedData(0,                          // src offset
                               reg_info->byte_size,        // src length
                               dst,                        // dst
                               reg_info->byte_size,        // dst length
                               m_reg_data.GetByteOrder())) // dst byte order
  {
    SetRegisterIsValid(reg, true);
    return true;
  }
  return false;
}

// Helper function for GDBRemoteRegisterContext::ReadRegisterBytes().
bool GDBRemoteRegisterContext::GetPrimordialRegister(
    const RegisterInfo *reg_info, GDBRemoteCommunicationClient &gdb_comm) {
  const uint32_t lldb_reg = reg_info->kinds[eRegisterKindLLDB];
  const uint32_t remote_reg = reg_info->kinds[eRegisterKindProcessPlugin];

  if (DataBufferSP buffer_sp =
          gdb_comm.ReadRegister(m_thread.GetProtocolID(), remote_reg))
    return PrivateSetRegisterValue(
        lldb_reg, llvm::ArrayRef<uint8_t>(buffer_sp->GetBytes(),
                                          buffer_sp->GetByteSize()));
  return false;
}

bool GDBRemoteRegisterContext::ReadRegisterBytes(const RegisterInfo *reg_info) {
  ExecutionContext exe_ctx(CalculateThread());

  Process *process = exe_ctx.GetProcessPtr();
  Thread *thread = exe_ctx.GetThreadPtr();
  if (process == nullptr || thread == nullptr)
    return false;

  GDBRemoteCommunicationClient &gdb_comm(
      ((ProcessGDBRemote *)process)->GetGDBRemote());

  InvalidateIfNeeded(false);

  const uint32_t reg = reg_info->kinds[eRegisterKindLLDB];

  if (!GetRegisterIsValid(reg)) {
    if (m_read_all_at_once && !m_gpacket_cached) {
      if (DataBufferSP buffer_sp =
              gdb_comm.ReadAllRegisters(m_thread.GetProtocolID())) {
        memcpy(const_cast<uint8_t *>(m_reg_data.GetDataStart()),
               buffer_sp->GetBytes(),
               std::min(buffer_sp->GetByteSize(), m_reg_data.GetByteSize()));
        if (buffer_sp->GetByteSize() >= m_reg_data.GetByteSize()) {
          SetAllRegisterValid(true);
          return true;
        } else if (buffer_sp->GetByteSize() > 0) {
          for (auto x : llvm::enumerate(m_reg_info_sp->registers())) {
            const struct RegisterInfo &reginfo = x.value();
            m_reg_valid[x.index()] =
                (reginfo.byte_offset + reginfo.byte_size <=
                 buffer_sp->GetByteSize());
          }

          m_gpacket_cached = true;
          if (GetRegisterIsValid(reg))
            return true;
        } else {
          Log *log(GetLog(GDBRLog::Thread | GDBRLog::Packets));
          LLDB_LOGF(
              log,
              "error: GDBRemoteRegisterContext::ReadRegisterBytes tried "
              "to read the "
              "entire register context at once, expected at least %" PRId64
              " bytes "
              "but only got %" PRId64 " bytes.",
              m_reg_data.GetByteSize(), buffer_sp->GetByteSize());
          return false;
        }
      }
    }
    if (reg_info->value_regs) {
      // Process this composite register request by delegating to the
      // constituent primordial registers.

      // Index of the primordial register.
      bool success = true;
      for (uint32_t idx = 0; success; ++idx) {
        const uint32_t prim_reg = reg_info->value_regs[idx];
        if (prim_reg == LLDB_INVALID_REGNUM)
          break;
        // We have a valid primordial register as our constituent. Grab the
        // corresponding register info.
        const RegisterInfo *prim_reg_info =
            GetRegisterInfo(eRegisterKindLLDB, prim_reg);
        if (prim_reg_info == nullptr)
          success = false;
        else {
          // Read the containing register if it hasn't already been read
          if (!GetRegisterIsValid(prim_reg))
            success = GetPrimordialRegister(prim_reg_info, gdb_comm);
        }
      }

      if (success) {
        // If we reach this point, all primordial register requests have
        // succeeded. Validate this composite register.
        SetRegisterIsValid(reg_info, true);
      }
    } else {
      // Get each register individually
      GetPrimordialRegister(reg_info, gdb_comm);
    }

    // Make sure we got a valid register value after reading it
    if (!GetRegisterIsValid(reg))
      return false;
  }

  return true;
}

bool GDBRemoteRegisterContext::WriteRegister(const RegisterInfo *reg_info,
                                             const RegisterValue &value) {
  DataExtractor data;
  if (value.GetData(data)) {
    if (reg_info->value_regs &&
        reg_info->value_regs[0] != LLDB_INVALID_REGNUM &&
        reg_info->value_regs[1] != LLDB_INVALID_REGNUM) {
      uint32_t combined_size = 0;
      for (int i = 0; reg_info->value_regs[i] != LLDB_INVALID_REGNUM; i++) {
        const RegisterInfo *parent_reg = GetRegisterInfo(
            eRegisterKindLLDB, reg_info->value_regs[i]);
        if (!parent_reg)
          return false;
        combined_size += parent_reg->byte_size;
      }

      if (data.GetByteSize() < combined_size)
        return false;

      uint32_t offset = 0;
      for (int i = 0; reg_info->value_regs[i] != LLDB_INVALID_REGNUM; i++) {
        const RegisterInfo *parent_reg = GetRegisterInfo(
            eRegisterKindLLDB, reg_info->value_regs[i]);
        assert(parent_reg);

        DataExtractor parent_data{data, offset, parent_reg->byte_size};
        if (!WriteRegisterBytes(parent_reg, parent_data, 0))
          return false;
        offset += parent_reg->byte_size;
      }
      assert(offset == combined_size);
      return true;
    } else
      return WriteRegisterBytes(reg_info, data, 0);
  }
  return false;
}

// Helper function for GDBRemoteRegisterContext::WriteRegisterBytes().
bool GDBRemoteRegisterContext::SetPrimordialRegister(
    const RegisterInfo *reg_info, GDBRemoteCommunicationClient &gdb_comm) {
  StreamString packet;
  StringExtractorGDBRemote response;
  const uint32_t reg = reg_info->kinds[eRegisterKindLLDB];
  // Invalidate just this register
  SetRegisterIsValid(reg, false);

  return gdb_comm.WriteRegister(
      m_thread.GetProtocolID(), reg_info->kinds[eRegisterKindProcessPlugin],
      {m_reg_data.PeekData(reg_info->byte_offset, reg_info->byte_size),
       reg_info->byte_size});
}

bool GDBRemoteRegisterContext::WriteRegisterBytes(const RegisterInfo *reg_info,
                                                  DataExtractor &data,
                                                  uint32_t data_offset) {
  ExecutionContext exe_ctx(CalculateThread());

  Process *process = exe_ctx.GetProcessPtr();
  Thread *thread = exe_ctx.GetThreadPtr();
  if (process == nullptr || thread == nullptr)
    return false;

  GDBRemoteCommunicationClient &gdb_comm(
      ((ProcessGDBRemote *)process)->GetGDBRemote());

  assert(m_reg_data.GetByteSize() >=
         reg_info->byte_offset + reg_info->byte_size);

  // If our register context and our register info disagree, which should never
  // happen, don't overwrite past the end of the buffer.
  if (m_reg_data.GetByteSize() < reg_info->byte_offset + reg_info->byte_size)
    return false;

  // Grab a pointer to where we are going to put this register
  uint8_t *dst = const_cast<uint8_t *>(
      m_reg_data.PeekData(reg_info->byte_offset, reg_info->byte_size));

  if (dst == nullptr)
    return false;

  // Code below is specific to AArch64 target in SVE state
  // If vector granule (vg) register is being written then thread's
  // register context reconfiguration is triggered on success.
  bool do_reconfigure_arm64_sve = false;
  const ArchSpec &arch = process->GetTarget().GetArchitecture();
  if (arch.IsValid() && arch.GetTriple().isAArch64())
    if (strcmp(reg_info->name, "vg") == 0)
      do_reconfigure_arm64_sve = true;

  if (data.CopyByteOrderedData(data_offset,                // src offset
                               reg_info->byte_size,        // src length
                               dst,                        // dst
                               reg_info->byte_size,        // dst length
                               m_reg_data.GetByteOrder())) // dst byte order
  {
    GDBRemoteClientBase::Lock lock(gdb_comm);
    if (lock) {
      if (m_write_all_at_once) {
        // Invalidate all register values
        InvalidateIfNeeded(true);

        // Set all registers in one packet
        if (gdb_comm.WriteAllRegisters(
                m_thread.GetProtocolID(),
                {m_reg_data.GetDataStart(), size_t(m_reg_data.GetByteSize())}))

        {
          SetAllRegisterValid(false);

          if (do_reconfigure_arm64_sve)
            AArch64SVEReconfigure();

          return true;
        }
      } else {
        bool success = true;

        if (reg_info->value_regs) {
          // This register is part of another register. In this case we read
          // the actual register data for any "value_regs", and once all that
          // data is read, we will have enough data in our register context
          // bytes for the value of this register

          // Invalidate this composite register first.

          for (uint32_t idx = 0; success; ++idx) {
            const uint32_t reg = reg_info->value_regs[idx];
            if (reg == LLDB_INVALID_REGNUM)
              break;
            // We have a valid primordial register as our constituent. Grab the
            // corresponding register info.
            const RegisterInfo *value_reg_info =
                GetRegisterInfo(eRegisterKindLLDB, reg);
            if (value_reg_info == nullptr)
              success = false;
            else
              success = SetPrimordialRegister(value_reg_info, gdb_comm);
          }
        } else {
          // This is an actual register, write it
          success = SetPrimordialRegister(reg_info, gdb_comm);

          if (success && do_reconfigure_arm64_sve)
            AArch64SVEReconfigure();
        }

        // Check if writing this register will invalidate any other register
        // values? If so, invalidate them
        if (reg_info->invalidate_regs) {
          for (uint32_t idx = 0, reg = reg_info->invalidate_regs[0];
               reg != LLDB_INVALID_REGNUM;
               reg = reg_info->invalidate_regs[++idx])
            SetRegisterIsValid(ConvertRegisterKindToRegisterNumber(
                                   eRegisterKindLLDB, reg),
                               false);
        }

        return success;
      }
    } else {
      Log *log(GetLog(GDBRLog::Thread | GDBRLog::Packets));
      if (log) {
        if (log->GetVerbose()) {
          StreamString strm;
          gdb_comm.DumpHistory(strm);
          LLDB_LOGF(log,
                    "error: failed to get packet sequence mutex, not sending "
                    "write register for \"%s\":\n%s",
                    reg_info->name, strm.GetData());
        } else
          LLDB_LOGF(log,
                    "error: failed to get packet sequence mutex, not sending "
                    "write register for \"%s\"",
                    reg_info->name);
      }
    }
  }
  return false;
}

bool GDBRemoteRegisterContext::ReadAllRegisterValues(
    RegisterCheckpoint &reg_checkpoint) {
  ExecutionContext exe_ctx(CalculateThread());

  Process *process = exe_ctx.GetProcessPtr();
  Thread *thread = exe_ctx.GetThreadPtr();
  if (process == nullptr || thread == nullptr)
    return false;

  GDBRemoteCommunicationClient &gdb_comm(
      ((ProcessGDBRemote *)process)->GetGDBRemote());

  uint32_t save_id = 0;
  if (gdb_comm.SaveRegisterState(thread->GetProtocolID(), save_id)) {
    reg_checkpoint.SetID(save_id);
    reg_checkpoint.GetData().reset();
    return true;
  } else {
    reg_checkpoint.SetID(0); // Invalid save ID is zero
    return ReadAllRegisterValues(reg_checkpoint.GetData());
  }
}

bool GDBRemoteRegisterContext::WriteAllRegisterValues(
    const RegisterCheckpoint &reg_checkpoint) {
  uint32_t save_id = reg_checkpoint.GetID();
  if (save_id != 0) {
    ExecutionContext exe_ctx(CalculateThread());

    Process *process = exe_ctx.GetProcessPtr();
    Thread *thread = exe_ctx.GetThreadPtr();
    if (process == nullptr || thread == nullptr)
      return false;

    GDBRemoteCommunicationClient &gdb_comm(
        ((ProcessGDBRemote *)process)->GetGDBRemote());

    return gdb_comm.RestoreRegisterState(m_thread.GetProtocolID(), save_id);
  } else {
    return WriteAllRegisterValues(reg_checkpoint.GetData());
  }
}

bool GDBRemoteRegisterContext::ReadAllRegisterValues(
    lldb::WritableDataBufferSP &data_sp) {
  ExecutionContext exe_ctx(CalculateThread());

  Process *process = exe_ctx.GetProcessPtr();
  Thread *thread = exe_ctx.GetThreadPtr();
  if (process == nullptr || thread == nullptr)
    return false;

  GDBRemoteCommunicationClient &gdb_comm(
      ((ProcessGDBRemote *)process)->GetGDBRemote());

  const bool use_g_packet =
      !gdb_comm.AvoidGPackets((ProcessGDBRemote *)process);

  GDBRemoteClientBase::Lock lock(gdb_comm);
  if (lock) {
    if (gdb_comm.SyncThreadState(m_thread.GetProtocolID()))
      InvalidateAllRegisters();

    if (use_g_packet) {
      if (DataBufferSP data_buffer =
              gdb_comm.ReadAllRegisters(m_thread.GetProtocolID())) {
        data_sp = std::make_shared<DataBufferHeap>(*data_buffer);
        return true;
      }
    }

    // We're going to read each register
    // individually and store them as binary data in a buffer.
    const RegisterInfo *reg_info;

    for (uint32_t i = 0; (reg_info = GetRegisterInfoAtIndex(i)) != nullptr;
         i++) {
      if (reg_info
              ->value_regs) // skip registers that are slices of real registers
        continue;
      ReadRegisterBytes(reg_info);
      // ReadRegisterBytes saves the contents of the register in to the
      // m_reg_data buffer
    }
    data_sp = std::make_shared<DataBufferHeap>(
        m_reg_data.GetDataStart(), m_reg_info_sp->GetRegisterDataByteSize());
    return true;
  } else {

    Log *log(GetLog(GDBRLog::Thread | GDBRLog::Packets));
    if (log) {
      if (log->GetVerbose()) {
        StreamString strm;
        gdb_comm.DumpHistory(strm);
        LLDB_LOGF(log,
                  "error: failed to get packet sequence mutex, not sending "
                  "read all registers:\n%s",
                  strm.GetData());
      } else
        LLDB_LOGF(log,
                  "error: failed to get packet sequence mutex, not sending "
                  "read all registers");
    }
  }

  data_sp.reset();
  return false;
}

bool GDBRemoteRegisterContext::WriteAllRegisterValues(
    const lldb::DataBufferSP &data_sp) {
  if (!data_sp || data_sp->GetBytes() == nullptr || data_sp->GetByteSize() == 0)
    return false;

  ExecutionContext exe_ctx(CalculateThread());

  Process *process = exe_ctx.GetProcessPtr();
  Thread *thread = exe_ctx.GetThreadPtr();
  if (process == nullptr || thread == nullptr)
    return false;

  GDBRemoteCommunicationClient &gdb_comm(
      ((ProcessGDBRemote *)process)->GetGDBRemote());

  const bool use_g_packet =
      !gdb_comm.AvoidGPackets((ProcessGDBRemote *)process);

  GDBRemoteClientBase::Lock lock(gdb_comm);
  if (lock) {
    // The data_sp contains the G response packet.
    if (use_g_packet) {
      if (gdb_comm.WriteAllRegisters(
              m_thread.GetProtocolID(),
              {data_sp->GetBytes(), size_t(data_sp->GetByteSize())}))
        return true;

      uint32_t num_restored = 0;
      // We need to manually go through all of the registers and restore them
      // manually
      DataExtractor restore_data(data_sp, m_reg_data.GetByteOrder(),
                                 m_reg_data.GetAddressByteSize());

      const RegisterInfo *reg_info;

      // The g packet contents may either include the slice registers
      // (registers defined in terms of other registers, e.g. eax is a subset
      // of rax) or not.  The slice registers should NOT be in the g packet,
      // but some implementations may incorrectly include them.
      //
      // If the slice registers are included in the packet, we must step over
      // the slice registers when parsing the packet -- relying on the
      // RegisterInfo byte_offset field would be incorrect. If the slice
      // registers are not included, then using the byte_offset values into the
      // data buffer is the best way to find individual register values.

      uint64_t size_including_slice_registers = 0;
      uint64_t size_not_including_slice_registers = 0;
      uint64_t size_by_highest_offset = 0;

      for (uint32_t reg_idx = 0;
           (reg_info = GetRegisterInfoAtIndex(reg_idx)) != nullptr; ++reg_idx) {
        size_including_slice_registers += reg_info->byte_size;
        if (reg_info->value_regs == nullptr)
          size_not_including_slice_registers += reg_info->byte_size;
        if (reg_info->byte_offset >= size_by_highest_offset)
          size_by_highest_offset = reg_info->byte_offset + reg_info->byte_size;
      }

      bool use_byte_offset_into_buffer;
      if (size_by_highest_offset == restore_data.GetByteSize()) {
        // The size of the packet agrees with the highest offset: + size in the
        // register file
        use_byte_offset_into_buffer = true;
      } else if (size_not_including_slice_registers ==
                 restore_data.GetByteSize()) {
        // The size of the packet is the same as concatenating all of the
        // registers sequentially, skipping the slice registers
        use_byte_offset_into_buffer = true;
      } else if (size_including_slice_registers == restore_data.GetByteSize()) {
        // The slice registers are present in the packet (when they shouldn't
        // be). Don't try to use the RegisterInfo byte_offset into the
        // restore_data, it will point to the wrong place.
        use_byte_offset_into_buffer = false;
      } else {
        // None of our expected sizes match the actual g packet data we're
        // looking at. The most conservative approach here is to use the
        // running total byte offset.
        use_byte_offset_into_buffer = false;
      }

      // In case our register definitions don't include the correct offsets,
      // keep track of the size of each reg & compute offset based on that.
      uint32_t running_byte_offset = 0;
      for (uint32_t reg_idx = 0;
           (reg_info = GetRegisterInfoAtIndex(reg_idx)) != nullptr;
           ++reg_idx, running_byte_offset += reg_info->byte_size) {
        // Skip composite aka slice registers (e.g. eax is a slice of rax).
        if (reg_info->value_regs)
          continue;

        const uint32_t reg = reg_info->kinds[eRegisterKindLLDB];

        uint32_t register_offset;
        if (use_byte_offset_into_buffer) {
          register_offset = reg_info->byte_offset;
        } else {
          register_offset = running_byte_offset;
        }

        const uint32_t reg_byte_size = reg_info->byte_size;

        const uint8_t *restore_src =
            restore_data.PeekData(register_offset, reg_byte_size);
        if (restore_src) {
          SetRegisterIsValid(reg, false);
          if (gdb_comm.WriteRegister(
                  m_thread.GetProtocolID(),
                  reg_info->kinds[eRegisterKindProcessPlugin],
                  {restore_src, reg_byte_size}))
            ++num_restored;
        }
      }
      return num_restored > 0;
    } else {
      // For the use_g_packet == false case, we're going to write each register
      // individually.  The data buffer is binary data in this case, instead of
      // ascii characters.

      bool arm64_debugserver = false;
      if (m_thread.GetProcess().get()) {
        const ArchSpec &arch =
            m_thread.GetProcess()->GetTarget().GetArchitecture();
        if (arch.IsValid() && (arch.GetMachine() == llvm::Triple::aarch64 ||
                               arch.GetMachine() == llvm::Triple::aarch64_32) &&
            arch.GetTriple().getVendor() == llvm::Triple::Apple &&
            arch.GetTriple().getOS() == llvm::Triple::IOS) {
          arm64_debugserver = true;
        }
      }
      uint32_t num_restored = 0;
      const RegisterInfo *reg_info;
      for (uint32_t i = 0; (reg_info = GetRegisterInfoAtIndex(i)) != nullptr;
           i++) {
        if (reg_info->value_regs) // skip registers that are slices of real
                                  // registers
          continue;
        // Skip the fpsr and fpcr floating point status/control register
        // writing to work around a bug in an older version of debugserver that
        // would lead to register context corruption when writing fpsr/fpcr.
        if (arm64_debugserver && (strcmp(reg_info->name, "fpsr") == 0 ||
                                  strcmp(reg_info->name, "fpcr") == 0)) {
          continue;
        }

        SetRegisterIsValid(reg_info, false);
        if (gdb_comm.WriteRegister(m_thread.GetProtocolID(),
                                   reg_info->kinds[eRegisterKindProcessPlugin],
                                   {data_sp->GetBytes() + reg_info->byte_offset,
                                    reg_info->byte_size}))
          ++num_restored;
      }
      return num_restored > 0;
    }
  } else {
    Log *log(GetLog(GDBRLog::Thread | GDBRLog::Packets));
    if (log) {
      if (log->GetVerbose()) {
        StreamString strm;
        gdb_comm.DumpHistory(strm);
        LLDB_LOGF(log,
                  "error: failed to get packet sequence mutex, not sending "
                  "write all registers:\n%s",
                  strm.GetData());
      } else
        LLDB_LOGF(log,
                  "error: failed to get packet sequence mutex, not sending "
                  "write all registers");
    }
  }
  return false;
}

uint32_t GDBRemoteRegisterContext::ConvertRegisterKindToRegisterNumber(
    lldb::RegisterKind kind, uint32_t num) {
  return m_reg_info_sp->ConvertRegisterKindToRegisterNumber(kind, num);
}

bool GDBRemoteRegisterContext::AArch64SVEReconfigure() {
  if (!m_reg_info_sp)
    return false;

  const RegisterInfo *reg_info = m_reg_info_sp->GetRegisterInfo("vg");
  if (!reg_info)
    return false;

  uint64_t fail_value = LLDB_INVALID_ADDRESS;
  uint32_t vg_reg_num = reg_info->kinds[eRegisterKindLLDB];
  uint64_t vg_reg_value = ReadRegisterAsUnsigned(vg_reg_num, fail_value);

  if (vg_reg_value != fail_value && vg_reg_value <= 32) {
    const RegisterInfo *reg_info = m_reg_info_sp->GetRegisterInfo("p0");
    if (!reg_info || vg_reg_value == reg_info->byte_size)
      return false;

    if (m_reg_info_sp->UpdateARM64SVERegistersInfos(vg_reg_value)) {
      // Make a heap based buffer that is big enough to store all registers
      m_reg_data.SetData(std::make_shared<DataBufferHeap>(
          m_reg_info_sp->GetRegisterDataByteSize(), 0));
      m_reg_data.SetByteOrder(GetByteOrder());

      InvalidateAllRegisters();

      return true;
    }
  }

  return false;
}

bool GDBRemoteDynamicRegisterInfo::UpdateARM64SVERegistersInfos(uint64_t vg) {
  // SVE Z register size is vg x 8 bytes.
  uint32_t z_reg_byte_size = vg * 8;

  // SVE vector length has changed, accordingly set size of Z, P and FFR
  // registers. Also invalidate register offsets it will be recalculated
  // after SVE register size update.
  for (auto &reg : m_regs) {
    if (reg.value_regs == nullptr) {
      if (reg.name[0] == 'z' && isdigit(reg.name[1]))
        reg.byte_size = z_reg_byte_size;
      else if (reg.name[0] == 'p' && isdigit(reg.name[1]))
        reg.byte_size = vg;
      else if (strcmp(reg.name, "ffr") == 0)
        reg.byte_size = vg;
    }
    reg.byte_offset = LLDB_INVALID_INDEX32;
  }

  // Re-calculate register offsets
  ConfigureOffsets();
  return true;
}
