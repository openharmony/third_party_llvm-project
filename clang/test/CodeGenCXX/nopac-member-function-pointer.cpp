// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics -emit-llvm -std=c++11 -O1 -disable-llvm-passes -o - %s | FileCheck -check-prefixes=CHECK,ELF %s
// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics -emit-llvm -std=c++17 -O1 -disable-llvm-passes -o - %s | FileCheck -check-prefixes=CHECK,ELF,CXX17 %s
// RUN: %clang_cc1 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics -emit-llvm -std=c++11 -O1 -disable-llvm-passes -debug-info-kind=limited -o - %s | FileCheck -check-prefixes=CHECK,ELF %s

// CHECK: @gmethod0 = global { i64, i64 } { i64 ptrtoint (ptr @_ZN5Base011nonvirtual0Ev to i64), i64 0 }, align 8
// CHECK: @gmethod1 = global { i64, i64 } { i64 ptrtoint (ptr @_ZN8Derived011nonvirtual5Ev to i64), i64 0 }, align 8
// CHECK: @gmethod2 = global { i64, i64 } { i64 0, i64 1 }, align 8

// CHECK: @__const._Z13testArrayInitv.p0 = private unnamed_addr constant [1 x { i64, i64 }] [{ i64, i64 } { i64 ptrtoint (ptr @_ZN5Base011nonvirtual0Ev to i64), i64 0 }], align 8
// CHECK: @__const._Z13testArrayInitv.p1 = private unnamed_addr constant [1 x { i64, i64 }] [{ i64, i64 } { i64 0, i64 1 }], align 8
// CHECK: @__const._Z13testArrayInitv.c0 = private unnamed_addr constant %struct.Class0 { { i64, i64 } { i64 ptrtoint (ptr @_ZN5Base011nonvirtual0Ev to i64), i64 0 } }, align 8
// CHECK: @__const._Z13testArrayInitv.c1 = private unnamed_addr constant %struct.Class0 { { i64, i64 } { i64 0, i64 1 } }, align 8

// CHECK: @_ZN22testNoexceptConversion6mfptr1E = global { i64, i64 } { i64 ptrtoint (ptr @_ZN22testNoexceptConversion1S19nonvirtual_noexceptEv to i64), i64 0 }, align 8
// CHECK: @_ZN22testNoexceptConversion6mfptr2E = global { i64, i64 } { i64 0, i64 1 }, align 8
// CHECK: @_ZN22testNoexceptConversion15mfptr3_noexceptE = global { i64, i64 } { i64 ptrtoint (ptr @_ZN22testNoexceptConversion1S19nonvirtual_noexceptEv to i64), i64 0 }, align 8

// CHECK: @_ZTV5Base0 = unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI5Base0,
// CHECK-SAME: ptr @_ZN5Base08virtual1Ev,
// CHECK-SAME: ptr @_ZN5Base08virtual3Ev,
// CHECK-SAME: ptr @_ZN5Base016virtual_variadicEiz] }, align 8

typedef __SIZE_TYPE__ size_t;

namespace std {
template <typename _Ep>
class __attribute__((nopac)) initializer_list {
  const _Ep *__begin_;
  size_t __size_;

  initializer_list(const _Ep *__b, size_t __s);
};
} // namespace std

struct __attribute__((nopac)) Base0 {
  void nonvirtual0();
  virtual void virtual1();
  virtual void virtual3();
  virtual void virtual_variadic(int, ...);
};

struct __attribute__((nopac)) A0 {
  int d[4];
};

struct __attribute__((nopac)) A1 {
  int d[8];
};

struct __attribute__((nopac)) __attribute__((trivial_abi)) TrivialS {
  TrivialS(const TrivialS &);
  ~TrivialS();
  int p[4];
};

struct __attribute__((nopac)) Derived0 : Base0 {
  void virtual1() override;
  void nonvirtual5();
  virtual void virtual6();
  virtual A0 return_agg();
  virtual A1 sret();
  virtual void trivial_abi(TrivialS);
};

struct __attribute__((nopac)) Base1 {
  virtual void virtual7();
};

struct __attribute__((nopac)) Derived1 : Base0, Base1 {
  void virtual1() override;
  void virtual7() override;
};

