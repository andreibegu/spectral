// clang-format off
// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

// COM: Contract is attached to function
// CHECK: @_Z3barf(
// CHECK-SAME: [[arg:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@[^(]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the ensures-Clause is wellformed (we do not check the mapping here, this is already done in fcontract_requires_wellformed.cpp)
// CHECK-DAG: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[rMD:![0-9]+]]}
// CHECK-DAG: [[rMD]] = !{!"pallas.ensures", {{![0-9]+}}, ptr [[wrapperFunc]], [[emp]], [[emp]], {{![0-9]+}}}
// CHECK-DAG: [[emp]] = !{}

/*@
ensures x >= 0;
@*/
float bar (float x) {
    return x; 
}