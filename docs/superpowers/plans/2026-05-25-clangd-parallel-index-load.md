# clangd Parallel Index Load Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make monolithic clangd `.idx` loading use caller-provided concurrency for RIFF section decoding and Dex construction while preserving the existing index file format.

**Architecture:** Add an `IndexLoadOptions` API that carries origin, Dex selection, and thread count. Keep the current serial path for `Threads == 1`, then add deterministic parallel section parsing and deterministic parallel Dex posting-list construction for `Threads > 1`.

**Tech Stack:** C++14, LLVM ADT/Error/ThreadPool, clangd RIFF index serialization, clangd Dex index, llvm-lit, GoogleTest, Google Benchmark.

---

## File Structure

- Modify `clang-tools-extra/clangd/index/Serialization.h`: declare `IndexLoadOptions`, threaded `readIndexFile()`, and threaded `loadIndex()`.
- Modify `clang-tools-extra/clangd/index/Serialization.cpp`: implement serial-compatible options plumbing, RIFF record boundary scanners, threaded section decoding, deterministic merge, and load-stage tracing.
- Modify `clang-tools-extra/clangd/index/dex/Dex.h`: add threaded Dex construction overloads.
- Modify `clang-tools-extra/clangd/index/dex/Dex.cpp`: implement threaded scoring and posting-list construction with deterministic merge.
- Modify `clang-tools-extra/clangd/tool/ClangdMain.cpp`: pass `-j` into static index file loading.
- Modify `clang-tools-extra/clangd/index/remote/server/Server.cpp`: add `--j`, reject zero, pass it to cold load and hot reload.
- Modify `clang-tools-extra/clangd/benchmarks/IndexBenchmark.cpp`: add an index-load thread flag and benchmark serial vs threaded build.
- Modify `clang-tools-extra/clangd/unittests/SerializationTests.cpp`: add semantic equivalence and deterministic error coverage.
- Modify `clang-tools-extra/clangd/test/index-tools.test`: exercise `IndexBenchmark` with index-load thread counts.
- Modify `clang-tools-extra/clangd/test/remote-index/pipeline.test`: exercise `clangd-index-server --j=2`.
- Modify `clang-tools-extra/clangd/index/remote/README.md`: document `clangd-index-server --j`.

## Task 1: Add Threaded Load API With Serial Semantics

**Files:**
- Modify: `clang-tools-extra/clangd/index/Serialization.h`
- Modify: `clang-tools-extra/clangd/index/Serialization.cpp`
- Modify: `clang-tools-extra/clangd/unittests/SerializationTests.cpp`

- [ ] **Step 1: Write the failing API equivalence test**

Add this helper and test near `BinaryConversions` in `clang-tools-extra/clangd/unittests/SerializationTests.cpp`:

```cpp
void expectSameIndexFile(const IndexFileIn &L, const IndexFileIn &R) {
  ASSERT_EQ(static_cast<bool>(L.Symbols), static_cast<bool>(R.Symbols));
  if (L.Symbols)
    EXPECT_THAT(yamlFromSymbols(*R.Symbols),
                UnorderedElementsAreArray(yamlFromSymbols(*L.Symbols)));

  ASSERT_EQ(static_cast<bool>(L.Refs), static_cast<bool>(R.Refs));
  if (L.Refs)
    EXPECT_THAT(yamlFromRefs(*R.Refs),
                UnorderedElementsAreArray(yamlFromRefs(*L.Refs)));

  ASSERT_EQ(static_cast<bool>(L.Relations), static_cast<bool>(R.Relations));
  if (L.Relations)
    EXPECT_THAT(yamlFromRelations(*R.Relations),
                UnorderedElementsAreArray(yamlFromRelations(*L.Relations)));

  ASSERT_EQ(static_cast<bool>(L.Cmd), static_cast<bool>(R.Cmd));
  if (L.Cmd) {
    EXPECT_EQ(R.Cmd->Directory, L.Cmd->Directory);
    EXPECT_EQ(R.Cmd->CommandLine, L.Cmd->CommandLine);
  }

  ASSERT_EQ(static_cast<bool>(L.Sources), static_cast<bool>(R.Sources));
  if (L.Sources) {
    ASSERT_EQ(R.Sources->size(), L.Sources->size());
    for (const auto &Entry : *L.Sources) {
      ASSERT_TRUE(R.Sources->count(Entry.getKey()));
      const auto &Other = R.Sources->lookup(Entry.getKey());
      EXPECT_EQ(Other.URI, Entry.getValue().URI);
      EXPECT_EQ(Other.Flags, Entry.getValue().Flags);
      EXPECT_EQ(Other.Digest, Entry.getValue().Digest);
      EXPECT_EQ(Other.DirectIncludes, Entry.getValue().DirectIncludes);
    }
  }
}

TEST(SerializationTest, BinaryConversionsWithThreadedAPI) {
  auto In = readIndexFile(YAML);
  ASSERT_TRUE(bool(In)) << In.takeError();

  IndexFileOut Out(*In);
  Out.Format = IndexFileFormat::RIFF;
  std::string Serialized = llvm::to_string(Out);

  auto Serial = readIndexFile(Serialized, SymbolOrigin::Static, 1);
  ASSERT_TRUE(bool(Serial)) << Serial.takeError();
  auto Threaded = readIndexFile(Serialized, SymbolOrigin::Static, 4);
  ASSERT_TRUE(bool(Threaded)) << Threaded.takeError();
  expectSameIndexFile(*Serial, *Threaded);
}
```

- [ ] **Step 2: Run the targeted test and verify it fails to compile**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.BinaryConversionsWithThreadedAPI
```

Expected: the build fails because `readIndexFile(Serialized, SymbolOrigin::Static, 4)` is not declared.

- [ ] **Step 3: Add the public options API**

In `clang-tools-extra/clangd/index/Serialization.h`, replace the existing `readIndexFile()` and `loadIndex()` declarations with:

```cpp
// Parse an index file. The input must be a RIFF or YAML file.
llvm::Expected<IndexFileIn> readIndexFile(llvm::StringRef, SymbolOrigin,
                                          unsigned Threads);
inline llvm::Expected<IndexFileIn> readIndexFile(llvm::StringRef Data,
                                                 SymbolOrigin Origin) {
  return readIndexFile(Data, Origin, 1);
}

struct IndexLoadOptions {
  SymbolOrigin Origin = SymbolOrigin::Static;
  bool UseDex = true;
  unsigned Threads = 1;
};

// Build an in-memory static index from an index file.
std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef Filename,
                                       IndexLoadOptions Opts);
std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef Filename,
                                       SymbolOrigin Origin,
                                       bool UseDex = true);
