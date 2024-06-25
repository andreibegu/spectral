/*@
 declare DEF_RESULT(int);
 declare DEF_OLD(int);
 @*/


/*@
 requires a >= 0 && b >= 0;
 ensures RESULT(int)() * a <= OLD(int)(b);
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


/*@
requires x >= 0 && y >= 0 && z >= 0;
ensures RESULT(int)() > 0;
@*/
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
