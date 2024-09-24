// clang-format off
// Test that expressions including the separanting conjunction are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration of implication is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.sepConj(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.sepConj"}

// COM: Unified specification-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.sepConj(

/*@
requires pallasSpec::sep(a > 0, b < 0);
@*/
void festiveFunction (int a, int b) {
    int c = a + b;
}