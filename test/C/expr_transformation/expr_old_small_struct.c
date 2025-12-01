// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// when used with small structs. 
// In this case we expect the small struct to be treated as an integer.
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created and has two arguments
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] i64 @"pallas.old i64_i64"(i64)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call i64 @"pallas.old i64_i64"(i64 {{%[0-9]+}})
#include <stdint.h>

typedef struct S {
    int32_t a;
    float b;
} SmallStruct;

/*@
 declare DEF_OLD(SmallStruct) ;
@*/

/*@
ensures _old(SmallStruct)(s).a == 0;
@*/
void fun (SmallStruct s) {
    int c = s.a + s.b;
}