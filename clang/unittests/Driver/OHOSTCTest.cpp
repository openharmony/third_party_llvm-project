//===- unittests/Driver/OHOSTCTest.cpp --- OHOS ToolChain tests -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Unit tests for the OHOS-specific ToolChain.
//
//===----------------------------------------------------------------------===//

#include "clang/Driver/ToolChain.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "gmock/gmock.h"
#include <iterator>

using namespace testing;

using namespace clang;
using namespace clang::driver;

namespace {

std::string rectifyPathSeparators(const char *S) {
  std::string Str(S);
  std::replace(Str.begin(), Str.end(), '\\', '/');
  return Str;
}

TEST(OHOSTCTest, VFSSysroot) {
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  struct TestDiagnosticConsumer : public DiagnosticConsumer {};
  IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem(
      new llvm::vfs::InMemoryFileSystem);

  InMemoryFileSystem->addFile("/ohos-sysroot", 0,
                              llvm::MemoryBuffer::getMemBuffer(""));
  InMemoryFileSystem->addFile("/include/libcxx-ohos/include/c++/v1", 0,
                              llvm::MemoryBuffer::getMemBuffer(""));

  DiagnosticsEngine Diags(DiagID, &*DiagOpts, new TestDiagnosticConsumer);
  Driver TheDriver("/bin/clang", "arm-linux-liteos", Diags,
                   "clang LLVM compiler", InMemoryFileSystem);
  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(
      {"-fsyntax-only", "--sysroot=/ohos-sysroot", "foo.cpp"}));
  ASSERT_TRUE(C);

  // Check that sysroot search uses provided VFS
  EXPECT_EQ(C->getDefaultToolChain().computeSysRoot(), "/ohos-sysroot");

  // Check additional include paths
  llvm::opt::ArgStringList Paths;
  C->getDefaultToolChain().AddClangCXXStdlibIncludeArgs(C->getArgs(), Paths);
  SmallVector<std::string> Converted;
  if (is_style_windows(llvm::sys::path::Style::native))
    std::transform(Paths.begin(), Paths.end(), std::back_inserter(Converted),
                   rectifyPathSeparators);
  else
    Converted = SmallVector<std::string>(Paths.begin(), Paths.end());

  ASSERT_THAT(Converted, Contains(llvm::StringRef(
                             "/bin/../include/libcxx-ohos/include/c++/v1")));
}

} // end anonymous namespace.
