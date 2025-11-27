// clang-format off
// Test that the result-construct is transformed correctly for small struct.
// It is expected that the struct is treated as a i64-value. 
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] i64 @"pallas.result i64"()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call i64 @"pallas.result i64"()
#include <stdint.h>

typedef struct S {
    int32_t i;
    float j;
} SmallStruct;

/*@
 declare DEF_RESULT(SmallStruct) ;
@*/

/*@
ensures _result(SmallStruct).i >= 0;
@*/
SmallStruct fun () {
    SmallStruct s;
    s.i = 0;
    return s;
}