typedef void (Base0::*__attribute__((nopac)) MethodTy0)();
#if __cplusplus >= 201703L
typedef void (Base0::*__attribute__((nopac)) NoExceptMethodTy0)() noexcept;
#endif
typedef void (Base0::*__attribute__((nopac)) VariadicMethodTy0)(int, ...);
typedef void (Derived0::*__attribute__((nopac)) MethodTy1)();

struct __attribute__((nopac)) Class0 {
  MethodTy1 m0;
};

// CHECK: define{{.*}} void @_ZN5Base08virtual1Ev(

// CHECK: define{{.*}} void @_Z5test0v()
// CHECK: %[[METHOD0:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[VARMETHOD1:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD2:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD3:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD4:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD5:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD6:.*]] = alloca { i64, i64 }, align 8
// CHECK-NEXT: %[[METHOD7:.*]] = alloca { i64, i64 }, align 8
// CHECK: store { i64, i64 } { i64 ptrtoint (ptr @_ZN5Base011nonvirtual0Ev to i64), i64 0 }, ptr %[[METHOD0]], align 8
// CHECK-NEXT: store { i64, i64 } { i64 0, i64 1 }, ptr %[[METHOD0]], align 8
// CHECK-NEXT: store { i64, i64 } { i64 8, i64 1 }, ptr %[[METHOD0]], align 8
// CHECK: store { i64, i64 } { i64 16, i64 1 }, ptr %[[VARMETHOD1]], align 8
// CHECK: store { i64, i64 } { i64 ptrtoint (ptr @_ZN5Base011nonvirtual0Ev to i64), i64 0 }, ptr %[[METHOD2]], align 8
// CHECK-NEXT: store { i64, i64 } { i64 0, i64 1 }, ptr %[[METHOD2]], align 8
// CHECK-NEXT: store { i64, i64 } { i64 8, i64 1 }, ptr %[[METHOD2]], align 8
// CHECK: store { i64, i64 } { i64 ptrtoint (ptr @_ZN8Derived011nonvirtual5Ev to i64), i64 0 }, ptr %[[METHOD2]], align 8
// CHECK: store { i64, i64 } { i64 24, i64 1 }, ptr %[[METHOD2]], align 8
// CHECK: store { i64, i64 } { i64 32, i64 1 }, ptr %[[METHOD3]], align 8
// CHECK: store { i64, i64 } { i64 40, i64 1 }, ptr %[[METHOD4]], align 8
// CHECK: store { i64, i64 } { i64 48, i64 1 }, ptr %[[METHOD5]], align 8
// CHECK: store { i64, i64 } { i64 0, i64 1 }, ptr %[[METHOD6]], align 8
// CHECK: store { i64, i64 } { i64 24, i64 1 }, ptr %[[METHOD7]], align 8
// CHECK: store { i64, i64 } { i64 0, i64 1 }, ptr %[[METHOD7]], align 8
// CHECK: ret void

// These are pure virtual function, so with nopac, there is nothing to define here.

// CHECK-NOT: define linkonce_odr hidden void @_ZN8Derived08virtual6Ev_vfpthunk_(ptr noundef %[[THIS:.*]])

// CHECK-NOT: define linkonce_odr hidden [2 x i64] @_ZN8Derived010return_aggEv_vfpthunk_(ptr noundef %{{.*}})

// CHECK-NOT: define linkonce_odr hidden void @_ZN8Derived04sretEv_vfpthunk_(ptr dead_on_unwind noalias writable sret(%struct.A1) align 4 %[[AGG_RESULT:.*]], ptr noundef %{{.*}})

// CHECK-NOT: define linkonce_odr hidden void @_ZN8Derived011trivial_abiE8TrivialS_vfpthunk_(ptr noundef %{{.*}}, [2 x i64] %{{.*}})

// CHECK-NOT: define linkonce_odr hidden void @_ZN5Base18virtual7Ev_vfpthunk_(ptr noundef %[[THIS:.*]])

// CHECK-NOT: define linkonce_odr hidden void @_ZN8Derived18virtual7Ev_vfpthunk_(ptr noundef %[[THIS:.*]])

void Base0::virtual1() {}

