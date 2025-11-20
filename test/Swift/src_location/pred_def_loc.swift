// clang-format off
// Check that the location in predicate definitions are rewritten correctly.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

class MyTuple {
    var fst : Int = 0
    var snd : Int = 0
}

/*@
predicate tupRead(_ t : MyTuple) := _perm(t, Frac(1, 2));
@*/

// COM: Find function of pred definition
// CHECK: define hidden swiftcc i1 @"{{.*}}tupReady{{.*}}"(ptr %0) {{#[0-9]+}} !dbg {{![0-9]+}} !pallas.predDef [[predDefMD:![0-9]+]] {

// COM: Match locations of instructions in the predicate definition
// CHECK: call void @pallas.fracOf(
// CHECK-SAME: !dbg [[fracOfLoc:![0-9]+]]

// CHECK: = call i1 @pallas.perm(
// CHECK-SAME: !dbg [[permLoc:![0-9]+]]

// COM: Check that the locations are correct 
// CHECK-DAG: [[fracOfLoc]] = !DILocation(line: 11, column: 46, scope: [[scopeMD:![0-9]+]])
// CHECK-DAG: [[permLoc]] = !DILocation(line: 11, column: 37, scope: [[scopeMD]])