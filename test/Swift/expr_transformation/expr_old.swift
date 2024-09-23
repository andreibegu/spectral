
// clang-format off
// Test that the old=-construct of the specification-library is correctly translated
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

// COM: Declarations are created
// CHECK-DAG: declare !pallas.specLib [[oldMD:![0-9]+]] float [[oldF:@pallas.old.[0-9]+]](float)
// CHECK-DAG: declare !pallas.specLib [[oldMD]] double [[oldD:@pallas.old.[0-9]+]](double)

// COM: Declarations have MD-NODE
// CHECK-DAG: [[oldMD]] = !{!"pallas.old"}
// COM: Old-functions are called
// CHECK-DAG: = call float [[oldF]](float 
// CHECK-DAG: = call double [[oldD]](double 

/*@
ensures _old(a) == a && _old(b) == b;
ensures _old(b) == b;
@*/
func foo(a: Float, b: Double, c: Double) {
    let x = -a
    let y = b + c
}