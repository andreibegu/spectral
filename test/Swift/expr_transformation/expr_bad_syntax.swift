// clang-format off
// Test expressions with incorrect syntax cause an error.
// RUN: not-15 %Spec2IR %s -lib %SwiftSpecLib -lang Swift


/*@
 requires a >= ;
 @*/
func bar (a: Bool, b: Bool) -> Bool {
    return b || !a
}