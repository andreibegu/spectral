
/*
    Some normal comment if front of a function
*/
int foo () {
    return 7;
}

/*@
 requires a >= b ;
 ensures \result >= a && \result >= b;
 @*/
int bar (int a, int b) {
    return a + b ;
}

