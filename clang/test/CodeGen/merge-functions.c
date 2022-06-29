// Check that code gen really activates function merging. In this case
// just check a trivial functions merging case.

// RUN: %clang_cc1 -emit-llvm %s -fmerge-functions -o - | FileCheck %s

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

