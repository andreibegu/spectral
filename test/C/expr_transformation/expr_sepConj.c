// clang-format off
// Test that expressions including the separanting conjunction are transformed correctly
// RUN: %Spec2IR %s -lib %CSpecLib -m -lang C | %FileCheck %s

// COM: Declaration of implication is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.sepConj(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.sepConj"}

// COM: Unified specification-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.sepConj(

/*@
requires _sep(x > 0, y < 0);
@*/
void friendlyFunction (double x, double y) {
    int z = x + y;
}