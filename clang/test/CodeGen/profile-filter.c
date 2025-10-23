// RUN: %clang_cc1 -fprofile-instrument=clang -fcoverage-mapping -dump-coverage-mapping -emit-llvm %s -o - | FileCheck %s

// RUN: echo "fun:test3" > %t-func.list
// RUN: %clang_cc1 -fprofile-instrument=clang -fcoverage-mapping -dump-coverage-mapping -fprofile-list=%t-func.list -emit-llvm %s -o - | FileCheck %s --check-prefix=FUNC

// RUN: echo "src:%s" | sed -e 's/\\/\\\\/g' > %t-file.list
// RUN: %clang_cc1 -fprofile-instrument=clang -fcoverage-mapping -dump-coverage-mapping -fprofile-list=%t-file.list -emit-llvm %s -o - | FileCheck %s --check-prefix=FILE

// RUN: echo -e "[clang]\nfun:test3\n[llvm]\nfun:test2" > %t-section.list
// RUN: %clang_cc1 -fprofile-instrument=llvm -fprofile-list=%t-section.list -emit-llvm %s -o - | FileCheck %s --check-prefix=SECTION

// RUN: echo -e "fun:test*\n!fun:test3" > %t-exclude.list
// RUN: %clang_cc1 -fprofile-instrument=clang -fcoverage-mapping -dump-coverage-mapping -fprofile-list=%t-exclude.list -emit-llvm %s -o - | FileCheck %s --check-prefix=EXCLUDE

// RUN: echo "!fun:test3" > %t-exclude-only.list
// RUN: %clang_cc1 -fprofile-instrument=clang -fcoverage-mapping -dump-coverage-mapping -fprofile-list=%t-exclude-only.list -emit-llvm %s -o - | FileCheck %s --check-prefix=EXCLUDE

unsigned i;

// CHECK: test3
// CHECK: test2
// FUNC: test3
// FUNC-NOT: test2
// FILE: test3
// FILE: test2
// EXCLUDE-NOT: test3
// EXCLUDE: test2

// CHECK-NOT: noprofile
// CHECK: @test3
// FUNC-NOT: noprofile
// FUNC: @test3
// FILE-NOT: noprofile
// FILE: @test3
// SECTION: noprofile
// SECTION: @test3
// EXCLUDE: noprofile
// EXCLUDE: @test3
unsigned test3(void) {
  // CHECK: %pgocount = load i64, ptr @__profc_test3
  // FUNC: %pgocount = load i64, ptr @__profc_test3
  // FILE: %pgocount = load i64, ptr @__profc_test3
  // SECTION-NOT: %pgocount = load i64, ptr @__profc_test3
  // EXCLUDE-NOT: %pgocount = load i64, ptr @__profc_test3
  return i + 1;
}

// CHECK-NOT: noprofile
// CHECK: @test2
// FUNC: noprofile
// FUNC: @test2
// FILE-NOT: noprofile
// FILE: @test2
// SECTION-NOT: noprofile
// SECTION: @test2
// EXCLUDE-NOT: noprofile
// EXCLUDE: @test2
unsigned test2(void) {
  // CHECK: %pgocount = load i64, ptr @__profc_test2
  // FUNC-NOT: %pgocount = load i64, ptr @__profc_test2
  // FILE: %pgocount = load i64, ptr @__profc_test2
  // SECTION: %pgocount = load i64, ptr @__profc_test2
  // EXCLUDE: %pgocount = load i64, ptr @__profc_test2
  return i - 1;
}
