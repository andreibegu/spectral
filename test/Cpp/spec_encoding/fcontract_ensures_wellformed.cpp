// clang-format off
// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | FileCheck-15 %s

// COM: Contract is attached to function
// CHECK: @_Z3barf(
// CHECK-SAME: [[arg:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Match the MD-nodes of the function arguments
// CHECK: store float [[arg]], ptr [[argLoc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[argLoc]], metadata [[argMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@[^(]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the ensures-Clause is wellformed
// CHECK: [[contractMD]] = !{{{![0-9]+}}, [[rMD:![0-9]+]]}
// CHECK: [[rMD]] = !{!"pallas.ensures", {{![0-9]+}}, ptr [[wrapperFunc]], [[argMD]]}

/*@
ensures x >= 0;
@*/
float bar (float x) {
    return x; 
}