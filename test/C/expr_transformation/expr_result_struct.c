// clang-format off
// Test that the result-construct is transformed correctly for structs
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] void @"pallas.result sret(%struct.S) align 8 void"(ptr sret(%struct.S) align 8)
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: call void @"pallas.result sret(%struct.S) align 8 void"(ptr sret(%struct.S) align 8 {{%[0-9]+}})

typedef struct S {
    double a, b, c, d, e, f, g;
} SillyStruct;

/*@
 declare DEF_RESULT(SillyStruct) ;
@*/

/*@
ensures _result(SillyStruct).a >= 0;
@*/
SillyStruct fun (SillyStruct s) {
    s.a = 0;
    return s;
}