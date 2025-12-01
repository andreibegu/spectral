// Test that MD-Node encoding of predicate definitions works as expected.
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Predicate-definition is present and labeled
// CHECK-DAG: define dso_local zeroext i1 @pZero(ptr noundef %0) {{#[0-9]}} !dbg {{![0-9]+}} !pallas.predDef [[pDefMarker:![0-9]+]]

// COM: Fold is encoded in MD
// CHECK-DAG: ret i32 42, !dbg {{![0-9]+}}, !pallas.stmntBlock [[sBlockMD:![0-9]+]]

// CHECK-DAG: [[sBlockMD]] = !{{{![0-9]+}}, [[foldMD:![0-9]+]]}
// CHECK-DAG: [[foldMD]] = !{!"pallas.fold", {{![0-9]+}}, ptr @PALLAS_SPEC_0,

// COM: Call to the pred-function exists
// CHECK-DAG:  = call zeroext i1 @pZero(

/*@
predicate pZero(int *p) := _Perm(p, _fracOf(1, 1)) && *p == 0;
@*/

int foo (int *a) {
    /*@
    fold pZero(a);
    @*/
    return 42;
}