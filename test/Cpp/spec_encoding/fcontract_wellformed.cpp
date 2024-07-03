// Test that MD-Node for contract is attached to function and refers 
// to tuple of two MD-Nodes
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// CHECK: i32 {{@[^(]+}}(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK: [[contractMD]] = !{[[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires",
// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures",

/*@
requires x >= 0;
ensures x >= 0;
@*/
int foo (int x) {
    return x; 
}