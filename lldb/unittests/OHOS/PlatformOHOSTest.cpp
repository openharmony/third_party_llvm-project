//===-- PlatformOHOSTest.cpp ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "Plugins/Platform/OHOS/PlatformOHOS.h"
#include "Plugins/Platform/OHOS/PlatformOHOSRemoteGDBServer.h"
#include "lldb/Utility/ArchSpec.h"

using namespace lldb_private;
using namespace lldb_private::platform_ohos;

// ---------------------------------------------------------------------------
// IsHostnameDeviceID
// ---------------------------------------------------------------------------

TEST(PlatformOHOSTest, IsHostnameDeviceID_EmptyString) {
  // Empty hostname is treated as a device ID (select default device).
  EXPECT_TRUE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID(""));
}

TEST(PlatformOHOSTest, IsHostnameDeviceID_Localhost) {
  EXPECT_FALSE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("localhost"));
}

TEST(PlatformOHOSTest, IsHostnameDeviceID_IPv4) {
  EXPECT_FALSE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("127.0.0.1"));
  EXPECT_FALSE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("192.168.1.1"));
  EXPECT_FALSE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("10.0.0.255"));
}

TEST(PlatformOHOSTest, IsHostnameDeviceID_IPv6) {
  EXPECT_FALSE(
      PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("2001:0db8:85a3:0000:0000:8a2e:0370:7334"));
  EXPECT_FALSE(
      PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("fe80:0000:0000:0000:0000:0000:0000:0001"));
}

TEST(PlatformOHOSTest, IsHostnameDeviceID_DeviceID) {
  // Anything that is not localhost/IPv4/IPv6 is a device ID.
  EXPECT_TRUE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("ABCDEF1234"));
  EXPECT_TRUE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("my-device"));
  EXPECT_TRUE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("127.0.0.1:8080"));
  EXPECT_TRUE(PlatformOHOSRemoteGDBServer::IsHostnameDeviceID("hostname"));
}

// ---------------------------------------------------------------------------
// GetMmapSymbolName
// ---------------------------------------------------------------------------

TEST(PlatformOHOSTest, GetMmapSymbolName_32Bit) {
  PlatformOHOS platform(false);
  ArchSpec arch32("arm-unknown-linux-ohos");
  EXPECT_EQ(platform.GetMmapSymbolName(arch32).GetStringRef(), "__lldb_mmap");
}

TEST(PlatformOHOSTest, GetMmapSymbolName_64Bit) {
  PlatformOHOS platform(false);
  ArchSpec arch64("aarch64-unknown-linux-ohos");
  // 64-bit falls through to PlatformLinux::GetMmapSymbolName which returns
  // "mmap" (the standard Linux symbol).
  EXPECT_EQ(platform.GetMmapSymbolName(arch64).GetStringRef(), "mmap");
}

// ---------------------------------------------------------------------------
// GetMmapArgumentList
// ---------------------------------------------------------------------------

// OHOS mmap constants (from PlatformOHOS.cpp).
#define OHOS_PROT_READ  1
#define OHOS_PROT_WRITE 2
#define OHOS_PROT_EXEC  4
#define OHOS_MAP_PRIVATE 2
#define OHOS_MAP_ANON   0x20
#define OHOS_MAP_JIT    0x1000

TEST(PlatformOHOSTest, GetMmapArgumentList_AnonPrivate64) {
  PlatformOHOS platform(false);
  ArchSpec arch64("aarch64-unknown-linux-ohos");

  auto args = platform.GetMmapArgumentList(
      arch64, /*addr=*/0x1000, /*length=*/4096,
      /*prot=*/OHOS_PROT_READ | OHOS_PROT_WRITE,
      /*flags=*/lldb_private::eMmapFlagsAnon | lldb_private::eMmapFlagsPrivate,
      /*fd=*/-1, /*offset=*/0);

  ASSERT_GE(args.size(), 6u);
  EXPECT_EQ(args[0], 0x1000u); // addr
  EXPECT_EQ(args[1], 4096u);   // length
  EXPECT_EQ(args[2], OHOS_PROT_READ | OHOS_PROT_WRITE); // prot
  EXPECT_EQ(args[3], OHOS_MAP_PRIVATE | OHOS_MAP_ANON); // flags
  EXPECT_EQ(args[4], static_cast<lldb::addr_t>(-1)); // fd
  EXPECT_EQ(args[5], 0u); // offset
}

TEST(PlatformOHOSTest, GetMmapArgumentList_AnonExec_HasMapJit) {
  PlatformOHOS platform(false);
  ArchSpec arch64("aarch64-unknown-linux-ohos");

  auto args = platform.GetMmapArgumentList(
      arch64, /*addr=*/0, /*length=*/4096,
      /*prot=*/OHOS_PROT_READ | OHOS_PROT_EXEC,
      /*flags=*/lldb_private::eMmapFlagsAnon | lldb_private::eMmapFlagsPrivate,
      /*fd=*/-1, /*offset=*/0);

  ASSERT_GE(args.size(), 6u);
  EXPECT_EQ(args[3], OHOS_MAP_PRIVATE | OHOS_MAP_ANON | OHOS_MAP_JIT);
}
