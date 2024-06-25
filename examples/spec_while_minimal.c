/*@
 requires a >= 0 && b >= 0;
 @*/
int bar (int a, int b) {
    int cnt = 0;

    /*@
    loop_invariant cnt >= 0;
    loop_invariant a >= 0 && b >= 0; 
    @*/
    while (b > a) {
        b = b - a;
        cnt ++;
    }
    return cnt;
}
