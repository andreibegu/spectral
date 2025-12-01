// clang-format off
// Test that expressions that include the old-construct are transformed correctly
// when used with small structs. 
// In this case we expect the small struct to be passed as its integers and 
// to be returned as a compound-type of integers.
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created and struct is decomposed as expected
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64, i16)

// COM: Declaration has MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-function is called
// CHECK-DAG: call { i64, i16 } @"pallas.old { i64, i16 }_i64,i16"(i64 {{%[0-9]+}}, i16 {{%[0-9]+}})


#include <stdint.h>

typedef struct S {
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
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