```

- [ ] **Step 4: Add serial-compatible implementation plumbing**

In `clang-tools-extra/clangd/index/Serialization.cpp`, change `readIndexFile()` and `loadIndex()` to this shape:

```cpp
llvm::Expected<IndexFileIn> readIndexFile(llvm::StringRef Data,
                                          SymbolOrigin Origin,
                                          unsigned Threads) {
  if (Data.startswith("RIFF")) {
    return readRIFF(Data, Origin, Threads);
  }
  if (auto YAMLContents = readYAML(Data, Origin)) {
    return std::move(*YAMLContents);
  } else {
    return error("Not a RIFF file and failed to parse as YAML: {0}",
                 YAMLContents.takeError());
  }
}

std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef SymbolFilename,
                                       IndexLoadOptions Opts) {
  trace::Span OverallTracer("LoadIndex");
  SPAN_ATTACH(OverallTracer, "threads", static_cast<int>(Opts.Threads));
  auto Buffer = llvm::MemoryBuffer::getFile(SymbolFilename);
  if (!Buffer) {
    elog("Can't open {0}: {1}", SymbolFilename, Buffer.getError().message());
    return nullptr;
  }

  SymbolSlab Symbols;
  RefSlab Refs;
  RelationSlab Relations;
  {
    trace::Span Tracer("ParseIndex");
    if (auto I = readIndexFile(Buffer->get()->getBuffer(), Opts.Origin,
                               Opts.Threads)) {
      if (I->Symbols)
        Symbols = std::move(*I->Symbols);
      if (I->Refs)
        Refs = std::move(*I->Refs);
      if (I->Relations)
        Relations = std::move(*I->Relations);
    } else {
      elog("Bad index file: {0}", I.takeError());
      return nullptr;
    }
  }

  size_t NumSym = Symbols.size();
  size_t NumRefs = Refs.numRefs();
  size_t NumRelations = Relations.size();

  trace::Span Tracer("BuildIndex");
  auto Index = Opts.UseDex ? dex::Dex::build(std::move(Symbols), std::move(Refs),
                                             std::move(Relations), Opts.Threads)
                           : MemIndex::build(std::move(Symbols), std::move(Refs),
                                             std::move(Relations));
  vlog("Loaded {0} from {1} with estimated memory usage {2} bytes\n"
       "  - number of symbols: {3}\n"
       "  - number of refs: {4}\n"
       "  - number of relations: {5}\n"
       "  - index load threads: {6}",
       Opts.UseDex ? "Dex" : "MemIndex", SymbolFilename,
       Index->estimateMemoryUsage(), NumSym, NumRefs, NumRelations,
       Opts.Threads);
  return Index;
}

std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef SymbolFilename,
                                       SymbolOrigin Origin, bool UseDex) {
  IndexLoadOptions Opts;
  Opts.Origin = Origin;
  Opts.UseDex = UseDex;
  Opts.Threads = 1;
  return loadIndex(SymbolFilename, Opts);
}
```

Also change the private declaration and definition from `readRIFF(Data, Origin)` to `readRIFF(Data, Origin, Threads)`. In this task, `readRIFF()` should ignore `Threads` and keep the current serial body.

- [ ] **Step 5: Add the Dex build compatibility overload**

In `clang-tools-extra/clangd/index/dex/Dex.h`, add this overload beside the existing `build()` declaration:

```cpp
static std::unique_ptr<SymbolIndex> build(SymbolSlab, RefSlab, RelationSlab,
                                          unsigned Threads);
```

In `clang-tools-extra/clangd/index/dex/Dex.cpp`, implement it as:

```cpp
std::unique_ptr<SymbolIndex> Dex::build(SymbolSlab Symbols, RefSlab Refs,
                                        RelationSlab Rels, unsigned Threads) {
  auto Size = Symbols.bytes() + Refs.bytes();
  auto Data = std::make_pair(std::move(Symbols), std::move(Refs));
  return std::make_unique<Dex>(Data.first, Data.second, Rels, std::move(Data),
                               Size, Threads);
}
```

Then update the existing three-argument `Dex::build()` to call the new overload with `Threads = 1`.

- [ ] **Step 6: Add threaded Dex constructor plumbing without behavior changes**

In `clang-tools-extra/clangd/index/dex/Dex.h`, add `unsigned Threads = 1` to the template constructors and call `buildIndex(Threads)`. The first constructor body should end with:

```cpp
buildIndex(Threads);
```

Change the private declaration to:

```cpp
void buildIndex(unsigned Threads);
```

In `Dex.cpp`, change `void Dex::buildIndex()` to `void Dex::buildIndex(unsigned Threads)` and add this first line:

```cpp
(void)Threads;
```

- [ ] **Step 7: Run the targeted test and commit**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.BinaryConversionsWithThreadedAPI
```

Expected: the test passes.

Commit:

```bash
git add clang-tools-extra/clangd/index/Serialization.h clang-tools-extra/clangd/index/Serialization.cpp clang-tools-extra/clangd/index/dex/Dex.h clang-tools-extra/clangd/index/dex/Dex.cpp clang-tools-extra/clangd/unittests/SerializationTests.cpp
git commit -m "clangd: add threaded index load API"
```

## Task 2: Add RIFF Record Boundary Scanners

**Files:**
- Modify: `clang-tools-extra/clangd/index/Serialization.cpp`
- Modify: `clang-tools-extra/clangd/unittests/SerializationTests.cpp`

- [ ] **Step 1: Add threaded malformed input tests**

Add this helper after `NoCrashOnBadArraySize` in `SerializationTests.cpp`:

```cpp
std::string corruptChunkPayload(llvm::StringRef Serialized,
                                llvm::StringRef ChunkName,
                                llvm::StringRef Payload) {
  auto Parsed = riff::readFile(Serialized);
  EXPECT_FALSE(!Parsed) << Parsed.takeError();
  auto Chunk = llvm::find_if(Parsed->Chunks, [&](riff::Chunk C) {
    return C.ID == riff::fourCC(ChunkName);
  });
  EXPECT_NE(Chunk, Parsed->Chunks.end());
  Chunk->Data = Payload;
  return llvm::to_string(*Parsed);
}
```

Add these tests:

