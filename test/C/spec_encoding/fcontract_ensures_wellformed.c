// Test that ensures-clause is correctly encoded as an MD-Node
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Contract is attached to function
// CHECK: float @bar(
// CHECK-SAME: [[arg1:%[0-9]+]]
// CHECK-SAME: !pallas.fcontract [[contractMD:![0-9]+]]

// CHECK: store float [[arg1]], ptr [[arg1Loc:%[0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr [[arg1Loc]], metadata [[arg1MD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the ensures-Clause is wellformed
// CHECK: [[contractMD]] = !{{{![0-9]+}}, i1 false, i1 false, [[cClauseMD:![0-9]+]]}
// CHECK: [[cClauseMD]] = !{!"pallas.ensures", {{![0-9]+}}, ptr [[wrapperFunc]], [[arg1MD]]}

/*@
ensures x >= 0;
@*/
float bar (float x) {
    return x; 
}