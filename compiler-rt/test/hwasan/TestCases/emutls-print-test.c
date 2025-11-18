// RUN: %clang_hwasan -mllvm -hwasan-instrument-without-TLS=true -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

// REQUIRES: stable-runtime
// REQUIRES: ohos_family

// Test emutls stack allocation records printing functionality
// This test verifies that emutls frame records are properly recorded
// and displayed in error reports.

#include <stdlib.h>
#include <sanitizer/hwasan_interface.h>

// External declaration for the emutls frame record function
extern void __hwasan_add_emutls_frame_record(unsigned long long frame_record_info);

// Simulate a frame record with some test data
#define FRAME_RECORD_INFO 0x123456789ABCDEF0ULL

__attribute__((noinline))
void add_emutls_records() {
  // Add some emutls frame records to test the recording functionality
  __hwasan_add_emutls_frame_record(FRAME_RECORD_INFO);
  __hwasan_add_emutls_frame_record(FRAME_RECORD_INFO + 1);
  __hwasan_add_emutls_frame_record(FRAME_RECORD_INFO + 2);
}

__attribute__((noinline))
void trigger_error() {
  // Trigger a stack buffer overflow to generate error report
  char buffer[10];
  char *volatile p = buffer;
  // This will cause a tag mismatch error
  p[15] = 0;  // Out of bounds access
}

int main() {
  add_emutls_records();
  trigger_error();

  // CHECK: Previously allocated frames with emutls:
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x{{.*}} {{.*}}emutls-print-test.c{{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x123456789abcdef2 {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x123456789abcdef1 {{.*}}
  // CHECK-NEXT: record_addr:0x{{.*}} record:0x123456789abcdef0 {{.*}}
}
