// clang-format off
// Test that the pure-flag of a function-contract is set
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

/*@
 pure;
 requires true;
 @*/
func bar (a: Double) -> Double {
    return a + 1.0;
}


// COM: Check that contract is attached to the correct function
// CHECK: define hidden swiftcc double @"$s13tmp_ir_source3bar1aS2d_tF"(double %0)
// CHECK-SAME: !pallas.fcontract [[contrMD:![0-9]+]]

// COM: Check that the contract has its pure-flag set
// CHECK-DAG: [[contrMD]] = !{[[sLoc:![0-9]+]], i1 true, i1 false, [[emp:![0-9]+]], [[emp]], {{![0-9]+}}}
// CHECK-DAG: [[emp]] = !{}

// CHECK-DAG: [[sLoc]] = !{!"pallas.srcLoc",