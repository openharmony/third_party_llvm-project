// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -fno-rtti -fptrauth-vtable-pointer-type-discrimination \
// RUN:   -fptrauth-vtable-pointer-address-discrimination -emit-llvm -o - %s | FileCheck %s --check-prefixes=CHECK,ELF

// CHECK: %struct.Base1 = type { ptr }
// CHECK: %struct.Base2 = type { ptr }
// CHECK: %struct.Derived1 = type { %struct.Base1, %struct.Base2 }
// CHECK: %struct.Derived2 = type { %struct.Base2, %struct.Base1 }
// CHECK: %struct.Derived3 = type { %struct.Base1, %struct.Base2 }
// CHECK: @_ZTV5Base1 = linkonce_odr unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN5Base11aEv] }
// CHECK: @g_b1 = global %struct.Base1 { ptr getelementptr inbounds inrange(-16, 8) ({ [3 x ptr] }, ptr @_ZTV5Base1, i32 0, i32 0, i32 2) }
// CHECK: @_ZTV5Base2 = linkonce_odr unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN5Base21bEv] }
// CHECK: @g_b2 = global %struct.Base2 { ptr getelementptr inbounds inrange(-16, 8) ({ [3 x ptr] }, ptr @_ZTV5Base2, i32 0, i32 0, i32 2) }
// CHECK: @_ZTV8Derived1 = linkonce_odr unnamed_addr constant { [5 x ptr], [3 x ptr] } { [5 x ptr] [ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN8Derived11cEv, ptr @_ZN8Derived11dEv], [3 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base21bEv] }
// CHECK: @g_d1 = global { ptr, ptr } { ptr getelementptr inbounds inrange(-16, 24) ({ [5 x ptr], [3 x ptr] }, ptr @_ZTV8Derived1, i32 0, i32 0, i32 2), ptr getelementptr inbounds inrange(-16, 8) ({ [5 x ptr], [3 x ptr] }, ptr @_ZTV8Derived1, i32 0, i32 1, i32 2) }
// CHECK: @_ZTV8Derived2 = linkonce_odr unnamed_addr constant { [5 x ptr], [3 x ptr] } { [5 x ptr] [ptr null, ptr null, ptr @_ZN5Base21bEv, ptr @_ZN8Derived21cEv, ptr @_ZN8Derived21eEv], [3 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base11aEv] }
// CHECK: @g_d2 = global { ptr, ptr } { ptr getelementptr inbounds inrange(-16, 24) ({ [5 x ptr], [3 x ptr] }, ptr @_ZTV8Derived2, i32 0, i32 0, i32 2), ptr getelementptr inbounds inrange(-16, 8) ({ [5 x ptr], [3 x ptr] }, ptr @_ZTV8Derived2, i32 0, i32 1, i32 2) }
// CHECK: @_ZTV8Derived3 = linkonce_odr unnamed_addr constant { [4 x ptr], [3 x ptr] } { [4 x ptr] [ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN8Derived31iEv], [3 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base21bEv] }
// CHECK: @g_d3 = global { ptr, ptr } { ptr getelementptr inbounds inrange(-16, 16) ({ [4 x ptr], [3 x ptr] }, ptr @_ZTV8Derived3, i32 0, i32 0, i32 2), ptr getelementptr inbounds inrange(-16, 8) ({ [4 x ptr], [3 x ptr] }, ptr @_ZTV8Derived3, i32 0, i32 1, i32 2) }
// CHECK: @g_vb1 = global %struct.VirtualBase1 zeroinitializer,{{.*}} align 8
// CHECK: @g_vb2 = global %struct.VirtualBase2 zeroinitializer,{{.*}} align 8
// CHECK: @g_d4 = global %struct.Derived4 zeroinitializer,{{.*}} align 8
// CHECK: @_ZTV12VirtualBase1 = linkonce_odr unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN12VirtualBase11fEv] }
// CHECK: @_ZTT12VirtualBase1 = linkonce_odr unnamed_addr constant [2 x ptr] [ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTV12VirtualBase1, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTV12VirtualBase1, i32 0, i32 0, i32 4)]
// CHECK: @_ZTV12VirtualBase2 = linkonce_odr unnamed_addr constant { [5 x ptr], [4 x ptr] } { [5 x ptr] [ptr inttoptr (i64 8 to ptr), ptr null, ptr null, ptr @_ZN5Base21bEv, ptr @_ZN12VirtualBase21gEv], [4 x ptr] [ptr null, ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base11aEv] }
// CHECK: @_ZTT12VirtualBase2 = linkonce_odr unnamed_addr constant [2 x ptr] [ptr getelementptr inbounds inrange(-24, 16) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTV12VirtualBase2, i32 0, i32 0, i32 3), ptr getelementptr inbounds inrange(-24, 8) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTV12VirtualBase2, i32 0, i32 1, i32 3)]
// CHECK: @_ZTV8Derived4 = linkonce_odr unnamed_addr constant { [7 x ptr], [5 x ptr] } { [7 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN12VirtualBase11fEv, ptr @_ZN8Derived41hEv], [5 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base21bEv, ptr @_ZN12VirtualBase21gEv] }
// CHECK: @_ZTT8Derived4 = linkonce_odr unnamed_addr constant [7 x ptr] [ptr getelementptr inbounds inrange(-32, 24) ({ [7 x ptr], [5 x ptr] }, ptr @_ZTV8Derived4, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTC8Derived40_12VirtualBase1, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTC8Derived40_12VirtualBase1, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-24, 16) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTC8Derived48_12VirtualBase2, i32 0, i32 0, i32 3), ptr getelementptr inbounds inrange(-24, 8) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTC8Derived48_12VirtualBase2, i32 0, i32 1, i32 3), ptr getelementptr inbounds inrange(-32, 24) ({ [7 x ptr], [5 x ptr] }, ptr @_ZTV8Derived4, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-24, 16) ({ [7 x ptr], [5 x ptr] }, ptr @_ZTV8Derived4, i32 0, i32 1, i32 3)]
// CHECK: @_ZTC8Derived40_12VirtualBase1 = linkonce_odr unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN12VirtualBase11fEv] }
// CHECK: @_ZTC8Derived48_12VirtualBase2 = linkonce_odr unnamed_addr constant { [5 x ptr], [4 x ptr] } { [5 x ptr] [ptr inttoptr (i64 -8 to ptr), ptr null, ptr null, ptr @_ZN5Base21bEv, ptr @_ZN12VirtualBase21gEv], [4 x ptr] [ptr null, ptr inttoptr (i64 8 to ptr), ptr null, ptr @_ZN5Base11aEv] }
// CHECK: @_ZTV8Derived5 = linkonce_odr unnamed_addr constant { [6 x ptr], [6 x ptr] } { [6 x ptr] [ptr inttoptr (i64 8 to ptr), ptr null, ptr null, ptr @_ZN5Base21bEv, ptr @_ZN12VirtualBase21gEv, ptr @_ZN8Derived51hEv], [6 x ptr] [ptr null, ptr null, ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base11aEv, ptr @_ZN12VirtualBase11fEv] }
// CHECK: @_ZTT8Derived5 = linkonce_odr unnamed_addr constant [7 x ptr] [ptr getelementptr inbounds inrange(-24, 24) ({ [6 x ptr], [6 x ptr] }, ptr @_ZTV8Derived5, i32 0, i32 0, i32 3), ptr getelementptr inbounds inrange(-24, 16) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTC8Derived50_12VirtualBase2, i32 0, i32 0, i32 3), ptr getelementptr inbounds inrange(-24, 8) ({ [5 x ptr], [4 x ptr] }, ptr @_ZTC8Derived50_12VirtualBase2, i32 0, i32 1, i32 3), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTC8Derived58_12VirtualBase1, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr] }, ptr @_ZTC8Derived58_12VirtualBase1, i32 0, i32 0, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr], [6 x ptr] }, ptr @_ZTV8Derived5, i32 0, i32 1, i32 4), ptr getelementptr inbounds inrange(-32, 16) ({ [6 x ptr], [6 x ptr] }, ptr @_ZTV8Derived5, i32 0, i32 1, i32 4)]
// CHECK: @_ZTC8Derived50_12VirtualBase2 = linkonce_odr unnamed_addr constant { [5 x ptr], [4 x ptr] } { [5 x ptr] [ptr inttoptr (i64 8 to ptr), ptr null, ptr null, ptr @_ZN5Base21bEv, ptr @_ZN12VirtualBase21gEv], [4 x ptr] [ptr null, ptr inttoptr (i64 -8 to ptr), ptr null, ptr @_ZN5Base11aEv] }
// CHECK: @_ZTC8Derived58_12VirtualBase1 = linkonce_odr unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr @_ZN5Base11aEv, ptr @_ZN12VirtualBase11fEv] }
// CHECK: @llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_nopac_global_constant_initializers.cpp, ptr null }]

