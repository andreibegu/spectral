/*@
declare DEF_RESULT(int);
@*/

/*@
requires a >= 0 && b >= 0;
ensures _result(int) == a * b; 
@*/
int mult(int a, int b) {
    int res = 0;
    /*@ 
    loop_invariant 0 <= i && i <= a;
    loop_invariant res == i * b;  
    @*/
    for (int i=0; i<a; ++i) {
        res += b;
    }
    return res;
} 