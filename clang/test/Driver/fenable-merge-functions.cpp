// First. Check that option is passed into clangcc1
// RUN: %clang -fenable-merge-functions -### %s 2>&1 | FileCheck %s -check-prefix=CHECK_CC1
// CHECK_CC1: "-cc1" {{.*}} "-fmerge-functions"

// Second. Check that code gen really activates function merging. In this case
// just check a trivial functions merging case.
// One may say, that this is an superfluous check,
// for it is covered by clang/test/CodeGen/merge-functions.c
// But it is worth keeping, because it also checks whole driver + clang interaction chain.

// RUN: %clang -emit-llvm %s -fenable-merge-functions -c -S -o - | FileCheck %s

void foo1() {}
void foo2() {}

int main() {

  // If merge functions pass is enabled,
  // then second and first calls will refer to foo1

  // CHECK: call void @{{[^\(]*}}foo1{{[^\(]*}}()
  foo1();

  // CHECK: call void @{{[^\(]*}}foo1{{[^\(]*}}()
  foo2();

  return 0;
}