void test0() {
  MethodTy0 method0;
  method0 = &Base0::nonvirtual0;
  method0 = &Base0::virtual1;
  method0 = &Base0::virtual3;

  VariadicMethodTy0 varmethod1;
  varmethod1 = &Base0::virtual_variadic;

  MethodTy1 method2;
  method2 = &Derived0::nonvirtual0;
  method2 = &Derived0::virtual1;
  method2 = &Derived0::virtual3;
  method2 = &Derived0::nonvirtual5;
  method2 = &Derived0::virtual6;

  A0 (Derived0::*method3)();
  method3 = &Derived0::return_agg;

  A1 (Derived0::*method4)();
  method4 = &Derived0::sret;

  void (Derived0::*method5)(TrivialS);
  method5 = &Derived0::trivial_abi;

  void (Base1::*method6)();
  method6 = &Base1::virtual7;

  void (Derived1::*method7)();
  method7 = &Derived1::virtual7;
  method7 = &Derived1::virtual1;
}

// CHECK: define{{.*}} void @_Z5test1P5Base0MS_FvvE(ptr noundef %[[A0:.*]], [2 x i64] %[[A1_COERCE:.*]])
// CHECK: %[[A1:.*]] = alloca { i64, i64 }, align 8
// CHECK: %[[A0_ADDR:.*]] = alloca ptr, align 8
// CHECK: %[[A1_ADDR:.*]] = alloca { i64, i64 }, align 8
// CHECK: store [2 x i64] %[[A1_COERCE]], ptr %[[A1]], align 8
// CHECK: %[[A11:.*]] = load { i64, i64 }, ptr %[[A1]], align 8
// CHECK: store ptr %[[A0]], ptr %[[A0_ADDR]], align 8
// CHECK: store { i64, i64 } %[[A11]], ptr %[[A1_ADDR]], align 8
// CHECK: %[[V1:.*]] = load ptr, ptr %[[A0_ADDR]], align 8
// CHECK: %[[V2:.*]] = load { i64, i64 }, ptr %[[A1_ADDR]], align 8
// CHECK: %[[MEMPTR_ADJ:.*]] = extractvalue { i64, i64 } %[[V2]], 1
// CHECK: %[[MEMPTR_ADJ_SHIFTED:.*]] = ashr i64 %[[MEMPTR_ADJ]], 1
// CHECK: %[[V4:.*]] = getelementptr inbounds i8, ptr %[[V1]], i64 %[[MEMPTR_ADJ_SHIFTED]]
// CHECK: %[[MEMPTR_PTR:.*]] = extractvalue { i64, i64 } %[[V2]], 0
// CHECK: %[[V5:.*]] = and i64 %[[MEMPTR_ADJ]], 1
// CHECK: %[[MEMPTR_ISVIRTUAL:.*]] = icmp ne i64 %[[V5]], 0
// CHECK: br i1 %[[MEMPTR_ISVIRTUAL]]

// CHECK:  %[[VTABLE:.*]] = load ptr, ptr %[[V4]], align 8
// ELF:    %[[V12:.*]] = getelementptr i8, ptr %[[VTABLE]], i64 %[[MEMPTR_PTR]]
// CHECK:  %[[MEMPTR_VIRTUALFN:.*]] = load ptr, ptr %[[V12]], align 8
// CHECK:  br

// CHECK: %[[MEMPTR_NONVIRTUALFN:.*]] = inttoptr i64 %[[MEMPTR_PTR]] to ptr
// CHECK: br

// CHECK: %[[V14:.*]] = phi ptr [ %[[MEMPTR_VIRTUALFN]], {{.*}} ], [ %[[MEMPTR_NONVIRTUALFN]], {{.*}} ]
// CHECK: call void %[[V14]](ptr noundef nonnull align {{[0-9]+}} dereferenceable(8) %[[V4]])
// CHECK: ret void

void test1(Base0 *a0, MethodTy0 a1) {
  (a0->*a1)();
}

// CXX17: define{{.*}} void @_Z14test1_noexceptP5Base0MS_DoFvvE(
// CXX17: %[[V14:.*]] = phi ptr [ %{{.*}}, {{.*}} ], [ %{{.*}}, {{.*}} ]
// CXX17: call void %[[V14]](ptr noundef nonnull align {{[0-9]+}} dereferenceable(8) %{{.*}}) {{.*}}
#if __cplusplus >= 201703L
void test1_noexcept(Base0 *a0, NoExceptMethodTy0 a1) {
  (a0->*a1)();
}
#endif

