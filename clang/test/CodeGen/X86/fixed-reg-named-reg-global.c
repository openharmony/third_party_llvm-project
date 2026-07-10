// REQUIRES: x86-registered-target
// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r8/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r8 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r8d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r8 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r8w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r8 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r8b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r8 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r9/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r9 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r9d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r9 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r9w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r9 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r9b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r9 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r10/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r10 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r10d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r10 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r10w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r10 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r10b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r10 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r11/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r11 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r11d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r11 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r11w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r11 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r11b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r11 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r12/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r12 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r12d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r12 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r12w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r12 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r12b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r12 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION


// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r13/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r13 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r13d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r13 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r13w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r13 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r13b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r13 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION


// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r14/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r14 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r14d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r14 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r14w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r14 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r14b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r14 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION


// RUN: sed 's/_TYPE_/uint64_t/g;s/_REG_/r15/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r15 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint32_t/g;s/_REG_/r15d/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r15 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint16_t/g;s/_REG_/r15w/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r15 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

// RUN: sed 's/_TYPE_/uint8_t/g;s/_REG_/r15b/g' %s > %t
// RUN: %clang -x c - < %t -O0 2> %t1 || echo
// RUN: FileCheck < %t1 %t
// RUN: %clang -x c - < %t -mfixed-r15 -O0 -S -emit-llvm -o - | FileCheck %t --check-prefix=CHECK-HAVE-RESERVATION

#include <stdint.h>

// CHECK: error:
// CHECK-SAME: register '_REG_' unsuitable for global register variables on this target
// CHECK-HAVE-RESERVATION-NOT: @_REG_ = common global
register _TYPE_ R asm("_REG_");

// CHECK-HAVE-RESERVATION: define{{.*}} i[[bits:[0-9]+]] @get_register_value
// CHECK-HAVE-RESERVATION: [[ret:%[0-9]+]] = call i[[bits]] @llvm.read_register.i[[bits]](metadata !0)
// CHECK-HAVE-RESERVATION: ret i[[bits]] [[ret]]
_TYPE_ get_register_value() {
    return R;
}
// CHECK-HAVE-RESERVATION: declare{{.*}} i[[bits]] @llvm.read_register.i[[bits]](metadata)


// CHECK-HAVE-RESERVATION: define{{.*}} void @set_register_value
// CHECK-HAVE-RESERVATION: [[sto:%[0-9]+]] = load i[[bits]], ptr %
// CHECK-HAVE-RESERVATION: call void @llvm.write_register.i[[bits]](metadata !0, i[[bits]] [[sto]])
// CHECK-HAVE-RESERVATION: ret void
void set_register_value(_TYPE_ value) {
    R = value;
}
// CHECK-HAVE-RESERVATION: declare{{.*}} void @llvm.write_register.i[[bits]](metadata, i[[bits]])

int main() {
    set_register_value(1);
    int res = get_register_value();
    set_register_value(2);
    return res;
}

// CHECK-HAVE-RESERVATION: !llvm.named.register._REG_ = !{!0}
// CHECK-HAVE-RESERVATION: !0 = !{!"_REG_"}
