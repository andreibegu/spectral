// Regression test. 
// Check that multiple variables with the same name do not cause issues as 
// long as they are in different scopes.
// RUN: %Spec2IR %s -lib %CSpecLib -m 

int foo (int x) {

    int tmp = 0;
    /*@
    loop_invariant 0 <= i && i <= 42;
    loop_invariant tmp >= 0;
    @*/
    for (int i = 0; i < 42; i++) {
        tmp *= 2 + 1;
    }

    /*@
    loop_invariant 0 <= i && i <= 42;
    loop_invariant tmp >= 0;
    @*/
    for (int i = 0; i < 42; i++) {
        tmp *= 2 + 1;
    }
    return tmp;
}
