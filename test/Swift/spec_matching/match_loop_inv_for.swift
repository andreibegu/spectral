// clang-format off
// Test that a loop-invariant-block is correctly matched to a simple for-loop
// that loops over a range of integers.
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | %FileCheck %s


func funkyFunc (a: Int, b: Int) -> Int {
    var res = 0
    /*@
    
        loop_invariant true;
    @*/
    
    for i in 0...a {
        res += a * b * i;
    }

    return res
}


// COM: Check that the invariant has been matched correctly
// CHECK: Matched invariant at 9-12 to loop at 14