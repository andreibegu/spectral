// clang-format off
// Test that the locations of the wrapper-funcction's DISubprogram and its 
// scope are translated correctly. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

/*@
requires x + 1.0 > 0.0;
@*/
func bar (x: Double) -> Double {
    return x - 1;
}

// COM: Match the subprogram of bar to identify file and cu
// CHECK: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-SAME: scope: [[barScopeMD:![0-9]+]], 
// CHECK-SAME: file: [[barFileMD:![0-9]+]], 
// CHECK-SAME: unit: [[barCU:![0-9]+]]

// COM: Check that the subprogram refers to the correct lines and scopes. 
// CHECK-DAG: {{![0-9]+}} = distinct !DISubprogram(name: "PALLAS_SPEC_0", 
// CHECK-SAME: scope: [[barScopeMD]]
// CHECK-SAME: file: [[barFileMD]]
// CHECK-SAME: line: 7
// CHECK-SAME: scopeLine: 7
// CHECK-SAME: unit: [[barCU]]