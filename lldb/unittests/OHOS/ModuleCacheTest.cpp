//===-- ModuleCacheTest.cpp -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include "Plugins/ObjectFile/ELF/ObjectFileELF.h"
#include "Plugins/SymbolFile/Symtab/SymbolFileSymtab.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/TestUtilities.h"
#include "lldb/Core/Module.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Target/ModuleCache.h"

#include <atomic>
#include <thread>
#include <vector>

using namespace lldb_private;
using namespace lldb;

namespace {

class ModuleCacheTest : public testing::Test {
  SubsystemRAII<FileSystem, HostInfo, ObjectFileELF, SymbolFileSymtab>
      subsystems;

public:
  void SetUp() override;

protected:
  FileSpec s_cache_dir;
  std::string s_test_executable;

  void TryGetAndPut(const FileSpec &cache_dir, const char *hostname,
                    bool expect_download);
};
} // namespace

static const char dummy_hostname[] = "dummy_hostname";
static const char dummy_remote_dir[] = "bin";
static const char module_name[] = "TestModule.so";
static const char module_uuid[] =
    "F4E7E991-9B61-6AD4-0073-561AC3D9FA10-C043A476";
static const size_t module_size = 5602;

static FileSpec GetDummyRemotePath() {
  FileSpec fs("/", FileSpec::Style::posix);
  fs.AppendPathComponent(dummy_remote_dir);
  fs.AppendPathComponent(module_name);
  return fs;
}

static FileSpec GetUuidView(FileSpec spec) {
  spec.AppendPathComponent(".cache");
  spec.AppendPathComponent(module_uuid);
  spec.AppendPathComponent(module_name);
  return spec;
}

static FileSpec GetSysrootView(FileSpec spec, const char *hostname) {
  spec.AppendPathComponent(hostname);
  spec.AppendPathComponent(dummy_remote_dir);
  spec.AppendPathComponent(module_name);
  return spec;
}

void ModuleCacheTest::SetUp() {
  s_cache_dir = HostInfo::GetProcessTempDir();
  s_test_executable = GetInputFilePath(module_name);
}

static void VerifyDiskState(const FileSpec &cache_dir, const char *hostname) {
  FileSpec uuid_view = GetUuidView(cache_dir);
  EXPECT_TRUE(FileSystem::Instance().Exists(uuid_view))
      << "uuid_view is: " << uuid_view.GetPath();
  EXPECT_EQ(module_size, FileSystem::Instance().GetByteSize(uuid_view));

  FileSpec sysroot_view = GetSysrootView(cache_dir, hostname);
  EXPECT_TRUE(FileSystem::Instance().Exists(sysroot_view))
      << "sysroot_view is: " << sysroot_view.GetPath();
  EXPECT_EQ(module_size, FileSystem::Instance().GetByteSize(sysroot_view));
}

void ModuleCacheTest::TryGetAndPut(const FileSpec &cache_dir,
                                   const char *hostname, bool expect_download) {
  ModuleCache mc;
  ModuleSpec module_spec;
  module_spec.GetFileSpec() = GetDummyRemotePath();
  module_spec.GetUUID().SetFromStringRef(module_uuid);
  module_spec.SetObjectSize(module_size);
  ModuleSP module_sp;
  bool did_create;
  bool download_called = false;

  Status error = mc.GetAndPut(
      cache_dir, hostname, module_spec,
      [&download_called, this](const ModuleSpec &module_spec,
                               const FileSpec &tmp_download_file_spec) {
        download_called = true;
        EXPECT_STREQ(GetDummyRemotePath().GetPath().c_str(),
                     module_spec.GetFileSpec().GetPath().c_str());
        std::error_code ec = llvm::sys::fs::copy_file(
            s_test_executable, tmp_download_file_spec.GetPath());
        EXPECT_FALSE(ec);
        return Status();
      },
      [](const ModuleSP &module_sp, const FileSpec &tmp_download_file_spec) {
        return Status::FromErrorString("Not supported.");
      },
      module_sp, &did_create);
  EXPECT_EQ(expect_download, download_called);

  EXPECT_TRUE(error.Success()) << "Error was: " << error.AsCString();
  EXPECT_TRUE(did_create);
  ASSERT_TRUE(bool(module_sp));

  SymbolContextList sc_list;
  module_sp->FindFunctionSymbols(ConstString("boom"), eFunctionNameTypeFull,
                                 sc_list);
  EXPECT_EQ(1u, sc_list.GetSize());
  EXPECT_STREQ(GetDummyRemotePath().GetPath().c_str(),
               module_sp->GetPlatformFileSpec().GetPath().c_str());
  EXPECT_STREQ(module_uuid, module_sp->GetUUID().GetAsString().c_str());
}

