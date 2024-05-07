// Check that diagnostics print the vtable symbol name, when it doesn't have CFI typeid

// RUN: %clangxx -fno-sanitize=cfi -fno-lto -g -DSTATIC_LIB -c -o %t_static.o %s
// RUN: %clangxx_cfi_diag -g -o %t_exe_suffix %s %t_static.o
// RUN: %t_exe_suffix 2>&1 | FileCheck %s

// REQUIRES: cxxabi

struct S1 {
  S1();
  virtual void f1();
};

#ifdef STATIC_LIB

S1::S1() = default;

void S1::f1() {}

#else

int main() {
  S1 *S = new S1();
  // CHECK: runtime error: control flow integrity check for type 'S1' failed during virtual call
  // CHECK: note: invalid vtable (address points to {{.*}}vtable for S1)
  S->f1(); // trigger cfi-vcall failure

  return 0;
}

#endif // SHARED_LIB
