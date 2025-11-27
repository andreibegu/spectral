// clang-format off
// Test that the result-construct is transformed correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration is created
// CHECK-DAG: declare !pallas.specLib [[resMD:![0-9]+]] double [[res:@"pallas.result double"]]()
// CHECK-DAG: [[resMD]] = !{!"pallas.result"}
// COM: result-function is called
// CHECK-DAG: = call double [[res]]()

/*@
ensures _result() == 0.0;
@*/
func anotherSuperAmazingFunction(_ p1: Double, _ p2: Double) -> Double {
    return p1 > p2 ? p1 : -p2;
}
