// clang-format off
// Test that statement/blocks with multiple statements are correctlz encoded. 
// RUN: %Spec2IR %s -lib %CSpecLib -m | FileCheck-15 %s

// MD-encoding of block is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: MD-Node of the statement-block is wellformed (i.e. a tuple with 
// COM: references to an assume- and an assert-node (in this order.))
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assumeMD:![0-9]+]], [[assertMD:![0-9]+]]}
// CHECK: [[assumeMD]] = !{!"pallas.assume", {{![0-9]+}}, ptr @PALLAS_SPEC_0,
// CHECK: [[assertMD]] = !{!"pallas.assert", {{![0-9]+}}, ptr @PALLAS_SPEC_1,


float bar (float x, double z) {
    /*@
    assume x > 0.0 && z > 0.0;
    assert x + z > 0.0;
    @*/
    return x + z * 5; 
}