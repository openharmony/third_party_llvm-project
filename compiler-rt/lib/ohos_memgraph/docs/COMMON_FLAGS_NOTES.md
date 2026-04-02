# Common Flags Notes For Memgraph

## Why this note exists

`ohos_memgraph` uses pieces of `sanitizer_common`, including mapping decoration
in `sanitizer_posix.cpp`. That decoration is controlled by
`common_flags()->decorate_proc_maps`.

During investigation we found that `ohos_memgraph` did not initialize common
sanitizer flags at all. As a result, `CommonFlags` could stay zero-initialized,
which makes `decorate_proc_maps == false` and causes `DecorateMapping()` to
return early. When that happens, internal runtime mappings may exist, but they
do not show user-readable names in `/proc/<pid>/maps`.

## What "fatal-on-OOM" means

For the sanitizer internal allocator, "fatal-on-OOM" means:

- an internal allocation failure is treated as a fatal runtime error
- the allocator reports the failure
- the process terminates instead of returning `nullptr` and trying to continue

In practice, `InternalAlloc(...)` calls the internal allocator and then aborts
through `ReportInternalAllocatorOutOfMemory(...)` if allocation fails.

This is different from a "return null on OOM" policy, where the runtime would
return `nullptr` to the caller and let upper layers handle it.

## How HWASan initializes common flags

HWASan performs explicit common-flag initialization in its own runtime startup
path:

1. `SetCommonFlagsDefaults()`
2. copy the default `CommonFlags`
3. override the subset of defaults that HWASan wants to change
4. `OverrideCommonFlags(cf)`
5. parse tool-specific and environment-provided flags
6. `InitializeCommonFlags()`

Relevant code:

- `compiler-rt/lib/hwasan/hwasan.cpp`
- `compiler-rt/lib/hwasan/hwasan_preinit.cpp`

This shows that common flags are not initialized "automatically" for every
runtime. Each runtime is expected to do this explicitly, and the initialization
must happen before the flags are first used.

## CommonFlags overrides used by HWASan

The following entries differ from the raw common defaults.

### Always overridden by HWASan

- `malloc_context_size: 1 -> 20`
  - keep deeper allocation stacks
- `handle_ioctl: false -> true`
  - enable ioctl-related handling
- `check_printf: true -> false`
  - disable printf argument checks
- `intercept_tls_get_addr: false -> true`
  - enable TLS interception
- `exitcode: 1 -> 99`
  - use a HWASan-specific error exit code
- `clear_shadow_mmap_threshold: 64KB -> 8KB(Android) / 32KB(non-Android)`
  - tune shadow clearing behavior
- `handle_sigtrap: No -> Exclusive`
  - reserve `SIGTRAP` for HWASan reporting

### Platform-specific HWASan overrides

Android:

- `handle_segv: Yes -> No`
- `handle_sigbus: Yes -> No`
- `handle_sigfpe: Yes -> No`

OHOS:

- `handle_segv: Yes -> No`
- `handle_sigbus: Yes -> No`
- `allocator_may_return_null: false -> true`
- `log_exe_name: false -> true`
- `print_module_map: 0 -> 1`
- `intercept_send: true -> false`

## Memgraph recommendation

For `ohos_memgraph`, we currently do not need the broad policy changes used by
HWASan. The runtime is a tracking/query runtime, not a crash-reporting
sanitizer.

The minimal safe choice is:

1. initialize common flags
2. keep common defaults
3. explicitly pin `decorate_proc_maps = true`

That gives us:

- sane platform defaults for common sanitizer behavior
- readable names for runtime-owned anonymous mappings on OHOS
- no unnecessary policy drift in signal handling, allocator behavior, or
  unrelated common sanitizer features

## Memgraph implementation choice

The current memgraph runtime uses a small early-init shim plus interceptor-entry
guarantees:

1. `SetCommonFlagsDefaults()`
2. copy `CommonFlags`
3. set `decorate_proc_maps = true`
4. `OverrideCommonFlags(cf)`
5. `InitializeCommonFlags()`

This work is done from a dedicated early-init translation unit and is invoked in
three places:

- a shared library constructor as a best-effort early path
- the top of every allocation interceptor as the earliest reliable path for the
  current `LD_PRELOAD` deployment model
- the top of `Initialize()` as a functional fallback

The constructor alone was not early enough on the current OHOS preload path,
but the interceptor-entry call is early enough to make allocator-owned mappings
show readable names in `/proc/<pid>/maps` while keeping memgraph functional
initialization unchanged.
