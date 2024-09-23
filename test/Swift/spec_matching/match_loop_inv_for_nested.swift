// clang-format off
// Test that a loop-invariant-block is correctly matched to a simple for-loops
// that are nested. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s


func funkyFunc (a: Int, b: Int) -> Int {
    var res = 0
    
    /*@
    loop_invariant true;
    @*/
    for i in 0..<a {
        res += a * b * i;

        /*@
        loop_invariant true;
        loop_invariant true;
        @*/
        for j in 0 ..< b {
            res *= b + j;
        }
    }


    var i2 = 5;
    /*@
        loop_invariant true;  
    @*/
    while (i2 > 0) {
        if (i2 % 2 == 0) {
            i2 -= 3;
        } else {
            i2 -= 1;
        }
    } 
    return res
}


// COM: Check that the invariant has been matched correctly
// CHECK: Matched invariant at 10-12 to loop at 13
// CHECK: Matched invariant at 16-19 to loop at 20
// CHECK: Matched invariant at 27-29 to loop at 30