```cpp
TEST(SerializationTest, ThreadedReadReportsMalformedSymbol) {
  auto In = readIndexFile(YAML);
  ASSERT_FALSE(!In) << In.takeError();
  IndexFileOut Out(*In);
  Out.Format = IndexFileFormat::RIFF;
  std::string Serialized = llvm::to_string(Out);

  std::string Corrupt = corruptChunkPayload(Serialized, "symb",
                                            llvm::StringRef("\x01\x02", 2));
  auto Parsed = readIndexFile(Corrupt, SymbolOrigin::Static, 4);
  ASSERT_TRUE(!Parsed);
  EXPECT_EQ(llvm::toString(Parsed.takeError()),
            "malformed or truncated symbol");
}

TEST(SerializationTest, ThreadedReadReportsMalformedRefs) {
  auto In = readIndexFile(YAML);
  ASSERT_FALSE(!In) << In.takeError();
  IndexFileOut Out(*In);
  Out.Format = IndexFileFormat::RIFF;
  std::string Serialized = llvm::to_string(Out);

  std::string Corrupt = corruptChunkPayload(Serialized, "refs",
                                            llvm::StringRef("\x01\x02", 2));
  auto Parsed = readIndexFile(Corrupt, SymbolOrigin::Static, 4);
  ASSERT_TRUE(!Parsed);
  EXPECT_EQ(llvm::toString(Parsed.takeError()),
            "malformed or truncated refs");
}
```

