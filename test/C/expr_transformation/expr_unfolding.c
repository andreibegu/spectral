// clang-format off
// Test that the _unfolding-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration of _unfolding is present and labeled
// COM: bool
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] zeroext i1 @"pallas.unfolding zeroext i1_noundef zeroext i1"(i1 noundef zeroext, i1 noundef zeroext)
// CHECK-DAG: [[mdNode]] = !{!"pallas.unfolding"}
// CHECK-DAG: {{%[0-9]+}} = call zeroext i1 @"pallas.unfolding zeroext i1_noundef zeroext i1"(

// COM: Struct1
// CHECK-DAG: declare !pallas.specLib [[mdNode]] i64 @"pallas.unfolding i64_i64"(i1 noundef zeroext, i64)
// CHECK-DAG: {{%[0-9]+}} = call i64 @"pallas.unfolding i64_i64"(

// COM: Struct2
// CHECK-DAG: declare !pallas.specLib [[mdNode]] { i64, i32 } @"pallas.unfolding { i64, i32 }_i64,i32"(i1 noundef zeroext, i64, i32)
// CHECK-DAG: {{%[0-9]+}} = call { i64, i32 } @"pallas.unfolding { i64, i32 }_i64,i32"(

// COM: Struct3
// CHECK-DAG: declare !pallas.specLib [[mdNode]] void @"pallas.unfolding sret(%struct.Struct3) align 8 void_noundef byval(%struct.Struct3) align 8 ptr"(ptr sret(%struct.Struct3) align 8, i1 noundef zeroext, ptr noundef byval(%struct.Struct3) align 8)
// CHECK-DAG: call void @"pallas.unfolding sret(%struct.Struct3) align 8 void_noundef byval(%struct.Struct3) align 8 ptr"(

#include <stdbool.h>
#include <stdint.h>

typedef struct Struct1
{
    int32_t a;
    int32_t b;
} AVerySmallStruct;

typedef struct Struct2
{
    int32_t a;
    int32_t b;
    int32_t c;
} ASlightlyLargerStruct;

typedef struct Struct3
{
    int64_t a;
    int64_t b;
    int64_t c;
    int64_t d;
    int64_t e;
} ARatherBigStruct;

/*@
declare DEF_UNFOLDING(bool);
declare DEF_UNFOLDING(AVerySmallStruct);
declare DEF_UNFOLDING(ASlightlyLargerStruct);
declare DEF_UNFOLDING(ARatherBigStruct);
@*/

/*@
predicate wPred(int *p) := _sep(p != NULL,
                                _Perm(p, _write));
@*/

/*@
requires _unfolding(bool)(wPred(a), *a == 0);
requires _unfolding(AVerySmallStruct)(wPred(a), s1).a == 1;
requires _unfolding(ASlightlyLargerStruct)(wPred(a), s2).a == 1;
requires _unfolding(ARatherBigStruct)(wPred(a), s3).a == 1;
@*/
bool fantasticFunction(int *a,
                       AVerySmallStruct s1,
                       ASlightlyLargerStruct s2,
                       ARatherBigStruct s3)
{
    int tmp = *a;
    return tmp == 0;
}