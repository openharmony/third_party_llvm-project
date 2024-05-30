// RUN: %clangxx_cfi_dso -g -DSRC_A -fPIC -c %s -o %t.a.o -fsplit-lto-unit -fsanitize-cfi-cross-dso-req
// RUN: %clangxx_cfi_dso -g -DSRC_B -fPIC -c %s -o %t.b.nocfi.o -fno-sanitize=cfi -fsplit-lto-unit
// RUN: not %clangxx_cfi_dso %t.a.o %t.b.nocfi.o -shared -o %dynamiclib %ld_flags_rpath_so 2>&1 | FileCheck --check-prefix=FAIL-REQ %s

// RUN: %clangxx_cfi_dso -g -DSRC_B -fPIC -c %s -o %t.b.o -fsplit-lto-unit -fsanitize-cfi-cross-dso-req
// RUN: %clangxx_cfi_dso %t.a.o %t.b.o -shared -o %dynamiclib %ld_flags_rpath_so
// RUN: %clangxx_cfi_dso %s -o %t %ld_flags_rpath_exe
// RUN: %t 2>&1 | FileCheck --check-prefix=CFI %s

// FAIL-REQ: linking module flags 'Cross-DSO CFI': does not have the required value

#include <cstdio>

struct A {
  virtual void f();
};

A *create_B();

#ifdef SRC_B

struct B : A {
  void f() override;
};
void B::f() {
  puts("B");
}

A *create_B() {
  return new B();
}

#elif defined SRC_A

void A::f() {
  puts("A");
}

#else

int main() {
  A *ptr = create_B();
  ptr->f();
  // CFI: B
  return 0;
}

#endif
