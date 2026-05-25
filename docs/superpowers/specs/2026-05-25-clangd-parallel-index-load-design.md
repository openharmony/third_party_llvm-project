# clangd Parallel Index Load Design

Date: 2026-05-25

## Context

The target workload is an extremely large clangd static index generated from a
compilation database with about 130,000 translation units. The compilation
database is about 6.3 GB and the generated `.idx` file is about 2.56 GB.

The immediate pain point is the load experience for:

- `clangd --index-file=...`
- `clangd-index-server <INDEX FILE> <PROJECT ROOT>`

`clangd-index-server` does not read the compilation database during startup. It
loads the monolithic `.idx` file through `loadIndex()`. `clangd --index-file`
also reaches the same `loadIndex()` implementation, though the current clangd
path schedules the load task asynchronously.

Today the expensive work inside `loadIndex()` is effectively single-threaded:

- file mapping/read
- RIFF chunk parsing
- string table read/decompression
- `srcs`, `symb`, `refs`, `rela`, and `cmdl` section decoding
- slab construction
- Dex construction

The existing `clangd -j` controls clangd async workers and background indexing,
but it does not make a single monolithic `.idx` load substantially parallel.

## Goals

- Preserve compatibility with existing `.idx` files.
- Keep the current RIFF index format version unchanged.
- Reuse `clangd -j` as the user-facing concurrency control for
  `clangd --index-file`.
- Add equivalent concurrency control to `clangd-index-server`.
- Improve cold-load and hot-reload time for very large monolithic indexes.
- Optimize for load speed, accepting higher peak memory usage.
- Add benchmark coverage so the effect is measurable without relying on a
  checked-in multi-GB index.

## Non-Goals

- Do not change the `.idx` on-disk format.
- Do not redesign static index generation.
- Do not solve large compilation database loading in this change.
- Do not introduce a separate user-facing index-load thread option.
- Do not require the 2.56 GB real-world index for automated tests.

## Recommended Approach

Implement parallel RIFF section decoding plus parallel Dex construction.

This is preferred over only parallelizing Dex construction because the real
2.56 GB workload may spend most of its time in `ParseIndex`, particularly in
`symb`, `refs`, and `srcs` decoding and slab construction. It is preferred over
outer async scheduling because that does not reduce the time needed to load one
large index, especially for `clangd-index-server` cold startup.

## User-Facing Behavior

`clangd --index-file=... -j=N` uses `N` for static index loading in addition to
the existing async worker and background index use.

`clangd-index-server` gains:

```text
--j=N
```

This controls index loading during both initial startup and hot reload.

`N=1` forces serial loading and is the baseline for benchmarks and debugging.
`N=0` is rejected with a diagnostic, matching clangd's existing stance that zero
worker threads is invalid outside explicit sync mode.

## Internal API

Introduce an options object for index loading:

```cpp
struct IndexLoadOptions {
  SymbolOrigin Origin = SymbolOrigin::Static;
  bool UseDex = true;
  unsigned Threads = 1;
};
```

Add a new main overload:

```cpp
std::unique_ptr<SymbolIndex> loadIndex(llvm::StringRef Filename,
                                       IndexLoadOptions Opts);
```

Keep existing overloads as wrappers with `Threads = 1` to reduce churn at
unrelated call sites.

## Load Pipeline

The new load pipeline is:

1. Read/map the file.
2. Parse the RIFF chunk table and validate `meta`.
3. Read and decompress `stri`.
4. Decode independent sections in parallel.
5. Build the requested index representation, normally Dex.

Steps 1 through 3 remain serial because the current file format has one string
table and does not provide precomputed record offsets. Steps 4 and 5 use
`IndexLoadOptions::Threads` when it is greater than 1.

## Parallel Section Decoding

The design uses two levels of parallelism.

First, independent sections are decoded in parallel:

- `srcs`
- `symb`
- `refs`
- `rela`
- `cmdl`

Second, large variable-length sections are split internally. The current format
does not include record counts or record offsets, so each large section first
gets a lightweight boundary scan:

- walk records with `Reader`
- skip fields using the existing binary encoding rules
- record each record's `[begin, end)` byte range
- do not construct symbols, refs, include graph nodes, or slabs in this scan

The recorded offsets are split across worker tasks by approximate byte size or
record count. Each worker parses its range into thread-local partial data.

