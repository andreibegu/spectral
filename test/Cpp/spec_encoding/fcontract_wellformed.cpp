// Test that MD-Node for contract is attached to function and refers 
// to tuple of two MD-Nodes
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// CHECK: i32 {{@[^(]+}}(
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[reqMD:![0-9]+]], [[ensMD:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}
// CHECK-DAG: [[reqMD]] = !{!"pallas.requires", {{![0-9]+}},
// CHECK-DAG: [[ensMD]] = !{!"pallas.ensures", {{![0-9]+}},

/*@
requires x >= 0;
ensures x >= 0;
@*/
int foo (int x) {
    return x; 
}