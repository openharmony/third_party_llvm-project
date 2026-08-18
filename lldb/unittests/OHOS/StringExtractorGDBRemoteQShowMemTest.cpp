//===-- StringExtractorGDBRemoteQShowMemTest.cpp --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "lldb/Utility/StringExtractorGDBRemote.h"

using namespace lldb_private;

// ---------------------------------------------------------------------------
// qShowMem packet type recognition (OHOS !825 memory show command)
// ---------------------------------------------------------------------------

TEST(StringExtractorGDBRemoteQShowMemTest, RecognizesQShowMem) {
  StringExtractorGDBRemote ex("qShowMem");
  EXPECT_EQ(ex.GetServerPacketType(),
            StringExtractorGDBRemote::eServerPacketType_qShowMem);
}

TEST(StringExtractorGDBRemoteQShowMemTest, RecognizesQShowMemWithArgs) {
  // qShowMem may be followed by address arguments.
  StringExtractorGDBRemote ex("qShowMem:0x1000,0x100");
  EXPECT_EQ(ex.GetServerPacketType(),
            StringExtractorGDBRemote::eServerPacketType_qShowMem);
}

TEST(StringExtractorGDBRemoteQShowMemTest, CaseSensitive) {
  // Packet matching is case-sensitive.
  StringExtractorGDBRemote ex("qshowmem");
  EXPECT_NE(ex.GetServerPacketType(),
            StringExtractorGDBRemote::eServerPacketType_qShowMem);
}

TEST(StringExtractorGDBRemoteQShowMemTest, EmptyPacket) {
  StringExtractorGDBRemote ex("");
  EXPECT_NE(ex.GetServerPacketType(),
            StringExtractorGDBRemote::eServerPacketType_qShowMem);
}
