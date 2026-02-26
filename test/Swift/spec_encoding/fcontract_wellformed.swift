// clang-format off
// Test that the MD-node for a function-contract is correctly attached and labeled
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
 requires true;
 ensures true;
 @*/
func bar (a: Double) -> Double {
    return a + 1.0;
}


// COM: Check that contract is attached to the correct function
// CHECK: define hidden swiftcc double @"$s13tmp_ir_source3bar1aS2d_tF"(double %0)
// CHECK-SAME: !pallas.fcontract [[contrMD:![0-9]+]]

// COM: Check that the contract is well-formed
// CHECK-DAG: [[contrMD]] = !{[[sLoc:![0-9]+]], i1 false, i1 false, [[emp:![0-9]+]], [[emp]], [[req:![0-9]+]], [[ens:![0-9]+]]}
// CHECK-DAG: [[emp]] = !{}

// CHECK-DAG: [[sLoc]] = !{!"pallas.srcLoc",
// CHECK-DAG: [[req]] = !{!"pallas.requires",
// CHECK-DAG: [[ens]] = !{!"pallas.ensures",