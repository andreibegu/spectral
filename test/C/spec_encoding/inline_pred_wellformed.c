// Test the inline-flag for predicates is encoded correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Predicate-definition is present and labeled
// CHECK-DAG: define dso_local zeroext i1 @myPred(ptr noundef %0, i32 noundef %1) {{#[0-9]}} !dbg {{![0-9]+}} !pallas.predDef [[pDefMarker:![0-9]+]]
// CHECK-DAG: define dso_local zeroext i1 @myInlinePred(ptr noundef %0, i32 noundef %1) {{#[0-9]}} !dbg {{![0-9]+}} !pallas.predDef [[pInlineDefMarker:![0-9]+]]

// CHECK-DAG: [[pDefMarker]] = !{i1 false}
// CHECK-DAG: [[pInlineDefMarker]] = !{i1 true}


/*@
       predicate myPred(int *p, int v) := _Perm(p, _fracOf(1, 1)) && *p == v;
inline predicate myInlinePred(int *p, int v) := _Perm(p, _fracOf(1, 1)) && *p == v;
@*/