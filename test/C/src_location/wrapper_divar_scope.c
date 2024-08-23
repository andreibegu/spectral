// clang-format off
// Test that the line-numbers and scope of DILocalVariable-nodes are 
// correctly modified. 
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | FileCheck-15 %s

/*@
requires x + 1 > y;
@*/
float bar (float x, double y) {
    return x - 1 + y; 
}

// COM: Match the IR and remember the MD-nodes of the debug-locations
// CHECK: define dso_local zeroext i1 @PALLAS_SPEC_0(
// CHECK-SAME: !dbg [[sProgMD:![0-9]+]]
// CHECK-NEXT: call void @llvm.dbg.value(metadata float %0, metadata [[xMD:![0-9]+]]
// CHECK-NEXT: call void @llvm.dbg.value(metadata double %1, metadata [[yMD:![0-9]+]]

// COM: Match the subprogram of bar to identify file and cu
// CHECK: [[barMD:![0-9]+]] = distinct !DISubprogram(name: "bar",
// CHECK-SAME: scope: [[barFileMD:![0-9]+]], 
// CHECK-SAME: file: [[barFileMD]], 
// CHECK-SAME: unit: [[barCU:![0-9]+]]

// COM: Check that the variables of the debug-intrinsics have the correct scope and line
// CHECK: [[xMD]] = !DILocalVariable(name: "x", arg: 1, scope: [[sProgMD]], file: [[barFileMD]], line: 7
// CHECK: [[yMD]] = !DILocalVariable(name: "y", arg: 2, scope: [[sProgMD]], file: [[barFileMD]], line: 7