- [ ] **Step 2: Run the new tests before implementing scanners**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.ThreadedReadReportsMalformed*
```

Expected: the tests pass through the serial implementation. They lock down error text before parallel parsing changes it.

- [ ] **Step 3: Extend `Reader` with offsets**

In `Serialization.cpp`, update `Reader` fields and constructor:

```cpp
class Reader {
  const char *Start, *Begin, *End;
  bool Err = false;

public:
  Reader(llvm::StringRef Data)
      : Start(Data.begin()), Begin(Data.begin()), End(Data.end()) {}
  void fail() { Err = true; }
  size_t offset() const { return Begin - Start; }
```

Keep all existing methods unchanged except for the constructor field initialization.

- [ ] **Step 4: Add record range and skip helpers**

Add these definitions in the anonymous namespace after `readCompileCommand()`:

```cpp
struct RecordRange {
  size_t Begin = 0;
  size_t End = 0;
  size_t Ordinal = 0;
};

constexpr size_t SerializedDigestSize = FileDigest{}.size();

bool consumeElementCount(Reader &Data, uint32_t &Count) {
  Count = Data.consumeVar();
  if (Count > static_cast<uint32_t>(Data.rest().size())) {
    Data.fail();
    return false;
  }
  return !Data.err();
}

void skipLocation(Reader &Data, llvm::ArrayRef<llvm::StringRef> Strings) {
  Data.consumeString(Strings);
  Data.consumeVar();
  Data.consumeVar();
  Data.consumeVar();
  Data.consumeVar();
}

void skipIncludeGraphNode(Reader &Data,
                          llvm::ArrayRef<llvm::StringRef> Strings) {
  Data.consume8();
  Data.consumeString(Strings);
  Data.consume(SerializedDigestSize);
  uint32_t Includes = 0;
  if (!consumeElementCount(Data, Includes))
    return;
  for (uint32_t I = 0; I < Includes; ++I)
    Data.consumeString(Strings);
}

void skipSymbol(Reader &Data, llvm::ArrayRef<llvm::StringRef> Strings) {
  Data.consumeID();
  Data.consume8();
  Data.consume8();
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  skipLocation(Data, Strings);
  skipLocation(Data, Strings);
  Data.consumeVar();
  Data.consume8();
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  Data.consumeString(Strings);
  uint32_t Includes = 0;
  if (!consumeElementCount(Data, Includes))
    return;
  for (uint32_t I = 0; I < Includes; ++I) {
    Data.consumeString(Strings);
    Data.consumeVar();
  }
}

void skipRefs(Reader &Data, llvm::ArrayRef<llvm::StringRef> Strings) {
  Data.consumeID();
  uint32_t Refs = 0;
  if (!consumeElementCount(Data, Refs))
    return;
  for (uint32_t I = 0; I < Refs; ++I) {
    Data.consume8();
    skipLocation(Data, Strings);
    Data.consumeID();
  }
}

void skipRelationRecord(Reader &Data) {
  Data.consumeID();
  Data.consume8();
  Data.consumeID();
}
```

- [ ] **Step 5: Add a reusable scanner**

Add this function after the skip helpers:

```cpp
template <typename Skip>
llvm::Expected<std::vector<RecordRange>>
scanRecords(llvm::StringRef Data, llvm::ArrayRef<llvm::StringRef> Strings,
            Skip SkipRecord, llvm::StringRef ErrorMessage) {
  Reader R(Data);
  std::vector<RecordRange> Result;
  size_t Ordinal = 0;
  while (!R.eof()) {
    size_t Begin = R.offset();
    SkipRecord(R, Strings);
    size_t End = R.offset();
    if (R.err() || End <= Begin)
      return error("{0}", ErrorMessage);
    Result.push_back({Begin, End, Ordinal++});
  }
  return std::move(Result);
}

llvm::Expected<std::vector<RecordRange>>
scanRelationRecords(llvm::StringRef Data, llvm::StringRef ErrorMessage) {
  Reader R(Data);
  std::vector<RecordRange> Result;
  size_t Ordinal = 0;
  while (!R.eof()) {
    size_t Begin = R.offset();
    skipRelationRecord(R);
    size_t End = R.offset();
    if (R.err() || End <= Begin)
      return error("{0}", ErrorMessage);
    Result.push_back({Begin, End, Ordinal++});
  }
  return std::move(Result);
}
```

- [ ] **Step 6: Use scanners for threaded `readRIFF()` without changing output**

In `readRIFF()`, before each large section's current decoding loop, call the
scanner only when `Threads > 1` and fail with the same error. For example,
before the `symb` loop:

```cpp
if (Threads > 1) {
  auto SymbolRanges = scanRecords(
      Chunks.lookup("symb"), Strings->Strings,
      [](Reader &R, llvm::ArrayRef<llvm::StringRef> S) { skipSymbol(R, S); },
      "malformed or truncated symbol");
  if (!SymbolRanges)
    return SymbolRanges.takeError();
}
```

Before the `srcs` loop:

```cpp
if (Threads > 1) {
  auto SourceRanges = scanRecords(
      Chunks.lookup("srcs"), Strings->Strings,
      [](Reader &R, llvm::ArrayRef<llvm::StringRef> S) {
        skipIncludeGraphNode(R, S);
      },
      "malformed or truncated include uri");
  if (!SourceRanges)
    return SourceRanges.takeError();
}
```

Before the `refs` loop:

```cpp
if (Threads > 1) {
  auto RefRanges = scanRecords(
      Chunks.lookup("refs"), Strings->Strings,
      [](Reader &R, llvm::ArrayRef<llvm::StringRef> S) { skipRefs(R, S); },
      "malformed or truncated refs");
  if (!RefRanges)
    return RefRanges.takeError();
}
```

Before the `rela` loop:

```cpp
if (Threads > 1) {
  auto RelationRanges =
      scanRelationRecords(Chunks.lookup("rela"),
                          "malformed or truncated relations");
  if (!RelationRanges)
    return RelationRanges.takeError();
}
```

Keep the existing decoding loops in place for this task.

- [ ] **Step 7: Run serialization tests and commit**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.*
```

Expected: all `SerializationTest` tests pass.

Commit:

```bash
git add clang-tools-extra/clangd/index/Serialization.cpp clang-tools-extra/clangd/unittests/SerializationTests.cpp
git commit -m "clangd: scan RIFF index record boundaries"
```

## Task 3: Parallelize RIFF Section Decoding

**Files:**
- Modify: `clang-tools-extra/clangd/index/Serialization.cpp`
- Modify: `clang-tools-extra/clangd/unittests/SerializationTests.cpp`

- [ ] **Step 1: Add a duplicate source-order regression test**

Add this test to `SerializationTests.cpp`:

```cpp
TEST(SerializationTest, ThreadedReadPreservesDuplicateSourceFileOrder) {
  auto In = readIndexFile(YAML);
  ASSERT_FALSE(!In) << In.takeError();

  IndexFileOut Out(*In);
  Out.Format = IndexFileFormat::RIFF;
  std::string Serialized = llvm::to_string(Out);

  auto ParsedFile = riff::readFile(Serialized);
  ASSERT_FALSE(!ParsedFile) << ParsedFile.takeError();
  auto Srcs = llvm::find_if(ParsedFile->Chunks, [](riff::Chunk C) {
    return C.ID == riff::fourCC("srcs");
  });
  ASSERT_NE(Srcs, ParsedFile->Chunks.end());
  ASSERT_FALSE(Srcs->Data.empty());

  std::string FirstRecord = Srcs->Data.str();
  std::string SecondRecord = FirstRecord;
  SecondRecord[0] = static_cast<char>(IncludeGraphNode::SourceFlag::HadErrors);
  std::string CombinedRecords = FirstRecord + SecondRecord;
  Srcs->Data = CombinedRecords;
  std::string DuplicateSources = llvm::to_string(*ParsedFile);

  auto Parsed = readIndexFile(DuplicateSources, SymbolOrigin::Static, 4);
  ASSERT_TRUE(bool(Parsed)) << Parsed.takeError();
  ASSERT_TRUE(Parsed->Sources);
  const auto *URI = "file:///path/source1.cpp";
  ASSERT_TRUE(Parsed->Sources->count(URI));
  EXPECT_EQ(Parsed->Sources->lookup(URI).Flags,
            IncludeGraphNode::SourceFlag::HadErrors);
}
```

This test creates two serialized `srcs` records with the same URI and different
flags, then verifies that the later file record wins after threaded parsing.

- [ ] **Step 2: Run the duplicate-order test**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.ThreadedReadPreservesDuplicateSourceFileOrder
```

Expected: the test passes through serial behavior before threaded parsing is implemented.

- [ ] **Step 3: Add parallel execution helpers**

In `Serialization.cpp`, include:

```cpp
#include "llvm/Support/ThreadPool.h"
#include <future>
#include <iterator>
#include <memory>
```

Add these helpers near the record range helpers:

```cpp
struct ChunkRange {
  size_t BeginRecord = 0;
  size_t EndRecord = 0;
};

std::vector<ChunkRange> splitRecordRanges(llvm::ArrayRef<RecordRange> Records,
                                          unsigned Threads) {
  std::vector<ChunkRange> Chunks;
  if (Records.empty())
    return Chunks;
  unsigned TaskCount = std::min<unsigned>(Threads, Records.size());
  Chunks.reserve(TaskCount);
  size_t RecordsPerTask = Records.size() / TaskCount;
  size_t Extra = Records.size() % TaskCount;
  size_t Begin = 0;
  for (unsigned I = 0; I < TaskCount; ++I) {
    size_t Count = RecordsPerTask + (I < Extra ? 1 : 0);
    Chunks.push_back({Begin, Begin + Count});
    Begin += Count;
  }
  return Chunks;
}
```

- [ ] **Step 4: Add partial result structs**

Add these structs near `RecordRange`:

```cpp
struct SymbolRecord {
  size_t Ordinal = 0;
  Symbol Value;
};

struct SourceRecord {
  size_t Ordinal = 0;
  IncludeGraphNode Value;
};

struct RefRecord {
  SymbolID ID;
  std::vector<Ref> Refs;
};

struct ParsedSections {
  llvm::Optional<std::vector<SourceRecord>> Sources;
  llvm::Optional<std::vector<SymbolRecord>> Symbols;
  llvm::Optional<std::vector<RefRecord>> Refs;
  llvm::Optional<std::vector<Relation>> Relations;
  llvm::Optional<tooling::CompileCommand> Cmd;
};
```

- [ ] **Step 5: Add range parsers**

Add these functions in `Serialization.cpp`:

```cpp
llvm::Expected<std::vector<SymbolRecord>>
parseSymbolRanges(llvm::StringRef Data, llvm::ArrayRef<RecordRange> Ranges,
                  llvm::ArrayRef<llvm::StringRef> Strings,
                  SymbolOrigin Origin) {
  std::vector<SymbolRecord> Result;
  Result.reserve(Ranges.size());
  for (const RecordRange &Range : Ranges) {
    Reader R(Data.slice(Range.Begin, Range.End));
    Symbol Sym = readSymbol(R, Strings, Origin);
    if (R.err() || !R.eof())
      return error("malformed or truncated symbol");
    Result.push_back({Range.Ordinal, std::move(Sym)});
  }
  return std::move(Result);
}

llvm::Expected<std::vector<SourceRecord>>
parseSourceRanges(llvm::StringRef Data, llvm::ArrayRef<RecordRange> Ranges,
                  llvm::ArrayRef<llvm::StringRef> Strings) {
  std::vector<SourceRecord> Result;
  Result.reserve(Ranges.size());
  for (const RecordRange &Range : Ranges) {
    Reader R(Data.slice(Range.Begin, Range.End));
    IncludeGraphNode IGN = readIncludeGraphNode(R, Strings);
    if (R.err() || !R.eof())
      return error("malformed or truncated include uri");
    Result.push_back({Range.Ordinal, std::move(IGN)});
  }
  return std::move(Result);
}

llvm::Expected<std::vector<RefRecord>>
parseRefRanges(llvm::StringRef Data, llvm::ArrayRef<RecordRange> Ranges,
               llvm::ArrayRef<llvm::StringRef> Strings) {
  std::vector<RefRecord> Result;
  Result.reserve(Ranges.size());
  for (const RecordRange &Range : Ranges) {
    Reader R(Data.slice(Range.Begin, Range.End));
    auto RefsBundle = readRefs(R, Strings);
    if (R.err() || !R.eof())
      return error("malformed or truncated refs");
    Result.push_back({RefsBundle.first, std::move(RefsBundle.second)});
  }
  return std::move(Result);
}

llvm::Expected<std::vector<Relation>>
parseRelationRanges(llvm::StringRef Data, llvm::ArrayRef<RecordRange> Ranges) {
  std::vector<Relation> Result;
  Result.reserve(Ranges.size());
  for (const RecordRange &Range : Ranges) {
    Reader R(Data.slice(Range.Begin, Range.End));
    Relation Rel = readRelation(R);
    if (R.err() || !R.eof())
      return error("malformed or truncated relations");
    Result.push_back(Rel);
  }
  return std::move(Result);
}
```

- [ ] **Step 6: Add a generic threaded parse helper**

Add this function template:

```cpp
template <typename T, typename Parse>
llvm::Expected<std::vector<T>>
parseRecordRangesThreaded(llvm::ArrayRef<RecordRange> Ranges,
                          unsigned Threads, Parse ParseChunk,
                          llvm::StringRef ErrorMessage) {
  if (Threads <= 1 || Ranges.size() < 2)
    return ParseChunk(Ranges);

  llvm::ThreadPool Pool(llvm::hardware_concurrency(Threads));
  using WorkerResult = std::shared_ptr<llvm::Expected<std::vector<T>>>;
  std::vector<std::shared_future<WorkerResult>> Futures;
  std::vector<ChunkRange> Chunks = splitRecordRanges(Ranges, Threads);
  Futures.reserve(Chunks.size());
  for (ChunkRange Chunk : Chunks) {
    Futures.push_back(Pool.async([=, &Ranges, &ParseChunk]() {
      return std::make_shared<llvm::Expected<std::vector<T>>>(
          ParseChunk(Ranges.slice(Chunk.BeginRecord,
                                  Chunk.EndRecord - Chunk.BeginRecord)));
    }));
  }
  Pool.wait();

  std::vector<T> Result;
  for (auto &Future : Futures) {
    WorkerResult PartialPtr = Future.get();
    llvm::Expected<std::vector<T>> &Partial = *PartialPtr;
    if (!Partial)
      return error("{0}", ErrorMessage);
    Result.insert(Result.end(), std::make_move_iterator(Partial->begin()),
                  std::make_move_iterator(Partial->end()));
  }
  return std::move(Result);
}
```

- [ ] **Step 7: Use threaded section parsing in `readRIFF()`**

First, move the current section-decoding body of `readRIFF()` into a helper with
this signature:

```cpp
llvm::Expected<IndexFileIn>
readRIFFSectionsSerial(const llvm::StringMap<llvm::StringRef> &Chunks,
                       llvm::ArrayRef<llvm::StringRef> Strings,
                       SymbolOrigin Origin);
```

The helper starts with `IndexFileIn Result;`, contains the current `srcs`,
`symb`, `refs`, `rela`, and `cmdl` decoding blocks, and ends with
`return std::move(Result);`.

In `readRIFF()`, call the helper before the threaded path:

```cpp
if (Threads <= 1)
  return readRIFFSectionsSerial(Chunks, Strings->Strings, Origin);
```

For the `Threads > 1` path, replace the direct section loops with:

```cpp
ParsedSections Sections;
```

For `symb`, after scanning:

```cpp
auto ParsedSymbols = parseRecordRangesThreaded<SymbolRecord>(
    *SymbolRanges, Threads,
    [&](llvm::ArrayRef<RecordRange> Ranges) {
      return parseSymbolRanges(Chunks.lookup("symb"), Ranges,
                               Strings->Strings, Origin);
    },
    "malformed or truncated symbol");
if (!ParsedSymbols)
  return ParsedSymbols.takeError();
Sections.Symbols = std::move(*ParsedSymbols);
```

For `srcs`, after scanning:

```cpp
auto ParsedSources = parseRecordRangesThreaded<SourceRecord>(
    *SourceRanges, Threads,
    [&](llvm::ArrayRef<RecordRange> Ranges) {
      return parseSourceRanges(Chunks.lookup("srcs"), Ranges,
                               Strings->Strings);
    },
    "malformed or truncated include uri");
if (!ParsedSources)
  return ParsedSources.takeError();
Sections.Sources = std::move(*ParsedSources);
```

For `refs`, after scanning:

```cpp
auto ParsedRefs = parseRecordRangesThreaded<RefRecord>(
    *RefRanges, Threads,
    [&](llvm::ArrayRef<RecordRange> Ranges) {
      return parseRefRanges(Chunks.lookup("refs"), Ranges, Strings->Strings);
    },
    "malformed or truncated refs");
if (!ParsedRefs)
  return ParsedRefs.takeError();
Sections.Refs = std::move(*ParsedRefs);
```

For `rela`, after scanning:

```cpp
auto ParsedRelations = parseRecordRangesThreaded<Relation>(
    *RelationRanges, Threads,
    [&](llvm::ArrayRef<RecordRange> Ranges) {
      return parseRelationRanges(Chunks.lookup("rela"), Ranges);
    },
    "malformed or truncated relations");
if (!ParsedRelations)
  return ParsedRelations.takeError();
Sections.Relations = std::move(*ParsedRelations);
```

Keep `cmdl` serial.

- [ ] **Step 8: Build final slabs from partial results**

After all section parsing, populate `IndexFileIn Result`.

For sources:

```cpp
if (Sections.Sources) {
  llvm::sort(*Sections.Sources, [](const SourceRecord &L,
                                   const SourceRecord &R) {
    return L.Ordinal < R.Ordinal;
  });
  Result.Sources.emplace();
  for (SourceRecord &Record : *Sections.Sources) {
    IncludeGraphNode IGN = std::move(Record.Value);
    auto Entry = Result.Sources->try_emplace(IGN.URI).first;
    Entry->getValue() = std::move(IGN);
    Entry->getValue().URI = Entry->getKey();
    for (auto &Include : Entry->getValue().DirectIncludes)
      Include = Result.Sources->try_emplace(Include).first->getKey();
  }
}
```

For symbols:

```cpp
if (Sections.Symbols) {
  llvm::sort(*Sections.Symbols, [](const SymbolRecord &L,
                                   const SymbolRecord &R) {
    return L.Ordinal < R.Ordinal;
  });
  SymbolSlab::Builder Symbols;
  for (const SymbolRecord &Record : *Sections.Symbols)
    Symbols.insert(Record.Value);
  Result.Symbols = std::move(Symbols).build();
}
```

For refs:

```cpp
if (Sections.Refs) {
  RefSlab::Builder Refs;
  for (const RefRecord &Record : *Sections.Refs)
    for (const Ref &R : Record.Refs)
      Refs.insert(Record.ID, R);
  Result.Refs = std::move(Refs).build();
}
```

For relations:

```cpp
if (Sections.Relations) {
  RelationSlab::Builder Relations;
  for (const Relation &R : *Sections.Relations)
    Relations.insert(R);
  Result.Relations = std::move(Relations).build();
}
```

Keep command-line handling unchanged, assigning to `Result.Cmd`.

- [ ] **Step 9: Run serialization tests and commit**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.*
```

Expected: all `SerializationTest` tests pass.

Commit:

```bash
git add clang-tools-extra/clangd/index/Serialization.cpp clang-tools-extra/clangd/unittests/SerializationTests.cpp
git commit -m "clangd: parse RIFF index sections in parallel"
```

## Task 4: Parallelize Dex Build

**Files:**
- Modify: `clang-tools-extra/clangd/index/dex/Dex.h`
- Modify: `clang-tools-extra/clangd/index/dex/Dex.cpp`
- Modify: `clang-tools-extra/clangd/unittests/SerializationTests.cpp`

- [ ] **Step 1: Add Dex threaded equivalence test**

In `SerializationTests.cpp`, include Dex:

```cpp
#include "index/dex/Dex.h"
```

Add helpers:

```cpp
std::vector<std::string> fuzzyFindNames(SymbolIndex &Index,
                                        llvm::StringRef Query) {
  FuzzyFindRequest Req;
  Req.Query = Query.str();
  Req.AnyScope = true;
  Req.Limit = 20;
  std::vector<std::string> Result;
  Index.fuzzyFind(Req, [&](const Symbol &S) {
    Result.push_back((S.Scope + S.Name).str());
  });
  return Result;
}

std::vector<std::string> lookupNames(SymbolIndex &Index,
                                     llvm::ArrayRef<SymbolID> IDs) {
  LookupRequest Req;
  for (SymbolID ID : IDs)
    Req.IDs.insert(ID);
  std::vector<std::string> Result;
  Index.lookup(Req, [&](const Symbol &S) {
    Result.push_back((S.Scope + S.Name).str());
  });
  llvm::sort(Result);
  return Result;
}
```

Add the test:

```cpp
TEST(SerializationTest, DexBuildThreadedMatchesSerial) {
  auto In = readIndexFile(YAML);
  ASSERT_TRUE(bool(In)) << In.takeError();
  ASSERT_TRUE(In->Symbols);
  ASSERT_TRUE(In->Refs);
  ASSERT_TRUE(In->Relations);

  SymbolSlab Symbols1 = std::move(*In->Symbols);
  RefSlab Refs1 = std::move(*In->Refs);
  RelationSlab Relations1 = std::move(*In->Relations);

  auto InAgain = readIndexFile(YAML);
  ASSERT_TRUE(bool(InAgain)) << InAgain.takeError();
  auto Serial = dex::Dex::build(std::move(Symbols1), std::move(Refs1),
                                std::move(Relations1), 1);
  auto Threaded = dex::Dex::build(std::move(*InAgain->Symbols),
                                  std::move(*InAgain->Refs),
                                  std::move(*InAgain->Relations), 4);

  EXPECT_EQ(fuzzyFindNames(*Threaded, "Foo"),
            fuzzyFindNames(*Serial, "Foo"));
  std::vector<SymbolID> IDs = {
      cantFail(SymbolID::fromStr("057557CEBF6E6B2D")),
      cantFail(SymbolID::fromStr("057557CEBF6E6B2E"))};
  EXPECT_EQ(lookupNames(*Threaded, IDs), lookupNames(*Serial, IDs));
}
```

- [ ] **Step 2: Run the Dex equivalence test before implementation**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.DexBuildThreadedMatchesSerial
```

Expected: the test passes through the serial `buildIndex()` implementation.

- [ ] **Step 3: Add merge support to `IndexBuilder`**

In `Dex.cpp`, add this private method to `IndexBuilder`:

```cpp
  void merge(IndexBuilder &&Other) {
    auto AppendStringMap =
        [](llvm::StringMap<std::vector<DocID>> &Target,
           llvm::StringMap<std::vector<DocID>> &Source) {
          for (auto &Entry : Source) {
            auto &Docs = Target[Entry.first()];
            Docs.insert(Docs.end(), Entry.second.begin(), Entry.second.end());
          }
        };
    AppendStringMap(TypeDocs, Other.TypeDocs);
    AppendStringMap(ScopeDocs, Other.ScopeDocs);
    AppendStringMap(ProximityDocs, Other.ProximityDocs);

    for (auto &Entry : Other.TrigramDocs) {
      auto &Docs = TrigramDocs[Entry.first];
      Docs.insert(Docs.end(), Entry.second.begin(), Entry.second.end());
    }
    RestrictedCCDocs.insert(RestrictedCCDocs.end(),
                            Other.RestrictedCCDocs.begin(),
                            Other.RestrictedCCDocs.end());
  }
```

- [ ] **Step 4: Add local chunk splitting in `Dex.cpp`**

Add this helper in the anonymous namespace:

```cpp
std::vector<std::pair<size_t, size_t>> splitIndexRange(size_t Size,
                                                       unsigned Threads) {
  std::vector<std::pair<size_t, size_t>> Result;
  if (Size == 0)
    return Result;
  unsigned TaskCount = std::min<unsigned>(Threads, Size);
  Result.reserve(TaskCount);
  size_t PerTask = Size / TaskCount;
  size_t Extra = Size % TaskCount;
  size_t Begin = 0;
  for (unsigned I = 0; I < TaskCount; ++I) {
    size_t Count = PerTask + (I < Extra ? 1 : 0);
    Result.emplace_back(Begin, Begin + Count);
    Begin += Count;
  }
  return Result;
}
```

Include:

```cpp
#include "llvm/Support/ThreadPool.h"
#include <future>
```

- [ ] **Step 5: Parallelize scoring and posting-list extraction**

Replace `Dex::buildIndex(unsigned Threads)` with:

```cpp
void Dex::buildIndex(unsigned Threads) {
  this->Corpus = dex::Corpus(Symbols.size());
  std::vector<std::pair<float, const Symbol *>> ScoredSymbols(Symbols.size());

  if (Threads <= 1 || Symbols.size() < 2) {
    for (size_t I = 0; I < Symbols.size(); ++I) {
      const Symbol *Sym = Symbols[I];
      ScoredSymbols[I] = {quality(*Sym), Sym};
    }
  } else {
    llvm::ThreadPool Pool(llvm::hardware_concurrency(Threads));
    std::vector<std::shared_future<void>> Futures;
    for (auto Range : splitIndexRange(Symbols.size(), Threads)) {
      Futures.push_back(Pool.async([&, Range] {
        for (size_t I = Range.first; I < Range.second; ++I) {
          const Symbol *Sym = Symbols[I];
          ScoredSymbols[I] = {quality(*Sym), Sym};
        }
      }));
    }
    Pool.wait();
    for (auto &Future : Futures)
      Future.get();
  }

  llvm::sort(ScoredSymbols, std::greater<std::pair<float, const Symbol *>>());

  SymbolQuality.resize(Symbols.size());
  LookupTable.clear();
  for (size_t I = 0; I < ScoredSymbols.size(); ++I) {
    SymbolQuality[I] = ScoredSymbols[I].first;
    Symbols[I] = ScoredSymbols[I].second;
    LookupTable[Symbols[I]->ID] = Symbols[I];
  }

  if (Threads <= 1 || Symbols.size() < 2) {
    IndexBuilder Builder;
    for (DocID SymbolRank = 0; SymbolRank < Symbols.size(); ++SymbolRank)
      Builder.add(*Symbols[SymbolRank], SymbolRank);
    InvertedIndex = std::move(Builder).build();
    return;
  }

  std::vector<IndexBuilder> Builders;
  auto Ranges = splitIndexRange(Symbols.size(), Threads);
  Builders.resize(Ranges.size());
  llvm::ThreadPool Pool(llvm::hardware_concurrency(Threads));
  std::vector<std::shared_future<void>> Futures;
  for (size_t Task = 0; Task < Ranges.size(); ++Task) {
    Futures.push_back(Pool.async([&, Task] {
      for (DocID SymbolRank = Ranges[Task].first;
           SymbolRank < Ranges[Task].second; ++SymbolRank)
        Builders[Task].add(*Symbols[SymbolRank], SymbolRank);
    }));
  }
  Pool.wait();
  for (auto &Future : Futures)
    Future.get();

  IndexBuilder Builder;
  for (IndexBuilder &Partial : Builders)
    Builder.merge(std::move(Partial));
  InvertedIndex = std::move(Builder).build();
}
```

- [ ] **Step 6: Run Dex and serialization tests and commit**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.*:DexTests.*
```

Expected: selected tests pass.

Commit:

```bash
git add clang-tools-extra/clangd/index/dex/Dex.h clang-tools-extra/clangd/index/dex/Dex.cpp clang-tools-extra/clangd/unittests/SerializationTests.cpp
git commit -m "clangd: build Dex posting lists in parallel"
```

## Task 5: Wire Thread Counts Through clangd and clangd-index-server

**Files:**
- Modify: `clang-tools-extra/clangd/tool/ClangdMain.cpp`
- Modify: `clang-tools-extra/clangd/index/remote/server/Server.cpp`
- Modify: `clang-tools-extra/clangd/test/remote-index/pipeline.test`
- Modify: `clang-tools-extra/clangd/index/remote/README.md`

- [ ] **Step 1: Add lit coverage for server `--j`**

In `clang-tools-extra/clangd/test/remote-index/pipeline.test`, change the pipeline helper invocation to:

```text
# RUN: %python %S/pipeline_helper.py --input-file-name=%s --project-root=%S --index-file=%t.idx --server-arg=--j=2 | FileCheck %s
```

- [ ] **Step 2: Run the lit test and verify it fails before server support**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 clangd-index-server clangd-index-server-monitor clangd
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/bin/llvm-lit -sv /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/remote-index/pipeline.test
```

Expected: the lit test fails because `clangd-index-server` does not recognize `--j`.

- [ ] **Step 3: Pass `-j` into `clangd --index-file` loading**

In `ClangdMain.cpp`, change the file index load lambda capture and call:

```cpp
auto IndexLoadTask = [File = External.Location, PlaceHolder = NewIndex.get(),
                      Threads = WorkerThreadsCount.getValue()] {
  IndexLoadOptions Opts;
  Opts.Origin = SymbolOrigin::Static;
  Opts.UseDex = true;
  Opts.Threads = Threads;
  if (auto Idx = loadIndex(File, Opts))
    PlaceHolder->reset(std::move(Idx));
};
```

- [ ] **Step 4: Add `--j` to `clangd-index-server`**

In `Server.cpp`, add the option near `LimitResults`:

```cpp
llvm::cl::opt<unsigned> WorkerThreadsCount{
    "j",
    llvm::cl::desc("Number of workers used to load the index. "
                   "Also used when hot-reloading the index."),
    llvm::cl::init(llvm::hardware_concurrency().compute_thread_count())};
```

After command-line parsing in `main()`, add:

```cpp
if (WorkerThreadsCount == 0) {
  llvm::errs() << "A number of index load worker threads cannot be 0.\n";
  return 1;
}
```

- [ ] **Step 5: Pass server `--j` into cold load and hot reload**

Change `hotReload()` signature:

```cpp
void hotReload(clangd::SwapIndex &Index, llvm::StringRef IndexPath,
               llvm::vfs::Status &LastStatus,
               llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> &FS,
               Monitor &Monitor, unsigned Threads)
```

Inside `hotReload()`, replace the load call with:

```cpp
IndexLoadOptions Opts;
Opts.Origin = SymbolOrigin::Static;
Opts.UseDex = true;
Opts.Threads = Threads;
std::unique_ptr<clang::clangd::SymbolIndex> NewIndex =
    loadIndex(IndexPath, Opts);
```

In `main()`, replace the cold load with the same options object:

```cpp
clang::clangd::IndexLoadOptions LoadOpts;
LoadOpts.Origin = clang::clangd::SymbolOrigin::Static;
LoadOpts.UseDex = true;
LoadOpts.Threads = WorkerThreadsCount;
auto SymIndex = clang::clangd::loadIndex(IndexPath, LoadOpts);
```

Update the hot reload thread call:

```cpp
hotReload(Index, llvm::StringRef(IndexPath), LastStatus, FS, Monitor,
          WorkerThreadsCount);
```

- [ ] **Step 6: Document the server flag**

In `clang-tools-extra/clangd/index/remote/README.md`, extend the Running section:

```markdown
`clangd-index-server` accepts `--j=N` to control the number of workers used to
load the monolithic index during startup and hot reload. Use `--j=1` to force
the serial load path when comparing performance or investigating a regression.
```

- [ ] **Step 7: Run server lit test and commit**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 clangd-index-server clangd-index-server-monitor clangd
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/bin/llvm-lit -sv /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/remote-index/pipeline.test
```

Expected: the lit test passes.

Commit:

```bash
git add clang-tools-extra/clangd/tool/ClangdMain.cpp clang-tools-extra/clangd/index/remote/server/Server.cpp clang-tools-extra/clangd/test/remote-index/pipeline.test clang-tools-extra/clangd/index/remote/README.md
git commit -m "clangd: thread index load workers through clangd tools"
```

## Task 6: Extend IndexBenchmark for Threaded Loading

**Files:**
- Modify: `clang-tools-extra/clangd/benchmarks/IndexBenchmark.cpp`
- Modify: `clang-tools-extra/clangd/test/index-tools.test`

- [ ] **Step 1: Add lit coverage for the benchmark flag**

In `clang-tools-extra/clangd/test/index-tools.test`, change the benchmark run line to:

```text
# RUN: if [ -f %clangd-benchmark-dir/IndexBenchmark ]; then %clangd-benchmark-dir/IndexBenchmark %t.index %p/Inputs/requests.json --index-load-threads=2 --benchmark_min_time=0.01 ; fi
```

Keep the invalid JSON run unchanged.

- [ ] **Step 2: Run the lit test and verify it fails before benchmark support**

Run:

```bash
/home/mkl/llvm-ohos/master/prebuilts/cmake/linux-x86/bin/cmake -S /home/mkl/llvm-ohos/master/toolchain/llvm-project/llvm -B /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 -DLLVM_INCLUDE_BENCHMARKS=ON
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 IndexBenchmark clangd-indexer
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/bin/llvm-lit -sv /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/index-tools.test
```

Expected before support: the command does not provide a benchmark-controlled
threaded load. Record the observed output; Step 3 adds explicit parsing.

- [ ] **Step 3: Add benchmark thread parsing**

In `IndexBenchmark.cpp`, add globals:

```cpp
unsigned IndexLoadThreads = 1;
```

Add this parser before `main()`:

```cpp
void parseIndexBenchmarkArgs(int &argc, char **argv) {
  std::vector<char *> Kept;
  Kept.push_back(argv[0]);
  for (int I = 1; I < argc; ++I) {
    llvm::StringRef Arg(argv[I]);
    if (Arg.consume_front("--index-load-threads=")) {
      unsigned Parsed = 0;
      if (Arg.getAsInteger(10, Parsed) || Parsed == 0) {
        llvm::errs() << "--index-load-threads must be a positive integer\n";
        exit(1);
      }
      IndexLoadThreads = Parsed;
      continue;
    }
    Kept.push_back(argv[I]);
  }
  for (size_t I = 0; I < Kept.size(); ++I)
    argv[I] = Kept[I];
  argc = static_cast<int>(Kept.size());
}
```

Call it before `::benchmark::Initialize(&argc, argv);`:

```cpp
parseIndexBenchmarkArgs(argc, argv);
```

- [ ] **Step 4: Use threaded load options in benchmark builders**

Replace `buildMem()` and `buildDex()` with:

```cpp
std::unique_ptr<SymbolIndex> buildMem() {
  IndexLoadOptions Opts;
  Opts.Origin = clang::clangd::SymbolOrigin::Static;
  Opts.UseDex = false;
  Opts.Threads = IndexLoadThreads;
  return loadIndex(IndexFilename, Opts);
}

std::unique_ptr<SymbolIndex> buildDex() {
  IndexLoadOptions Opts;
  Opts.Origin = clang::clangd::SymbolOrigin::Static;
  Opts.UseDex = true;
  Opts.Threads = IndexLoadThreads;
  return loadIndex(IndexFilename, Opts);
}
```

- [ ] **Step 5: Add explicit serial and threaded build benchmark names**

Keep `dexBuild` and add:

```cpp
static void dexBuildSerial(benchmark::State &State) {
  unsigned SavedThreads = IndexLoadThreads;
  IndexLoadThreads = 1;
  for (auto _ : State)
    buildDex();
  IndexLoadThreads = SavedThreads;
}
BENCHMARK(dexBuildSerial);
```

The normal `dexBuild` uses `--index-load-threads=N`.

- [ ] **Step 6: Run benchmark lit test and commit**

Run:

```bash
/home/mkl/llvm-ohos/master/prebuilts/cmake/linux-x86/bin/cmake -S /home/mkl/llvm-ohos/master/toolchain/llvm-project/llvm -B /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 -DLLVM_INCLUDE_BENCHMARKS=ON
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 IndexBenchmark clangd-indexer
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/bin/llvm-lit -sv /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/index-tools.test
```

Expected: the lit test passes.

Commit:

```bash
git add clang-tools-extra/clangd/benchmarks/IndexBenchmark.cpp clang-tools-extra/clangd/test/index-tools.test
git commit -m "clangd: benchmark threaded index loading"
```

## Task 7: Final Verification and Real Index Measurement

**Files:**
- No source edits expected.

- [ ] **Step 1: Run targeted unit tests**

Run:

```bash
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 ClangdTests
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/tools/clang/tools/extra/clangd/unittests/ClangdTests --gtest_filter=SerializationTest.*:DexTests.*
```

Expected: tests pass.

- [ ] **Step 2: Run targeted lit tests**

Run:

```bash
/home/mkl/llvm-ohos/master/prebuilts/cmake/linux-x86/bin/cmake -S /home/mkl/llvm-ohos/master/toolchain/llvm-project/llvm -B /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 -DLLVM_INCLUDE_BENCHMARKS=ON
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 IndexBenchmark clangd-indexer clangd-index-server clangd-index-server-monitor clangd
/home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3/bin/llvm-lit -sv /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/index-tools.test /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/remote-index/pipeline.test
```

Expected: both lit tests pass.

- [ ] **Step 3: Build production binaries used by the workflow**

Run:

```bash
/home/mkl/llvm-ohos/master/prebuilts/cmake/linux-x86/bin/cmake -S /home/mkl/llvm-ohos/master/toolchain/llvm-project/llvm -B /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 -DLLVM_INCLUDE_BENCHMARKS=ON
ninja -C /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 clangd clangd-index-server IndexBenchmark
```

Expected: all targets build.

- [ ] **Step 4: Measure the real 2.56 GB index**

Run with the real index path substituted:

```bash
BenchmarkBin=$(/usr/bin/find /home/mkl/llvm-ohos/master/out/clangd-remote-stdcxx3 -name IndexBenchmark -type f | /usr/bin/head -n1)
"${BenchmarkBin}" /absolute/path/to/large.idx /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/Inputs/requests.json --index-load-threads=1 --benchmark_filter=dexBuild --benchmark_min_time=1
"${BenchmarkBin}" /absolute/path/to/large.idx /home/mkl/llvm-ohos/master/toolchain/llvm-project/clang-tools-extra/clangd/test/Inputs/requests.json --index-load-threads=8 --benchmark_filter=dexBuild --benchmark_min_time=1
```

Expected: both benchmark invocations complete. Record wall time and benchmark time for `dexBuild`; the second run should be faster unless the string table or file I/O dominates.

- [ ] **Step 5: Capture final git state**

Run:

```bash
git status --short
git log --oneline -8
```

Expected: no uncommitted changes except user-created files. The log includes the task commits from this plan.
