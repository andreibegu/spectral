// clang-format off
// Test that a loop-invariant-block is not matched to a loop if it incorrectly
// placed within the loop.
// RUN: %not %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift


func leisurelyLoops (a: Int) -> Int {
    var res = 0

    for i in 0...a {
        /*@
        loop_invariant true;
        @*/
        res += a * i;
    }

    return res
}