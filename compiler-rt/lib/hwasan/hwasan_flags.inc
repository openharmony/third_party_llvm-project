//===-- hwasan_flags.inc ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Hwasan runtime flags.
//
//===----------------------------------------------------------------------===//
#ifndef HWASAN_FLAG
#  error "Define HWASAN_FLAG prior to including this file!"
#endif

// HWASAN_FLAG(Type, Name, DefaultValue, Description)
// See COMMON_FLAG in sanitizer_flags.inc for more details.

HWASAN_FLAG(bool, verbose_threads, false,
            "inform on thread creation/destruction")
HWASAN_FLAG(bool, tag_in_malloc, true, "")
HWASAN_FLAG(bool, tag_in_free, true, "")
HWASAN_FLAG(bool, print_stats, false, "")
HWASAN_FLAG(bool, halt_on_error, true, "")
HWASAN_FLAG(bool, atexit, false, "")

HWASAN_FLAG(bool, memory_debug, false, "")  // OHOS_LOCAL

// Test only flag to disable malloc/realloc/free memory tagging on startup.
// Tagging can be reenabled with __hwasan_enable_allocator_tagging().
HWASAN_FLAG(bool, disable_allocator_tagging, false, "")

// If false, use simple increment of a thread local counter to generate new
// tags.
HWASAN_FLAG(bool, random_tags, true, "")

HWASAN_FLAG(
    int, max_malloc_fill_size, 0,
    "HWASan allocator flag. max_malloc_fill_size is the maximal amount of "
    "bytes that will be filled with malloc_fill_byte on malloc.")

HWASAN_FLAG(bool, free_checks_tail_magic, 1,
            "If set, free() will check the magic values "
            "to the right of the allocated object "
            "if the allocation size is not a divident of the granule size")
HWASAN_FLAG(
    int, max_free_fill_size, 0,
    "HWASan allocator flag. max_free_fill_size is the maximal amount of "
    "bytes that will be filled with free_fill_byte during free.")
HWASAN_FLAG(int, malloc_fill_byte, 0xbe,
            "Value used to fill the newly allocated memory.")
HWASAN_FLAG(int, free_fill_byte, 0x55, "Value used to fill deallocated memory.")
HWASAN_FLAG(int, heap_history_size, 1023,
            "The number of heap (de)allocations remembered per thread. "
            "Affects the quality of heap-related reports, but not the ability "
            "to find bugs.")
// OHOS_LOCAL begin
HWASAN_FLAG(
    int, heap_history_size_main_thread, 102300,
    "The number of heap (de)allocations remembered for the main thread. "
    "Affects the quality of heap-related reports, but not the ability "
    "to find bugs.")
// OHOS_LOCAL end
HWASAN_FLAG(bool, export_memory_stats, true,
            "Export up-to-date memory stats through /proc")
HWASAN_FLAG(int, stack_history_size, 1024,
            "The number of stack frames remembered per thread. "
            "Affects the quality of stack-related reports, but not the ability "
            "to find bugs.")

// Malloc / free bisection. Only tag malloc and free calls when a hash of
// allocation size and stack trace is between malloc_bisect_left and
// malloc_bisect_right (both inclusive). [0, 0] range is special and disables
// bisection (i.e. everything is tagged). Once the range is narrowed down
// enough, use malloc_bisect_dump to see interesting allocations.
HWASAN_FLAG(uptr, malloc_bisect_left, 0,
            "Left bound of malloc bisection, inclusive.")
HWASAN_FLAG(uptr, malloc_bisect_right, 0,
            "Right bound of malloc bisection, inclusive.")
HWASAN_FLAG(bool, malloc_bisect_dump, false,
            "Print all allocations within [malloc_bisect_left, "
            "malloc_bisect_right] range ")