// CHECK: define{{.*}} void @_Z15testConversion0M5Base0FvvEM8Derived0FvvE([2 x i64] %[[METHOD0_COERCE:.*]], [2 x i64] %[[METHOD1_COERCE:.*]])
// CHECK: %[[METHOD0:.*]] = alloca { i64, i64 }, align 8
// CHECK: %[[METHOD1:.*]] = alloca { i64, i64 }, align 8
// CHECK: %[[METHOD0_ADDR:.*]] = alloca { i64, i64 }, align 8
// CHECK: %[[METHOD1_ADDR:.*]] = alloca { i64, i64 }, align 8
// CHECK: store [2 x i64] %[[METHOD0_COERCE]], ptr %[[METHOD0]], align 8
// CHECK: %[[METHOD01:.*]] = load { i64, i64 }, ptr %[[METHOD0]], align 8
// CHECK: store [2 x i64] %[[METHOD1_COERCE]], ptr %[[METHOD1]], align 8
// CHECK: %[[METHOD12:.*]] = load { i64, i64 }, ptr %[[METHOD1]], align 8
// CHECK: store { i64, i64 } %[[METHOD01]], ptr %[[METHOD0_ADDR]], align 8
// CHECK: store { i64, i64 } %[[METHOD12]], ptr %[[METHOD1_ADDR]], align 8
// CHECK: %[[V2:.*]] = load { i64, i64 }, ptr %[[METHOD0_ADDR]], align 8
// CHECK: store { i64, i64 } %[[V2]], ptr %[[METHOD1_ADDR]], align 8
// CHECK: ret void

void testConversion0(MethodTy0 method0, MethodTy1 method1) {
  method1 = method0;
}

// CHECK: define{{.*}} void @_Z15testConversion1M5Base0FvvE(
// CHECK-NOT: call i64 @llvm.ptrauth.resign

void testConversion1(MethodTy0 method0) {
  MethodTy1 method1 = reinterpret_cast<MethodTy1>(method0);
}

// CHECK: define{{.*}} void @_Z15testConversion2M8Derived0FvvE(
// CHECK-NOT: call i64 @llvm.ptrauth.resign

void testConversion2(MethodTy1 method1) {
  MethodTy0 method0 = static_cast<MethodTy0>(method1);
}

// CHECK: define{{.*}} void @_Z15testConversion3M8Derived0FvvE(
// CHECK-NOT: call i64 @llvm.ptrauth.resign

void testConversion3(MethodTy1 method1) {
  MethodTy0 method0 = reinterpret_cast<MethodTy0>(method1);
}

// No need to call @llvm.ptrauth.resign if the source member function
// pointer is a constant.

// CHECK: define{{.*}} void @_Z15testConversion4v(
// CHECK: %[[METHOD0:.*]] = alloca { i64, i64 }, align 8
// CHECK: store { i64, i64 } { i64 0, i64 1 }, ptr %[[METHOD0]]
// CHECK: ret void

void testConversion4() {
  MethodTy0 method0 = reinterpret_cast<MethodTy0>(&Derived0::virtual1);
}

// This code used to crash.
namespace testNonVirtualThunk {
  struct __attribute__((nopac)) R {};

  struct __attribute__((nopac)) B0 {
    virtual void bar();
  };

  struct __attribute__((nopac)) B1 {
    virtual R foo();
  };

  struct __attribute__((nopac)) D : B0, B1 {
    virtual R foo();
  };

  D d;
}

namespace TestAnonymousNamespace {
namespace {
struct __attribute__((nopac)) S {
  virtual void foo(){};
};
} // namespace

void test() {
  auto t = &S::foo;
}
} // namespace TestAnonymousNamespace

MethodTy1 gmethod0 = reinterpret_cast<MethodTy1>(&Base0::nonvirtual0);
MethodTy0 gmethod1 = reinterpret_cast<MethodTy0>(&Derived0::nonvirtual5);
MethodTy0 gmethod2 = reinterpret_cast<MethodTy0>(&Derived0::virtual1);

