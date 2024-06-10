// Example to test the translation of the \old()-construct in the C specification-library.
/*@
 declare DEF_OLD(int) ;
 declare DEF_OLD(float);
 declare DEF_RESULT(int);
 @*/

/*@
 requires imply(a >= b, a >= 0);
 ensures OLD(int)(a) > a + 42 && b == OLD(float)(b);
 ensures RESULT(int)() > 0;
 @*/
int bar (int a, float b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}

