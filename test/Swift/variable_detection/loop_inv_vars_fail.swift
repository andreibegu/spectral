// clang-format off
// Check that variables that occur after a loop are not available in its invariant.
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift

func legendaryLoop(a: Int, b: Int) -> Bool{
    var c = a;
    var i = 0;
    /*@
    loop_invariant r >= 0;
    @*/
    while (i < b) {
        c += b;
        i += 1;
    }
    let r = a + i * b;
    return c == r;
}

