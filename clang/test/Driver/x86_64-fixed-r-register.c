// RUN: %clang %s -mfixed-r8 -### 2>&1 | FileCheck %s --check-prefix=R8
// RUN: %clang %s -mfixed-r9 -### 2>&1 | FileCheck %s --check-prefix=R9
// RUN: %clang %s -mfixed-r10 -### 2>&1 | FileCheck %s --check-prefix=R10
// RUN: %clang %s -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R11
// RUN: %clang %s -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R12
// RUN: %clang %s -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R13
// RUN: %clang %s -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R14
// RUN: %clang %s -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R15

// RUN: %clang %s -mfixed-r8 -mfixed-r9 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9
// RUN: %clang %s -mfixed-r8 -mfixed-r10 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R10
// RUN: %clang %s -mfixed-r8 -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R11
// RUN: %clang %s -mfixed-r8 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R12
// RUN: %clang %s -mfixed-r8 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R13
// RUN: %clang %s -mfixed-r8 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R14
// RUN: %clang %s -mfixed-r8 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R15
// RUN: %clang %s -mfixed-r9 -mfixed-r10 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R10
// RUN: %clang %s -mfixed-r9 -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R11
// RUN: %clang %s -mfixed-r9 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R12
// RUN: %clang %s -mfixed-r9 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R13
// RUN: %clang %s -mfixed-r9 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R14
// RUN: %clang %s -mfixed-r9 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R15
// RUN: %clang %s -mfixed-r10 -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R11
// RUN: %clang %s -mfixed-r10 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R12
// RUN: %clang %s -mfixed-r10 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R13
// RUN: %clang %s -mfixed-r10 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R14
// RUN: %clang %s -mfixed-r10 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R15
// RUN: %clang %s -mfixed-r11 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R12
// RUN: %clang %s -mfixed-r11 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R13
// RUN: %clang %s -mfixed-r11 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R14
// RUN: %clang %s -mfixed-r11 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R15
// RUN: %clang %s -mfixed-r12 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R12 --check-prefix=R13
// RUN: %clang %s -mfixed-r12 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R12 --check-prefix=R14
// RUN: %clang %s -mfixed-r12 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R12 --check-prefix=R15
// RUN: %clang %s -mfixed-r13 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R13 --check-prefix=R14
// RUN: %clang %s -mfixed-r13 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R13 --check-prefix=R15
// RUN: %clang %s -mfixed-r14 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R14 --check-prefix=R15

// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10
// RUN: %clang %s -mfixed-r9 -mfixed-r10 -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R10 --check-prefix=R11
// RUN: %clang %s -mfixed-r10 -mfixed-r11 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R11 --check-prefix=R12
// RUN: %clang %s -mfixed-r11 -mfixed-r12 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R11 --check-prefix=R12 --check-prefix=R13
// RUN: %clang %s -mfixed-r12 -mfixed-r13 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R12 --check-prefix=R13 --check-prefix=R14
// RUN: %clang %s -mfixed-r13 -mfixed-r14 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R13 --check-prefix=R14 --check-prefix=R15

// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -mfixed-r11 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10 --check-prefix=R11
// RUN: %clang %s -mfixed-r9 -mfixed-r10 -mfixed-r11 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R9 --check-prefix=R10 --check-prefix=R11 --check-prefix=R12
// RUN: %clang %s -mfixed-r10 -mfixed-r11 -mfixed-r12 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R10 --check-prefix=R11 --check-prefix=R12 --check-prefix=R13
// RUN: %clang %s -mfixed-r11 -mfixed-r12 -mfixed-r13 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R11 --check-prefix=R12 --check-prefix=R13 --check-prefix=R14
// RUN: %clang %s -mfixed-r12 -mfixed-r13 -mfixed-r14 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R12 --check-prefix=R13 --check-prefix=R14 --check-prefix=R15

// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -mfixed-r11 -mfixed-r12 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10 --check-prefix=R11 --check-prefix=R12
// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -mfixed-r11 -mfixed-r12 -mfixed-r13 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10 --check-prefix=R11 --check-prefix=R12 --check-prefix=R13
// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -mfixed-r11 -mfixed-r12 -mfixed-r13 -mfixed-r14 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10 --check-prefix=R11 --check-prefix=R12 --check-prefix=R13 --check-prefix=R14
// RUN: %clang %s -mfixed-r8 -mfixed-r9 -mfixed-r10 -mfixed-r11 -mfixed-r12 -mfixed-r13 -mfixed-r14 -mfixed-r15 -### 2>&1 | FileCheck %s --check-prefix=R8 --check-prefix=R9 --check-prefix=R10 --check-prefix=R11 --check-prefix=R12 --check-prefix=R13 --check-prefix=R14 --check-prefix=R15

// R8-NOT: error:
// R9-NOT: error:
// R10-NOT: error:
// R11-NOT: error:
// R12-NOT: error:
// R13-NOT: error:
// R14-NOT: error:
// R15-NOT: error:
