// clang-format off
// Check that the correct variables are detected for use in loop invariants.
// This only needs to pass to check that the compilation succeeded. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func legendaryLoop(a: Int, b: Int) {
    var c = a;
    var i = 0;
    /*@
    loop_invariant c == a + i * b;
    loop_invariant i >= 0 && i <= b;
    loop_invariant a != b;
    @*/
    while (i < b) {
        c += b;
        i += 1;

        /*@
        loop_invariant 0 <= j && j <= 42;
        loop_invariant c >= 0 && a != b;
        loop_invariant i >= 0;
        @*/
        var j = 0
        while j <= 42 {
            c %= 2
            j += 1
        }
    }
}


