// Test that expressions that include a short-circuting and 
// are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m | %FileCheck %s

// COM: Declaration of is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.scAnd(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.scAnd"}

// COM: Spec-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.scAnd(


/*@
requires _and(a > 0, b > 0);
@*/
void fun (int a, int b) {
    int c = a + b;
}