// clang-format off
// Test that the result-construct is transformed correctly for small structs.
// In this case, the struct is returned as a compound-value {i64, i16}. 
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] { i64, i16 } @"pallas.result { i64, i16 }"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call { i64, i16 } @"pallas.result { i64, i16 }"()
#include <stdint.h>

typedef struct S {
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
} SmallStruct;

/*@
 declare DEF_RESULT(SmallStruct) ;
@*/

/*@
ensures _result(SmallStruct).a == _result(SmallStruct).a;
@*/
SmallStruct fun () {
    SmallStruct s;
    s.a = 0;
    return s;
}