/*@
 declare DEF_RESULT(int);
 @*/


/*@
 requires Perm(&a, fracOf(1, 2));
 ensures RESULT(int)() > a + 42 && Perm(&b, fracOf(1, 4));
 @*/
int bar (int a, float b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

