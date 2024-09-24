// clang-format off
// Test that assume-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Collect variables that are available in the encoding
// CHECK: float @bar(
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[xMD:![0-9]+]]

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} [[wrapperFunc:@PALLAS_SPEC_[0-9]+]]
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the assert-statement is wellformed
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assumeMD:![0-9]+]]}
// CHECK: [[assumeMD]] = !{!"pallas.assume", {{![0-9]+}}, ptr [[wrapperFunc]], [[xMD]]}


float bar (float x) {
    /*@
    assume x > 0;
    @*/
    return x + 1; 
}