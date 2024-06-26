// Test that a function-contact is correctly detected and matched.
// TODO: Currently only checks that the tool does not crash.
// RUN: %Spec2IR %s -lib %CSpecLib

void foo(int i) {
    i++;
}

/*@
 requires a >= b ;
 ensures 0 >= a && a >= b;
 @*/
int bar (int a, int b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

// Normal comments should be ignored
// ensures \result >= a && \result >= b;
float foobar(int x, float y) {
    return x + y;
}

