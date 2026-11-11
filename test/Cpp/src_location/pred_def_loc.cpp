// clang-format off
// Test that source-locations are correctly translated in predicate definitions
// RUN: %Spec2IR %s -lib %CppSpecLib -lang Cpp | %FileCheck %s

/*@
declare using namespace pallasSpec;
@*/

/*@
predicate pArr(int *a, int n) :=  _forall(_inRange(0, _bv<int>("i"), n), 
                                         _Perm(&a[_bv<int>("i")], _writePerm));
@*/


// COM: Find predicate definition
// CHECK: define dso_local noundef zeroext i1 @_Z4pArrPii(ptr noundef %0, i32 noundef %1) 
// CHACK-SAME: !pallas.predDef [[defMarker:![0-9]+]]

// COM: Find md-locations of instructions in predicate
// CHECK: call void @pallas.fracOf(
// CHECK-SAME: , !dbg [[fracLoc:![0-9]+]]

// CHECK: call i1 @pallas.perm(
// CHECK-SAME: , !dbg [[permLoc:![0-9]+]]

// CHECK: call i1 @pallas.forall(
// CHECK-SAME: , !dbg [[forallLoc:![0-9]+]]

// COM: Check that the locations are correct
// CHECK-DAG: [[sProg:![0-9]+]] = distinct !DISubprogram(name: "pArr", 
// CHECK-DAG: [[fracLoc]] = !DILocation(line: 11, column: 67, scope: [[sProg]])
// CHECK-DAG: [[permLoc]] = !DILocation(line: 11, column: 42, scope: [[sProg]])
// CHECK-DAG: [[forallLoc]] = !DILocation(line: 10, column: 35, scope: [[sProg]])