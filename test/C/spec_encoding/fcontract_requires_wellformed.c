// Test that requires-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

// COM: Contract is attached to function
// CHECK: i32 @fun(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: [[arg2:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Match the MD-nodes of the function arguments
// CHECK: @llvm.dbg.value(
// CHECK-SAME: [[arg1]]
// CHECK-SAME: metadata [[arg1MD:![0-9]+]]
// CHECK: @llvm.dbg.value(
// CHECK-SAME: [[arg2]]
// CHECK-SAME: metadata [[arg2MD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the requires-Clause is wellformed
// CHECK: !{!"pallas.requires", ptr [[wrapperFunc]], [[arg1MD]], [[arg2MD]]}

/*@
requires x >= 0;
@*/
int fun (int x, int y) {
    return x + y; 
}