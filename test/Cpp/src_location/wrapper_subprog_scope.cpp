// clang-format off
// Test that the locations of the wrapper-funcctions DISubprogram and its 
// scope are translated correctly. 
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | FileCheck-15 %s

/*@
requires x + 1 > 0;
@*/
float bar (float x) {
    return x - 1; 
}

// COM: Match the IR and remember the MD-nodes of the debug-locations
// CHECK: define dso_local noundef zeroext i1 @_Z13PALLAS_SPEC_0f(
// CHECK-SAME: !dbg [[sProgMD:![0-9]+]]

// COM: Match the subprogram of bar to identify file and cu
// CHECK: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-SAME: scope: [[barFileMD:![0-9]+]], 
// CHECK-SAME: file: [[barFileMD]], 
// CHECK-SAME: unit: [[barCU:![0-9]+]]

// COM: Check that the subprogram refers to the correct lines and scopes. 
// CHECK-DAG: [[sProgMD]] = distinct !DISubprogram(name: "PALLAS_SPEC_0", linkageName: "_Z13PALLAS_SPEC_0f"
// CHECK-SAME: scope: [[barFileMD]]
// CHECK-SAME: file: [[barFileMD]]
// CHECK-SAME: line: 7
// CHECK-SAME: scopeLine: 7
// CHECK-SAME: unit: [[barCU]]