// clang-format off
// Test that requires-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

// COM: Contract is attached to function
// CHECK: float @fun(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: [[arg2:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// COM: Match the MD-nodes of the function arguments
// CHECK: store float [[arg1]], ptr [[arg1Loc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[arg1Loc]], metadata [[arg1MD:![0-9]+]]
// CHECK: store float [[arg2]], ptr [[arg2Loc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[arg2Loc]], metadata [[arg2MD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the requires-Clause is wellformed
// CHECK: !{!"pallas.requires", ptr [[wrapperFunc]], [[arg1MD]], [[arg2MD]]}

/*@
requires x >= 0;
@*/
float fun (float x, float y) {
    return x + y; 
}