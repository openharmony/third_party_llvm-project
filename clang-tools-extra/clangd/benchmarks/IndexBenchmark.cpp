//===--- IndexBenchmark.cpp - Clangd index benchmarks -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../index/Serialization.h"
#include "../index/dex/Dex.h"
#include "../support/Trace.h"
#include "benchmark/benchmark.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <vector>

const char *IndexFilename;
const char *RequestsFilename;
unsigned IndexLoadThreads = 1;
std::string TraceFilename;

namespace clang {
namespace clangd {
namespace {

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

// Reads JSON array of serialized FuzzyFindRequest's from user-provided file.
std::vector<FuzzyFindRequest> extractQueriesFromLogs() {

  auto Buffer = llvm::MemoryBuffer::getFile(RequestsFilename);
  if (!Buffer) {
    llvm::errs() << "Error cannot open JSON request file:" << RequestsFilename
                 << ": " << Buffer.getError().message() << "\n";
    exit(1);
  }

  StringRef Log = Buffer.get()->getBuffer();

  std::vector<FuzzyFindRequest> Requests;
  auto JSONArray = llvm::json::parse(Log);

  // Panic if the provided file couldn't be parsed.
  if (!JSONArray) {
    llvm::errs() << "Error when parsing JSON requests file: "
                 << llvm::toString(JSONArray.takeError());
    exit(1);
  }
  if (!JSONArray->getAsArray()) {
    llvm::errs() << "Error: top-level value is not a JSON array: " << Log
                 << '\n';
    exit(1);
  }

  for (const auto &Item : *JSONArray->getAsArray()) {
    FuzzyFindRequest Request;
    // Panic if the provided file couldn't be parsed.
    llvm::json::Path::Root Root("FuzzyFindRequest");
    if (!fromJSON(Item, Request, Root)) {
      llvm::errs() << llvm::toString(Root.getError()) << "\n";
      Root.printErrorContext(Item, llvm::errs());
      exit(1);
    }
    Requests.push_back(Request);
  }
  return Requests;
}

static void memQueries(benchmark::State &State) {
  const auto Mem = buildMem();
  const auto Requests = extractQueriesFromLogs();
  for (auto _ : State)
    for (const auto &Request : Requests)
      Mem->fuzzyFind(Request, [](const Symbol &S) {});
}
BENCHMARK(memQueries);

static void dexQueries(benchmark::State &State) {
  const auto Dex = buildDex();
  const auto Requests = extractQueriesFromLogs();
  for (auto _ : State)
    for (const auto &Request : Requests)
      Dex->fuzzyFind(Request, [](const Symbol &S) {});
}
BENCHMARK(dexQueries);

static void memBuild(benchmark::State &State) {
  for (auto _ : State)
    buildMem();
}
BENCHMARK(memBuild);

static void memBuildSerial(benchmark::State &State) {
  unsigned SavedThreads = IndexLoadThreads;
  IndexLoadThreads = 1;
  for (auto _ : State)
    buildMem();
  IndexLoadThreads = SavedThreads;
}
BENCHMARK(memBuildSerial);

static void dexBuild(benchmark::State &State) {
  for (auto _ : State)
    buildDex();
}
BENCHMARK(dexBuild);

static void dexBuildSerial(benchmark::State &State) {
  unsigned SavedThreads = IndexLoadThreads;
  IndexLoadThreads = 1;
  for (auto _ : State)
    buildDex();
  IndexLoadThreads = SavedThreads;
}
BENCHMARK(dexBuildSerial);

} // namespace
} // namespace clangd
} // namespace clang

void parseIndexBenchmarkArgs(int &argc, char **argv) {
  std::vector<char *> Kept;
  Kept.push_back(argv[0]);
  for (int I = 1; I < argc; ++I) {
    llvm::StringRef Arg(argv[I]);
    if (Arg == "--index-load-threads") {
      llvm::errs() << "--index-load-threads must be passed as "
                      "--index-load-threads=N\n";
      exit(1);
    }
    if (Arg.consume_front("--index-load-threads=")) {
      unsigned Parsed = 0;
      if (Arg.getAsInteger(10, Parsed) || Parsed == 0) {
        llvm::errs() << "--index-load-threads must be a positive integer\n";
        exit(1);
      }
      IndexLoadThreads = Parsed;
      continue;
    }
    if (Arg.consume_front("--trace-file=")) {
      TraceFilename = Arg.str();
      continue;
    }
    Kept.push_back(argv[I]);
  }
  for (size_t I = 0; I < Kept.size(); ++I)
    argv[I] = Kept[I];
  argc = static_cast<int>(Kept.size());
}

// FIXME(kbobyrev): Add index building time benchmarks.
// FIXME(kbobyrev): Add memory consumption "benchmarks" by manually measuring
// in-memory index size and reporting it as time.
// FIXME(kbobyrev): Create a logger wrapper to suppress debugging info printer.
int main(int argc, char *argv[]) {
  if (argc < 3) {
    llvm::errs() << "Usage: " << argv[0]
                 << " global-symbol-index.dex requests.json "
                    "BENCHMARK_OPTIONS...\n";
    return -1;
  }
  IndexFilename = argv[1];
  RequestsFilename = argv[2];
  // Trim first two arguments of the benchmark invocation and pretend no
  // arguments were passed in the first place.
  argv[2] = argv[0];
  argv += 2;
  argc -= 2;
  parseIndexBenchmarkArgs(argc, argv);
  std::unique_ptr<llvm::raw_fd_ostream> TraceOS;
  std::unique_ptr<clang::clangd::trace::EventTracer> Tracer;
  llvm::Optional<clang::clangd::trace::Session> TracingSession;
  if (!TraceFilename.empty()) {
    std::error_code EC;
    TraceOS = std::make_unique<llvm::raw_fd_ostream>(TraceFilename, EC,
                                                     llvm::sys::fs::OF_Text);
    if (EC) {
      llvm::errs() << "Failed to open trace file: " << EC.message() << "\n";
      return 1;
    }
    Tracer = clang::clangd::trace::createJSONTracer(*TraceOS);
    TracingSession.emplace(*Tracer);
  }
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
