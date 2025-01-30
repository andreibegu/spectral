// Test that expressions with short circuting and are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

// COM: Declaration is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.scAnd(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.scAnd"}

// COM: Function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.scAnd(

/*@
requires pallasSpec::_and(a > 0, b > 0);
@*/
void fun (int a, int b) {
    int c = a + b;
}