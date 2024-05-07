// Check that diagnostics print CFI typeid names when RTTI is disabled

// RUN: %clangxx_cfi_diag -g -fno-rtti -o %t1 %s
// RUN: %t1 2>&1 | FileCheck %s

// REQUIRES: cxxabi

struct S1 {
  virtual void f1() {}
};

struct S2 {
  virtual void f2() {}
};

// One of the CFI typeids aliases all-vtables symbol, so trigger two checks
void f(S1 *S1p, S2 *S2p) {
  // CHECK-DAG: runtime error: control flow integrity check for type 'S1' failed during cast to unrelated type
  // CHECK-DAG: runtime error: control flow integrity check for type 'S2' failed during cast to unrelated type
  // CHECK-DAG: vtable CFI typeid is (or aliases) {{.*}}typeinfo name for S
  S2 *S2cast = reinterpret_cast<S2*>(S1p); // trigger cfi-unrelated-cast failure
  S1 *S1cast = reinterpret_cast<S1*>(S2p); // trigger cfi-unrelated-cast failure
}

int main() {
  S1 S1v;
  S2 S2v;
  f(&S1v, &S2v);
  return 0;
}
