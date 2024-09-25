// clang-format off
// Test that a loop-invariant-block is not matched to a while-loop
// that is in front of it.
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift


func bar (a: Int, b: Int) -> Int {
    var i = 0
    while i < a {
        i += b;
    } 
    /*@
    loop_invariant i >= 0;
    @*/
    return i
}