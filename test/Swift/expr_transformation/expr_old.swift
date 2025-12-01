
// clang-format off
// Test that the old=-construct of the specification-library is correctly translated
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] float @"pallas.old float_float"(float)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] double @"pallas.old double_double"(double)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call float @"pallas.old float_float"(float 
// CHECK-DAG: = call double @"pallas.old double_double"(double 

/*@
ensures _old(a) == a && _old(b) == b;
ensures _old(b) == b;
@*/
func foo(a: Float, b: Double, c: Double) {
    let x = -a
    let y = b + c
}