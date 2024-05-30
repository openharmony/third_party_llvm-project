//===-- sanitizer_libignore.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sanitizer_platform.h"

#if SANITIZER_FREEBSD || SANITIZER_LINUX || SANITIZER_APPLE || \
    SANITIZER_NETBSD

#include "sanitizer_libignore.h"
#include "sanitizer_flags.h"
#include "sanitizer_posix.h"
#include "sanitizer_procmaps.h"

namespace __sanitizer {

LibIgnore::LibIgnore(LinkerInitialized) {
}

void LibIgnore::AddIgnoredLibrary(const char *name_templ) {
  Lock lock(&mutex_);
  if (count_ >= kMaxLibs) {
    Report("%s: too many ignored libraries (max: %zu)\n", SanitizerToolName,
           kMaxLibs);
    Die();
  }
  VPrintf(2, "[Ignore] Add ignored library %s.\n", name_templ); // OHOS_LOCAL
  Lib *lib = &libs_[count_++];
  lib->templ = internal_strdup(name_templ);
  lib->name = nullptr;
  lib->real_name = nullptr;
  lib->loaded = false;
#if SANITIZER_OHOS
  lib->idx = 0; // OHOS_LOCAL
#endif
}

void LibIgnore::OnLibraryLoaded(const char *name) {
  Lock lock(&mutex_);
  // Try to match suppressions with symlink target.
  InternalMmapVector<char> buf(kMaxPathLength);
  if (name && internal_readlink(name, buf.data(), buf.size() - 1) > 0 &&
      buf[0]) {
    for (uptr i = 0; i < count_; i++) {
      Lib *lib = &libs_[i];
      if (!lib->loaded && (!lib->real_name) &&
          TemplateMatch(lib->templ, name))
        lib->real_name = internal_strdup(buf.data());
    }
  }

  // Scan suppressions list and find newly loaded and unloaded libraries.
  ListOfModules modules;
  modules.init();
  for (uptr i = 0; i < count_; i++) {
    Lib *lib = &libs_[i];
    bool loaded = false;
    for (const auto &mod : modules) {
      for (const auto &range : mod.ranges()) {
        if (!range.executable)
          continue;
        if (!TemplateMatch(lib->templ, mod.full_name()) &&
            !(lib->real_name &&
            internal_strcmp(lib->real_name, mod.full_name()) == 0))
          continue;
        if (loaded) {
          Report("%s: called_from_lib suppression '%s' is matched against"
                 " 2 libraries: '%s' and '%s'\n",
                 SanitizerToolName, lib->templ, lib->name, mod.full_name());
          Die();
        }
        loaded = true;
        if (lib->loaded) {
#if SANITIZER_OHOS
          // OHOS_LOCAL begin
          if (ignored_code_ranges_[lib->idx].begin == range.beg) {
            continue;
          }
          VReport(1,
                  "[Ignore] invalidate ignore code range 0x%zx-0x%zx from "
                  "library '%s' idx:0x%zx\n",
                  ignored_code_ranges_[lib->idx].begin,
                  ignored_code_ranges_[lib->idx].end, lib->name, lib->idx);
          // It is possible because of dlclose.
          atomic_store(&(ignored_code_ranges_[lib->idx].invalid), 1,
                       memory_order_release);
          // OHOS_LOCAL end
#else
          continue;
#endif
        }
        VReport(1,
                "Matched called_from_lib suppression '%s' against library"
                " '%s'\n",
                lib->templ, mod.full_name());
        lib->loaded = true;
        lib->name = internal_strdup(mod.full_name());
        const uptr idx =
            atomic_load(&ignored_ranges_count_, memory_order_relaxed);
#if SANITIZER_OHOS
        lib->idx = idx; // OHOS_LOCAL
#endif
        CHECK_LT(idx, ARRAY_SIZE(ignored_code_ranges_));
#if SANITIZER_OHOS
        // OHOS_LOCAL
        VReport(1,
                "[Ignore] Adding ignore code range 0x%zx-0x%zx from library "
                "'%s' idx:0x%zx\n",
                ignored_code_ranges_[idx].begin, ignored_code_ranges_[idx].end,
                lib->name, lib->idx);
#endif
        ignored_code_ranges_[idx].begin = range.beg;
        ignored_code_ranges_[idx].end = range.end;
        atomic_store(&ignored_ranges_count_, idx + 1, memory_order_release);
        break;
      }
    }
    if (lib->loaded && !loaded) {
#if SANITIZER_OHOS
      // OHOS_LOCAL begin
      VReport(1,
              "[Ignore] invalidate ignore code range 0x%zx-0x%zx from library "
              "'%s' idx:0x%zx\n",
              ignored_code_ranges_[lib->idx].begin,
              ignored_code_ranges_[lib->idx].end, lib->name, lib->idx);
      // Invalidate the address range when the library is unloaded.
      lib->loaded = false;
      atomic_store(&(ignored_code_ranges_[lib->idx].invalid), 1,
                   memory_order_release);
      // OHOS_LOCAL end
#else
      Report("%s: library '%s' that was matched against called_from_lib"
             " suppression '%s' is unloaded\n",
             SanitizerToolName, lib->name, lib->templ);
      Die();
#endif
    }
  }

  // Track instrumented ranges.
#if SANITIZER_OHOS
  // OHOS_LOCAL begin
  if (track_instrumented_libs_) {
    for (const auto &mod : modules) {
      for (const auto &range : mod.ranges()) {
        if (!range.executable)
          continue;
        if (!mod.instrumented()) {
          // Try to find if it overlap some instrumented ranges, if true, we
          // should invalidate them, This is happen when we dlclose an
          // instrumented module and reload an uninstrumented module.
          auto InvalidateCallback = [&](uptr idx) -> void {
            VReport(1,
                    "[Ignore] invalidate instrumented code range 0x%zx-0x%zx "
                    "from library '%s'\n",
                    instrumented_code_ranges_[idx].begin,
                    instrumented_code_ranges_[idx].end, mod.full_name());
            atomic_store(&(instrumented_code_ranges_[idx].invalid), 1,
                         memory_order_release);
          };
          IterateOverlappingRanges(range.beg, range.end, InvalidateCallback);
          continue;
        }
        // we need to check if whole range is instrumented, consider the
        // following scenario:
        // - instrumented module A is loaded at address[1-10]
        // - instrumented module B is loaded at address[20-30]
        // - unload A and B
        // - instrumented module C is load at address[5-25]
        // we need to make sure [10-20] will be instrumented so we should
        // compare the whole range.
        if (IsAddressRangeInstrumented(range.beg, range.end))
          continue;
        VReport(1, "Adding instrumented range 0x%zx-0x%zx from library '%s'\n",
                range.beg, range.end, mod.full_name());
        const uptr idx =
            atomic_load(&instrumented_ranges_count_, memory_order_relaxed);
        CHECK_LT(idx, ARRAY_SIZE(instrumented_code_ranges_));
        instrumented_code_ranges_[idx].begin = range.beg;
        instrumented_code_ranges_[idx].end = range.end;
        atomic_store(&instrumented_ranges_count_, idx + 1,
                     memory_order_release);
      }
    }
  }
  // OHOS_LOCAL end
#else
  if (track_instrumented_libs_) {
    for (const auto &mod : modules) {
      if (!mod.instrumented())
        continue;
      for (const auto &range : mod.ranges()) {
        if (!range.executable)
          continue;
        if (IsPcInstrumented(range.beg) && IsPcInstrumented(range.end - 1))
          continue;
        VReport(1, "Adding instrumented range 0x%zx-0x%zx from library '%s'\n",
                range.beg, range.end, mod.full_name());
        const uptr idx =
            atomic_load(&instrumented_ranges_count_, memory_order_relaxed);
        CHECK_LT(idx, ARRAY_SIZE(instrumented_code_ranges_));
        instrumented_code_ranges_[idx].begin = range.beg;
        instrumented_code_ranges_[idx].end = range.end;
        atomic_store(&instrumented_ranges_count_, idx + 1,
                     memory_order_release);
      }
    }
  }
#endif
}

void LibIgnore::OnLibraryUnloaded() {
  OnLibraryLoaded(nullptr);
}

} // namespace __sanitizer

#endif  // SANITIZER_FREEBSD || SANITIZER_LINUX || SANITIZER_APPLE ||
        // SANITIZER_NETBSD
