// clang-format off
// Test that assume-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Collect variables that are available in the encoding
// CHECK: i1 @_Z16fabulousFunctionff(
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[xMD:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[yMD:![0-9]+]]

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} @_Z13PALLAS_SPEC_0ff
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the assert-statement is wellformed
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assumeMD:![0-9]+]]}
// CHECK: [[assumeMD]] = !{!"pallas.assume", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0ff, [[xMD]], [[yMD]]}

bool fabulousFunction (float x, float y) {
    /*@
    assume x > 0 && y > 0;
    @*/
    return x && !y; 
}