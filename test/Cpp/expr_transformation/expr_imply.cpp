// Test that expressions that include implications are transformed correctly
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | FileCheck-15 %s

// COM: Declaration of implication is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.imply(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.imply"}

// COM: Imply-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.imply(

/*@
requires pallasSpec::imply(a > 0, b > 0) && a > 0;
@*/
void fun (int a, int b) {
    int c = a + b;
}