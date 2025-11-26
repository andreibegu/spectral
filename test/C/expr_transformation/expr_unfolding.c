// clang-format off
// Test that the _unfolding-construct is transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration of _unfolding is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] zeroext i1 @pallas.unfolding(i1 noundef zeroext, i1 noundef zeroext)
// CHECK-DAG: [[mdNode]] = !{!"pallas.unfolding"}

// COM: Unified specification-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.unfolding(

/*@
predicate wPred(int *p) := _sep(p != NULL, 
                                _Perm(p, _write));
@*/

/*@
requires _unfolding(wPred(a), *a = 0);
@*/
void fantasticFunction (int *a) {
    int tmp = *a;
}