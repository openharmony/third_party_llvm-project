// REQUIRES: gwp_asan
// RUN: %clangxx_gwp_asan -fsanitize=gwp_asan %s -o %t
// RUN: %expect_crash %run %t 2>&1 | FileCheck %s

// CHECK: GWP-ASan detected a memory error
// CHECK: Double Free at 0x{{[a-f0-9]+}} (a 1-byte allocation)

#include <cstdlib>

__attribute__((sanitize_with_gwp)) void doubleFree() {
  char *Ptr = new char;
  delete Ptr;
  delete Ptr;
  return 0;
}

int main() {
  doubleFree();
  return 0;
}
