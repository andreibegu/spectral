// clang-format off
// Test that requires-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Contract is attached to function
// CHECK: float @fun(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} @PALLAS_SPEC_0
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the requires-Clause is wellformed
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[cClauseMD:![0-9]+]]}
// CHECK-DAG: [[cClauseMD]] = !{!"pallas.requires", {{![0-9]+}}, ptr @PALLAS_SPEC_0, [[emp]], [[emp]], {{![0-9]+}}}
// CHECK-DAG: [[emp]] = !{}


/*@
requires x >= 0;
@*/
float fun (float x, float y) {
    return x + y; 
}