// clang-format off
// Test that the separating conjunction of the spec-lib is translated correctly
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s

// COM: Declaration of separating conjunction is present and labeled
// CHECK-DAG: declare !pallas.specLib [[mdNode:![0-9]+]] i1 @pallas.sepConj(i1, i1)
// CHECK-DAG: [[mdNode]] = !{!"pallas.sepConj"}

// COM: Specification-function is called 
// CHECK-DAG: {{%[0-9]+}} = call i1 @pallas.sepConj(i1 %0, i1 %1)

/*@
 requires _sep(a, b);
 @*/
func bar (a: Bool, b: Bool) -> Bool {
    return b || !a
}