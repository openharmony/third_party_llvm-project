//===- MemoryMapper.cpp - Cross-process memory mapper ------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ExecutionEngine/Orc/MemoryMapper.h"

#include "llvm/ExecutionEngine/Orc/Shared/OrcRTBridge.h"
#include "llvm/Support/WindowsError.h"

// OHOS_LOCAL
#if defined(LLVM_ON_UNIX) && !defined(__ANDROID__) && !defined(__OHOS__)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace llvm {
namespace orc {

MemoryMapper::~MemoryMapper() {}

InProcessMemoryMapper::InProcessMemoryMapper(size_t PageSize)
    : PageSize(PageSize) {}

Expected<std::unique_ptr<InProcessMemoryMapper>>
InProcessMemoryMapper::Create() {
  auto PageSize = sys::Process::getPageSize();
  if (!PageSize)
    return PageSize.takeError();
  return std::make_unique<InProcessMemoryMapper>(*PageSize);
}

void InProcessMemoryMapper::reserve(size_t NumBytes,
                                    OnReservedFunction OnReserved) {
  std::error_code EC;
  auto MB = sys::Memory::allocateMappedMemory(
      NumBytes, nullptr, sys::Memory::MF_READ | sys::Memory::MF_WRITE, EC);

  if (EC)
    return OnReserved(errorCodeToError(EC));

  {
    std::lock_guard<std::mutex> Lock(Mutex);
    Reservations[MB.base()].Size = MB.allocatedSize();
  }

  OnReserved(
      ExecutorAddrRange(ExecutorAddr::fromPtr(MB.base()), MB.allocatedSize()));
}

char *InProcessMemoryMapper::prepare(ExecutorAddr Addr, size_t ContentSize) {
  return Addr.toPtr<char *>();
}

void InProcessMemoryMapper::initialize(MemoryMapper::AllocInfo &AI,
                                       OnInitializedFunction OnInitialized) {
  ExecutorAddr MinAddr(~0ULL);

  for (auto &Segment : AI.Segments) {
    auto Base = AI.MappingBase + Segment.Offset;
    auto Size = Segment.ContentSize + Segment.ZeroFillSize;

    if (Base < MinAddr)
      MinAddr = Base;

    std::memset((Base + Segment.ContentSize).toPtr<void *>(), 0,
                Segment.ZeroFillSize);

    if (auto EC = sys::Memory::protectMappedMemory({Base.toPtr<void *>(), Size},
                                                   Segment.Prot)) {
      return OnInitialized(errorCodeToError(EC));
    }
    if (Segment.Prot & sys::Memory::MF_EXEC)
      sys::Memory::InvalidateInstructionCache(Base.toPtr<void *>(), Size);
  }

  auto DeinitializeActions = shared::runFinalizeActions(AI.Actions);
  if (!DeinitializeActions)
    return OnInitialized(DeinitializeActions.takeError());

  {
    std::lock_guard<std::mutex> Lock(Mutex);
    Allocations[MinAddr].DeinitializationActions =
        std::move(*DeinitializeActions);
    Reservations[AI.MappingBase.toPtr<void *>()].Allocations.push_back(MinAddr);
  }

  OnInitialized(MinAddr);
}

void InProcessMemoryMapper::deinitialize(
    ArrayRef<ExecutorAddr> Bases,
    MemoryMapper::OnDeinitializedFunction OnDeinitialized) {
  Error AllErr = Error::success();

  {
    std::lock_guard<std::mutex> Lock(Mutex);

    for (auto Base : Bases) {

      if (Error Err = shared::runDeallocActions(
              Allocations[Base].DeinitializationActions)) {
        AllErr = joinErrors(std::move(AllErr), std::move(Err));
      }

      Allocations.erase(Base);
    }
  }

  OnDeinitialized(std::move(AllErr));
}

void InProcessMemoryMapper::release(ArrayRef<ExecutorAddr> Bases,
                                    OnReleasedFunction OnReleased) {
  Error Err = Error::success();

  for (auto Base : Bases) {
    std::vector<ExecutorAddr> AllocAddrs;
    size_t Size;
    {
      std::lock_guard<std::mutex> Lock(Mutex);
      auto &R = Reservations[Base.toPtr<void *>()];
      Size = R.Size;
      AllocAddrs.swap(R.Allocations);
    }

    // deinitialize sub allocations
    std::promise<MSVCPError> P;
    auto F = P.get_future();
    deinitialize(AllocAddrs, [&](Error Err) { P.set_value(std::move(Err)); });
    if (Error E = F.get()) {
      Err = joinErrors(std::move(Err), std::move(E));
    }

    // free the memory
    auto MB = sys::MemoryBlock(Base.toPtr<void *>(), Size);

    auto EC = sys::Memory::releaseMappedMemory(MB);
    if (EC) {
      Err = joinErrors(std::move(Err), errorCodeToError(EC));
    }

    std::lock_guard<std::mutex> Lock(Mutex);
    Reservations.erase(Base.toPtr<void *>());
  }

  OnReleased(std::move(Err));
}

InProcessMemoryMapper::~InProcessMemoryMapper() {
  std::vector<ExecutorAddr> ReservationAddrs;
  {
    std::lock_guard<std::mutex> Lock(Mutex);

    ReservationAddrs.reserve(Reservations.size());
    for (const auto &R : Reservations) {
      ReservationAddrs.push_back(ExecutorAddr::fromPtr(R.getFirst()));
    }
  }

  std::promise<MSVCPError> P;
  auto F = P.get_future();
  release(ReservationAddrs, [&](Error Err) { P.set_value(std::move(Err)); });
  cantFail(F.get());
}

// SharedMemoryMapper

SharedMemoryMapper::SharedMemoryMapper(ExecutorProcessControl &EPC,
                                       SymbolAddrs SAs, size_t PageSize)
    : EPC(EPC), SAs(SAs), PageSize(PageSize) {
// OHOS_LOCAL
#if (!defined(LLVM_ON_UNIX) || defined(__ANDROID__) || defined(__OHOS__)) && !defined(_WIN32)
  llvm_unreachable("SharedMemoryMapper is not supported on this platform yet");
#endif
}

Expected<std::unique_ptr<SharedMemoryMapper>>
SharedMemoryMapper::Create(ExecutorProcessControl &EPC, SymbolAddrs SAs) {
// OHOS_LOCAL
#if (defined(LLVM_ON_UNIX) && !defined(__ANDROID__) && !defined(__OHOS__)) || defined(_WIN32)
  auto PageSize = sys::Process::getPageSize();
  if (!PageSize)
    return PageSize.takeError();

  return std::make_unique<SharedMemoryMapper>(EPC, SAs, *PageSize);
#else
  return make_error<StringError>(
      "SharedMemoryMapper is not supported on this platform yet",
      inconvertibleErrorCode());
#endif
}

void SharedMemoryMapper::reserve(size_t NumBytes,
                                 OnReservedFunction OnReserved) {
// OHOS_LOCAL
#if (defined(LLVM_ON_UNIX) && !defined(__ANDROID__) && !defined(__OHOS__)) || defined(_WIN32)

  EPC.callSPSWrapperAsync<
      rt::SPSExecutorSharedMemoryMapperServiceReserveSignature>(
      SAs.Reserve,
      [this, NumBytes, OnReserved = std::move(OnReserved)](
          Error SerializationErr,
          Expected<std::pair<ExecutorAddr, std::string>> Result) mutable {
        if (SerializationErr) {
          cantFail(Result.takeError());
          return OnReserved(std::move(SerializationErr));
        }

        if (!Result)
          return OnReserved(Result.takeError());

        ExecutorAddr RemoteAddr;
        std::string SharedMemoryName;
        std::tie(RemoteAddr, SharedMemoryName) = std::move(*Result);

        void *LocalAddr = nullptr;

#if defined(LLVM_ON_UNIX)

        int SharedMemoryFile = shm_open(SharedMemoryName.c_str(), O_RDWR, 0700);
        if (SharedMemoryFile < 0) {
          return OnReserved(errorCodeToError(
              std::error_code(errno, std::generic_category())));
        }

        // this prevents other processes from accessing it by name
        shm_unlink(SharedMemoryName.c_str());

        LocalAddr = mmap(nullptr, NumBytes, PROT_READ | PROT_WRITE, MAP_SHARED,
                         SharedMemoryFile, 0);
        if (LocalAddr == MAP_FAILED) {
          return OnReserved(errorCodeToError(
              std::error_code(errno, std::generic_category())));
        }

        close(SharedMemoryFile);

#elif defined(_WIN32)

        std::wstring WideSharedMemoryName(SharedMemoryName.begin(),
                                          SharedMemoryName.end());
        HANDLE SharedMemoryFile = OpenFileMappingW(
            FILE_MAP_ALL_ACCESS, FALSE, WideSharedMemoryName.c_str());
        if (!SharedMemoryFile)
          return OnReserved(errorCodeToError(mapWindowsError(GetLastError())));

        LocalAddr =
            MapViewOfFile(SharedMemoryFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (!LocalAddr) {
          CloseHandle(SharedMemoryFile);
          return OnReserved(errorCodeToError(mapWindowsError(GetLastError())));
        }

        CloseHandle(SharedMemoryFile);

#endif
        {
          std::lock_guard<std::mutex> Lock(Mutex);
          Reservations.insert({RemoteAddr, {LocalAddr, NumBytes}});
        }

        OnReserved(ExecutorAddrRange(RemoteAddr, NumBytes));
      },
      SAs.Instance, static_cast<uint64_t>(NumBytes));

#else
  OnReserved(make_error<StringError>(
      "SharedMemoryMapper is not supported on this platform yet",
      inconvertibleErrorCode()));
#endif
}

char *SharedMemoryMapper::prepare(ExecutorAddr Addr, size_t ContentSize) {
  auto R = Reservations.upper_bound(Addr);
  assert(R != Reservations.begin() && "Attempt to prepare unknown range");
  R--;

  ExecutorAddrDiff Offset = Addr - R->first;

  return static_cast<char *>(R->second.LocalAddr) + Offset;
}

void SharedMemoryMapper::initialize(MemoryMapper::AllocInfo &AI,
                                    OnInitializedFunction OnInitialized) {
  auto Reservation = Reservations.find(AI.MappingBase);
  assert(Reservation != Reservations.end() &&
         "Attempt to initialize unreserved range");

  tpctypes::SharedMemoryFinalizeRequest FR;

  AI.Actions.swap(FR.Actions);

  FR.Segments.reserve(AI.Segments.size());

  for (auto Segment : AI.Segments) {
    char *Base =
        static_cast<char *>(Reservation->second.LocalAddr) + Segment.Offset;
    std::memset(Base + Segment.ContentSize, 0, Segment.ZeroFillSize);

    tpctypes::SharedMemorySegFinalizeRequest SegReq;
    SegReq.Prot = tpctypes::toWireProtectionFlags(
        static_cast<sys::Memory::ProtectionFlags>(Segment.Prot));
    SegReq.Addr = AI.MappingBase + Segment.Offset;
    SegReq.Size = Segment.ContentSize + Segment.ZeroFillSize;

    FR.Segments.push_back(SegReq);
  }

  EPC.callSPSWrapperAsync<
      rt::SPSExecutorSharedMemoryMapperServiceInitializeSignature>(
      SAs.Initialize,
      [OnInitialized = std::move(OnInitialized)](
          Error SerializationErr, Expected<ExecutorAddr> Result) mutable {
        if (SerializationErr) {
          cantFail(Result.takeError());
          return OnInitialized(std::move(SerializationErr));
        }

        OnInitialized(std::move(Result));
      },
      SAs.Instance, AI.MappingBase, std::move(FR));
}

void SharedMemoryMapper::deinitialize(
    ArrayRef<ExecutorAddr> Allocations,
    MemoryMapper::OnDeinitializedFunction OnDeinitialized) {
  EPC.callSPSWrapperAsync<
      rt::SPSExecutorSharedMemoryMapperServiceDeinitializeSignature>(
      SAs.Deinitialize,
      [OnDeinitialized = std::move(OnDeinitialized)](Error SerializationErr,
                                                     Error Result) mutable {
        if (SerializationErr) {
          cantFail(std::move(Result));
          return OnDeinitialized(std::move(SerializationErr));
        }

        OnDeinitialized(std::move(Result));
      },
      SAs.Instance, Allocations);
}

void SharedMemoryMapper::release(ArrayRef<ExecutorAddr> Bases,
                                 OnReleasedFunction OnReleased) {
// OHOS_LOCAL
#if (defined(LLVM_ON_UNIX) && !defined(__ANDROID__) && !defined(__OHOS__)) || defined(_WIN32)
  Error Err = Error::success();

  {
    std::lock_guard<std::mutex> Lock(Mutex);

    for (auto Base : Bases) {

#if defined(LLVM_ON_UNIX)

      if (munmap(Reservations[Base].LocalAddr, Reservations[Base].Size) != 0)
        Err = joinErrors(std::move(Err), errorCodeToError(std::error_code(
                                             errno, std::generic_category())));

#elif defined(_WIN32)

      if (!UnmapViewOfFile(Reservations[Base].LocalAddr))
        Err = joinErrors(std::move(Err),
                         errorCodeToError(mapWindowsError(GetLastError())));

#endif

      Reservations.erase(Base);
    }
  }

  EPC.callSPSWrapperAsync<
      rt::SPSExecutorSharedMemoryMapperServiceReleaseSignature>(
      SAs.Release,
      [OnReleased = std::move(OnReleased),
       Err = std::move(Err)](Error SerializationErr, Error Result) mutable {
        if (SerializationErr) {
          cantFail(std::move(Result));
          return OnReleased(
              joinErrors(std::move(Err), std::move(SerializationErr)));
        }

        return OnReleased(joinErrors(std::move(Err), std::move(Result)));
      },
      SAs.Instance, Bases);
#else
  OnReleased(make_error<StringError>(
      "SharedMemoryMapper is not supported on this platform yet",
      inconvertibleErrorCode()));
#endif
}

SharedMemoryMapper::~SharedMemoryMapper() {
  std::vector<ExecutorAddr> ReservationAddrs;
  if (!Reservations.empty()) {
    std::lock_guard<std::mutex> Lock(Mutex);
    {
      ReservationAddrs.reserve(Reservations.size());
      for (const auto &R : Reservations) {
        ReservationAddrs.push_back(R.first);
      }
    }
  }

  std::promise<MSVCPError> P;
  auto F = P.get_future();
  release(ReservationAddrs, [&](Error Err) { P.set_value(std::move(Err)); });
  // FIXME: Release can actually fail. The error should be propagated.
  // Meanwhile, a better option is to explicitly call release().
  cantFail(F.get());
}

} // namespace orc

} // namespace llvm
