// RUN: %clang_cc1 %s -I%S -triple=aarch64-linux-gnu -fptrauth-calls -std=c++11 -emit-llvm -o - | FileCheck --check-prefix=ELF %s

#include <nopac_typeinfo>

struct __attribute__((nopac)) A { int a; };

// ELF: @_ZTI1A = linkonce_odr constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS1A }
// ELF: @_ZTVN10__cxxabiv117__class_type_infoE = external global [0 x ptr]
// ELF: @_ZTS1A = linkonce_odr constant [3 x i8] c"1A\00"

auto ATI = typeid(A);
