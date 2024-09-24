// clang-format off
// Test that a loop-invariant-block is correctly matched to a while-loop
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


func bar (a: Int, b: Int) -> Int {
    var i = 0
    /*@
    loop_invariant i >= 0;
    @*/
    while i < a {
        i += b;
    } 
    return i
}



// COM: Check that the invariant has been matched correctly
// CHECK: Matched invariant at 8-10 to loop at 11