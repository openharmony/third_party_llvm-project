# OHOS Memgraph Tests

This directory groups the latest `ohos_memgraph` tests by validation focus.

- `core/`: core runtime behavior, hook semantics, end-to-end functional checks,
  and capacity drop-new checks.
- `frontend/`: tests that depend on frontend-inserted metadata, including
  hybrid/frontend-facing smokes.
- `frontend/shared/`: shared-library frontend smokes and the shared runner.
- `observability/`: tests around observability-facing enumeration and related
  diagnostics behavior.
- `stress/`: concurrency, slab-growth, and high-pressure correctness tests.
- `bench/`: performance and memory-curve benchmarks.
- `selftest/`: minimal selftest sources kept with the runtime tree.

This layout keeps the runtime tests under one `tests/` tree while making it
easier to find:

- basic functional coverage
- frontend-instrumentation-dependent coverage
- observability-related coverage
- stress tests
- benchmarks