Small sections may stay serial to avoid scheduling overhead.

## Merging

Existing builders are not treated as thread-safe. Workers produce local partial
results; a deterministic merge step creates the final slabs.

`symb`:

- workers parse symbols into local builders or local vectors
- final merge preserves current file-order semantics when duplicate `SymbolID`
  records exist: later records in the section overwrite earlier records
- final `SymbolSlab` remains sorted by `SymbolID`

`refs`:

- workers parse refs into local refs
- final merge groups by symbol, sorts refs in each group, and removes duplicates
- final `RefSlab` preserves current ordering semantics

`rela`:

- workers parse relation vectors
- final merge sorts and deduplicates as `RelationSlab::Builder::build()` does

`srcs`:

- workers parse include graph nodes
- final merge stores nodes in a `StringMap<IncludeGraphNode>`
- duplicate URI records preserve current file-order semantics: later records in
  the section overwrite earlier records
- URI keys and direct include references are canonicalized through the map keys,
  preserving current lifetime assumptions

`cmdl`:

- remains trivial and may be serial even when other sections are parallel

## Parallel Dex Construction

Dex construction should use the same thread count.

Safe parallelization targets include:

- computing per-symbol quality scores
- preparing lookup data that can be merged deterministically
- token extraction for posting-list construction
- building local token-to-doc lists and merging them after final doc ranks are
  known

The final doc rank order must remain deterministic. The sort that determines
symbol rank remains globally ordered, and merged posting lists are ordered by
that final rank, not by worker completion order.

## Error Handling

Worker tasks return `llvm::Expected<PartialResult>` or an equivalent error
carrier. They do not log and continue after parse failure.

Errors are reported in a deterministic section order:

1. `srcs`
2. `symb`
3. `refs`
4. `rela`
5. `cmdl`

Within a section, if multiple worker ranges fail, the error associated with the
lowest file offset is reported.

If any section fails, `loadIndex()` fails. For `clangd-index-server` hot reload,
the existing behavior is preserved: the old index continues serving if the new
index cannot be loaded.

Malformed files should keep current diagnostic style, such as:

- malformed or truncated include URI
- malformed or truncated symbol
- malformed or truncated refs
- malformed or truncated relations
- malformed or truncated commandline section

## Determinism

Parallel loading must not change query results.

Required invariants:

- `SymbolSlab` is sorted by `SymbolID`.
- `RefSlab` is grouped by symbol and refs are sorted within each group.
- relations are sorted and deduplicated.
- duplicate `symb` and `srcs` records are resolved by section file order, not
  worker completion order.
- Dex lookup, fuzzy find, refs, and relations results match serial loading for
  the same index.
- posting-list order is based on final symbol rank, not thread scheduling.

## Instrumentation

Add trace spans or benchmark-visible measurements for:

- `ReadIndexFile`
- `ParseIndex`
- `ParseStringTable`
- `ParseSections`
- per-section parse time where practical
- `BuildIndex`
- `BuildDex`

Logs should include the effective thread count for index loading.

## Tests

Unit tests:

- load an existing small test `.idx` with `Threads = 1`, `2`, and `4`
- verify lookup, fuzzy find, refs, and relations results are equivalent
- verify malformed/truncated `symb` and `refs` input reports stable errors

Integration tests:

- update remote-index pipeline coverage so `clangd-index-server --j=2` is
  exercised

Benchmarks:

- extend `IndexBenchmark` to accept an index-load thread count
- include serial and parallel Dex build benchmarks
- make it possible to compare `Threads = 1` and `Threads = N` on the same
  `.idx`

Manual validation:

- run `IndexBenchmark` on the real 2.56 GB index
- compare serial and parallel load times
- inspect stage timings to confirm whether time is dominated by parse, Dex
  build, or still-serial file/string-table work

## Risks

- Peak memory usage will increase due to local worker results and merge-time
  temporary structures.
- Boundary scanning adds serial work before worker decoding starts.
- If the string table dominates load time, format-compatible parallelism will
  have limited impact.
- Parallel parsing increases implementation complexity around ownership and
  lifetime of strings.

These risks are acceptable for this workload because the priority is reducing
load time for a very large index, and the design preserves `Threads = 1` as a
low-risk fallback.
