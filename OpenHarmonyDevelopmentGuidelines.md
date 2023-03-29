## Generic Guidelines

---------------------

### LLVM-related rules

All changes should adhere to LLVM Developer Policy / Coding Standarts:
- https://llvm.org/docs/CodingStandards.html
- https://llvm.org/docs/DeveloperPolicy.html

---------------------

### Mark changes properly

All OHOS-related changes to mainline LLVM / clang code MUST be clearly marked such as:

```
unsigned LoopSizeThreshold = 32; // OHOS_LOCAL
```

or in case of multiline change:

```
// OHOS_LOCAL begin

Some local OHOS change

// OHOS_LOCAL begin
```

The presence of such marks greatly simplifies porting such code snippets to new LLVM versions. All such changes MUST be accompanied with a test case that MUST fail should the change is reverted.

---------------------

### ABI Breakage

All ABI-breaking changes MUST be scheduled to a major toolchain releases. One should explicitly discuss and document such changes. Ideally ABI-breaking change should cause linking error, it should not cause silent and hard to track bugs.

---------------------

### Writing tests

Each pull request which changes existing functionality or adds new one should contains tests (modified or new ones). To add tests, please refer official LLVM documentation:
- General testing guide https://llvm.org/docs/TestingGuide.html
- Filecheck description https://llvm.org/docs/CommandGuide/FileCheck.html
- LIT description https://llvm.org/docs/CommandGuide/lit.html

And also you can check online presentations from LLVM Conferences about LLVM testing infrastructure, e.g.: https://youtu.be/isVQ8kYqaSA
