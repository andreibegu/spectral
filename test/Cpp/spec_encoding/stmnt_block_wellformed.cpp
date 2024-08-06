// clang-format off
// Test that staement-blocks are correctly encoded as MD-nodes.
// I.e. the order of the statements has to be maintained.  
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// MD-encoding of block is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: MD-Node of the statement-block is wellformed
// COM: Statements are in correct order
// CHECK: [[blockMD]] = !{[[assumeMD1:![0-9]+]], [[assumeMD2:![0-9]+]], [[assertMD:![0-9]+]]}
// CHECK: [[assumeMD1]] = !{!"pallas.assume", ptr @_Z13PALLAS_SPEC_0ff,
// CHECK: [[assumeMD2]] = !{!"pallas.assume", ptr @_Z13PALLAS_SPEC_1ff,
// CHECK: [[assertMD]] = !{!"pallas.assert", ptr @_Z13PALLAS_SPEC_2ff,

bool ferociousFunction (float x, float y) {
    /*@
    assume x > 0 && y > 0;
    assume y >= 0;
    assert x + y > 0;
    @*/
    return x + y * x;
}