// Exit if we fail to enable the AArch64 kernel ABI relaxation which allows
// tagged pointers in syscalls.  This is the default, but being able to disable
// that behaviour is useful for running the testsuite on more platforms (the
// testsuite can run since we manually ensure any pointer arguments to syscalls
// are untagged before the call.
HWASAN_FLAG(bool, fail_without_syscall_abi, true,
            "Exit if fail to request relaxed syscall ABI.")

// OHOS_LOCAL begin
HWASAN_FLAG(
    bool, print_uaf_stacks_with_same_tag, true,
    "Control the output content of use-after-free, deciding whether to print "
    "all stack traces of matched allocations with the same tag restriction.")

// Heap allocation information for freed threads
HWASAN_FLAG(uptr, freed_threads_history_size, 100,
            "The number of freed threads can be recorded.")
HWASAN_FLAG(bool, verbose_freed_threads, false,
            "Print the heap allocation information of freed threads.")

// Limits the size of the heap memory allocated to be recorded in order to
// reduce the data. As a result, information may be missing. By default, the
// minimum/maximum threshold is not set.
HWASAN_FLAG(int, heap_record_min, 0,
            "Only recording the heap memory allocation information that is >= "
            "heap_record_min.")
HWASAN_FLAG(int, heap_record_max, 0,
            "Only recording the heap memory allocation information that is <= "
            "heap_record_max.")

HWASAN_FLAG(int, memory_around_register_size, 128,
            "When reporting, the memory content of the address in register "
            "±memory_around_register_size is printed.")

// Set the quarantine area for freed heap, which is used to detect UAF-Write and
// Overflow-Write. Provide the detection capability for dynamic libraries
// compiled without hwasan option.
HWASAN_FLAG(int, heap_quarantine_thread_max_count, 128,
            "Set the maximum count for heap quarantine per thread.")
HWASAN_FLAG(int, heap_quarantine_min, 0,
            "The freed heap size should be larger than the minimum size before "
            "it is placed into the heap quarantine.")
HWASAN_FLAG(
    int, heap_quarantine_max, 0,
    "The freed heap size should be smaller than the maximum size before "
    "it is placed into the heap quarantine.")

// Default minimum configuration
HWASAN_FLAG(bool, allow_user_segv_handler, true,
            "Deprecated. True has no effect, use handle_sigbus=1. If false, "
            "handle_*=1 will be upgraded to handle_*=2.")
HWASAN_FLAG(int, detect_odr_violation, 0,
            "Default does not detect single defined rule (ODR) violations.")
HWASAN_FLAG(bool, alloc_dealloc_mismatch, false,
            "Report errors on malloc/delete, new/free, new/delete[], etc.")
HWASAN_FLAG(bool, allocator_may_return_null, true,
            "If true, the allocator does not crash and returns nullptr when "
            "out of memory.")
HWASAN_FLAG(bool, detect_container_overflow, false,
            "If false, disabling out-of-bounds detection. See "
            "https://github.com/google/sanitizers/wiki/"
            "AddressSanitizerContainerOverflow")
HWASAN_FLAG(bool, detect_leaks, false, "Disable memory leak detection.")
HWASAN_FLAG(
    int, report_globals, 0,
    "Controls the way to handle globals (0 - don't detect buffer overflow on "
    "globals, 1 - detect buffer overflow, 2 - print data about registered "
    "globals).")
HWASAN_FLAG(bool, allow_user_poisoning, true,
            "If set, user may manually mark memory regions as poisoned or "
            "unpoisoned.")
HWASAN_FLAG(bool, log_exe_name, true,
            "If set true, mention name of executable when reporting error and "
            "append executable name to logs (as in \"log_path.exe_name.pid\").")
HWASAN_FLAG(bool, detect_stack_use_after_return, false,
            "Disables stack-use-after-return checking at run-time.")
HWASAN_FLAG(int, print_module_map, 2,
            "Print the process module map where supported (0 - don't print, "
            "1 - print only once before process exits, 2 - print after each "
            "report).")
HWASAN_FLAG(bool, intercept_send, false,
            "If set true, uses custom wrappers for send* functions "
            "to find more errors.")
// OHOS_LOCAL end