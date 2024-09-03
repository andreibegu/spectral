// clang-format off
// Test that the pure-flag of a function-contract is set
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

/*@
 pure;
 requires true;
 @*/
func bar (a: Double) -> Double {
    return a + 1.0;
}


// COM: Check that contract is attached to the correct function
// CHECK: define hidden swiftcc double @"$s14tmp_ir_source03bar1aS2d_tF"(double %0)
// CHECK-SAME: !pallas.fcontract [[contrMD:![0-9]+]]

// COM: Check that the contract is well-formed
// CHECK: [[contrMD]] = !{[[sLoc:![0-9]+]], i1 true, {{![0-9]+}}}

// CHECK-DAG: [[sLoc]] = !{!"pallas.srcLoc",