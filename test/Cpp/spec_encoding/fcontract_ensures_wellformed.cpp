// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Contract is attached to function
// CHECK: i32 @bar(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]
// CHECK: @llvm.dbg.value(
// CHECK-SAME: [[arg1]]
// CHECK-SAME: metadata [[arg1MD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the ensures-Clause is wellformed
// CHECK: !{!"pallas.ensures", ptr [[wrapperFunc]], [[arg1MD]]}

/*@
ensures x >= 0;
@*/
int bar (int x) {
    return x; 
}