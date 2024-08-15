// clang-format off
// Test that assert-statements are correctly encoded as MD-nodes. 
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Collect variables that are available in the encoding
// CHECK: i1 @_Z13funkyFunctionff(
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[xMD:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[yMD:![0-9]+]]
// CHECK: call void @llvm.dbg.declare(metadata ptr {{%[0-9]+}}, metadata [[tmpMD:![0-9]+]]

// MD-encoding is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: Generated encoding-function is labeled
// CHECK: define {{.*}} @_Z13PALLAS_SPEC_0ffb
// CHECK-SAME: !pallas.exprWrapper

// COM: MD-Node of the assert-statement is wellformed
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assertMD:![0-9]+]]}
// CHECK: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr @_Z13PALLAS_SPEC_0ffb, [[xMD]], [[yMD]], [[tmpMD]]}

bool funkyFunction (float x, float y) {
    bool tmp = x > y;
    /*@
    assert tmp == x > y;
    @*/
    return tmp; 
}