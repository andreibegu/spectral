// Tests that check that the correct variables are available within a 
// Expected to pass, only valid variables are referenced.
// RUN: %Spec2IR %s -lib %CSpecLib -m 

int foo (int x) {
    int cnt = 0;
    /*@
    loop_invariant cnt >= 0 && x >= 0;
    @*/
    while (cnt < 5) {
        ++cnt;
    } 
    return x; 
}

int bar (int a) {
    int cnt = 0;
    /*@
    loop_invariant a >= 0;
    loop_invariant cnt >= 0;
    @*/
    while (cnt < 10) {
       /*@
       loop_invariant a >= 0;
       loop_invariant cnt >= 0;
       loop_invariant i <= 5;
       @*/ 
        for (int i = 5; i > 0; i--) {
            cnt ++;
        }
    } 
    return cnt;
} 