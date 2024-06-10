// Example to test the translation of the \old()-construct in the C specification-library.
/*@
 declare DEF_BV(int);
 declare DEF_BV(float);
 @*/

/*@
 requires imply(a >= b, a >= 0);
 ensures exists(0 <= BV(int)("x") && BV(int)("x") <= 10,  BV(int)("x") != 42);
 @*/
int bar (int a, int b) {
    int c = 5;
    c += 2;
    a += 5;
    return a + b + c ;
}


/*@
 requires forall(0 <= BV(int)("i"), BV(int)("i") >= 0);
 ensures forallP(0 <= BV(int)("j"), BV(int)("j") >= 0);
 @*/
int foo (int a) {
    return a + 10 ;
}

