// RUN: %clang_cc1 %s -x c++ -std=c++11 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics \
// RUN:   -emit-llvm -o - | FileCheck --check-prefixes=CHECK,NODISC %s

// RUN: %clang_cc1 %s -x c++ -std=c++11 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics \
// RUN:   -fptrauth-vtable-pointer-type-discrimination \
// RUN:   -emit-llvm -o - | FileCheck --check-prefixes=CHECK,TYPE %s

// RUN: %clang_cc1 %s -x c++ -std=c++11 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics \
// RUN:   -fptrauth-vtable-pointer-address-discrimination \
// RUN:   -emit-llvm -o - | FileCheck --check-prefixes=CHECK,ADDR %s

// RUN: %clang_cc1 %s -x c++ -std=c++11 -triple aarch64-linux-gnu -fptrauth-calls -fptrauth-intrinsics \
// RUN:   -fptrauth-vtable-pointer-type-discrimination \
// RUN:   -fptrauth-vtable-pointer-address-discrimination \
// RUN:   -emit-llvm -o - | FileCheck --check-prefixes=CHECK,BOTH %s

#include <ptrauth.h>

namespace test1 {

#define authenticated(a...) ptrauth_cxx_vtable_pointer(a)

struct __attribute__((nopac)) NoExplicitAuth {
  virtual ~NoExplicitAuth();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(no_authentication, no_address_discrimination, no_extra_discrimination) ExplicitlyDisableAuth {
  virtual ~ExplicitlyDisableAuth();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(default_key, address_discrimination, default_extra_discrimination) ExplicitAddressDiscrimination {
  virtual ~ExplicitAddressDiscrimination();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(default_key, no_address_discrimination, default_extra_discrimination) ExplicitNoAddressDiscrimination {
  virtual ~ExplicitNoAddressDiscrimination();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(default_key, default_address_discrimination, no_extra_discrimination) ExplicitNoExtraDiscrimination {
  virtual ~ExplicitNoExtraDiscrimination();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(default_key, default_address_discrimination, type_discrimination) ExplicitTypeDiscrimination {
  virtual ~ExplicitTypeDiscrimination();
  virtual void f();
  virtual void g();
};

struct __attribute__((nopac)) authenticated(default_key, default_address_discrimination, custom_discrimination, 42424) ExplicitCustomDiscrimination {
  virtual ~ExplicitCustomDiscrimination();
  virtual void f();
  virtual void g();
};

template <typename T>
struct __attribute__((nopac)) SubClass : T {
  virtual void g();
  virtual T *h();
};

template <typename T>
SubClass<T> *make_subclass(T *);

struct __attribute__((nopac)) authenticated(default_key, address_discrimination, type_discrimination) BasicStruct {
  virtual ~BasicStruct();
};

template <typename T>
struct __attribute__((nopac)) PrimaryBasicStruct : BasicStruct, T {};
template <typename T>
struct __attribute__((nopac)) PrimaryBasicStruct<T> *make_multiple_primary(T *);

template <typename T>
struct __attribute__((nopac)) VirtualSubClass : virtual T {
  virtual void g();
  virtual T *h();
};
template <typename T>
struct __attribute__((nopac)) VirtualPrimaryStruct : virtual T, VirtualSubClass<T> {};
template <typename T>
struct __attribute__((nopac)) VirtualPrimaryStruct<T> *make_virtual_primary(T *);

extern "C" {

// CHECK: @TVDisc_NoExplicitAuth = global i32 [[DISC_DEFAULT:49565]], align 4
int TVDisc_NoExplicitAuth = ptrauth_string_discriminator("_ZTVN5test114NoExplicitAuthE");

// CHECK: @TVDisc_ExplicitlyDisableAuth = global i32 [[DISC_DISABLED:24369]], align 4
int TVDisc_ExplicitlyDisableAuth = ptrauth_string_discriminator("_ZTVN5test121ExplicitlyDisableAuthE");

// CHECK: @TVDisc_ExplicitAddressDiscrimination = global i32 [[DISC_ADDR:56943]], align 4
int TVDisc_ExplicitAddressDiscrimination = ptrauth_string_discriminator("_ZTVN5test129ExplicitAddressDiscriminationE");

// CHECK: @TVDisc_ExplicitNoAddressDiscrimination = global i32 [[DISC_NO_ADDR:6022]], align 4
int TVDisc_ExplicitNoAddressDiscrimination = ptrauth_string_discriminator("_ZTVN5test131ExplicitNoAddressDiscriminationE");

// CHECK: @TVDisc_ExplicitNoExtraDiscrimination = global i32 [[DISC_NO_EXTRA:9072]], align 4
int TVDisc_ExplicitNoExtraDiscrimination = ptrauth_string_discriminator("_ZTVN5test129ExplicitNoExtraDiscriminationE");

// CHECK: @TVDisc_ExplicitTypeDiscrimination = global i32 [[DISC_TYPE:6177]], align 4
int TVDisc_ExplicitTypeDiscrimination = ptrauth_string_discriminator("_ZTVN5test126ExplicitTypeDiscriminationE");


// CHECK-LABEL: define{{.*}} void @test_default(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_default(NoExplicitAuth *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_disabled(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_disabled(ExplicitlyDisableAuth *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_addr_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_addr_disc(ExplicitAddressDiscrimination *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_no_addr_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_no_addr_disc(ExplicitNoAddressDiscrimination *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_no_extra_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_no_extra_disc(ExplicitNoExtraDiscrimination *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_type_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_type_disc(ExplicitTypeDiscrimination *a) {
  a->f();
}

// CHECK-LABEL: define{{.*}} void @test_custom_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = load ptr, ptr {{%.*}}, align 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_custom_disc(ExplicitCustomDiscrimination *a) {
  a->f();
}

//
// Test some simple single inheritance cases.
// Codegen should be the same as the simple cases above once we have a vtable.
//

// CHECK-LABEL: define{{.*}} void @test_subclass_default(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_default(NoExplicitAuth *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_disabled(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_disabled(ExplicitlyDisableAuth *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_addr_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_addr_disc(ExplicitAddressDiscrimination *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_no_addr_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_no_addr_disc(ExplicitNoAddressDiscrimination *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_no_extra_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_no_extra_disc(ExplicitNoExtraDiscrimination *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_type_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_type_disc(ExplicitTypeDiscrimination *a) {
  make_subclass(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_subclass_custom_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[VTADDR:%.*]] = call noundef ptr @_ZN5test113make_subclass
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_subclass_custom_disc(ExplicitCustomDiscrimination *a) {
  make_subclass(a)->f();
}


//
// Test some simple multiple inheritance cases.
// Codegen should be the same as the simple cases above once we have a vtable.
//

// CHECK-LABEL: define{{.*}} void @test_multiple_default(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[CALL:%.*]] = call noundef ptr @_ZN5test121make_multiple_primary
// CHECK:         [[VTADDR:%.*]] = getelementptr inbounds i8, ptr [[CALL]], i64 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_multiple_default(NoExplicitAuth *a) {
  make_multiple_primary(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_multiple_disabled(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[CALL:%.*]] = call noundef ptr @_ZN5test121make_multiple_primary
// CHECK:         [[VTADDR:%.*]] = getelementptr inbounds i8, ptr [[CALL]], i64 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_multiple_disabled(ExplicitlyDisableAuth *a) {
  make_multiple_primary(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_multiple_custom_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK:         [[CALL:%.*]] = call noundef ptr @_ZN5test121make_multiple_primary
// CHECK:         [[VTADDR:%.*]] = getelementptr inbounds i8, ptr [[CALL]], i64 8
// CHECK:         [[VTABLE:%.*]] = load ptr, ptr [[VTADDR]], align 8
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_multiple_custom_disc(ExplicitCustomDiscrimination *a) {
  make_multiple_primary(a)->f();
}

//
// Test some virtual inheritance cases.
// Codegen should be the same as the simple cases above once we have a vtable,
// but twice for vtt/vtable.  The names in the vtt version have "VTT" prefixes.
//

// CHECK-LABEL: define{{.*}} void @test_virtual_default(ptr noundef {{%.*}}) {{#.*}} {
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_virtual_default(NoExplicitAuth *a) {
  make_virtual_primary(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_virtual_disabled(ptr noundef {{%.*}}) {{#.*}} {
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_virtual_disabled(ExplicitlyDisableAuth *a) {
  make_virtual_primary(a)->f();
}

// CHECK-LABEL: define{{.*}} void @test_virtual_custom_disc(ptr noundef {{%.*}}) {{#.*}} {
// CHECK-NOT:     call i64 @llvm.ptrauth.auth
void test_virtual_custom_disc(ExplicitCustomDiscrimination *a) {
  make_virtual_primary(a)->f();
}

} // extern "C"
} // namespace test1
