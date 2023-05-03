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

or

```
// OHOS_LOCAL
unsigned LoopSizeThreshold = Really(Long()).LineOfExtremely()->NecessaryCode();
```

or in case of multiline change:

```
// OHOS_LOCAL begin

Some local OHOS change

// OHOS_LOCAL end
```

The presence of such marks greatly simplifies porting such code snippets to new LLVM versions. All such changes MUST be accompanied with a test case that MUST fail should the change is reverted.

---------------------

### Commit title and message

Commit title and message should be in English.

Commit title should starts with tag in brackets (name of changed component), e.g.:

```
[lldb] Add support for backward debugging
```

or, in case of several components:

```
[lldb][mips][test] Add tests for support for backward debugging on MIPS
```

Commit message should briefly describe **how** it is achieved, e.g.:

```
[lldb] Add support for backward debugging

To support backward steps, auxiliary information is stored in SomeUsefulComponent
```

Commit message should contain a link to related issue:

```
[lldb] Add support for backward debugging

To support backward steps, auxiliary information is stored in SomeUsefulComponent

Issue: https://gitee.com/openharmony/third_party_llvm-project/issues/I6ZYBW
```

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