struct __attribute__((nopac)) Base1 { virtual void a() {} };
struct __attribute__((nopac)) Base2 { virtual void b() {} };
struct __attribute__((nopac)) Derived1 : public Base1, public Base2 {
  virtual void c() {}
  virtual void d() {}
};
struct __attribute__((nopac)) Derived2 : public Base2, public Base1 {
  virtual void c() {}
  virtual void e() {}
};

struct __attribute__((nopac)) Derived3 : public Base1, public Base2 {
  constexpr Derived3(){}
  virtual void i() {}
};

Base1 g_b1;
Base2 g_b2;
Derived1 g_d1;
Derived2 g_d2;
Derived3 g_d3;

extern "C" void test_basic_inheritance() {
  Base1 g_b1;
  Base2 g_b2;
  Derived1 g_d1;
  Derived2 g_d2;
  Derived3 g_d3;
}

struct __attribute__((nopac)) VirtualBase1 : virtual Base1 {
  VirtualBase1(){}
  virtual void f() {}
};
struct __attribute__((nopac)) VirtualBase2 : virtual Base1, Base2 {
  VirtualBase2(){}
  virtual void g() {}
};
struct __attribute__((nopac)) Derived4 : VirtualBase1, VirtualBase2 {
  virtual void h() {}
};
struct __attribute__((nopac)) Derived5 : VirtualBase2, VirtualBase1 {
  virtual void h() {}
};

