//===-- LZMA.cpp ----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Host/Config.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

#if LLDB_ENABLE_LZMA
// OHOS_LOCAL begin
#if !LLDB_ENABLE_LZMA_7ZIP
#include <lzma.h>
#else
#include <7zCrc.h>
#include <Xz.h>
#include <XzCrc64.h">
#define EXPAND_FACTOR 2
#endif
// OHOS_LOCAL end
#endif // LLDB_ENABLE_LZMA

namespace lldb_private {

namespace lzma {

#if !LLDB_ENABLE_LZMA
bool isAvailable() { return false; }
llvm::Expected<uint64_t>
getUncompressedSize(llvm::ArrayRef<uint8_t> InputBuffer) {
  llvm_unreachable("lzma::getUncompressedSize is unavailable");
}

llvm::Error uncompress(llvm::ArrayRef<uint8_t> InputBuffer,
                       llvm::SmallVectorImpl<uint8_t> &Uncompressed) {
  llvm_unreachable("lzma::uncompress is unavailable");
}

#else // LLDB_ENABLE_LZMA

bool isAvailable() { return true; }

// OHOS_LOCAL
#if !LLDB_ENABLE_LZMA_7ZIP
static const char *convertLZMACodeToString(lzma_ret Code) {
  switch (Code) {
  case LZMA_STREAM_END:
    return "lzma error: LZMA_STREAM_END";
  case LZMA_NO_CHECK:
    return "lzma error: LZMA_NO_CHECK";
  case LZMA_UNSUPPORTED_CHECK:
    return "lzma error: LZMA_UNSUPPORTED_CHECK";
  case LZMA_GET_CHECK:
    return "lzma error: LZMA_GET_CHECK";
  case LZMA_MEM_ERROR:
    return "lzma error: LZMA_MEM_ERROR";
  case LZMA_MEMLIMIT_ERROR:
    return "lzma error: LZMA_MEMLIMIT_ERROR";
  case LZMA_FORMAT_ERROR:
    return "lzma error: LZMA_FORMAT_ERROR";
  case LZMA_OPTIONS_ERROR:
    return "lzma error: LZMA_OPTIONS_ERROR";
  case LZMA_DATA_ERROR:
    return "lzma error: LZMA_DATA_ERROR";
  case LZMA_BUF_ERROR:
    return "lzma error: LZMA_BUF_ERROR";
  case LZMA_PROG_ERROR:
    return "lzma error: LZMA_PROG_ERROR";
  default:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

llvm::Expected<uint64_t>
getUncompressedSize(llvm::ArrayRef<uint8_t> InputBuffer) {
  lzma_stream_flags opts{};
  if (InputBuffer.size() < LZMA_STREAM_HEADER_SIZE) {
    return llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "size of xz-compressed blob (%lu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%lu bytes)",
        InputBuffer.size(), LZMA_STREAM_HEADER_SIZE);
  }

  // Decode xz footer.
  lzma_ret xzerr = lzma_stream_footer_decode(
      &opts, InputBuffer.take_back(LZMA_STREAM_HEADER_SIZE).data());
  if (xzerr != LZMA_OK) {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "lzma_stream_footer_decode()=%s",
                                   convertLZMACodeToString(xzerr));
  }
  if (InputBuffer.size() < (opts.backward_size + LZMA_STREAM_HEADER_SIZE)) {
    return llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "xz-compressed buffer size (%lu bytes) too small (required at "
        "least %lu bytes) ",
        InputBuffer.size(), (opts.backward_size + LZMA_STREAM_HEADER_SIZE));
  }

  // Decode xz index.
  lzma_index *xzindex;
  uint64_t memlimit(UINT64_MAX);
  size_t inpos = 0;
  xzerr = lzma_index_buffer_decode(
      &xzindex, &memlimit, nullptr,
      InputBuffer.take_back(LZMA_STREAM_HEADER_SIZE + opts.backward_size)
          .data(),
      &inpos, InputBuffer.size());
  if (xzerr != LZMA_OK) {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "lzma_index_buffer_decode()=%s",
                                   convertLZMACodeToString(xzerr));
  }

  // Get size of uncompressed file to construct an in-memory buffer of the
  // same size on the calling end (if needed).
  uint64_t uncompressedSize = lzma_index_uncompressed_size(xzindex);

  // Deallocate xz index as it is no longer needed.
  lzma_index_end(xzindex, nullptr);

  return uncompressedSize;
}

