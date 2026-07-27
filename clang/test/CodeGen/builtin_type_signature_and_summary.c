// RUN: %clang_cc1 %s -emit-llvm -o - | FileCheck %s

typedef struct {
    unsigned int val;
} StructVal1;
typedef struct {
    unsigned int val;
} StructVal2;
typedef struct {
    unsigned int val;
} StructVal3;
enum EnumType {
    TYPE1 = 0,
    TYPE2 = 1,
    TYPE3 = 2,
};
struct StructVal {
    union {
        StructVal1 uid;
        StructVal2 gid;
        StructVal3 projid;
    };
    enum EnumType type;
};

struct Composite {
    unsigned int hash;
    void *inuse;
    void *free;
    void *dirty;
    void *lock;
    void *block;
    void *count;
    void *val;
    struct StructVal id;
    unsigned long flags;
};
typedef struct Composite CompositeS;

struct Test1 {
    unsigned int a;
    unsigned int b;
};

struct Test2 {
    unsigned long a;
};

struct Test3 {
    unsigned int a;
    void *ptr;
};

union TestUnion {
    void *ptr;
    unsigned long data;
};

struct Test4 {
    unsigned int a;
    union TestUnion b;
    void *ptr;
};

struct TypeWithZeroLengthZrray {
    int data[0];
};

struct Test5 {
    unsigned int a;
    struct Test4 b[10];
    struct TypeWithZeroLengthZrray array;
};

struct Test6 {
    unsigned a : 1;
    unsigned b : 2;
    unsigned c : 1;
    unsigned d : 1;
    unsigned e : 1;
    unsigned f : 1;
    unsigned g : 4;
};

union Test7 {
    unsigned a : 1;
    unsigned b : 2;
    unsigned c : 1;
    unsigned d : 1;
    unsigned e : 1;
    unsigned f : 1;
    unsigned g : 4;
};

struct Test8 {
    float a;
    double b;
};

struct Test9 {
    __int128_t a;
    __int128_t b;
};

union Test10 {
    __int128_t a;
    __int128_t b;
};

struct Test11 {
    unsigned int c;
    __int128_t a;
    unsigned int d;
    __int128_t b;
};

struct TestAll {
    struct Test1 a;    // 2             size 64
    struct Test2 b;    // 2  offset 64  size 64
    struct Test3 c;    // 21 offset 128 size 128
    union TestUnion d; // 3 offset 256 size 64
    struct Test4 e;    // 231 offset 320 size 192
    struct Test5 f;    // 2 231*10  512 size 1984
    struct Test6 g;    //     offset 2496 size 32
    union Test7 h;     // gh 和在一起 2 2528   32
    struct Test8 i;    // 22           2560   128
    struct Test9 j;    // 2222         2688   256
    union Test10 k;    // 22           2944   128
    CompositeS l;
    struct Test11 m;
} __attribute__((used));


int main(void)
{
    // CHECK: @.str = private unnamed_addr constant [2 x i8] c"2\00", align 1
    // CHECK: @.str.1 = private unnamed_addr constant [3 x i8] c"21\00", align 1
    // CHECK: @.str.2 = private unnamed_addr constant [2 x i8] c"3\00", align 1
    // CHECK: @.str.3 = private unnamed_addr constant [11 x i8] c"2111111122\00", align 1
    // CHECK: @.str.4 = private unnamed_addr constant [4 x i8] c"231\00", align 1
    // CHECK: @.str.5 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
    // CHECK: @.str.6 = private unnamed_addr constant [32 x i8] c"2231231231231231231231231231231\00", align 1
    // CHECK: @.str.7 = private unnamed_addr constant [3 x i8] c"22\00", align 1
    // CHECK: @.str.8 = private unnamed_addr constant [5 x i8] c"2222\00", align 1
    // CHECK: @.str.9 = private unnamed_addr constant [9 x i8] c"20222022\00", align 1
    // CHECK: @.str.10 = private unnamed_addr constant [67 x i8]
    // c"222132312231231231231231231231231231231222222222211111112220222022\00", align 1

    char *sign = __builtin_hm_type_signature(struct Test1);
    unsigned long sum = __builtin_hm_type_summary(struct Test1);
    // expect 2, 4
    // CHECK: store ptr @.str, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test2);
    sum = __builtin_hm_type_summary(struct Test2);
    // expect 2, 4
    // CHECK: store ptr @.str, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test3);
    sum = __builtin_hm_type_summary(struct Test3);
    // expect 21, 6
    // CHECK: store ptr @.str.1, ptr %sign, align 8
    // CHECK: store i64 6, ptr %sum, align 8

    char *unionSign = __builtin_hm_type_signature(union TestUnion);
    unsigned long sumUnion = __builtin_hm_type_summary(union TestUnion);
    // expect 3,8
    // CHECK: store ptr @.str.2, ptr %unionSign, align 8
    // CHECK: store i64 8, ptr %sumUnion, align 8

    char *structValSign = __builtin_hm_type_signature(struct StructVal);
    sum = __builtin_hm_type_summary(struct StructVal);
    // expect 2,4
    // CHECK: store ptr @.str, ptr %structValSign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(CompositeS);
    sum = __builtin_hm_type_summary(CompositeS);
    // expect 2111111122,6
    // CHECK: store ptr @.str.3, ptr %sign, align 8
    // CHECK: store i64 6, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test4);
    sum = __builtin_hm_type_summary(struct Test4);
    // expect 231, 14
    // CHECK: store ptr @.str.4, ptr %sign, align 8
    // CHECK: store i64 14, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct TypeWithZeroLengthZrray);
    sum = __builtin_hm_type_summary(struct TypeWithZeroLengthZrray);
    // expect , 0
    // CHECK: store ptr @.str.5, ptr %sign, align 8
    // CHECK: store i64 0, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test5);
    sum = __builtin_hm_type_summary(struct Test5);
    // expect 2231231231231231231231231231231, 14
    // CHECK: store ptr @.str.6, ptr %sign, align 8
    // CHECK: store i64 14, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test6);
    sum = __builtin_hm_type_summary(struct Test6);
    // expect 2, 4
    // CHECK: store ptr @.str, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(union Test7);
    sum = __builtin_hm_type_summary(union Test7);
    // expect 2, 4
    // CHECK: store ptr @.str, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test8);
    sum = __builtin_hm_type_summary(struct Test8);
    // expect 22, 4
    // CHECK: store ptr @.str.7, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test9);
    sum = __builtin_hm_type_summary(struct Test9);

    // expect 2222, 4
    // CHECK: store ptr @.str.8, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(union Test10);
    sum = __builtin_hm_type_summary(union Test10);
    // expect 22, 4
    // CHECK: store ptr @.str.7, ptr %sign, align 8
    // CHECK: store i64 4, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct Test11);
    sum = __builtin_hm_type_summary(struct Test11);
    // expect 20222022, 5
    // CHECK: store ptr @.str.9, ptr %sign, align 8
    // CHECK: store i64 5, ptr %sum, align 8

    sign = __builtin_hm_type_signature(struct TestAll);
    sum = __builtin_hm_type_summary(struct TestAll);
    // expect 222132312231231231231231231231231231231222222222211111112220222022, 15
    // CHECK: store ptr @.str.10, ptr %sign, align 8
    // CHECK: store i64 15, ptr %sum, align 8

    return 0;
}
