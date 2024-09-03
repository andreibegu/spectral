// clang-format off
// Test that statement-blocks are correctly matched to instructions. 
// RUN: %Spec2IR %s -lib %SwiftSpecLib -m -lang Swift | FileCheck-15 %s

func funkyFunc (a: Int, b: Int) -> Int {
    /*@
    assume a > 0;
    assume b > 0;
    @*/
    var res = 0
    /*@ assert res >= 0; @*/
    res += a + 5
    /*@ assert res > 0; @*/
    res = res * b
    return res
}

// COM: Check that the statement-blocks are matched
// CHECK: Matched statement-block at 6-9 to instruction at 10::15
// CHECK: Matched statement-block at 11-11 to instruction at 12::14
// CHECK: Matched statement-block at 13-13 to instruction at 14::15
