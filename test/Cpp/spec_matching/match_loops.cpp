// Test that loop invariants are correctly matched to (nested) loops.
// RUN: %Spec2IR %s -lib %CppSpecLib -m -lang Cpp | %FileCheck %s

int foo (int x, int y, int z) {
    int cnt = 0;

    /*@
    loop_invariant x >= 0;
    loop_invariant cnt >= 0;
    @*/
    while (x < cnt) {
        cnt ++;

        /*@
        loop_invariant y <= z;
        @*/
        while (y < z) {
            y ++;    
        } 

        /*@
        loop_invariant 0 <= j && j <= 42;
        @*/
        for (int j = 42; j > 0; --j) {
            cnt += 1;
        }

    }

    return x + y + z; 
}

// CHECK: Matched invariant at 7-10 to loop at 11
// CHECK: Matched invariant at 14-16 to loop at 17
// CHECK: Matched invariant at 21-23 to loop at 24