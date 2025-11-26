// Test that the _unfolding construct for predicates is transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration of _unfolding is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] zeroext i1 @pallas.unfolding(i1 noundef zeroext, i1 noundef zeroext)
// CHECK-DAG: [[mdNode]] = !{!"pallas.unfolding"}

// COM: Unified specification-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.unfolding(

/*@
declare using namespace pallasSpec;
@*/

/*@
predicate pPtrWrite(int *a) := _sep(a != NULL, _Perm(a, _write));
@*/

/*@
requires pPtrWrite(p);
requires _unfolding(pPtrWrite(p), 
                    *p == 0);
@*/
void flamboyantFunction(int *p) {
    *p = 42;
}