// Test that concurrent GetAndPut calls for the same module do not crash.
// This exercises the m_cache_mutex (recursive_mutex) on the in-memory
// m_loaded_modules map added by the OHOS port.
TEST_F(ModuleCacheTest, ConcurrentGetAndPut) {
  FileSpec test_cache_dir = s_cache_dir;
  test_cache_dir.AppendPathComponent("ConcurrentGetAndPut");

  // Use a shared ModuleCache to exercise m_cache_mutex on the
  // in-memory m_loaded_modules map.
  ModuleCache shared_mc;

  constexpr int kNumThreads = 4;
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};
  std::atomic<int> fail_count{0};

  for (int i = 0; i < kNumThreads; ++i) {
    threads.emplace_back([this, &test_cache_dir, &shared_mc, &success_count,
                          &fail_count]() {
      ModuleSpec module_spec;
      module_spec.GetFileSpec() = GetDummyRemotePath();
      module_spec.GetUUID().SetFromStringRef(module_uuid);
      module_spec.SetObjectSize(module_size);
      ModuleSP module_sp;
      bool did_create;
      Status error = shared_mc.GetAndPut(
          test_cache_dir, dummy_hostname, module_spec,
          [this](const ModuleSpec &, const FileSpec &tmp_file) {
            std::error_code ec =
                llvm::sys::fs::copy_file(s_test_executable, tmp_file.GetPath());
            return Status(ec);
          },
          [](const ModuleSP &, const FileSpec &) {
            return Status::FromErrorString("Not supported.");
          },
          module_sp, &did_create);
      if (error.Success())
        ++success_count;
      else
        ++fail_count;
    });
  }

  for (auto &t : threads)
    t.join();

  // At least one thread should have succeeded. Other threads may fail
  // due to lock-file contention on the filesystem, which is expected.
  // The primary goal is to verify no crash or data race (use
  // ThreadSanitizer for full coverage).
  EXPECT_GE(success_count.load(), 1);
  EXPECT_EQ(kNumThreads, success_count.load() + fail_count.load());

  // Cache should still be consistent.
  VerifyDiskState(test_cache_dir, dummy_hostname);
}

// Test that an existing sysroot file without a UUID is not replaced. This
// exercises the UUID validity check in CreateHostSysRootModuleLink.
TEST_F(ModuleCacheTest, InvalidExistingSysrootUuidIsNotReplaced) {
  llvm::SmallString<128> test_cache_dir_path;
  ASSERT_FALSE(llvm::sys::fs::createUniqueDirectory(
      "InvalidExistingSysrootUuid", test_cache_dir_path));
  FileSpec test_cache_dir(test_cache_dir_path);

  FileSpec sysroot_view = GetSysrootView(test_cache_dir, dummy_hostname);
  std::error_code ec = llvm::sys::fs::create_directories(
      sysroot_view.GetDirectory().GetCString());
  ASSERT_FALSE(ec);

  constexpr char invalid_module_contents[] = "not an ELF module";
  llvm::raw_fd_ostream invalid_module(sysroot_view.GetPath(), ec);
  ASSERT_FALSE(ec);
  invalid_module << invalid_module_contents;
  invalid_module.close();

  ASSERT_FALSE(
      std::make_shared<Module>(ModuleSpec(sysroot_view))->GetUUID().IsValid());

  ModuleCache mc;
  ModuleSpec module_spec;
  module_spec.GetFileSpec() = GetDummyRemotePath();
  module_spec.GetUUID().SetFromStringRef(module_uuid);
  module_spec.SetObjectSize(module_size);
  ModuleSP module_sp;
  bool did_create = false;
  bool download_called = false;

  Status error = mc.GetAndPut(
      test_cache_dir, dummy_hostname, module_spec,
      [&download_called, this](const ModuleSpec &, const FileSpec &tmp_file) {
        download_called = true;
        std::error_code ec =
            llvm::sys::fs::copy_file(s_test_executable, tmp_file.GetPath());
        return Status(ec);
      },
      [](const ModuleSP &, const FileSpec &) {
        return Status::FromErrorString("Not supported.");
      },
      module_sp, &did_create);
  EXPECT_TRUE(error.Success()) << "Error was: " << error.AsCString();
  EXPECT_TRUE(download_called);
  EXPECT_TRUE(FileSystem::Instance().Exists(sysroot_view));
  EXPECT_EQ(sizeof(invalid_module_contents) - 1,
            FileSystem::Instance().GetByteSize(sysroot_view));
}

// Test that a second GetAndPut with matching UUID skips re-download
// and reuses the cached module. This exercises the UUID match guard.
TEST_F(ModuleCacheTest, UuidMatchGuardSkipsRedownload) {
  FileSpec test_cache_dir = s_cache_dir;
  test_cache_dir.AppendPathComponent("UuidMatchGuard");

  // First call: should download.
  {
    ModuleCache mc;
    ModuleSpec module_spec;
    module_spec.GetFileSpec() = GetDummyRemotePath();
    module_spec.GetUUID().SetFromStringRef(module_uuid);
    module_spec.SetObjectSize(module_size);
    ModuleSP module_sp;
    bool did_create;
    bool download_called = false;
    Status error = mc.GetAndPut(
        test_cache_dir, dummy_hostname, module_spec,
        [&download_called, this](const ModuleSpec &, const FileSpec &tmp_file) {
          download_called = true;
          std::error_code ec =
              llvm::sys::fs::copy_file(s_test_executable, tmp_file.GetPath());
          return Status(ec);
        },
        [](const ModuleSP &, const FileSpec &) {
          return Status::FromErrorString("Not supported.");
        },
        module_sp, &did_create);
    EXPECT_TRUE(error.Success());
    EXPECT_TRUE(download_called);
    EXPECT_TRUE(did_create);
  }

  // Second call with the same ModuleCache: should find cached, no download.
  {
    ModuleCache mc2;
    ModuleSpec module_spec;
    module_spec.GetFileSpec() = GetDummyRemotePath();
    module_spec.GetUUID().SetFromStringRef(module_uuid);
    module_spec.SetObjectSize(module_size);
    ModuleSP module_sp;
    bool did_create;
    bool download_called = false;
    Status error = mc2.GetAndPut(
        test_cache_dir, dummy_hostname, module_spec,
        [&download_called, this](const ModuleSpec &, const FileSpec &tmp_file) {
          download_called = true;
          std::error_code ec =
              llvm::sys::fs::copy_file(s_test_executable, tmp_file.GetPath());
          return Status(ec);
        },
        [](const ModuleSP &, const FileSpec &) {
          return Status::FromErrorString("Not supported.");
        },
        module_sp, &did_create);
    EXPECT_TRUE(error.Success());
    EXPECT_FALSE(download_called) << "Second GetAndPut should reuse cache";
  }
}