// CHECK-LABEL: define{{.*}} void @_Z13testArrayInitv()
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %p0, ptr align 8 @__const._Z13testArrayInitv.p0, i64 16, i1 false)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %p1, ptr align 8 @__const._Z13testArrayInitv.p1, i64 16, i1 false)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %c0, ptr align 8 @__const._Z13testArrayInitv.c0, i64 16, i1 false)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %c1, ptr align 8 @__const._Z13testArrayInitv.c1, i64 16, i1 false)
// CHECK-NOT: store { i64, i64 } { i64 ptrtoint (ptr ptrauth (ptr @_ZN5Base011nonvirtual0Ev
// CHECK-NOT: store { i64, i64 } { i64 ptrtoint (ptr ptrauth (ptr @_ZN5Base08virtual1Ev_vfpthunk_

void initList(std::initializer_list<MethodTy1>);

void testArrayInit() {
  MethodTy1 p0[] = {&Base0::nonvirtual0};
  MethodTy1 p1[] = {&Base0::virtual1};
  Class0 c0{&Base0::nonvirtual0};
  Class0 c1{&Base0::virtual1};
  initList({&Base0::nonvirtual0});
  initList({&Base0::virtual1});
}



// STACK-PROT: define {{.*}}_vfpthunk{{.*}}[[ATTRS:#[0-9]+]]
// STACK-PROT: attributes [[ATTRS]] =
// STACK-PROT-NOT: ssp
// STACK-PROT-NOT: sspstrong
// STACK-PROT-NOT: sspreq
// STACK-PROT-NEXT: attributes

// CHECK: define{{.*}} void @_Z15testConvertNullv(
// CHECK: %[[T:.*]] = alloca { i64, i64 },
// store { i64, i64 } zeroinitializer, { i64, i64 }* %[[T]],

void testConvertNull() {
  VariadicMethodTy0 t = (VariadicMethodTy0)(MethodTy0{});
}

namespace testNoexceptConversion {

// CHECK-LABEL: define internal void @__cxx_global_var_init()
// CHECK: %[[V0:.*]] = load { i64, i64 }, ptr @_ZN22testNoexceptConversion15mfptr0_noexceptE, align 8
// CHECK: store { i64, i64 } %[[V0]], ptr @_ZN22testNoexceptConversion6mfptr4E, align 8

// CHECK: define {{.*}}void @_ZN22testNoexceptConversion5test0Ev()
// CHECK: %[[P0:.*]] = alloca { i64, i64 }, align 8
// CHECK: store { i64, i64 } { i64 ptrtoint (ptr @_ZN22testNoexceptConversion1S19nonvirtual_noexceptEv to i64), i64 0 }, ptr %[[P0]], align 8,

// CHECK: define {{.*}}void @_ZN22testNoexceptConversion5test1Ev()
// CHECK: %[[P0:.*]] = alloca { i64, i64 }, align 8
// CHECK: store { i64, i64 } { i64 0, i64 1 }, ptr %[[P0]], align 8

// CHECK: define {{.*}}void @_ZN22testNoexceptConversion5test2Ev()
// CHECK: %[[P0:.*]] = alloca { i64, i64 }, align 8
// CHECK: %[[V0:.*]] = load { i64, i64 }, ptr @_ZN22testNoexceptConversion15mfptr0_noexceptE, align 8
// CHECK: store { i64, i64 } %[[V0]], ptr %[[P0]], align 8,

struct __attribute__((nopac)) S {
  void nonvirtual_noexcept() noexcept;
  virtual void virtual_noexcept() noexcept;
};

void (S::*mfptr0_noexcept)() noexcept;
void (S::*mfptr1)() = &S::nonvirtual_noexcept;
void (S::*mfptr2)() = &S::virtual_noexcept;
void (S::*mfptr3_noexcept)() noexcept = &S::nonvirtual_noexcept;
void (S::*mfptr4)() = mfptr0_noexcept;

void test0() {
  void (S::*p0)() = &S::nonvirtual_noexcept;
}

void test1() {
  void (S::*p0)() = &S::virtual_noexcept;
}

void test2() {
  void (S::*p0)() = mfptr0_noexcept;
}

}

// CHECK: declare void @_ZN5Base08virtual3Ev(ptr noundef nonnull align 8 dereferenceable(8))

// CHECK: declare void @_ZN5Base016virtual_variadicEiz(ptr noundef nonnull align 8 dereferenceable(8), i32 noundef, ...)