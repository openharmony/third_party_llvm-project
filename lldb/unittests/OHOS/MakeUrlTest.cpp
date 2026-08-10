//===-- MakeUrlTest.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "Plugins/Platform/gdb-server/PlatformRemoteGDBServer.h"

using namespace lldb_private;
using namespace lldb_private::platform_gdb_server;

// MakeUrl is a protected member, so we need a test helper.
class PlatformRemoteGDBServerTestHelper : public PlatformRemoteGDBServer {
public:
  using PlatformRemoteGDBServer::MakeUrl;
};

// ---------------------------------------------------------------------------
// MakeUrl OHOS behavior: unix-abstract-connect must not wrap hostname
// in brackets.  All other schemes wrap non-empty hostname in brackets.
// ---------------------------------------------------------------------------

TEST(MakeUrlTest, ConnectWithIPv4) {
  PlatformRemoteGDBServerTestHelper p;
  std::string url = p.MakeUrl("connect", "127.0.0.1", 1234, nullptr);
  EXPECT_EQ(url, "connect://[127.0.0.1]:1234");
}

TEST(MakeUrlTest, ConnectWithHostname) {
  PlatformRemoteGDBServerTestHelper p;
  std::string url = p.MakeUrl("connect", "myhost", 5678, nullptr);
  EXPECT_EQ(url, "connect://[myhost]:5678");
}

TEST(MakeUrlTest, ConnectEmptyHostname) {
  PlatformRemoteGDBServerTestHelper p;
  std::string url = p.MakeUrl("connect", "", 1234, nullptr);
  EXPECT_EQ(url, "connect://:1234");
}

TEST(MakeUrlTest, UnixAbstractConnect_NoHostname) {
  PlatformRemoteGDBServerTestHelper p;
  // unix-abstract-connect: hostname is completely omitted from the URL.
  std::string url = p.MakeUrl("unix-abstract-connect", "lldb-sock",
                               0, "/path");
  EXPECT_EQ(url, "unix-abstract-connect:///path");
}

TEST(MakeUrlTest, UnixConnect_WithBrackets) {
  PlatformRemoteGDBServerTestHelper p;
  // unix-connect (not abstract): hostname SHOULD be wrapped.
  std::string url = p.MakeUrl("unix-connect", "mysock", 0, "/path");
  EXPECT_EQ(url, "unix-connect://[mysock]/path");
}

TEST(MakeUrlTest, ConnectWithPortAndPath) {
  PlatformRemoteGDBServerTestHelper p;
  std::string url = p.MakeUrl("connect", "10.0.0.1", 9999, "/lldb.sock");
  EXPECT_EQ(url, "connect://[10.0.0.1]:9999/lldb.sock");
}

TEST(MakeUrlTest, ConnectZeroPort) {
  PlatformRemoteGDBServerTestHelper p;
  std::string url = p.MakeUrl("connect", "10.0.0.1", 0, nullptr);
  EXPECT_EQ(url, "connect://[10.0.0.1]");
}
