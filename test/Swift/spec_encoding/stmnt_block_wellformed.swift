// clang-format off
// Test that a block of specification-statements is correctly encoded.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


func bar (a: Double) -> Double {
    /*@
    assume a >= 0.0;
    assert a >= 0.0;
    assert a + 1.0 >= 1.0;
    @*/
    return a + 1.0;
}

// COM: MD-encoding of block is attached to instruction:
// CHECK: !pallas.stmntBlock [[blockMD:![0-9]+]]

// COM: MD-Node of the statement-block is wellformed
// COM: Statements are in correct order
// CHECK: [[blockMD]] = !{{{![0-9]+}}, [[assumeMD:![0-9]+]], [[assertMD1:![0-9]+]], [[assertMD2:![0-9]+]]}
// CHECK: [[assumeMD]] = !{!"pallas.assume", {{![0-9]+}}, ptr @
// CHECK: [[assertMD1]] = !{!"pallas.assert", {{![0-9]+}}, ptr @
// CHECK: [[assertMD2]] = !{!"pallas.assert", {{![0-9]+}}, ptr @
