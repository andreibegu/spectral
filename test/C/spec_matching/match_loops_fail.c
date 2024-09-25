// Test that loop invariants are not matched to loops if they are located 
// after the loop or in the loop
// RUN: rm -rf %t && %split-file %s %t && cd %t

//--- after_loop.c
// RUN: %not %Spec2IR after_loop.c -lib %CSpecLib -m 
int foo (int x) {
    int cnt = 0;

    while (x < cnt) {
        cnt ++;
    }
    /*@
    loop_invariant x >= 0;
    loop_invariant cnt >= 0;
    @*/

    return x; 
}

//--- in_loop.c
// RUN: %not %Spec2IR in_loop.c -lib %CSpecLib -m 
int foo (int x) {
    int cnt = 0;

    while (x < cnt) {
    /*@
    loop_invariant x >= 0;
    @*/
        cnt ++;
    }

    return x; 
}