llvm::Error uncompress(llvm::ArrayRef<uint8_t> InputBuffer,
                       llvm::SmallVectorImpl<uint8_t> &Uncompressed) {
  llvm::Expected<uint64_t> uncompressedSize = getUncompressedSize(InputBuffer);

  if (auto err = uncompressedSize.takeError())
    return err;

  Uncompressed.resize(*uncompressedSize);

  // Decompress xz buffer to buffer.
  uint64_t memlimit = UINT64_MAX;
  size_t inpos = 0;
  size_t outpos = 0;
  lzma_ret ret = lzma_stream_buffer_decode(
      &memlimit, 0, nullptr, InputBuffer.data(), &inpos, InputBuffer.size(),
      Uncompressed.data(), &outpos, Uncompressed.size());
  if (ret != LZMA_OK) {
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "lzma_stream_buffer_decode()=%s",
                                   convertLZMACodeToString(ret));
  }

  return llvm::Error::success();
}
// OHOS_LOCAL begin
#else
static const char *convertLZMACodeToString(SRes Code) {
  switch (Code) {
  case SZ_ERROR_DATA:
    return "lzma error: SZ_ERROR_DATA";
  case SZ_ERROR_MEM:
    return "lzma error: SZ_ERROR_MEM";
  case SZ_ERROR_CRC:
    return "lzma error: SZ_ERROR_CRC";
  case SZ_ERROR_UNSUPPORTED:
    return "lzma error: SZ_ERROR_UNSUPPORTED";
  case SZ_ERROR_PARAM:
    return "lzma error: SZ_ERROR_PARAM";
  case SZ_ERROR_INPUT_EOF:
    return "lzma error: SZ_ERROR_INPUT_EOF";
  case SZ_ERROR_OUTPUT_EOF:
    return "lzma error: SZ_ERROR_OUTPUT_EOF";
  case SZ_ERROR_READ:
    return "lzma error: SZ_ERROR_READ";
  case SZ_ERROR_WRITE:
    return "lzma error: SZ_ERROR_WRITE";
  case SZ_ERROR_PROGRESS:
    return "lzma error: SZ_ERROR_PROGRESS";
  case SZ_ERROR_FAIL:
    return "lzma error: SZ_ERROR_FAIL";
  case SZ_ERROR_THREAD:
    return "lzma error: SZ_ERROR_THREAD";
  case SZ_ERROR_ARCHIVE:
    return "lzma error: SZ_ERROR_ARCHIVE";
  case SZ_ERROR_NO_ARCHIVE:
    return "lzma error: SZ_ERROR_NO_ARCHIVE";
  default:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

static void *XzAlloc(ISzAllocPtr, size_t size) {
    return malloc(size);
}

static void XzFree(ISzAllocPtr, void *address) {
    free(address);
}

llvm::Error uncompress(llvm::ArrayRef<uint8_t> InputBuffer,
                       llvm::SmallVectorImpl<uint8_t> &Uncompressed) {
  const uint8_t *src = InputBuffer.data();
  ISzAlloc alloc;
  CXzUnpacker state;
  alloc.Alloc = XzAlloc;
  alloc.Free = XzFree;
  XzUnpacker_Construct(&state, &alloc);
  CrcGenerateTable();
  Crc64GenerateTable();
  size_t srcOff = 0;
  size_t dstOff = 0;
  size_t srcLen = InputBuffer.size();
  std::vector<uint8_t> dst(srcLen, 0);
  ECoderStatus status = CODER_STATUS_NOT_FINISHED;
  while (status == CODER_STATUS_NOT_FINISHED) {
      dst.resize(dst.size() * EXPAND_FACTOR);
      size_t srcRemain = srcLen - srcOff;
      size_t dstRemain = dst.size() - dstOff;
      SRes res = XzUnpacker_Code(&state,
                                reinterpret_cast<Byte*>(&dst[dstOff]), &dstRemain,
                                reinterpret_cast<const Byte*>(&src[srcOff]), &srcRemain,
                                true, CODER_FINISH_ANY, &status);
      if (res != SZ_OK) {
          XzUnpacker_Free(&state);
          return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                  "XzUnpacker_Code()=%s", convertLZMACodeToString(res));
      }
      srcOff += srcRemain;
      dstOff += dstRemain;
  }
  XzUnpacker_Free(&state);
  if (!XzUnpacker_IsStreamWasFinished(&state)) {
      return llvm::createStringError(llvm::inconvertibleErrorCode(),
                      "XzUnpacker_IsStreamWasFinished()=lzma error: return False");
  }
  Uncompressed.resize(dstOff);
  memcpy(Uncompressed.data(), dst.data(), dstOff);
  return llvm::Error::success();
}
#endif
// OHOS_LOCAL end
#endif // LLDB_ENABLE_LZMA

} // end of namespace lzma
} // namespace lldb_private
