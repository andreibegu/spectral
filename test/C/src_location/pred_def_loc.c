// clang-format off
// Test that source-locations are correctly translated in predicate definitions
// RUN: %Spec2IR %s -lib %CSpecLib -lang C | %FileCheck %s
/*@
predicate pZero(int *p) := _Perm(p, _fracOf(1, 1)) 
                            && *p == 0;
@*/

/*@
requires pZero(a);
@*/
int foo (int *a) {
    /*@
    unfold pZero(a);
    assert *a == 0;
    fold pZero(a);
    @*/
    return 42;
}

// COM: Find predicate definition
// CHECK: define dso_local zeroext i1 @pZero(ptr noundef %0)
// CHECK-SAME: !pallas.predDef [[defMarker:![0-9]+]]

// COM: Find md-locations of instructions in predicate
// CHECK: call void @pallas.fracOf(
// CHECK-SAME: , !dbg [[fracLoc:![0-9]+]]

// CHECK: call i1 @pallas.perm(
// CHECK-SAME: , !dbg [[permLoc:![0-9]+]]

// CHECK: = icmp eq i32 {{%[0-9]+}}, 0, !dbg [[cmpLoc:![0-9]+]]

// COM: Check that the locations are correct
// CHECK-DAG: [[sProg:![0-9]+]] = distinct !DISubprogram(name: "pZero", 
// CHECK-DAG: [[fracLoc]] = !DILocation(line: 5, column: 37, scope: [[sProg]])
// CHECK-DAG: [[permLoc]] = !DILocation(line: 5, column: 28, scope: [[sProg]])
// CHECK-DAG: [[cmpLoc]] = !DILocation(line: 6, column: 35, scope: [[sProg]])