// ELF-LABEL:    define {{.*}} void @_ZN12VirtualBase1C1Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN12VirtualBase2C1Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN8Derived4C1Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN8Derived5C1Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64


VirtualBase1 g_vb1;
VirtualBase2 g_vb2;
Derived4 g_d4;
Derived5 g_d5;


extern "C" void cross_check_vtables(Base1 *__attribute__((nopac)) b1,
                   Base2 *__attribute__((nopac)) b2,
                   Derived1 *__attribute__((nopac)) d1,
                   Derived2 *__attribute__((nopac)) d2,
                   Derived3 *__attribute__((nopac)) d3,
                   VirtualBase1 *__attribute__((nopac)) vb1,
                   VirtualBase2 *__attribute__((nopac)) vb2,
                   Derived4 *__attribute__((nopac)) d4,
                   Derived4 *__attribute__((nopac)) d5) {
  asm("; b1->a()" ::: "memory");
  b1->a();
  asm("; b2->b()" ::: "memory");
  b2->b();
  asm("; d1->a()" ::: "memory");
  d1->a();
  asm("; d1->c()" ::: "memory");
  d1->c();
  asm("; d2->a()" ::: "memory");
  d2->a();
  asm("; d2->c()" ::: "memory");
  d2->c();
  asm("; d3->a()" ::: "memory");
  d3->a();
  asm("; d3->b()" ::: "memory");
  d3->b();
  asm("; d3->i()" ::: "memory");
  d3->i();
  asm("; vb1->a()" ::: "memory");
  vb1->a();
  asm("; vb1->f()" ::: "memory");
  vb1->f();
  asm("; vb2->a()" ::: "memory");
  vb2->a();
  asm("; vb2->g()" ::: "memory");
  vb2->g();
  asm("; d4->a()" ::: "memory");
  d4->a();
  asm("; d4->b()" ::: "memory");
  d4->b();
  asm("; d4->f()" ::: "memory");
  d4->f();
  asm("; d4->g()" ::: "memory");
  d4->g();
  asm("; d4->h()" ::: "memory");
  d4->h();
  asm("; d5->a()" ::: "memory");
  d5->a();
  asm("; d5->b()" ::: "memory");
  d5->b();
  asm("; d5->f()" ::: "memory");
  d5->f();
  asm("; d5->g()" ::: "memory");
  d5->g();
  asm("; d5->h()" ::: "memory");
  d5->h();
}

// CHECK-LABEL: define{{.*}} void @cross_check_vtables(
// CHECK: "; b1->a()",
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; b2->b()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d1->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d1->c()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d2->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d2->c()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d3->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d3->b()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d3->i()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; vb1->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; vb1->f()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; vb2->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; vb2->g()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d4->a()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d4->b()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d4->f()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d4->g()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK: "; d4->h()"
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN5Base1C2Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN5Base2C2Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN8Derived1C2Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN8Derived2C2Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64

// ELF-LABEL:    define {{.*}} void @_ZN8Derived3C2Ev
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
// CHECK-NOT: call i64 @llvm.ptrauth.blend(i64 {{%.*}}, i64
