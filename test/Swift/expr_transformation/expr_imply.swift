// clang-format off
// Test that the implication of the spec-lib is translated correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration of implication is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.imply(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.imply"}

// COM: Imply-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.imply(i1 %0, i1 %1)

/*@
 requires a ==> b;
 @*/
func bar (a: Bool, b: Bool) -> Bool {
    return b || !a
}